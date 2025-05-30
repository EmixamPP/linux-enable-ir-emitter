use linux_enable_ir_emitter::ir::IrCamera;
use linux_enable_ir_emitter::tools::configurator::{ConfigureCtx, configure};
use linux_enable_ir_emitter::utils::logger::init_logger;

use std::path::PathBuf;

use anyhow::{Context, Result};
use clap::Parser;
use log::debug;

#[derive(Parser, Debug)]
#[command(
    version,
    before_help = format!("{} - {}", env!("CARGO_PKG_AUTHORS"), env!("CARGO_PKG_LICENSE")),
    about = "Provides support for infrared cameras.",
    after_help = "https://github.com/EmixamPP/linux-enable-ir-emitter"
)]
struct Cli {
    #[command(subcommand)]
    command: Commands,

    #[arg(
        short,
        long,
        help = "Specify the device to use.",
        default_value = "/dev/video2", // TODO: find grey camera here
    )]
    device: PathBuf,

    #[arg(
        short = 't',
        long,
        help = "Specify the resolution height.",
        default_value = None,
    )]
    height: Option<u32>,

    #[arg(
        short,
        long,
        help = "Specify the resolution width.",
        default_value = None
    )]
    width: Option<u32>,

    #[arg(short, long, help = "Enables verbose information.")]
    verbose: bool,
}

#[derive(clap::Subcommand, Debug)]
enum Commands {
    #[command(about = "Find an ir camera configuration enabling the emitter.")]
    Configure {
        #[arg(
            short,
            long,
            help = "Specify the number of emitters.",
            default_value = "1"
        )]
        emitters: usize,

        #[arg(
            short,
            long,
            help = "Specify the negative answer limit, -1 for unlimited.",
            default_value = "5"
        )]
        limit: i16,

        #[arg(
            short,
            long,
            help = "Enable manual verification.",
            default_value = "false"
        )]
        manual: bool,
    },

    #[command(about = "Reload the camera proxy.")]
    Run {},

    #[command(about = "Test an ir camera.")]
    Test {},

    #[command(about = "Tweak an ir camera configuration.")]
    Tweak {},
}

fn main() -> Result<()> {
    let args = Cli::parse();
    init_logger(args.verbose, true)?;

    debug!("Version: {}", env!("CARGO_PKG_VERSION"));
    debug!("Args:\n{:#?}", args);

    match args.command {
        Commands::Configure {
            emitters,
            limit,
            manual,
        } => configure(ConfigureCtx::new(
            IrCamera::open(args.device, args.width, args.height)
                .context(format!("failed to open or setup the ir camera"))?,
            emitters,
            limit,
            manual,
        )),

        Commands::Run {} => Ok(()),

        Commands::Test {} => Ok(()),

        Commands::Tweak {} => Ok(()),
    }
}
