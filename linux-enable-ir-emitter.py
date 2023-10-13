#!/usr/bin/python3

import argparse
import logging
import subprocess

from command import boot, configure, delete, run, test
from globals import ExitCode, check_root

if __name__ == "__main__":
    logging.basicConfig(
        format="%(levelname)s: %(message)s", level=logging.INFO)

    parser = argparse.ArgumentParser(
        description="Provides support for infrared cameras.",
        prog="linux-enable-ir-emitter",
        epilog="https://github.com/EmixamPP/linux-enable-ir-emitter",
        formatter_class=argparse.RawTextHelpFormatter,
    )
    parser.add_argument(
        "-v",
        "--verbose",
        help="print verbose information",
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "-V",
        "--version",
        action="version",
        version="%(prog)s @version@\nDevelopped by Maxime Dirksen - EmixamPP\nMIT License",
        help="show version information and exit",
    )
    parser.add_argument(
        "-d",
        "--device",
        metavar="device",
        help="specify the infrared camera, automatic detection by default",
    )
    command_subparser = parser.add_subparsers(dest="command")
    command_run = command_subparser.add_parser(
        "run",
        help="apply drivers",
    )
    command_configure = command_subparser.add_parser(
        "configure",
        help="generate ir emitter driver",
    )
    command_configure.add_argument(
        "-m",
        "--manual",
        help="activate manual configuration",
        action="store_true",
        default=False,
    )
    command_configure.add_argument(
        "-e",
        "--emitters",
        metavar="<count>",
        help="the number of emitters on the device, by default is 1",
        default=1,
        type=int,
    )
    command_configure.add_argument(
        "-l",
        "--limit",
        metavar="<count>",
        help="the number of negative answer before the pattern is skipped, by default is 40. Use -1 for unlimited",
        default=40,
        type=int,
    )
    command_test = command_subparser.add_parser(
        "test",
        help="test a camera",
    )
    command_boot = command_subparser.add_parser(
        "boot",
        help="enable ir at boot",
    )
    command_boot.add_argument(
        "boot_status",
        choices=["enable", "disable", "status"],
        help="specify the boot action to perform",
    )
    command_delete = command_subparser.add_parser(
        "delete",
        help="delete drivers",
    )

    args = parser.parse_args()

    if args.verbose:  # enable verbosity
        logging.getLogger().setLevel(logging.DEBUG)

    device: str | None = None
    # Determine the device path if needed
    if args.device and args.command in ("configure", "run", "delete"):
        device = args.device
        # Find the v4l path
        v4l_device = subprocess.run(
            f"find -L /dev/v4l/by-path -samefile {device}",
            shell=True,
            capture_output=True,
            text=True,
        ).stdout.strip()
        if len(v4l_device) == 0:
            logging.critical(
                f"The device {device} does not exists or is not a supported v4l camera."
            )
            exit(ExitCode.FAILURE)
        device = v4l_device

    # Execute the desired command
    if args.command == "run":
        run(device)

    elif args.command == "configure":
        check_root()
        configure(device, args.manual, args.emitters, args.limit)

    elif args.command == "test":
        test(device)

    elif args.command == "boot":
        check_root()
        boot(args.boot_status)

    elif args.command == "delete":
        check_root()
        delete(device)

    else:
        parser.print_help()
