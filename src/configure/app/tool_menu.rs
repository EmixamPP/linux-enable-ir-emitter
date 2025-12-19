use crate::configure::app::ir_enabler::App as IREnablerApp;
use crate::configure::app::tweaker::App as TweakerApp;
use crate::configure::ui::tool_menu::ui;

use anyhow::Result;
use crossterm::event::{self, KeyCode};
use crossterm::event::{Event, KeyEventKind};

/// Application state for the tool menu.
pub enum State {
    IREnablerSelected,
    UVCTweakerSelected,
}

/// "Tool menu" application.
pub struct App {
    pub state: State,
}

impl App {
    /// Creates a new "Tool menu" application.
    pub fn new() -> Self {
        Self {
            state: State::IREnablerSelected,
        }
    }

    /// Runs the tool menu application.
    pub async fn run(&mut self, terminal: &mut ratatui::DefaultTerminal) -> Result<&'static str> {
        loop {
            terminal.draw(|f| ui(f, self))?;
            match event::read()? {
                Event::Key(key_event) if key_event.kind == KeyEventKind::Press => {
                    match key_event.code {
                        KeyCode::Tab => self.next_tool(),
                        KeyCode::Enter => return self.start_tool(terminal).await,
                        KeyCode::Esc => return Ok(""),
                        _ => {}
                    }
                }
                _ => {}
            };
        }
    }

    /// Switches to the next tool selected in the menu.
    pub fn next_tool(&mut self) {
        self.state = match self.state {
            State::IREnablerSelected => State::UVCTweakerSelected,
            State::UVCTweakerSelected => State::IREnablerSelected,
        };
    }

    pub async fn start_tool(
        &self,
        terminal: &mut ratatui::DefaultTerminal,
    ) -> Result<&'static str> {
        match self.state {
            State::IREnablerSelected => IREnablerApp::new().run(terminal).await,
            State::UVCTweakerSelected => TweakerApp::new().run(terminal).await,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_new_app() {
        let app = App::new();
        assert!(matches!(app.state, State::IREnablerSelected));
    }

    #[test]
    fn test_next_tool() {
        let mut app = App::new();
        app.next_tool();
        assert!(matches!(app.state, State::UVCTweakerSelected));
        app.next_tool();
        assert!(matches!(app.state, State::IREnablerSelected));
        app.next_tool();
        assert!(matches!(app.state, State::UVCTweakerSelected));
    }
}
