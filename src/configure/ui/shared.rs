use super::keys;
use crate::video::stream::Image;

use ansi_to_tui::IntoText as _;
use ratatui::{
    Frame,
    layout::{Constraint, Flex, Layout, Rect},
    style::{Color, Style, Stylize},
    text::Line,
    widgets::{Block, BorderType, Borders, HighlightSpacing, List, ListItem, ListState, Paragraph},
};

pub fn render_main_window(frame: &mut Frame, commands: &[keys::Key]) -> Rect {
    let area = frame.area();

    let window = Block::bordered()
        .border_type(BorderType::Thick)
        .title(Line::from(" linux-enable-ir-emitter ").bold().centered());
    let window_area = window.inner(area);

    let [main_area, command_area] =
        Layout::vertical([Constraint::Fill(1), Constraint::Length(2)]).areas(window_area);

    let command_block = Block::new().borders(Borders::TOP);
    let command_line = keys::keys_to_line(commands);
    let command_paragraph = Paragraph::new(command_line).centered().block(command_block);

    frame.render_widget(window, area);
    frame.render_widget(command_paragraph, command_area);
    main_area
}

pub fn render_video_preview(frame: &mut Frame, area: Rect, img: Option<&Image>) {
    let video_block = Block::bordered().title(Line::from(" Camera Preview ").bold());
    if let Some(img) = img {
        let img_area = video_block.inner(area);
        let img_str =
            crate::configure::ui::helper::image_to_ansi(img, (img_area.width, img_area.height));
        let img_txt = img_str
            .into_text()
            .unwrap_or("Failed to render image".into());
        frame.render_widget(img_txt, img_area);
    }
    frame.render_widget(video_block, area);
}

pub trait DeviceSettingsCtx {
    fn device_settings_list_state(&mut self) -> &mut ListState;
    fn device_valid(&self) -> (String, bool);
    fn height(&self) -> Option<u32>;
    fn width(&self) -> Option<u32>;
    fn emitters(&self) -> usize;
    fn fps(&self) -> Option<u32>;
}

pub trait SearchSettingsCtx {
    fn search_settings_list_state(&mut self) -> &mut ListState;
    fn limit(&self) -> Option<u16>;
    fn manual(&self) -> bool;
    fn analyzer_img_count(&self) -> u64;
    fn ref_intensity_var_coef(&self) -> u64;
    fn inc_step(&self) -> u8;
}

/// Renders the device settings section of the menu.
fn render_device_settings<A>(frame: &mut Frame, area: Rect, app: &mut A)
where
    A: DeviceSettingsCtx,
{
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
    frame.render_stateful_widget(device_settings_list, area, app.device_settings_list_state());
}

/// Renders the search settings section of the menu.
fn render_search_settings<A>(frame: &mut Frame, area: Rect, app: &mut A)
where
    A: SearchSettingsCtx,
{
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
    frame.render_stateful_widget(search_settings_list, area, app.search_settings_list_state());
}

/// Renders the info lines of the menu.
fn render_info(frame: &mut Frame, top_info_area: Rect, top_info: &str, bot_info_area: Rect) {
    let top_info_line = Line::from(top_info).blue().centered();
    frame.render_widget(top_info_line, top_info_area);

    let bot_info_line = Line::from(vec![
        "For more explanation, visit ".into(),
        "https://github.com/EmixamPP/linux-enable-ir-emitter".underlined(),
    ])
    .blue()
    .centered();
    frame.render_widget(bot_info_line, bot_info_area);
}

/// Renders the main menu with device and search settings.
pub fn render_full_menu<A>(frame: &mut Frame, area: Rect, app: &mut A, top_info: &str)
where
    A: DeviceSettingsCtx + SearchSettingsCtx,
{
    let [top_info_area, device_area, search_area, bot_info_area] = Layout::vertical([
        Constraint::Length(1),
        Constraint::Length(7),
        Constraint::Length(7),
        Constraint::Length(1),
    ])
    .flex(Flex::Center)
    .areas(area);

    render_info(frame, top_info_area, top_info, bot_info_area);
    render_device_settings(frame, device_area, app);
    render_search_settings(frame, search_area, app);
}

/// Renders the main menu with only the device settings.
pub fn render_device_menu<A>(frame: &mut Frame, area: Rect, app: &mut A, top_info: &str)
where
    A: DeviceSettingsCtx,
{
    let [top_info_area, device_area, bot_info_area] = Layout::vertical([
        Constraint::Length(1),
        Constraint::Length(7),
        Constraint::Length(1),
    ])
    .flex(Flex::Center)
    .areas(area);

    render_info(frame, top_info_area, top_info, bot_info_area);
    render_device_settings(frame, device_area, app);
}
#[cfg(test)]
mod tests {
    use super::*;
    use crate::assert_ui_snapshot;

    #[test]
    fn test_render_main_window() {
        assert_ui_snapshot!(|frame| {
            let _ = render_main_window(
                frame,
                &[keys::KEY_NAVIGATE, keys::KEY_CONTINUE, keys::KEY_EXIT],
            );
        });
    }

    #[test]
    fn test_render_video_preview_empty() {
        assert_ui_snapshot!(|frame| {
            let _ = render_video_preview(frame, frame.area(), None);
        });
    }

    #[test]
    fn test_render_video_preview() {
        let img = image::open("tests/data/ferris.png").unwrap();
        assert_ui_snapshot!(|frame| {
            let _ = render_video_preview(frame, frame.area(), Some(&img));
        });
    }
}
