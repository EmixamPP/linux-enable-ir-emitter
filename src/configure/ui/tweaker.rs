use super::{
    keys::{KEY_CONTINUE, KEY_EDIT, KEY_EXIT, KEY_NAVIGATE, KEY_NO, KEY_YES, keys_to_line},
    popup_area, render_device_menu, render_main_window, render_video_preview,
};
use crate::video::uvc::XuControl;
use crate::{configure::ui::DeviceSettingsCtx, video::stream::Image};

use ratatui::{
    Frame,
    layout::{Constraint, Layout, Rect},
    style::{Color, Style, Stylize},
    text::Line,
    widgets::{Block, BorderType, Clear, HighlightSpacing, List, ListItem, ListState, Paragraph},
};

#[derive(Debug, PartialEq, Eq, Clone, Copy, Default)]
pub enum View<'a> {
    #[default]
    Menu,
    Main,
    Edition(&'a XuControl),
}

pub trait TweakerCtx {
    fn view(&self) -> View<'_>;
    fn show_save_exit_prompt(&self) -> bool;

    fn controls_list_state(&mut self) -> &mut ListState;
    fn controls(&self) -> &[crate::video::uvc::XuControl];

    fn image(&self) -> Option<&Image>;
    fn error_message(&self) -> Option<&String>;
}

/// Renders a confirmation popup to exit the process without saving.
fn render_save_exit_popup(frame: &mut Frame, area: Rect) {
    let block = Block::bordered().border_type(BorderType::Double);
    let area = popup_area(area, 4, 50);
    frame.render_widget(Clear, area);
    frame.render_widget(block, area);

    let [info_area, command_area] =
        Layout::vertical([Constraint::Length(1), Constraint::Length(1)])
            .margin(1)
            .areas(area);

    let paragraph = Paragraph::new("Do you want to save this configuration?").centered();
    frame.render_widget(paragraph, info_area);

    let command_line = keys_to_line(&[KEY_YES, KEY_NO]);
    let command_paragraph = Paragraph::new(command_line).centered();
    frame.render_widget(command_paragraph, command_area);
}

/// Renders the main application interface showing the camera preview and the list of controls,
/// as well as extra information for the current control.
///
/// Returns the area used for the list of controls.
fn render_main<A>(frame: &mut Frame, area: Rect, app: &mut A)
where
    A: TweakerCtx,
{
    let [list_area, video_area] =
        Layout::horizontal([Constraint::Percentage(50), Constraint::Percentage(50)]).areas(area);

    let list_block = Block::bordered().title(" UVC Controls ".bold());
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
    A: TweakerCtx + DeviceSettingsCtx,
{
    match app.view() {
        View::Menu => {
            let main_area = render_main_window(frame, &[KEY_NAVIGATE, KEY_CONTINUE, KEY_EXIT]);
            render_device_menu(
                frame,
                main_area,
                app,
                "The tool allows you to modify the UVC camera controls.",
            );
        }
        View::Main | View::Edition(_) => {
            let keys = match app.view() {
                View::Main => vec![KEY_NAVIGATE, KEY_EDIT, KEY_EXIT],
                _ => vec![KEY_EXIT],
            };
            let main_area = render_main_window(frame, &keys);
            render_main(frame, main_area, app);
            
            if let Some(err) = app.error_message() {
                let error_block = Block::bordered()
                    .title(Line::from(" Error ").bold())
                    .border_type(BorderType::Double)
                    .border_style(Style::default().fg(Color::Red));
                let area = popup_area(main_area, 3, 50);
                let p = Paragraph::new(Line::from(err.as_str()).style(Style::default().fg(Color::Red)))
                    .block(error_block)
                    .centered();
                frame.render_widget(Clear, area);
                frame.render_widget(p, area);
            } else if app.show_save_exit_prompt() {
                render_save_exit_popup(frame, main_area);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::assert_ui_snapshot;

    #[derive(Default)]
    struct App<'a> {
        view: View<'a>,
        show_save_exit_prompt: bool,
        device_settings_list_state: ListState,
        controls_list_state: ListState,
        device_valid: (String, bool),
        height: Option<u32>,
        width: Option<u32>,
        emitters: usize,
        fps: Option<u32>,
        controls: Vec<crate::video::uvc::XuControl>,
        image: Option<Image>,
    }

    impl<'a> TweakerCtx for App<'a> {
        fn view(&self) -> View<'_> {
            self.view
        }
        fn show_save_exit_prompt(&self) -> bool {
            self.show_save_exit_prompt
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
        fn error_message(&self) -> Option<&String> {
            None // TODO test
        }
    }

    impl<'a> DeviceSettingsCtx for App<'a> {
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
    fn test_main_with_abort_prompt() {
        assert_ui_snapshot!(|frame| {
            let mut app = App::default();
            app.view = View::Main;
            app.show_save_exit_prompt = true;
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
