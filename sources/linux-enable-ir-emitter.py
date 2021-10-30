#!/usr/bin/python3

import argparse
import logging
import re
import sys
import os

from command import boot, fix, manual, run, configure
from globals import ExitCode, check_root


if __name__ == "__main__":
    logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    parser = argparse.ArgumentParser(
        description="Provides support for infrared cameras.",
        prog="linux-enable-ir-emitter",
        epilog="For support visit https://github.com/EmixamPP/linux-enable-ir-emitter/wiki",
        formatter_class=argparse.RawTextHelpFormatter
    )

    parser.add_argument(
        "-v", "--verbose", 
        help="print verbose information",
        action='store_true', 
        default=False
    )
    parser.add_argument(
        "-V", "--version", 
        action="version", 
        version="%(prog)s 3.2.1\nDevelopped by Maxime Dirksen - EmixamPP\nMIT License",
        help="show version information and exit"
    )

    command_subparser = parser.add_subparsers(dest='command')
    command_run = command_subparser.add_parser("run", help="apply the driver")
    command_configure = command_subparser.add_parser("configure", help="automatic ir configuration")
    command_manual = command_subparser.add_parser("manual", help="manual ir configuration")
    command_boot = command_subparser.add_parser("boot", help="enable ir at boot")
    command_test = command_subparser.add_parser("test", help="try to trigger the ir emitter")
    command_fix = command_subparser.add_parser("fix", help="fix well know problems")

    command_boot.add_argument(
        "boot_status", 
        choices=["enable", "disable", "status"], 
        help="specify the boot action to perform"
    )
    command_fix.add_argument(
        "fix_target", 
        choices=["driver", "chicony"], 
        help="specify the target to fix: {reset the driver, uninstall chicony-ir-toggle}"
    )
    command_configure.add_argument(
        "-d", "--device",
        metavar="device",
        help="specify your infrared camera, by default is '/dev/video2'",
        default=["/dev/video2"],
        nargs=1
    )
    command_configure.add_argument(
        "-l", "--limit",
        metavar="k",
        help="after k negative answer the pattern will be skiped, by default is 5. Use 256 for unlimited",
        default=[5],
        type=int,
        nargs=1
    ) 
    
    args = parser.parse_args()

    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)

    if args.command == "run":
        run.execute()

    elif args.command == "configure":
        if not re.fullmatch("/dev/video[0-9]+", args.device[0]):
            args.device[0] = os.path.realpath(args.device[0])
            if not re.fullmatch("/dev/video[0-9]+", args.device[0]):
                logging.critical("The device {} does not exists.".format(args.device[0]))
                sys.exit(ExitCode.FAILURE)
        check_root()
        configure.execute(args.device[0], args.limit[0])

    elif args.command == "manual":
        check_root()
        manual.execute()

    elif args.command == "boot":
        check_root()
        boot.execute(args.boot_status)

    elif args.command == "test":
        run.execute(True)

    elif args.command == "fix":
        check_root()
        fix.execute(args.fix_target)
