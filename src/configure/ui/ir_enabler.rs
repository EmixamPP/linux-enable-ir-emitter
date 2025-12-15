use super::{
    keys::{KEY_CONTINUE, KEY_EXIT, KEY_NAVIGATE, KEY_NO, KEY_YES, keys_to_line},
    popup_area, render_main_window, render_video_preview,
};
use crate::video::stream::Image;

use ratatui::{
    Frame,
    layout::{Constraint, Flex, Layout, Rect},
    style::{Color, Style, Stylize},
    text::{Line, Text, ToLine},
    widgets::{Block, BorderType, Clear, HighlightSpacing, List, ListItem, ListState, Paragraph},
};

#[derive(Debug, PartialEq, Eq, Clone, Copy, Default)]
pub enum View {
    #[default]
    Menu,
    Main,
}

pub trait IrEnablerCtx {
    fn view(&self) -> View;
    fn show_working_question(&self) -> bool;
    fn show_main_abort_prompt(&self) -> bool;
    fn show_menu_start_prompt(&self) -> bool;

    fn device_settings_list_state(&mut self) -> &mut ListState;
    fn device_valid(&self) -> (String, bool);
    fn height(&self) -> Option<u32>;
    fn width(&self) -> Option<u32>;
    fn emitters(&self) -> usize;
    fn fps(&self) -> Option<u32>;

    fn search_settings_list_state(&mut self) -> &mut ListState;
    fn limit(&self) -> Option<u16>;
    fn manual(&self) -> bool;
    fn analyzer_img_count(&self) -> u64;
    fn ref_intensity_var_coef(&self) -> u64;
    fn inc_step(&self) -> u8;

    fn controls_list_state(&mut self) -> &mut ListState;
    fn controls(&self) -> &[crate::video::uvc::XuControl];
    fn image(&self) -> Option<&Image>;
}

/// Renders a confirmation popup to start the enabling process.
fn render_confirm_start_popup(frame: &mut Frame, area: Rect) {
    let block = Block::bordered().border_type(BorderType::Double);
    let area = popup_area(area, 12, 70);
    frame.render_widget(Clear, area);
    frame.render_widget(block, area);

    let [info_area, command_area] = Layout::vertical([Constraint::Fill(0), Constraint::Length(1)])
        .margin(1)
        .areas(area);

    let info_text = Text::from(
        "Be patient, never abort the process without the dedicated key;\nyou could break the camera firmware.\nIf the process is stuck, e.g. displayed values are not updating\nsomething is wrong, you can abort the process.\n\nFor better results, stand in front of the camera\nand ensure the room has good, constant lighting.\n\nDo you want to start?",
    );
    let paragraph = Paragraph::new(info_text).centered();
    frame.render_widget(paragraph, info_area);

    let command_line = keys_to_line(&[KEY_YES, KEY_NO]);
    let command_paragraph = Paragraph::new(command_line).centered();
    frame.render_widget(command_paragraph, command_area);
}

/// Renders a confirmation popup to abort the process.
fn render_confirm_abort_popup(frame: &mut Frame, area: Rect) {
    let block = Block::bordered().border_type(BorderType::Double);
    let area = popup_area(area, 4, 50);
    frame.render_widget(Clear, area);
    frame.render_widget(block, area);

    let [info_area, command_area] =
        Layout::vertical([Constraint::Length(1), Constraint::Length(1)])
            .margin(1)
            .areas(area);

    let paragraph = Paragraph::new("Are you sure you want to abort the process?").centered();
    frame.render_widget(paragraph, info_area);

    let command_line = keys_to_line(&[KEY_YES, KEY_NO]);
    let command_paragraph = Paragraph::new(command_line).centered();
    frame.render_widget(command_paragraph, command_area);
}

