use super::helper::*;
use crate::configure::ui::ir_enabler::{IrEnablerCtx, View, ui};
use crate::configure::ui::{DeviceSettingsCtx, SearchSettingsCtx};
use crate::video::ir::analyzer::{
    self, IsIrWorking as AnalyzerResponse, Message as AnalyzerRequest, StreamAnalyzer,
};
use crate::video::ir::enabler::{
    Config as IREnablerConfig, Enabler, IsIrWorking as IREnablerResponse,
    Message as IREnablerRequest,
};
use crate::video::stream::{Image, Stream, grey_devices};
use crate::video::uvc::XuControl;

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
const KEY_CONTINUE: KeyCode = KeyCode::Enter;
const KEY_DELETE: KeyCode = KeyCode::Backspace;

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
    /// Limit of negative answers before incrementing the next byte of a control.
    pub limit: Option<u16>,
    /// If true, the user will always manually confirm if the IR emitter is working.
    pub manual: bool,
    /// If manual is false, number of images to analyze before determining if the IR emitter is working.
    pub analyzer_img_count: u64,
    /// Coefficient for the reference intensity variation.
    pub ref_intensity_var_coef: u64,
    /// Increment step for the current modified control byte.
    pub inc_step: u8,
}

impl Default for Config {
    fn default() -> Self {
        Self {
            device: PathBuf::from("/dev/video"),
            height: None,
            width: None,
            emitters: 1,
            fps: None,
            limit: Some(5),
            manual: false,
            ref_intensity_var_coef: 50,
            inc_step: 1,
            analyzer_img_count: 30,
        }
    }
}

/// Application states.
#[derive(Debug, PartialEq, Eq, Copy, Clone)]
pub enum State {
    /// Configuration settings menu
    Menu,
    /// Confirm to start the configuration
    ConfirmStart,
    /// Configuration is running
    Running,
    /// Confirm that the IR emitter is working via algorithm
    ConfirmWorking,
    /// Confirm that the IR emitter is working
    ConfirmWorkingManual,
    /// Confirm to abort the configuration
    ConfirmAbort,
    /// The configuration was successful
    Success,
    /// The configuration failed
    Failure,
    /// Exit the application and abort the configuration
    Abort,
}

/// "Enabler" application.
pub struct App {
    /// Receiver for captured images from the video stream task.
    image_rx: Receiver<Image>,
    /// Sender used by the video stream task for captured images.
    image_tx: Option<Sender<Image>>,
    /// Receiver for state and requests from the configurator task.
    search_msg_rx: Receiver<IREnablerRequest>,
    /// Sender used by the configurator task to share its state and requests.
    search_msg_tx: Option<Sender<IREnablerRequest>>,
    /// Receiver used by the configurator task for responses of requests.
    response_rx: Option<Receiver<IREnablerResponse>>,
    /// Sender used to send responses to the configurator task.
    response_tx: Sender<IREnablerResponse>,
    /// Sender used to send requests to the analyzer task.
    analyzer_request_tx: Sender<AnalyzerRequest>,
    /// Receiver used to receive requests in the analyzer task.
    analyzer_request_rx: Option<Receiver<AnalyzerRequest>>,
    /// Sender used to send responses in the analyzer task.
    analyzer_response_tx: Option<Sender<AnalyzerResponse>>,
    /// Receiver used to receive responses from the analyzer task.
    analyzer_response_rx: Receiver<AnalyzerResponse>,
    /// Previous application state.
    /// Should be accessed through [`Self::prev_state()`].
    _prev_state: State,
    /// Current application state.
    /// Should be accessed through [`Self::state()`], and modified with [`Self::set_state()`].
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
    /// State of the list of search settings.
    search_settings_list_state: ListState,
    /// The last captured image from the video stream.
    image: Option<Image>,
}

