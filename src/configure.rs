mod app;
mod ui;

pub async fn configure() -> anyhow::Result<()> {
    let res = app::run(&mut ratatui::init()).await;
    ratatui::restore();

    // Print any successful message to the user once the TUI is closed
    if let Ok(msg) = &res
        && !msg.is_empty()
    {
        println!("{}", msg);
    }
    res.map(|_| ()) // Delete the success message
}