/// Renders the main menu with device and search settings.
fn render_menu<A>(frame: &mut Frame, area: Rect, app: &mut A)
where
    A: IrEnablerCtx,
{
    let [top_info_area, device_area, search_area, bot_info_area] = Layout::vertical([
        Constraint::Length(1),
        Constraint::Length(7),
        Constraint::Length(7),
        Constraint::Length(1),
    ])
    .flex(Flex::Center)
    .areas(area);

    let top_info_line =
        Line::from("The tool will iterate through the UVC camera controls and modify them.")
            .blue()
            .centered();
    frame.render_widget(top_info_line, top_info_area);

    let bot_info_line = Line::from(vec![
        "For more explanation, visit ".into(),
        "https://github.com/EmixamPP/linux-enable-ir-emitter".underlined(),
    ])
    .blue()
    .centered();
    frame.render_widget(bot_info_line, bot_info_area);

    let device_block =
        Block::bordered().title(Line::from(" Device settings ".bold()).left_aligned());
    let device_settings_list = List::new(vec![
        ListItem::new(Line::from(vec!["Path: ".into(), {
            let (device, valid_device) = app.device_valid();
            if valid_device {
                device.green()
            } else {
                format!("{device} (not a grey camera device)").red()
            }
        }])),
        ListItem::new(Line::from(vec![
            "Number of emitters: ".into(),
            app.emitters().to_string().green(),
        ])),
        ListItem::new(Line::from(vec![
            "Resolution height: ".into(),
            app.height()
                .map_or("auto".to_string(), |h| h.to_string())
                .green(),
        ])),
        ListItem::new(Line::from(vec![
            "Resolution width: ".into(),
            app.width()
                .map_or("auto".to_string(), |w| w.to_string())
                .green(),
        ])),
        ListItem::new(Line::from(vec![
            "FPS: ".into(),
            app.fps()
                .map_or("auto".to_string(), |f| f.to_string())
                .green(),
        ])),
    ])
    .highlight_style(Style::default().fg(Color::Yellow))
    .highlight_symbol(">")
    .highlight_spacing(HighlightSpacing::Always)
    .block(device_block);
    frame.render_stateful_widget(
        device_settings_list,
        device_area,
        app.device_settings_list_state(),
    );

    let search_block = Block::bordered()
        .title(Line::from(vec![" Search settings".bold(), " (advanced) ".dim()]).left_aligned());
    let search_settings_list = List::new(vec![
        ListItem::new(Line::from(vec![
            "Manual validation: ".into(),
            app.manual().to_string().green(),
        ])),
        ListItem::new(Line::from(vec![
            "Images to analyze in auto validation: ".into(),
            app.analyzer_img_count().to_string().green(),
        ])),
        ListItem::new(Line::from(vec![
            "Light difference significance factor: ".into(),
            app.ref_intensity_var_coef().to_string().green(),
        ])),
        ListItem::new(Line::from(vec![
            "Rejection threshold per control: ".into(),
            app.limit()
                .map_or("none".to_string(), |w| w.to_string())
                .green(),
        ])),
        ListItem::new(Line::from(vec![
            "Increment step: ".into(),
            app.inc_step().to_string().green(),
        ])),
    ])
    .highlight_style(Style::default().fg(Color::Yellow))
    .highlight_symbol(">")
    .highlight_spacing(HighlightSpacing::Always)
    .block(search_block);
    frame.render_stateful_widget(
        search_settings_list,
        search_area,
        app.search_settings_list_state(),
    );
}

/// Renders the main application interface showing the camera preview and the list of controls,
/// as well as extra information for the current control.
///
/// Returns the area used for the list of controls.
fn render_main<A>(frame: &mut Frame, area: Rect, app: &mut A)
where
    A: IrEnablerCtx,
{
    let [mut list_area, video_area] =
        Layout::horizontal([Constraint::Percentage(50), Constraint::Percentage(50)]).areas(area);

    let [question_area, new_list_area] =
        Layout::vertical([Constraint::Length(4), Constraint::Fill(0)]).areas(list_area);
    list_area = new_list_area;
    let question_block = Block::bordered().title(" Question ".bold());

    let question_text = if app.show_working_question() {
        Text::from(vec![
            "Is the camera emitter or preview blinking?"
                .to_line()
                .light_yellow(),
            keys_to_line(&[KEY_YES, KEY_NO]),
        ])
    } else {
        Text::from("Please wait...")
    };
    let question_paragraph = Paragraph::new(question_text)
        .block(question_block)
        .centered();
    frame.render_widget(question_paragraph, question_area);

    let list_block = Block::bordered().title(" Modifiable UVC Controls ".bold());
    let items: Vec<ListItem> = app
        .controls()
        .iter()
        .map(|ctrl| {
            let mut lines = vec![
                Line::from(format!(
                    "unit: {} selector: {}",
                    ctrl.unit(),
                    ctrl.selector()
                )),
                Line::from(format!("   initial: {:?}", ctrl.init())),
                Line::from(format!("   current: {:?}", ctrl.cur())),
            ];
            if let Some(max) = ctrl.max() {
                lines.push(Line::from(format!("   maximum: {:?}", max)));
            }
            ListItem::new(lines)
        })
        .collect();
    let controls_list = List::new(items)
        .highlight_style(Style::default().fg(Color::Yellow))
        .highlight_spacing(HighlightSpacing::Always)
        .scroll_padding(1)
        .block(list_block);
    frame.render_stateful_widget(controls_list, list_area, app.controls_list_state());

    render_video_preview(frame, video_area, app.image());
}

