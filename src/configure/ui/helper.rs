use image::{DynamicImage, imageops::FilterType};
use ratatui::layout::{Constraint, Flex, Layout, Rect};

/// Renders a widget to a test terminal for snapshot testing.
#[cfg(test)]
#[macro_export]
macro_rules! assert_ui_snapshot {
    ($render_fn:expr) => {{
        let backend = ratatui::backend::TestBackend::new(100, 30);
        let mut terminal = ratatui::Terminal::new(backend).unwrap();
        terminal.draw($render_fn).unwrap();
        insta::assert_snapshot!(terminal.backend());
    }};
}

/// Creates a centered popup area within the given `area` with specified `height` and `width`.
pub fn popup_area(area: Rect, height: u16, width: u16) -> Rect {
    let vertical = Layout::vertical([Constraint::Length(height)]).flex(Flex::Center);
    let horizontal = Layout::horizontal([Constraint::Length(width)]).flex(Flex::Center);
    let [area] = vertical.areas(area);
    let [area] = horizontal.areas(area);
    area
}

/// Convert an image to an ANSI string representation fitting to the exact given size.
pub fn image_to_ansi(image: &DynamicImage, size: (u16, u16)) -> String {
    let image = image.resize_exact(size.0 as u32, size.1 as u32, FilterType::Nearest);
    ansipix::convert(image, 100, false)
}

#[cfg(test)]
mod tests {
    use super::*;
    use ratatui::widgets::Block;

    #[test]
    fn test_popup_area_snapshot() {
        assert_ui_snapshot!(|frame| {
            let block = Block::bordered().title("popup_area");
            let popup = popup_area(frame.area(), 15, 50);
            frame.render_widget(block, popup);
        });
    }
}
