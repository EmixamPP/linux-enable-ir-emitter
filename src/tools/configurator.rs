use crate::config::{BlackControl, Config};
use crate::ir::{IrCamera, IrControl};
use crate::uvc::Control;

use std::collections::VecDeque;

use anyhow::{Result, anyhow};
use log::{debug, error, info};

/// Context for the configuration tool.
pub struct ConfigureCtx {
    camera: IrCamera,
    emitters_to_configure: usize,
    neg_answer_limit: i16,
    manual: bool,
    config: Config,
}

impl ConfigureCtx {
    pub fn new(
        camera: IrCamera,
        emitters_to_configure: usize,
        neg_answer_limit: i16,
        manual: bool,
    ) -> Self {
        debug!("{camera}");

        let mut config = Config::load(camera.dev_path());
        // Reset the white listed controls
        config.white_list.clear();

        Self {
            config,
            camera,
            emitters_to_configure,
            neg_answer_limit,
            manual,
        }
    }

    pub fn controls(&self) -> Result<VecDeque<IrControl>> {
        debug!("Black list: {:?}", self.config.black_list);

        Ok(VecDeque::from(
            self.camera
                .controls()?
                .into_iter()
                .filter(|c| {
                    let is_blacklisted = !self
                        .config
                        .black_list
                        .iter()
                        .any(|b| b.unit == c.unit() && b.selector == c.selector());
                    if is_blacklisted {
                        debug!(
                            "Skipping control at unit: {} and selector: {}, it has been black listed.",
                            c.unit(),
                            c.selector()
                        );
                    }
                    is_blacklisted
                })
                .collect::<VecDeque<_>>()
        ))
    }
}

/// Tries to enable the infrared camera.
pub fn configure(mut ctx: ConfigureCtx) -> Result<()> {
    debug!(
        "Executing configure command for {} emitters and a negative answer limit of {}.",
        ctx.emitters_to_configure, ctx.neg_answer_limit
    );
    info!("Configuring camera {:?}", ctx.camera.dev_path());
    info!("Stand in front of and close to the camera and make sure the room is well lit.");
    info!("Ensure to not use the camera during the execution.");

    match configurator(&mut ctx) {
        Ok(..) => info!("Configuration successfully completed."),
        Err(e) => {
            error!("Configuration failed: {e}");
            info!("Do not hesitate to visit the GitHub!");
            info!("https://github.com/EmixamPP/linux-enable-ir-emitter/blob/master/docs/README.md");
        }
    }

    // Save the configuration no matter error or not
    ctx.config.save(ctx.camera.dev_path())?;

    Ok(())
}

/// Configurator function that iterates through the possible controls and tries to enable the emitter(s).
fn configurator(ctx: &mut ConfigureCtx) -> Result<()> {
    let mut ctrls = ctx.controls()?;
    while let Some(mut ctrl) = ctrls.pop_front() {
        match check_ctrl(ctx, &mut ctrl) {
            Ok(..) => {}
            Err(e) => {
                ctx.config
                    .black_list
                    .push(BlackControl::from(&ctrl as &dyn Control));
                return Err(e.context(
                    "the control seems to break the camera, please reboot and try again",
                ));
            }
        }

        if ctx.config.white_list.len() == ctx.emitters_to_configure {
            info!("All the emitters are configured.");
            return Ok(());
        }
    }
    // no more controls to test
    Err(anyhow!("failed to enable the emitter, try the -m option"))
}

/// Check if a control enables the infrared emitter.
fn check_ctrl(ctx: &mut ConfigureCtx, ctrl: &mut IrControl) -> Result<()> {
    info!(
        "Trying the control at unit: {} and selector: {}",
        ctrl.unit(),
        ctrl.selector()
    );
    let mut ctrl_neg_answer_remaining = ctx.neg_answer_limit;

    while ctrl_neg_answer_remaining != 0 && ctrl.increment().is_ok() {
        info!("With the value: {:?}", ctrl.value_ref());

        let may_enable = ctx.camera.apply_and_check_ir(ctrl)?;
        if may_enable || ctx.manual {
            // Ask confirmation by the user
            // TODO run separate thread to wait input

            // ctx.camera.play()?;
            // ctx.config
            //     .white_list
            //     .push(WhiteControl::from(&ctrl as &dyn Control));
            // return Ok(ctx);
        }

        ctrl_neg_answer_remaining -= 1;
        if ctrl_neg_answer_remaining == 1 {
            // last chance, try to set the maximum
            let _ = ctrl.try_set_max();
        }
    }

    ctrl.reset();
    debug!("Resetting control to initial value: {:?}", ctrl.value_ref());
    ctx.camera.apply(ctrl)?;

    Ok(())
}
