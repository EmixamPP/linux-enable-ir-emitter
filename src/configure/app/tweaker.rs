use super::helper::*;
use crate::configure::ui::DeviceSettingsCtx;
use crate::configure::ui::tweaker::{TweakerCtx, View, ui};
use crate::video::stream::{Image, Stream, grey_devices};
use crate::video::uvc::{Device, XuControl};

use std::cell::OnceCell;
use std::path::PathBuf;

use anyhow::{Context, Result, anyhow, bail};
use crossterm::event::{Event, EventStream, KeyCode, KeyEventKind};
use futures::{FutureExt, StreamExt};
use ratatui::{DefaultTerminal, widgets::ListState};
use tokio::{
    select,
    sync::mpsc,
    sync::mpsc::{Receiver, Sender},
    task,
};

const KEY_YES: KeyCode = KeyCode::Char('y');
const KEY_NO: KeyCode = KeyCode::Char('n');
const KEY_EXIT: KeyCode = KeyCode::Esc;
const KEY_NAVIGATE: KeyCode = KeyCode::Tab;
const KEY_NAVIGATE_UP: KeyCode = KeyCode::Up;
const KEY_NAVIGATE_DOWN: KeyCode = KeyCode::Down;
const KEY_CONTINUE: KeyCode = KeyCode::Enter;
const KEY_DELETE: KeyCode = KeyCode::Backspace;
// TODO add key to delete from the config

#[derive(Debug)]
pub struct Config {
    /// Path to the video device.
    pub device: PathBuf,
    /// Height of the video stream.
    pub height: Option<u32>,
    /// Width of the video stream.
    pub width: Option<u32>,
    /// Number of emitters to configure.
    pub emitters: usize,
    /// Frames per second of the video stream.
    pub fps: Option<u32>,
}

impl Default for Config {
    fn default() -> Self {
        Self {
            device: PathBuf::from("/dev/video"),
            height: None,
            width: None,
            emitters: 1,
            fps: None,
        }
    }
}

/// Application states.
#[derive(Debug)]
pub enum State {
    /// Configuration settings menu
    Menu,
    /// Configuration is waiting
    Waiting,
    /// Editing a XuControl at index
    Editing(usize),
    /// Confirm to save or not the configuration before exiting
    SaveBeforeExit,
    /// Exit
    Exit,
}

/// "Enabler" application.
pub struct App {
    /// Receiver for captured images from the video stream task.
    image_rx: Receiver<Image>,
    /// Sender used by the video stream task for captured images.
    image_tx: Option<Sender<Image>>,
    /// Current application state.
    /// Should be accessed through [`Self::state()`]
    _state: State,
    /// List of all the video devices that support grey scale pixel format.
    grey_devices: Vec<PathBuf>,
    /// List of all the device controls that can be modified.
    controls: Vec<XuControl>,
    /// State of the list of device controls.
    controls_list_state: ListState,
    /// Settings for the whole application.
    config: Config,
    /// State of the list of device settings.
    device_settings_list_state: ListState,
    /// The last captured image from the video stream.
    image: Option<Image>,
    /// Device open to apply controls. Should be accessed through [`Self::get_device()`].
    _device: OnceCell<Device>,
    /// The optional error that occurred during last control configuration.
    error: Option<String>,
}

impl App {
    /// Creates a new "Tweaker" application.
    pub fn new() -> Self {
        let mut device_settings_list_state = ListState::default();
        device_settings_list_state.select_first(); // already select the first item

        let (image_tx, image_rx) = mpsc::channel(1); // do not increase, we drop images to avoid "video lag"
        let mut controls_list_state = ListState::default();
        controls_list_state.select_first();
        Self {
            _state: State::Menu,
            config: Config::default(),
            device_settings_list_state,
            image: None,
            image_tx: Some(image_tx),
            image_rx,
            error: None,
            controls: Vec::new(),
            controls_list_state,
            grey_devices: Vec::new(),
            _device: OnceCell::new(),
        }
    }

    /// Initializes the application by detecting grey scale video devices.
    /// Returns an error if no grey scale video device is found.
    fn init(&mut self) -> Result<()> {
        let grey_devices = grey_devices();
        log::debug!("Grey scale video devices: {:?}.", grey_devices);
        if grey_devices.is_empty() {
            bail!(
                "no V4L grey scale video device found (the system probably does not support your infrared camera)"
            );
        } else {
            self.config.device = grey_devices[0].clone();
            self.grey_devices = grey_devices;
        }
        Ok(())
    }

