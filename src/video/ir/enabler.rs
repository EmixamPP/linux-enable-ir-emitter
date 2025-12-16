use crate::configuration::Configuration;
use crate::video::uvc::{Device, XuControl};

use std::path::PathBuf;

use anyhow::{Context, Result, bail};
use futures::FutureExt;
use tokio::sync::mpsc::{Receiver, Sender};

/// Requests sent from the [`Enabler`].
#[derive(Debug)]
pub enum Message {
    /// Asks if the IR emitter is working. Expects a response of type [`IsIrWorking`].
    IsIrWorking,
    /// Contains all the controls available on the device. Does not expect a response.
    Controls(Vec<XuControl>),
    /// Only contains the updated value for [`XuControl::cur`],
    /// and the [`XuControl::unit`] [`XuControl::selector`] for identification. Does not expect a response.
    UpdateControl(XuControl),
    /// The IR emitter is already working. Does not expect a response.
    AlreadyWorking,
    /// The configuration ended and was successful. Does not expect a response.
    Success,
    /// The configuration ended and failed. Does not expect a response.
    Failure,
}

/// Responses to a [`Message::IsIrWorking`] from the [`Enabler`].
#[derive(Debug, PartialEq, Eq)]
pub enum IsIrWorking {
    Yes,
    No,
    /// Must be sent in case the configuration process should be aborted.
    Abort,
}

/// Device configuration for the [`Enabler`].
#[derive(Debug)]
pub struct Config {
    /// Path to the UVC device.
    pub device: PathBuf,
    /// Number of IR emitters on the device.
    pub emitters: usize,
    /// Maximum number of negative answers before incrementing the next byte of a control.
    /// If None, defaults to 256 which is the maximum possible.
    pub neg_answer_limit: Option<u16>,
    /// Increment step for each byte of a control when trying to find the right value.
    pub inc_step: u8,
}

/// Tool that tries to find the right configuration for enabling the IR emitter(s).
/// It is designed to support asynchronous communication.
pub struct Enabler {
    device: Device,
    neg_answer_limit: u16,
    emitters: usize,
    configuration: Configuration,
    inc_step: u8,
    request_tx: Sender<Message>,
    response_rx: Receiver<IsIrWorking>,
    abort: bool,
}

impl Enabler {
    /// Creates a new enabler.
    pub fn new(
        config: Config,
        request_tx: Sender<Message>,
        response_rx: Receiver<IsIrWorking>,
    ) -> Result<Self> {
        Ok(Self {
            configuration: Configuration::new(&config.device),
            device: Device::open(config.device)?,
            neg_answer_limit: config.neg_answer_limit.unwrap_or(256),
            emitters: config.emitters,
            request_tx,
            response_rx,
            abort: false,
            inc_step: config.inc_step,
        })
    }

    /// Gets all the writable controls and not blacklisted controls from the device.
    fn controls(&self) -> Vec<XuControl> {
        self.device
            .controls()
            .into_iter()
            .filter(|c| {
                if c.writable() && !self.configuration.is_blacklisted(c) {
                    true
                } else {
                    log::info!("Ignoring read-only control: {}", c);
                    false
                }
            })
            .collect()
    }

    /// Tries to find a configuration for the controls that enables the IR emitter(s).
    pub async fn configure(&mut self) -> Result<()> {
        // check if not already working
        self.ask_isworking().await?;
        if self.recv_isworking().await? {
            self.send(Message::AlreadyWorking).await?;
            bail!("the IR emitter is already working");
        }

        let mut controls = self.controls();
        self.send(Message::Controls(controls.clone())).await?;

        for ctrl in &mut controls {
            match self.try_control(ctrl).await {
                Ok(true) => {
                    log::info!("The control enables the IR emitter.");
                    self.configuration.add_to_savelist(ctrl);
                    self.emitters -= 1;
                    if self.emitters == 0 {
                        self.send(Message::Success).await?;
                        return Ok(());
                    }
                }
                failed => {
                    log::info!("Resetting control to initial value.");
                    ctrl.reset();
                    if let Err(err) = self.device.apply_control(ctrl) {
                        self.configuration.add_to_blacklist(ctrl);
                        log::warn!("The control has been added to the blacklist.");
                        self.send(Message::Failure).await?;
                        return Err(err).context("impossible to reset the control");
                    }
                    self.send_ctrl(ctrl).await?;

                    if self.abort {
                        log::info!("Configuration aborted by user.");
                        self.send_now_or_never(Message::Failure).await;
                        return Ok(());
                    }

                    if let Err(err) = failed {
                        // If the try_control failure was not related to the device, return the error
                        if err.downcast_ref::<std::io::Error>().is_none() {
                            self.send(Message::Failure).await?;
                            return Err(err);
                        }
                    }
                }
            };
        }

        self.send(Message::Failure).await?;
        bail!("failed to find the controls that enables the ir emitter(s)");
    }