impl App {
    /// Creates a new "Enabler" application.
    pub fn new() -> Self {
        let mut device_settings_list_state = ListState::default();
        device_settings_list_state.select_first(); // already select the first item

        let (image_tx, image_rx) = mpsc::channel(1); // do not increase, we drop images to avoid "video lag"
        let (search_msg_tx, search_msg_rx) = mpsc::channel(3);
        let (response_tx, response_rx) = mpsc::channel(3);
        let (analyzer_request_tx, analyzer_request_rx) = mpsc::channel(30); // high number give some processing time but not unbounded to avoid leaks
        let (analyzer_response_tx, analyzer_response_rx) = mpsc::channel(3);

        Self {
            _prev_state: State::Menu,
            _state: State::Menu,
            config: Config::default(),
            device_settings_list_state,
            search_settings_list_state: ListState::default(),
            image: None,
            image_tx: Some(image_tx),
            image_rx,
            search_msg_tx: Some(search_msg_tx),
            search_msg_rx,
            response_tx,
            response_rx: Some(response_rx),
            analyzer_request_rx: Some(analyzer_request_rx),
            analyzer_request_tx,
            analyzer_response_tx: Some(analyzer_response_tx),
            analyzer_response_rx,
            controls: Vec::new(),
            controls_list_state: ListState::default(),
            grey_devices: Vec::new(),
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
    pub fn state(&self) -> State {
        self._state
    }

    /// Returns the previous application state.
    fn prev_state(&self) -> State {
        self._prev_state
    }

    /// Sets the current application state and updates the previous state.
    fn set_state(&mut self, state: State) {
        self._prev_state = self._state;
        self._state = state;
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

    /// Spawns a new asynchronous task that runs the configurator.
    fn spawn_search_task(&mut self) -> Result<()> {
        let config = IREnablerConfig {
            device: self.config.device.clone(),
            neg_answer_limit: self.config.limit,
            emitters: self.config.emitters,
            inc_step: self.config.inc_step,
        };
        let search_msg_tx = self
            .search_msg_tx
            .take()
            .context("configurator task already started")?;
        let response_rx = self
            .response_rx
            .take()
            .context("configurator task already started")?;

        task::spawn(async move {
            let mut configurator = match Enabler::new(config, search_msg_tx, response_rx) {
                Ok(cfg) => cfg,
                Err(err) => {
                    log::error!("{err:?}");
                    return;
                }
            };

            if let Err(err) = configurator.configure().await {
                log::error!("Error while configuring: {err:?}",);
            } else {
                log::debug!("Exiting configurator task exited.");
            }
        });

        Ok(())
    }

    fn spawn_stream_analyzer_task(&mut self) -> Result<()> {
        let ref_intensity_var_coef = self.config.ref_intensity_var_coef;
        let analyzer_img_count = self.config.analyzer_img_count;
        let request_rx = self
            .analyzer_request_rx
            .take()
            .context("analyzer task already started")?;
        let response_tx = self
            .analyzer_response_tx
            .take()
            .context("analyzer task already started")?;

        task::spawn(async move {
            let analyzer = StreamAnalyzer::new(ref_intensity_var_coef);
            if let Err(err) =
                analyzer::analyze(analyzer, response_tx, request_rx, analyzer_img_count).await
            {
                log::error!("Error in analyzer task: {err:?}");
            } else {
                log::debug!("Exiting analyzer task exited.");
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
                search_msg = self.search_msg_rx.recv() => self.handle_search_msg(search_msg).await?,
                analyzer_msg = self.analyzer_response_rx.recv() => self.handle_analyzer_msg(analyzer_msg).await?,
            }

            if self.state() == State::Failure {
                bail!("failed to enable the infrared emitter");
            } else if self.state() == State::Success {
                return Ok("The infrared emitter has been successfully enabled!");
            } else if self.state() == State::Abort {
                bail!("configuration aborted by the user");
            }
        }
    }

    /// Handles terminal events.
    async fn handle_term_event(&mut self, event: Event) -> Result<()> {
        match event {
            Event::Key(key_event) if key_event.kind == KeyEventKind::Press => {
                self.handle_key_press(key_event.code.into()).await?;
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
            // ignore if too many images are buffered
            let _ = self
                .analyzer_request_tx
                .try_send(AnalyzerRequest::Image(image.clone()));
            self.image = Some(image);
            Ok(())
        } else {
            Err(anyhow!("the video stream has been closed unexpectedly"))
        }
    }

    /// Handles a new message from the configurator task.
    ///
    /// Returns an error if the message is None.
    async fn handle_search_msg(&mut self, msg: Option<IREnablerRequest>) -> Result<()> {
        if let Some(msg) = msg {
            match msg {
                IREnablerRequest::Controls(controls) => self.controls = controls,
                IREnablerRequest::UpdateControl(control) => {
                    let (idx, updated_control) = self
                        .controls
                        .iter_mut()
                        .enumerate()
                        .find(|(_, c)| c.unit() == control.unit() && c.selector() == control.selector())
                        .context("Received update for unknown control. This is a bug, please report this issue.")?;
                    updated_control.cur_mut().copy_from_slice(control.cur());
                    self.controls_list_state.select(Some(idx));
                }
                IREnablerRequest::IsIrWorking => match self.config.manual {
                    false => {
                        self.analyzer_request_tx
                            .send(AnalyzerRequest::IsIrWorking)
                            .await
                            .context(
                                "failed to send the is IR working message to the analyzer task",
                            )?;
                        self.set_state(State::ConfirmWorking);
                    }
                    true => self.set_state(State::ConfirmWorkingManual),
                },
                IREnablerRequest::AlreadyWorking => bail!("the IR emitter is already working"),
                IREnablerRequest::Success => self.set_state(State::Success),
                IREnablerRequest::Failure => self.set_state(State::Failure),
            }
            Ok(())
        } else {
            Err(anyhow!(
                "the configurator task has been closed unexpectedly"
            ))
        }
    }

    /// Handles a new message from the analyzer task.
    ///
    /// Returns an error if the message is None.
    async fn handle_analyzer_msg(&mut self, msg: Option<AnalyzerResponse>) -> Result<()> {
        if let Some(msg) = msg {
            match msg {
                AnalyzerResponse::Yes | AnalyzerResponse::Maybe => {
                    self.set_state(State::ConfirmWorkingManual);
                }
                AnalyzerResponse::No => {
                    self.response_tx
                        .send(IREnablerResponse::No)
                        .await
                        .context("failed to send the confirmation message")?;
                    self.set_state(State::Running);
                }
            }
            Ok(())
        } else {
            Err(anyhow!("the analyzer task has been closed unexpectedly"))
        }
    }

    /// Handles a key event based on the current application state.
    async fn handle_key_press(&mut self, key: KeyCode) -> Result<()> {
        match self.state() {
            State::Menu => match key {
                KEY_EXIT => self.set_state(State::Failure),
                KEY_NAVIGATE => self.next_setting(),
                KEY_DELETE => self.edit_setting(None),
                KeyCode::Char(c) => self.edit_setting(Some(c)),
                KEY_CONTINUE => self.set_state(State::ConfirmStart),
                _ => {}
            },
            State::ConfirmStart => self.start_or_back(key)?,
            State::Running => {
                if key == KEY_EXIT {
                    self.set_state(State::ConfirmAbort)
                }
            }
            State::ConfirmAbort => self.abort_or_continue(key).await?,
            State::ConfirmWorkingManual => match key {
                KEY_EXIT => self.set_state(State::ConfirmAbort),
                key => self.confirm_working(key).await?,
            },
            State::ConfirmWorking => {
                if key == KEY_EXIT {
                    self.set_state(State::ConfirmAbort)
                }
            }
            _ => {}
        }
        Ok(())
    }

    /// If the key is [`KEY_YES`], sends [`crate::video::ir::enabler::IsIrWorking::Yes`] to the configurator task.
    /// If the key is [`KEY_NO`], sends [`crate::video::ir::enabler::IsIrWorking::No`] to the configurator task.
    /// In both of the two case, also changes the state to [`State::Running`].
    ///
    /// Otherwise, does nothing.
    async fn confirm_working(&mut self, k: KeyCode) -> Result<()> {
        let mut response = IREnablerResponse::No;
        if k == KEY_YES {
            response = IREnablerResponse::Yes;
        } else if k != KEY_NO {
            return Ok(());
        }
        self.set_state(State::Running);

        self.response_tx
            .send(response)
            .await
            .context("failed to send the confirmation message")
    }

    /// If the key is [`KEY_YES`], change the state to [`State::Abort`]
    /// and sends [`IREnablerResponse::Abort`] to the configurator task.
    ///
    /// If the key is [`KEY_NO`], change the state back to [`State::Running`].
    async fn abort_or_continue(&mut self, k: KeyCode) -> Result<()> {
        match k {
            KEY_NO | KEY_EXIT => self.set_state(self.prev_state()),
            KEY_YES => {
                self.set_state(State::Abort);
                self.response_tx
                    .send(IREnablerResponse::Abort)
                    .await
                    .context("failed to send the abort message")?;
            }
            _ => {}
        }
        Ok(())
    }

    /// If the configuration is valid and if the key is [`KEY_YES`], tries to spawn the video stream task
    /// and change the state to [`State::Running`]. If it fails, returns the error.
    ///
    /// If the key is [`KEY_NO`], change the state back to the previous state.
    ///
    /// Returns directly an error if the video stream is already started.
    fn start_or_back(&mut self, k: KeyCode) -> Result<()> {
        // check that the path exists
        if !self.is_device_valid() {
            self.set_state(State::Menu);
            return Ok(());
        }

        match k {
            KEY_NO | KEY_EXIT => self.set_state(self.prev_state()),
            KEY_YES => {
                self.spawn_video_stream_task()?;
                self.spawn_search_task()?;
                if !self.config.manual {
                    self.spawn_stream_analyzer_task()?;
                }
                self.set_state(State::Running);
            }
            _ => {}
        }
        Ok(())
    }

    /// Check that the current device is grey scale.
    fn is_device_valid(&self) -> bool {
        self.grey_devices.contains(&self.config.device)
    }

    /// Moves the selection to the next setting in the settings lists.
    fn next_setting(&mut self) {
        if let Some(i) = self.device_settings_list_state.selected() {
            if i < 4 {
                self.device_settings_list_state.select_next();
            } else {
                self.device_settings_list_state.select(None);
                self.search_settings_list_state.select_first();
            }
        } else if let Some(i) = self.search_settings_list_state.selected() {
            if i < 3 {
                self.search_settings_list_state.select_next();
            } else {
                self.search_settings_list_state.select(None);
                self.device_settings_list_state.select_first();
            }
        } else {
            self.device_settings_list_state.select_first();
        }
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

        match self.search_settings_list_state.selected() {
            Some(0) => add_or_remove_char_from_bool(&mut self.config.manual, ch),
            Some(1) => add_or_remove_char_from_numeric(&mut self.config.analyzer_img_count, ch),
            Some(2) => add_or_remove_char_from_numeric(&mut self.config.ref_intensity_var_coef, ch),
            Some(3) => add_or_remove_char_from_opt_numeric(&mut self.config.limit, ch),
            Some(4) => add_or_remove_char_from_numeric(&mut self.config.inc_step, ch),
            _ => {}
        }
    }
}

impl IrEnablerCtx for App {
    fn view(&self) -> View {
        match self.state() {
            State::Menu => View::Menu,
            State::ConfirmStart => View::Menu,
            State::Running => View::Main,
            State::ConfirmAbort => View::Main,
            State::ConfirmWorking => View::Main,
            State::ConfirmWorkingManual => View::Main,
            State::Success => View::Main,
            State::Failure => View::Main,
            State::Abort => View::Main,
        }
    }
    fn show_working_question(&self) -> bool {
        self.state() == State::ConfirmWorkingManual
    }
    fn show_main_abort_prompt(&self) -> bool {
        self.state() == State::ConfirmAbort
    }
    fn show_menu_start_prompt(&self) -> bool {
        self.state() == State::ConfirmStart
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

impl SearchSettingsCtx for App {
    fn search_settings_list_state(&mut self) -> &mut ListState {
        &mut self.search_settings_list_state
    }
    fn limit(&self) -> Option<u16> {
        self.config.limit
    }
    fn manual(&self) -> bool {
        self.config.manual
    }
    fn analyzer_img_count(&self) -> u64 {
        self.config.analyzer_img_count
    }
    fn ref_intensity_var_coef(&self) -> u64 {
        self.config.ref_intensity_var_coef
    }
    fn inc_step(&self) -> u8 {
        self.config.inc_step
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::video::uvc::XuControl;
    use crossterm::event::{KeyCode, KeyEvent, KeyEventKind, KeyModifiers};
    use std::path::PathBuf;

    fn make_app() -> App {
        App::new()
    }

    fn make_key_event(keycode: KeyCode) -> KeyEvent {
        KeyEvent {
            code: keycode,
            modifiers: KeyModifiers::NONE,
            kind: KeyEventKind::Press,
            state: crossterm::event::KeyEventState::NONE,
        }
    }

    fn make_term_key_event(keycode: KeyCode) -> Event {
        Event::Key(make_key_event(keycode))
    }

    #[test]
    fn test_initialized_configs() {
        let app = make_app();
        // no image yet
        assert!(app.image.is_none());
        // first setting is selected
        assert_eq!(app.device_settings_list_state.selected(), Some(0));
        assert!(app.search_settings_list_state.selected().is_none());
        // first state is Menu
        assert_eq!(app.state(), State::Menu);
        assert_eq!(app.config.device, PathBuf::from("/dev/video"));
        assert_eq!(app.config.height, None);
        assert_eq!(app.config.width, None);
        assert_eq!(app.config.emitters, 1);
        assert_eq!(app.config.fps, None);
        assert_eq!(app.config.limit, Some(5));
        assert_eq!(app.config.manual, false);
        assert_eq!(app.config.ref_intensity_var_coef, 50);
        assert_eq!(app.config.inc_step, 1);
        assert_eq!(app.config.analyzer_img_count, 30);
    }

    #[test]
    fn test_set_state() {
        let mut app = make_app();
        app.set_state(State::Running);
        assert_eq!(app.prev_state(), State::Menu);
        assert_eq!(app.state(), State::Running);
    }

    #[test]
    fn test_handle_image_sets_image() {
        let mut app = make_app();
        let img = Image::new_rgba8(1, 1);
        assert!(app.handle_image(Some(img.clone())).is_ok());
        assert_eq!(app.image, Some(img));
    }

    #[test]
    fn test_handle_image_none() {
        let mut app = make_app();
        assert!(app.handle_image(None).is_err());
    }

    #[test]
    fn test_next_setting_device_to_search_and_back() {
        let mut app = make_app();
        // Move through device settings (0..4)
        for i in 0..4 {
            assert_eq!(app.device_settings_list_state.selected(), Some(i));
            app.next_setting();
        }
        // After 4, should move to search settings first
        app.next_setting();
        assert!(app.device_settings_list_state.selected().is_none());
        assert_eq!(app.search_settings_list_state.selected(), Some(0));
        // Move through search settings (0..2)
        for i in 0..3 {
            app.next_setting();
            assert_eq!(app.search_settings_list_state.selected(), Some(i + 1));
        }
        // After 2, should wrap to device settings first
        app.next_setting();
        assert!(app.search_settings_list_state.selected().is_none());
        assert_eq!(app.device_settings_list_state.selected(), Some(0));
    }

    #[test]
    fn test_edit_setting_device_path() {
        let mut app = make_app();
        app.device_settings_list_state.select(Some(0));
        assert!(
            !app.config.device.to_string_lossy().contains('1'),
            "test pre-condition"
        );
        app.edit_setting(Some('1'));
        assert!(app.config.device.to_string_lossy().contains('1'));
        app.edit_setting(None);
        assert!(!app.config.device.to_string_lossy().contains('1'));
    }

    #[test]
    fn test_edit_setting_device_height() {
        let mut app = make_app();
        app.device_settings_list_state.select(Some(2));
        app.edit_setting(Some('1'));
        assert_eq!(app.config.height, Some(1));
        app.edit_setting(None);
        assert_eq!(app.config.height, None);
    }

    #[test]
    fn test_edit_setting_device_width() {
        let mut app = make_app();
        app.device_settings_list_state.select(Some(3));
        app.edit_setting(Some('2'));
        assert_eq!(app.config.width, Some(2));
        app.edit_setting(None);
        assert_eq!(app.config.width, None);
    }

    #[test]
    fn test_edit_setting_device_emitters() {
        let mut app = make_app();
        app.device_settings_list_state.select(Some(1));
        let initial = app.config.emitters;
        app.edit_setting(Some('2'));
        assert_eq!(app.config.emitters.to_string(), format!("{initial}2"));
        app.edit_setting(None);
        assert_eq!(app.config.emitters, initial);
    }

    #[test]
    fn test_edit_setting_device_fps() {
        let mut app = make_app();
        app.device_settings_list_state.select(Some(4));
        app.edit_setting(Some('4'));
        assert_eq!(app.config.fps, Some(4));
        app.edit_setting(None);
        assert_eq!(app.config.fps, None);
    }

    #[test]
    fn test_edit_setting_search_limit() {
        let mut app = make_app();
        app.search_settings_list_state.select(Some(3));
        let initial = app.config.limit.unwrap(); // pre-condition is not none
        app.edit_setting(Some('5'));
        assert_eq!(app.config.limit.unwrap().to_string(), format!("{initial}5"));
        app.edit_setting(None);
        assert_eq!(app.config.limit, Some(initial));
    }

    #[test]
    fn test_edit_setting_search_manual() {
        let mut app = make_app();
        app.search_settings_list_state.select(Some(0));
        app.edit_setting(Some('t'));
        assert!(app.config.manual);
        app.edit_setting(None);
        assert!(!app.config.manual);
    }

    #[test]
    fn test_edit_setting_search_analyzer_img_count() {
        let mut app = make_app();
        app.search_settings_list_state.select(Some(1));
        let initial = app.config.analyzer_img_count;
        app.edit_setting(Some('4'));
        assert_eq!(
            app.config.analyzer_img_count.to_string(),
            format!("{initial}4")
        );
        app.edit_setting(None);
        assert_eq!(app.config.analyzer_img_count, initial);
    }

    #[test]
    fn test_edit_setting_search_ref_intensity_var_coef() {
        let mut app = make_app();
        app.search_settings_list_state.select(Some(2));
        let initial = app.config.ref_intensity_var_coef;
        app.edit_setting(Some('7'));
        assert_eq!(
            app.config.ref_intensity_var_coef.to_string(),
            format!("{initial}7")
        );
        app.edit_setting(None);
        assert_eq!(app.config.ref_intensity_var_coef, initial);
    }

    #[test]
    fn test_edit_setting_search_inc_step() {
        let mut app = make_app();
        app.search_settings_list_state.select(Some(4));
        let initial = app.config.inc_step;
        app.edit_setting(Some('3'));
        assert_eq!(app.config.inc_step.to_string(), format!("{initial}3"));
        app.edit_setting(None);
        assert_eq!(app.config.inc_step, initial);
    }

    #[tokio::test]
    async fn test_handle_term_event_key_menu_exit() {
        let mut app = make_app();
        app.set_state(State::Menu);
        let key_event = make_term_key_event(KEY_EXIT);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.state(), State::Failure);
    }

    #[tokio::test]
    async fn test_handle_term_event_key_release_menu_exit() {
        let mut app = make_app();
        app.set_state(State::Menu);
        let key_event = Event::Key(KeyEvent {
            code: KEY_EXIT.into(),
            modifiers: KeyModifiers::NONE,
            kind: KeyEventKind::Release,
            state: crossterm::event::KeyEventState::NONE,
        });
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.state(), State::Menu);
    }

    #[tokio::test]
    async fn test_handle_term_event_key_menu_navigate() {
        let mut app = make_app();
        app.set_state(State::Menu);
        app.device_settings_list_state.select(Some(0));
        let key_event = make_term_key_event(KEY_NAVIGATE);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.device_settings_list_state.selected(), Some(1));
        assert_eq!(app.state(), State::Menu);
    }

    #[tokio::test]
    async fn test_handle_term_event_key_menu_backspace() {
        let mut app = make_app();
        app.set_state(State::Menu);
        app.config.device = PathBuf::from("/dev/video2");
        let key_event = make_term_key_event(KEY_DELETE);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.config.device, PathBuf::from("/dev/video"));
        assert_eq!(app.state(), State::Menu);
    }

    #[tokio::test]
    async fn test_handle_term_event_key_menu_char() {
        let mut app = make_app();
        app.set_state(State::Menu);
        app.config.device = PathBuf::from("/dev/video");
        let key_event = make_term_key_event(KeyCode::Char('2').into());
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.config.device, PathBuf::from("/dev/video2"));
        assert_eq!(app.state(), State::Menu);
    }

    #[tokio::test]
    async fn test_handle_key_event_menu_continue() {
        let mut app = make_app();
        app.set_state(State::Menu);
        let key_event = make_term_key_event(KEY_CONTINUE);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.state(), State::ConfirmStart);
    }

    #[tokio::test]
    async fn test_handle_is_device_valid_not_grey() {
        let mut app = make_app();
        app.config.device = "/dev/video0".into();
        assert!(!app.is_device_valid());
    }

    #[tokio::test]
    async fn test_handle_is_device_valid_grey() {
        let mut app = make_app();
        app.config.device = "/dev/video0".into();
        app.grey_devices.push("/dev/video0".into());
        assert!(app.is_device_valid());
    }

    #[tokio::test]
    async fn test_handle_key_event_start_or_back_device_not_grey() {
        let mut app = make_app();
        app.set_state(State::ConfirmStart);
        app.config.device = "/dev/video0".into();
        let key_event = make_term_key_event(KEY_YES);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.state(), State::Menu);
    }

    #[tokio::test]
    async fn test_handle_key_event_start_or_back_yes() {
        let mut app = make_app();
        app.set_state(State::ConfirmStart);
        app.config.device = "/dev/video0".into();
        app.grey_devices.push("/dev/video0".into());
        let key_event = make_term_key_event(KEY_YES);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.state(), State::Running);
    }