    /// Returns the current application state.
    pub fn state(&self) -> &State {
        &self._state
    }

    /// Sets the current application state and updates the previous state.
    fn set_state(&mut self, state: State) {
        self._state = state;
    }

    /// Gets the video device specified in the config.
    ///
    /// If this is the first time, open it and load the list of controls.
    ///
    /// # Note
    /// This operation is blocking.
    fn get_device(&mut self) -> Result<&Device> {
        match self._device.get() {
            Some(device) => Ok(device),
            None => {
                let device = Device::open(&self.config.device)
                    .with_context(|| format!("failed to open device {:?}", self.config.device))?;
                self.controls = device.controls();
                self._device.set(device).unwrap(); // safe because we checked that it was None
                Ok(self._device.get().unwrap())
            }
        }
    }

    /// Spawns a new asynchronous task that continuously captures video frames from the stream device
    /// and sends them through [`Self::image_tx`].
    ///
    /// Returns an error if the video stream is already started.
    fn spawn_video_stream_task(&mut self) -> Result<()> {
        let device = self.config.device.clone();
        let width = self.config.width;
        let height = self.config.height;
        let fps = self.config.fps;

        // we take the sender, so if the task exit, the receiver will be notified
        let image_tx = self
            .image_tx
            .take()
            .context("video stream task already started")?;

        task::spawn_blocking(move || {
            let mut stream = match Stream::open(device, width, height, fps) {
                Ok(s) => s,
                Err(err) => {
                    log::error!("Error opening video stream: {err:?}");
                    return;
                }
            };
            log::debug!("{stream}");

            loop {
                match stream.capture() {
                    Ok(img) => {
                        if image_tx.blocking_send(img).is_err() {
                            log::debug!("Exiting video stream task.");
                            return;
                        }
                    }
                    Err(err) => {
                        log::error!("Error while capturing image: {err:?}");
                        return;
                    }
                }
            }
        });

        Ok(())
    }

    /// Runs the application.
    pub async fn run(&mut self, terminal: &mut DefaultTerminal) -> Result<&'static str> {
        self.init()?;
        let mut term_event_stream = EventStream::new();