/// Renders the application UI based on the current application state.
pub fn ui<A>(frame: &mut Frame, app: &mut A)
where
    A: IrEnablerCtx,
{
    match app.view() {
        View::Menu => {
            let main_area = render_main_window(frame, &[KEY_NAVIGATE, KEY_CONTINUE, KEY_EXIT]);
            render_menu(frame, main_area, app);
            if app.show_menu_start_prompt() {
                render_confirm_start_popup(frame, main_area);
            }
        }
        View::Main => {
            let main_area = render_main_window(frame, &[KEY_EXIT]);
            render_main(frame, main_area, app);
            if app.show_main_abort_prompt() {
                render_confirm_abort_popup(frame, main_area);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::assert_ui_snapshot;

    #[derive(Default)]
    struct App {
        view: View,
        show_working_question: bool,
        show_main_abort_prompt: bool,
        show_menu_start_prompt: bool,
        device_settings_list_state: ListState,
        controls_list_state: ListState,
        device_valid: (String, bool),
        height: Option<u32>,
        width: Option<u32>,
        emitters: usize,
        fps: Option<u32>,
        search_settings_list_state: ListState,
        limit: Option<u16>,
        manual: bool,
        analyzer_img_count: u64,
        ref_intensity_var_coef: u64,
        inc_step: u8,
        controls: Vec<crate::video::uvc::XuControl>,
        image: Option<Image>,
    }

    impl IrEnablerCtx for App {
        fn view(&self) -> View {
            self.view
        }
        fn show_working_question(&self) -> bool {
            self.show_working_question
        }
        fn show_main_abort_prompt(&self) -> bool {
            self.show_main_abort_prompt
        }
        fn show_menu_start_prompt(&self) -> bool {
            self.show_menu_start_prompt
        }
        fn device_settings_list_state(&mut self) -> &mut ListState {
            &mut self.device_settings_list_state
        }
        fn device_valid(&self) -> (String, bool) {
            self.device_valid.clone()
        }
        fn height(&self) -> Option<u32> {
            self.height
        }
        fn width(&self) -> Option<u32> {
            self.width
        }
        fn emitters(&self) -> usize {
            self.emitters
        }
        fn fps(&self) -> Option<u32> {
            self.fps
        }
        fn search_settings_list_state(&mut self) -> &mut ListState {
            &mut self.search_settings_list_state
        }
        fn limit(&self) -> Option<u16> {
            self.limit
        }
        fn manual(&self) -> bool {
            self.manual
        }
        fn ref_intensity_var_coef(&self) -> u64 {
            self.ref_intensity_var_coef
        }
        fn analyzer_img_count(&self) -> u64 {
            self.analyzer_img_count
        }
        fn inc_step(&self) -> u8 {
            self.inc_step
        }
        fn controls_list_state(&mut self) -> &mut ListState {
            &mut self.controls_list_state
        }
        fn controls(&self) -> &[crate::video::uvc::XuControl] {
            &self.controls
        }
        fn image(&self) -> Option<&Image> {
            self.image.as_ref()
        }
    }

    #[test]
    fn test_menu_empty() {
        assert_ui_snapshot!(|frame| {
            let mut app = App::default();
            ui(frame, &mut app);
        });
    }

    #[test]
    fn test_menu_valid_values() {
        assert_ui_snapshot!(|frame| {
            let mut app = App::default();
            app.device_valid = ("/dev/video2".to_string(), true);
            app.height = Some(720);
            app.width = Some(1280);
            app.emitters = 1;
            app.fps = Some(30);
            app.limit = Some(5);
            app.manual = true;
            app.ref_intensity_var_coef = 50;
            app.inc_step = 1;
            app.analyzer_img_count = 30;
            ui(frame, &mut app);
        });
    }

    #[test]
    fn test_menu_strart() {
        assert_ui_snapshot!(|frame| {
            let mut app = App::default();
            app.show_menu_start_prompt = true;
            ui(frame, &mut app);
        });
    }

    #[test]
    fn test_main_empty() {
        assert_ui_snapshot!(|frame| {
            let mut app = App::default();
            app.view = View::Main;
            ui(frame, &mut app);
        });
    }

    #[test]
    fn test_main_with_question() {
        assert_ui_snapshot!(|frame| {
            let mut app = App::default();
            app.view = View::Main;
            app.show_working_question = true;
            ui(frame, &mut app);
        });
    }

    #[test]
    fn test_main_with_abort_prompt() {
        assert_ui_snapshot!(|frame| {
            let mut app = App::default();
            app.view = View::Main;
            app.show_main_abort_prompt = true;
            ui(frame, &mut app);
        });
    }

    #[test]
    fn test_main_with_controls() {
        assert_ui_snapshot!(|frame| {
            let mut app = App::default();
            app.view = View::Main;
            app.controls = vec![
                crate::video::uvc::XuControl::new(
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
                crate::video::uvc::XuControl::new(
                    2,
                    2,
                    vec![2, 2],
                    Some(vec![2, 2]),
                    Some(vec![2, 2]),
                    Some(vec![2, 2]),
                    Some(vec![2, 2]),
                    true,
                )
                .unwrap(),
                crate::video::uvc::XuControl::new(
                    3,
                    3,
                    vec![3, 3, 3],
                    Some(vec![3, 3, 3]),
                    Some(vec![3, 3, 3]),
                    Some(vec![3, 3, 3]),
                    Some(vec![3, 3, 3]),
                    true,
                )
                .unwrap(),
            ];

            ui(frame, &mut app);
        });
    }

    #[test]
    fn test_main_with_image() {
        assert_ui_snapshot!(|frame| {
            let mut app = App::default();
            app.view = View::Main;
            let img = image::open("tests/data/ferris.png").unwrap();
            app.image = Some(img);
            ui(frame, &mut app);
        });
    }
}
