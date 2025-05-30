use anyhow::Result;
use log::LevelFilter;
use log4rs::{
    append::{
        console::{ConsoleAppender, Target},
        file::FileAppender,
    },
    config::{Appender, Config, Root},
    encode::pattern::PatternEncoder,
    filter::threshold::ThresholdFilter,
};

pub fn init_logger(verbose_console: bool, enable_file: bool) -> Result<()> {
    // Log to console, level can be Trace or Info depending on `verbose_console`
    let console = ConsoleAppender::builder()
        .target(Target::Stdout)
        .encoder(Box::new(PatternEncoder::new("{l} - {m}\n")))
        .build();
    let mut config = Config::builder().appender(
        Appender::builder()
            .filter(Box::new(ThresholdFilter::new(if verbose_console {
                LevelFilter::Trace
            } else {
                LevelFilter::Info
            })))
            .build("console_logger", Box::new(console)),
    );
    let mut root_logger = Root::builder().appender("console_logger");

    // Log to file if `enable_file` is true, level is de default one
    if enable_file {
        let file = FileAppender::builder()
            .encoder(Box::new(PatternEncoder::new("{d(%H:%M:%S)} {l} - {m}\n")))
            .build(env!("LOG_PATH"))?;
        config = config.appender(Appender::builder().build("file_logger", Box::new(file)));
        root_logger = root_logger.appender("file_logger");
    }

    // set the logger, with default level Trace
    let root_logger = root_logger.build(LevelFilter::Trace);
    let config = config.build(root_logger)?;
    log4rs::init_config(config)?;

    Ok(())
}
