mod helper;
use helper::*;
pub mod ir_enabler;
mod shared;
use shared::*;
pub use shared::{DeviceSettingsCtx, SearchSettingsCtx};
pub mod tool_menu;
pub mod tweaker;

mod keys {
    use crossterm::event::KeyCode;
    use ratatui::{
        style::Stylize,
        style::{Color, Style},
        text::{Line, Span},
    };

    #[derive(Debug, Clone, Copy)]
    enum KeyRepr {
        Code(KeyCode),
        Str(&'static str),
    }

    #[derive(Debug, Clone, Copy)]
    pub struct Key {
        repr: KeyRepr,
        name: &'static str,
        color: Color,
    }

    impl Key {
        const fn new(code: KeyCode, name: &'static str, color: Color) -> Self {
            Self {
                repr: KeyRepr::Code(code),
                name,
                color,
            }
        }

        const fn custom(repr: &'static str, name: &'static str, color: Color) -> Self {
            Self {
                repr: KeyRepr::Str(repr),
                name,
                color,
            }
        }
    }

    pub fn keys_to_line(keys: &[Key]) -> Line<'static> {
        let mut spans = Vec::with_capacity(keys.len() * 3);
        for (i, key) in keys.iter().enumerate() {
            spans.push(format!("{} <", key.name).bold());
            spans.push(Span::styled(
                match key.repr {
                    KeyRepr::Code(code) => code.to_string(),
                    KeyRepr::Str(s) => s.to_string(),
                },
                Style::default().fg(key.color),
            ));
            spans.push(">".bold());

            if i != keys.len() - 1 {
                spans.push(Span::raw("   "));
            }
        }
        Line::from(spans)
    }

    pub const KEY_EXIT: Key = Key::new(KeyCode::Esc, "Quit", Color::Red);
    pub const KEYS_NAVIGATE: Key = Key::custom("↑↓", "Navigate", Color::Yellow);
    pub const KEYS_EDITING_NAVIGATE: Key = Key::custom("←↑↓→", "Navigate/Modify", Color::Yellow);
    pub const KEY_NAVIGATE: Key = Key::new(KeyCode::Tab, "Navigate", Color::Yellow);
    pub const KEY_CONTINUE: Key = Key::new(KeyCode::Enter, "Continue", Color::Green);
    pub const KEY_YES: Key = Key::new(KeyCode::Char('y'), "Yes", Color::Green);
    pub const KEY_NO: Key = Key::new(KeyCode::Char('n'), "No", Color::Red);
    pub const KEY_EDIT: Key = Key::new(KeyCode::Enter, "Edit", Color::Green);
}

#[cfg(test)]
mod tests {
    use super::keys::*;
    use crate::assert_ui_snapshot;
    use ratatui::layout::{Constraint, Layout};

    #[test]
    fn test_render_keys() {
        assert_ui_snapshot!(|frame| {
            let keys = [KEY_EXIT, KEYS_NAVIGATE, KEY_CONTINUE, KEY_YES, KEY_NO];

            let chunks =
                Layout::vertical(vec![Constraint::Length(1); keys.len()]).split(frame.area());

            for (i, key) in keys.into_iter().enumerate() {
                frame.render_widget(keys_to_line(&[key]), chunks[i]);
            }
        });
    }
}
