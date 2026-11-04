mod helper;
pub mod ir_enabler;
pub mod tool_menu;
pub mod tweaker;

pub async fn run(terminal: &mut ratatui::DefaultTerminal) -> anyhow::Result<&'static str> {
    tool_menu::App::new().run(terminal).await
}