        // Draw the UI then wait an event in loop
        loop {
            terminal.draw(|f| ui(f, self))?;
            select! {
                Some(event) = term_event_stream.next().fuse() => self.handle_term_event(event?).await?,
                image = self.image_rx.recv() => self.handle_image(image)?,
            }

            if matches!(self.state(), State::Exit) {
                return Ok("Tweaker exited successfully.");
            }
        }
    }

    /// Handles terminal events.
    async fn handle_term_event(&mut self, event: Event) -> Result<()> {
        match event {
            Event::Key(key_event) if key_event.kind == KeyEventKind::Press => {
                self.handle_key_press(key_event.code).await?;
            }
            _ => {}
        };
        Ok(())
    }

    /// Handles a new captured image.
    ///
    /// Returns an error if the image is None.
    fn handle_image(&mut self, image: Option<Image>) -> Result<()> {
        if let Some(image) = image {
            self.image = Some(image);
            Ok(())
        } else {
            Err(anyhow!("the video stream has been closed unexpectedly"))
        }
    }

    /// Handles a key event based on the current application state.
    async fn handle_key_press(&mut self, key: KeyCode) -> Result<()> {
        if self.error.is_some() {
            self.error = None;
            return Ok(());
        }
        match self.state() {
            State::Menu => match key {
                KEY_EXIT => self.set_state(State::Exit),
                KEY_NAVIGATE | KEY_NAVIGATE_DOWN => self.next_setting(),
                KEY_NAVIGATE_UP => self.prev_setting(),
                KEY_DELETE => self.edit_setting(None),
                KeyCode::Char(c) => self.edit_setting(Some(c)),
                KEY_CONTINUE => self.start_or_back()?,
                _ => {}
            },
            State::Waiting => {
                if key == KEY_EXIT {
                    self.set_state(State::SaveBeforeExit)
                } else if key == KEY_CONTINUE {
                    if let Some(selected) = self.controls_list_state.selected() {
                        if selected < self.controls.len() {
                            self.set_state(State::Editing(selected));
                        }
                    }
                } else if key == KEY_NAVIGATE || key == KEY_NAVIGATE_DOWN {
                    self.controls_list_state.select_next();
                } else if key == KEY_NAVIGATE_UP {
                    self.controls_list_state.select_previous();
                }
            }
            State::Editing(_index) => {
                // TODO modify the control depending on the key
                if key == KEY_EXIT {
                    self.set_state(State::Waiting);
                } else {
                    self.error = Some("Editing controls is not yet fully implemented".to_string());
                }
            }
            State::SaveBeforeExit => self.save_before_exit(key).await?,
            _ => {}
        }
        Ok(())
    }

    /// Set the state to exit, if:
    /// the key is [`KEY_YES`], save the current configuration,
    /// or the key is [`KEY_NO`], discard the current configuration
    ///
    /// For any other key, set the state back to [`State::Waiting`].
    async fn save_before_exit(&mut self, k: KeyCode) -> Result<()> {
        // TODO
        match k {
            KEY_NO => (),
            KEY_YES => (),
            _ => {
                self.set_state(State::Waiting);
                return Ok(());
            }
        }
        self.set_state(State::Exit);
        Ok(())
    }

    /// If the configuration is valid, tries to spawn the video stream task
    /// and change the state to [`State::Waiting`]. If it fails, returns the error.
    ///
    /// Returns directly an error if the video stream is already started.
    fn start_or_back(&mut self) -> Result<()> {
        // check that the path exists
        if !self.is_device_valid() {
            return Ok(());
        }
        self.spawn_video_stream_task()?;
        self.get_device()?.controls();
        self.set_state(State::Waiting);
        Ok(())
    }

    /// Check that the current device is grey scale.
    fn is_device_valid(&self) -> bool {
        self.grey_devices.contains(&self.config.device)
    }

    /// Moves the selection to the next setting in the settings lists.
    fn next_setting(&mut self) {
        self.device_settings_list_state.select_next();
    }

    /// Moves the selection to the previous setting in the settings lists.
    fn prev_setting(&mut self) {
        self.device_settings_list_state.select_previous();
    }

    /// Edits the currently selected setting in the settings lists by adding or removing a character.
    ///
    /// If `ch` is `Some(c)`, adds the character `c` (depending on the setting type).
    /// If `ch` is `None`, removes the last character.
    fn edit_setting(&mut self, ch: Option<char>) {
        match self.device_settings_list_state.selected() {
            Some(0) => add_or_remove_char_from_path(&mut self.config.device, ch, 10),
            Some(1) => add_or_remove_char_from_numeric(&mut self.config.emitters, ch),
            Some(2) => add_or_remove_char_from_opt_numeric(&mut self.config.height, ch),
            Some(3) => add_or_remove_char_from_opt_numeric(&mut self.config.width, ch),
            Some(4) => add_or_remove_char_from_opt_numeric(&mut self.config.fps, ch),
            _ => {}
        }
    }
}

impl TweakerCtx for App {
    fn view(&self) -> View<'_> {
        match self.state() {
            State::Menu => View::Menu,
            State::Waiting => View::Main,
            State::SaveBeforeExit => View::Main,
            State::Exit => View::Menu,
            State::Editing(index) => View::Edition(&self.controls[*index]),
        }
    }
    fn controls_list_state(&mut self) -> &mut ListState {
        &mut self.controls_list_state
    }
    fn controls(&self) -> &[XuControl] {
        &self.controls
    }
    fn image(&self) -> Option<&Image> {
        self.image.as_ref()
    }
    fn show_save_exit_prompt(&self) -> bool {
        matches!(self.state(), State::SaveBeforeExit)
    }
    fn error_message(&self) -> Option<&String> {
        self.error.as_ref()
    }
}

impl DeviceSettingsCtx for App {
    fn device_settings_list_state(&mut self) -> &mut ListState {
        &mut self.device_settings_list_state
    }
    fn device_valid(&self) -> (String, bool) {
        (
            self.config.device.to_string_lossy().to_string(),
            self.is_device_valid(),
        )
    }
    fn height(&self) -> Option<u32> {
        self.config.height
    }
    fn width(&self) -> Option<u32> {
        self.config.width
    }
    fn emitters(&self) -> usize {
        self.config.emitters
    }
    fn fps(&self) -> Option<u32> {
        self.config.fps
    }
}
