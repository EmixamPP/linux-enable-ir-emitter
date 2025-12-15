mod app;
mod ui;

pub async fn configure() -> anyhow::Result<()> {
    let res = app::run(&mut ratatui::init()).await;
    ratatui::restore();
    if let Ok(msg) = &res
        && !msg.is_empty()
    {
        println!("{}", msg);
    }
    res.map(|_| ())
}
