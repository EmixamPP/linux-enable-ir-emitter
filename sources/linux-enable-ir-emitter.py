#!/usr/bin/python3

import argparse
import logging

from command import boot, fix, manual, run, test, configure
from globals import check_root


if __name__ == "__main__":
    logging.basicConfig(encoding='utf-8', format='%(levelname)s: %(message)s', level=logging.INFO)

    parser = argparse.ArgumentParser(
        description="Provides support for infrared cameras.",
        formatter_class=argparse.RawTextHelpFormatter,
        prog="linux-enable-ir-emitter",
        epilog="For support visit https://github.com/EmixamPP/linux-enable-ir-emitter/"
    )

    parser.add_argument(
        "-V", "--version", 
        action="version", 
        version="%(prog)s 3.1.0\nDevelopped by Maxime Dirksen - EmixamPP\nMIT License",
        help="show version information and exit"
    )
    parser.add_argument(
        "-d",
        metavar="device",
        help="specify your infrared camera '/dev/videoX', by default is '/dev/video2'",
        default=["/dev/video2"],
        nargs=1
    )
    parser.add_argument(
        '--verbose', 
        help="print verbose information",
        action='store_true', 
        default=False
    )

    command_subparser = parser.add_subparsers(dest='command')
    command_run = command_subparser.add_parser("run", help="apply the actual configuration")
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
        choices=["config", "chicony"], 
        help="specify the target to fix: {reset the config, uninstall chicony-ir-toggle}"
    )

    args = parser.parse_args()

    if args.verbose:
            logging.getLogger().setLevel(logging.DEBUG)

    if args.command == "run":
        run.execute()
    elif args.command == "configure":
        check_root()
        configure.execute(args.d[0])
    elif args.command == "manual":
        check_root()
        manual.execute(args.d[0])
    elif args.command == "boot":
        check_root()
        boot.execute(args.boot_status)
    elif args.command == "test":
        test.execute()
    elif args.command == "fix":
        check_root()
        fix.execute(args.fix_target)
    else:
        parser.print_help()
