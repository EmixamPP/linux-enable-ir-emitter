mod configuration;
mod configure;
mod logger;
mod run;
mod video;

use anyhow::Result;
use clap::Parser;

#[tokio::main]
async fn main() -> Result<()> {
    let cli = Cli::parse();

    if cli.config {
        return configuration::print_config();
    }

    if cli.log {
        return logger::print_log();
    }

    if cli.grey_devices {
        return video::print_grey_devices();
    }

    match cli.command {
        Some(Commands::Configure) => {
            logger::init_file()?;
            log::debug!("Version {}.", env!("CARGO_PKG_VERSION"));
            configure::configure().await
        }
        Some(Commands::Run { device, fd, config }) => {
            logger::init_term()?;
            run::run(device.as_deref(), fd, config.as_deref())
        }
        None => Ok(()),
    }
}

#[derive(Parser, Debug)]
#[command(
        version,
        before_help = format!("{} - {}", env!("CARGO_PKG_AUTHORS"), env!("CARGO_PKG_LICENSE")),
        about = "Provides support for infrared cameras.",
        after_help = "https://github.com/EmixamPP/linux-enable-ir-emitter"
    )]
struct Cli {
    #[command(subcommand)]
    command: Option<Commands>,
    #[arg(long, help = "Print the configuration file.")]
    config: bool,
    #[arg(long, help = "Print the recorded logs (for receiving help).")]
    log: bool,
    #[arg(long, help = "Print the list of grey devices with their UVC controls.")]
    grey_devices: bool,
}

#[derive(clap::Subcommand, Debug)]
enum Commands {
    #[command(about = "Configure your ir camera.")]
    Configure,
    #[command(about = "Apply saved configurations to devices.")]
    Run {
        #[arg(
            short,
            long,
            help = "Specify a device. Default: all configured devices.",
            default_value = None
        )]
        device: Option<std::path::PathBuf>,
        #[arg(
            short,
            long,
            help = "Specify an opened file descriptor (in RDWR) for the device. Default: open a new one.",
            default_value = None,
            requires = "device"
        )]
        fd: Option<i32>,
        #[arg(
            short,
            long,
            help = "Specify the configuration file to use. Default: use the default configuration path.",
            default_value = None,
        )]
        config: Option<std::path::PathBuf>,
    },
}
