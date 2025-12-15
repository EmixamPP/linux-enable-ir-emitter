use super::keys;
use crate::video::stream::Image;

use ansi_to_tui::IntoText as _;
use ratatui::{
    Frame,
    layout::{Constraint, Layout, Rect},
    style::Stylize,
    text::Line,
    widgets::{Block, BorderType, Borders, Paragraph},
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
