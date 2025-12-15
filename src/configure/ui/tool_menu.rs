use super::render_main_window;
use crate::configure::app::tool_menu::{App, State};

use ratatui::{
    Frame,
    layout::{Constraint, Flex, Layout},
    style::Stylize,
    text::{Line, Text},
    widgets::{Block, BorderType, Paragraph},
};

/// Renders the application UI based on the current application state.
pub fn ui(frame: &mut Frame, app: &App) {
    let main_area = render_main_window(
        frame,
        &[
            crate::configure::ui::keys::KEY_NAVIGATE,
            crate::configure::ui::keys::KEY_CONTINUE,
            crate::configure::ui::keys::KEY_EXIT,
        ],
    );

    let [menu_area] = Layout::vertical([Constraint::Length(4)])
        .flex(Flex::Center)
        .areas(main_area);
    let [enabler_area, tweaker_area] =
        Layout::horizontal([Constraint::Length(45), Constraint::Length(45)])
            .flex(Flex::Center)
            .areas(menu_area);

    let enabler_block = match app.state {
        State::IREnablerSelected => Block::bordered()
            .border_type(BorderType::Thick)
            .bold()
            .yellow(),
        _ => Block::bordered().border_type(BorderType::Plain),
    }
    .title(Line::from(" IR Enabler ").bold().centered());
    let enabler_text =
        Text::from("Enable your IR emitter(s) automatically\n(recommended for most users)");
    let enabler_paragraph = Paragraph::new(enabler_text).block(enabler_block).centered();
    frame.render_widget(enabler_paragraph, enabler_area);

    let tweaker_block = match app.state {
        State::UVCTweakerSelected => Block::bordered()
            .border_type(BorderType::Thick)
            .bold()
            .yellow(),
        _ => Block::bordered().border_type(BorderType::Plain),
    }
    .title(Line::from(" UVC Tweaker ").bold().centered());
    let tweaker_text = Text::from("Tweak your UVC controls manually\n(advanced)");
    let tweaker_paragraph = Paragraph::new(tweaker_text).block(tweaker_block).centered();
    frame.render_widget(tweaker_paragraph, tweaker_area);
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::assert_ui_snapshot;

    #[test]
    fn test_tool_menu_enabler_selected() {
        let app = App {
            state: State::IREnablerSelected,
        };
        assert_ui_snapshot!(|frame| ui(frame, &app));
    }

    #[test]
    fn test_tool_menu_tweaker_selected() {
        let app = App {
            state: State::UVCTweakerSelected,
        };
        assert_ui_snapshot!(|frame| ui(frame, &app));
    }
}