    async fn send(&mut self, msg: Message) -> Result<()> {
        self.request_tx
            .send(msg)
            .await
            .context("channel closed while sending message")
    }

    async fn send_now_or_never(&mut self, msg: Message) {
        let _ = self.request_tx.send(msg).now_or_never();
    }

    async fn recv_isworking(&mut self) -> Result<bool> {
        match self
            .response_rx
            .recv()
            .await
            .context("channel closed while waiting for response")?
        {
            IsIrWorking::Yes => Ok(true),
            IsIrWorking::No => Ok(false),
            IsIrWorking::Abort => {
                self.abort = true;
                bail!("configuration aborted");
            }
        }
    }

    async fn send_ctrl(&mut self, ctrl: &XuControl) -> Result<()> {
        self.send(Message::UpdateControl(ctrl.essential_clone()))
            .await
    }

    async fn ask_isworking(&mut self) -> Result<()> {
        self.send(Message::IsIrWorking).await
    }

    /// Tries to find if `ctrl` can enable the IR emitter when modified.
    ///
    /// True if the control can enable the IR emitter otherwise false.
    /// In case an error is returned, if the kind is std::io::Error, then
    /// it means that the control may have temporarily broke the device.
    ///
    /// # Errors
    /// Returns either an std::io::Error if the device failed to apply a control,
    /// or an anyhow::Error if a communication channel was closed.
    async fn try_control(&mut self, ctrl: &mut XuControl) -> Result<bool> {
        let mut neg_answer = 0;
        let mut cur_byte = 0; // the current control byte that is being incremented

        while self.increment(ctrl, &mut cur_byte, &mut neg_answer) {
            self.send_ctrl(ctrl).await?;
            self.device.apply_control(ctrl)?;
            self.ask_isworking().await?;

            log::debug!("IR enabler waiting for response...");
            match self.recv_isworking().await? {
                true => return Ok(true),
                false => neg_answer += 1,
            }
        }
        Ok(false)
    }

    /// Increments the control value by 1 in an big endian representation.
    ///
    /// `cur_byte` is the current control byte that is being incremented.
    /// It is used to keep track of which byte to increment when incrementing in loop the `ctrl`.
    /// Can be seen as the "state" of the increment operation.
    ///
    /// If the number of `neg_answer` is above `self.neg_answer_limit`,
    /// or if the current byte is at its maximum value (if defined) or 255,
    /// reset the current byte and increment the next byte.
    ///
    /// In case of increment success, returns true.
    /// Returns false, when all the bytes has been incremented.
    pub fn increment(
        &self,
        ctrl: &mut XuControl,
        cur_byte: &mut usize,
        neg_answer: &mut u16,
    ) -> bool {
        // check if all bytes have been incremented
        if *cur_byte >= ctrl.cur().len() {
            return false;
        }

        // check if neg answer is above the limit
        // or the the maximum value for the byte has been reached
        if *neg_answer > self.neg_answer_limit
            || ctrl.cur_mut()[*cur_byte] == {
                ctrl.max()
                    .and_then(|v| v.get(*cur_byte).copied())
                    .unwrap_or(255)
            }
        {
            // reset
            ctrl.cur_mut()[*cur_byte] = ctrl.init()[*cur_byte];
            *neg_answer = 0;
            // increment next byte
            *cur_byte += 1;
            return self.increment(ctrl, cur_byte, neg_answer);
        }

        // increment current byte
        ctrl.cur_mut()[*cur_byte] += self.inc_step;
        true
    }
}

impl Drop for Enabler {
    fn drop(&mut self) {
        if let Err(err) = self.configuration.save() {
            log::error!("Error while saving configuration: {err:?}");
        }
    }
}