    #[tokio::test]
    async fn test_handle_key_event_start_or_back_no() {
        let mut app = make_app();
        app.set_state(State::ConfirmStart);
        let key_event = make_term_key_event(KEY_NO);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.state(), State::Menu);
    }

    #[tokio::test]
    async fn test_handle_key_event_start_or_back_exit() {
        let mut app = make_app();
        app.set_state(State::ConfirmStart);
        let key_event = make_term_key_event(KEY_EXIT);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.state(), State::Menu);
    }

    #[tokio::test]
    async fn test_handle_key_event_running_exit() {
        let mut app = make_app();
        app.set_state(State::Running);
        let key_event = make_term_key_event(KEY_EXIT);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.state(), State::ConfirmAbort);
    }

    #[tokio::test]
    async fn test_handle_key_event_abort_or_continue_exit() {
        let mut app = make_app();
        app.set_state(State::Running);
        app.set_state(State::ConfirmAbort);
        let key_event = make_term_key_event(KEY_EXIT);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.state(), State::Running);
    }

    #[tokio::test]
    async fn test_handle_key_event_abort_or_continue_no() {
        let mut app = make_app();
        app.set_state(State::Running);
        app.set_state(State::ConfirmAbort);
        let key_event = make_term_key_event(KEY_NO);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.state(), State::Running);
    }

    #[tokio::test]
    async fn test_handle_key_event_abort_or_continue_yes() {
        let mut app = make_app();
        app.set_state(State::Running);
        app.set_state(State::ConfirmAbort);
        let key_event = make_term_key_event(KEY_YES);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.state(), State::Abort);
        let response = app.response_rx.as_mut().unwrap().recv().await;
        assert_eq!(response, Some(IREnablerResponse::Abort));
    }

    #[tokio::test]
    async fn test_handle_key_event_confirm_working_manual_yes() {
        let mut app = make_app();
        app.config.manual = true;
        app.set_state(State::ConfirmWorkingManual);
        let key_event = make_term_key_event(KEY_YES);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        let response = app.response_rx.as_mut().unwrap().recv().await;
        assert_eq!(response, Some(IREnablerResponse::Yes));
        assert_eq!(app.state(), State::Running);
    }

    #[tokio::test]
    async fn test_handle_key_event_confirm_working_manual_no() {
        let mut app = make_app();
        app.config.manual = true;
        app.set_state(State::ConfirmWorkingManual);
        let key_event = make_term_key_event(KEY_NO);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        let response = app.response_rx.as_mut().unwrap().recv().await;
        assert_eq!(response, Some(IREnablerResponse::No));
        assert_eq!(app.state(), State::Running);
    }

    #[tokio::test]
    async fn test_handle_key_event_confirm_working_manual_other() {
        let mut app = make_app();
        app.config.manual = true;
        app.set_state(State::ConfirmWorkingManual);
        let key_event = make_term_key_event(KeyCode::Char('a').into());
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        let response = app.response_rx.as_mut().unwrap().try_recv();
        assert_eq!(response, Err(mpsc::error::TryRecvError::Empty));
        assert_eq!(app.state(), State::ConfirmWorkingManual);
    }

    #[tokio::test]
    async fn test_handle_key_event_confirm_working_manual_abort() {
        let mut app = make_app();
        app.config.manual = true;
        app.set_state(State::ConfirmWorkingManual);
        let key_event = make_term_key_event(KEY_EXIT);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        let response = app.response_rx.as_mut().unwrap().try_recv();
        assert_eq!(response, Err(mpsc::error::TryRecvError::Empty));
        assert_eq!(app.state(), State::ConfirmAbort);
    }

    #[tokio::test]
    async fn test_handle_key_event_confirm_working() {
        let mut app = make_app();
        app.config.manual = false;
        app.set_state(State::ConfirmWorking);
        let key_event = make_term_key_event(KEY_YES);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        let response = app.response_rx.as_mut().unwrap().try_recv();
        assert_eq!(response, Err(mpsc::error::TryRecvError::Empty));
        assert_eq!(app.state(), State::ConfirmWorking);
    }

    #[tokio::test]
    async fn test_handle_key_event_confirm_working_abort() {
        let mut app = make_app();
        app.config.manual = false;
        app.set_state(State::ConfirmWorking);
        let key_event = make_term_key_event(KEY_EXIT);
        let res = app.handle_term_event(key_event).await;
        assert!(res.is_ok(), "{:?}", res.err());
        let response = app.response_rx.as_mut().unwrap().try_recv();
        assert_eq!(response, Err(mpsc::error::TryRecvError::Empty));
        assert_eq!(app.state(), State::ConfirmAbort);
    }

    /// TODO: need to mockup Stream
    // #[tokio::test]
    // async fn test_init() {}
    // #[tokio::test]
    // async fn test_run() {}
    // #[tokio::test]
    // async fn test_spawn_video_stream_task_invalid_device_returns_err() {
    //     let mut app = make_app();
    //     // Set an invalid device so Stream::open fails
    //     app.config.device = PathBuf::from("/dev/video10");
    //     assert!(app.spawn_video_stream_task().is_ok());
    //     assert!(app.image_rx.recv().await.is_none());
    // }
    // #[tokio::test]
    // async fn test_spawn_video_stream_task_close() {
    //     let mut app = make_app();
    //     assert!(app.spawn_video_stream_task().is_ok());
    //     app.image_rx.close();
    //     assert!(app.image_rx.recv().await.is_none());
    // }
    // #[tokio::test]
    // async fn test_spawn_video_stream_task_success_image() {
    //     let mut app = make_app();
    //     assert!(app.spawn_video_stream_task().is_ok());
    //     for _ in 0..3 {
    //         assert!(app.image_rx.recv().await.is_some());
    //     }
    // }
    // #[tokio::test]
    // async fn test_double_spawn_video_stream_task() {
    //     let mut app = make_app();
    //     assert!(app.spawn_video_stream_task().is_ok());
    //     assert!(app.spawn_video_stream_task().is_err());
    // }

    /// TODO: need to mockup Device
    // #[tokio::test]
    // async fn test_spawn_search_task_close() {
    //     let mut app = make_app();
    //     assert!(app.spawn_search_task().is_ok());
    //     app.search_msg_rx.close();
    //     assert!(app.search_msg_rx.recv().await.is_none());
    // }
    // #[tokio::test]
    // async fn test_spawn_search_task_double_spawn() {
    //     let mut app = make_app();
    //     assert!(app.spawn_search_task().is_ok());
    //     // Second call should fail because sender/receiver are taken
    //     assert!(app.spawn_search_task().is_err());
    // }

    #[tokio::test]
    async fn test_handle_search_msg_controls_and_update() {
        let mut app = make_app();
        // Controls message
        let controls = vec![
            XuControl::new(
                1,
                1,
                vec![1],
                Some(vec![1]),
                Some(vec![1]),
                Some(vec![1]),
                Some(vec![1]),
                true,
            )
            .unwrap(),
            XuControl::new(
                2,
                2,
                vec![2],
                Some(vec![2]),
                Some(vec![2]),
                Some(vec![2]),
                Some(vec![2]),
                true,
            )
            .unwrap(),
            XuControl::new(
                3,
                3,
                vec![3],
                Some(vec![3]),
                Some(vec![3]),
                Some(vec![3]),
                Some(vec![3]),
                true,
            )
            .unwrap(),
        ];
        let msg = Some(IREnablerRequest::Controls(controls.clone()));
        let res = app.handle_search_msg(msg).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.controls, controls);

        // UpdateControl message
        let updated = XuControl::new(1, 1, vec![11], None, None, None, None, true).unwrap();
        let msg = Some(IREnablerRequest::UpdateControl(updated.clone()));
        let res = app.handle_search_msg(msg).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.controls[0].cur(), updated.cur());
    }

    #[tokio::test]
    async fn test_handle_search_msg_isirworking() {
        let mut app = make_app();
        // IsIrWorking message
        let msg = Some(IREnablerRequest::IsIrWorking);
        // Should send IsIrWorking::No to response_tx
        let res = app.handle_search_msg(msg).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.state(), State::ConfirmWorking);
    }

    #[tokio::test]
    async fn test_handle_search_msg_already_working() {
        let mut app = make_app();
        // AlreadyWorking message
        let msg = Some(IREnablerRequest::AlreadyWorking);
        let res = app.handle_search_msg(msg).await;
        assert!(res.is_err());
    }

    #[tokio::test]
    async fn test_handle_search_msg_success_failure() {
        let mut app = make_app();
        // Success message
        let msg = Some(IREnablerRequest::Success);
        let res = app.handle_search_msg(msg).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.state(), State::Success);
        // Failure message
        let msg = Some(IREnablerRequest::Failure);
        let res = app.handle_search_msg(msg).await;
        assert!(res.is_ok(), "{:?}", res.err());
        assert_eq!(app.state(), State::Failure);
    }

    #[tokio::test]
    async fn test_handle_search_msg_none() {
        let mut app = make_app();
        let msg = None;
        let res = app.handle_search_msg(msg).await;
        assert!(res.is_err());
    }

    #[tokio::test]
    async fn test_handle_analyzer_msg_yes() {
        let mut app = make_app();
        app.set_state(State::ConfirmWorking);
        let msg = Some(AnalyzerResponse::Yes);
        let res = app.handle_analyzer_msg(msg).await;
        assert!(res.is_ok(), "{:?}", res.err());
        let response = app.response_rx.as_mut().unwrap().try_recv();
        assert_eq!(response, Err(mpsc::error::TryRecvError::Empty));
        assert_eq!(app.state(), State::ConfirmWorkingManual);
    }

    #[tokio::test]
    async fn test_handle_analyzer_msg_maybe() {
        let mut app = make_app();
        app.set_state(State::ConfirmWorking);
        let msg = Some(AnalyzerResponse::Maybe);
        let res = app.handle_analyzer_msg(msg).await;
        assert!(res.is_ok(), "{:?}", res.err());
        let response = app.response_rx.as_mut().unwrap().try_recv();
        assert_eq!(response, Err(mpsc::error::TryRecvError::Empty));
        assert_eq!(app.state(), State::ConfirmWorkingManual);
    }

    #[tokio::test]
    async fn test_handle_analyzer_msg_no() {
        let mut app = make_app();
        app.set_state(State::ConfirmWorking);
        let msg = Some(AnalyzerResponse::No);
        let res = app.handle_analyzer_msg(msg).await;
        assert!(res.is_ok(), "{:?}", res.err());
        let response = app.response_rx.as_mut().unwrap().recv().await;
        assert_eq!(response, Some(IREnablerResponse::No));
        assert_eq!(app.state(), State::Running);
    }

    #[tokio::test]
    async fn test_handle_analyzer_msg_none() {
        let mut app = make_app();
        app.set_state(State::ConfirmWorking);
        let msg = None;
        let res = app.handle_analyzer_msg(msg).await;
        assert!(res.is_err());
        assert_eq!(app.state(), State::ConfirmWorking);
    }
}
