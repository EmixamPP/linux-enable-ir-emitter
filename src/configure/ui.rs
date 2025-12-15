mod helper;
use helper::*;
pub mod ir_enabler;
mod shared;
use shared::*;
pub mod tool_menu;

pub mod keys {
    use crossterm::event::KeyCode;
    use derive_more::{From, Into};
    use ratatui::{
        style::Stylize,
        style::{Color, Style},
        text::{Line, Span},
    };

    #[derive(Debug, Clone, Copy, PartialEq, Eq, From, Into)]
    pub(crate) struct Key(pub KeyCode);

    pub fn keys_to_line(keys: &[Key]) -> Line<'static> {
        let mut spans = Vec::with_capacity(keys.len() * 3);
        for (i, key) in keys.iter().enumerate() {
            match key.0 {
                KeyCode::Esc => {
                    spans.push("Quit <".bold());
                    spans.push(Span::styled("Esc", Style::default().fg(Color::Red)));
                    spans.push(">".bold());
                }
                KeyCode::Tab => {
                    spans.push("Navigate <".bold());
                    spans.push(Span::styled("Tab", Style::default().fg(Color::Yellow)));
                    spans.push(">".bold());
                }
                KeyCode::Enter => {
                    spans.push("Continue <".bold());
                    spans.push(Span::styled("Enter", Style::default().fg(Color::Green)));
                    spans.push(">".bold());
                }
                KeyCode::Char('y') => {
                    spans.push("Yes <".bold());
                    spans.push(Span::styled("y", Style::default().fg(Color::Green)));
                    spans.push(">".bold());
                }
                KeyCode::Char('n') => {
                    spans.push("No <".bold());
                    spans.push(Span::styled("n", Style::default().fg(Color::Red)));
                    spans.push(">".bold());
                }
                _ => {
                    spans.push("? <".bold());
                    spans.push(Span::raw(format!("{:?}", key.0)));
                    spans.push(">".bold());
                }
            }

            if i != keys.len() - 1 {
                spans.push(Span::raw("   "));
            }
        }
        Line::from(spans)
    }

    pub const KEY_EXIT: Key = Key(KeyCode::Esc);
    pub const KEY_NAVIGATE: Key = Key(KeyCode::Tab);
    pub const KEY_CONTINUE: Key = Key(KeyCode::Enter);
    pub const KEY_YES: Key = Key(KeyCode::Char('y'));
    pub const KEY_NO: Key = Key(KeyCode::Char('n'));
    pub const KEY_DELETE: Key = Key(KeyCode::Backspace);
}

#[cfg(test)]
mod tests {
    use super::keys::*;
    use crate::assert_ui_snapshot;
    use ratatui::layout::{Constraint, Layout};

    #[test]
    fn test_render_keys() {
        assert_ui_snapshot!(|frame| {
            let keys = [KEY_EXIT, KEY_NAVIGATE, KEY_CONTINUE, KEY_YES, KEY_NO];

            let chunks =
                Layout::vertical(vec![Constraint::Length(1); keys.len()]).split(frame.area());

            for (i, key) in keys.into_iter().enumerate() {
                frame.render_widget(keys_to_line(&[key]), chunks[i]);
            }
        });
    }
}
