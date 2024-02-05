#!/usr/bin/python3

import argparse
import logging
import subprocess
import sys

sys.path.append("@libdir@")

from utils import ExitCode, check_root
from command import boot, cpp_commands

if __name__ == "__main__":
    logging.basicConfig(format="%(levelname)s: %(message)s", level=logging.INFO)

    parser = argparse.ArgumentParser(
        description="Provides support for infrared cameras.",
        prog="linux-enable-ir-emitter",
        epilog="https://github.com/EmixamPP/linux-enable-ir-emitter",
        formatter_class=argparse.RawTextHelpFormatter,
    )

    parser.add_argument(
        "-V",
        "--version",
        action="version",
        version="%(prog)s @version@\nDevelopped by Maxime Dirksen - EmixamPP\nMIT License",
        help="show version information and exit",
    )

    parser.add_argument(
        "-v",
        "--verbose",
        help="print verbose information",
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "-d",
        "--device",
        metavar="device",
        help="specify the camera, automatic by default",
        default="",
    )
    parser.add_argument(
        "-w",
        "--width",
        metavar="width",
        help="specify the width, automatic by default",
        default=-1,
        type=int,
    )
    parser.add_argument(
        "-t",
        "--height",
        metavar="height",
        help="specify the height, automatic by default",
        default=-1,
        type=int,
    )

    command_subparser = parser.add_subparsers(dest="command")
    command_run = command_subparser.add_parser(
        "run",
        help="apply a configuration",
    )

    command_configure = command_subparser.add_parser(
        "configure",
        help="create an ir emitter configuration",
    )
    command_configure.add_argument(
        "-m",
        "--manual",
        help="manual verification",
        action="store_true",
        default=False,
    )
    command_configure.add_argument(
        "-e",
        "--emitters",
        metavar="<count>",
        help="specify the number of emitters, by default is 1",
        default=1,
        type=int,
    )
    command_configure.add_argument(
        "-l",
        "--limit",
        metavar="<count>",
        help="specify the negative answer limit, by default is 10. Use -1 for unlimited",
        default=10,
        type=int,
    )
    command_configure.add_argument(
        "-g",
        "--no-gui",
        help="disable video feedback",
        action="store_true",
        default=False,
    )

    command_test = command_subparser.add_parser(
        "tweak",
        help="create a camera configuration",
    )

    command_test = command_subparser.add_parser(
        "test",
        help="test a camera",
    )

    command_boot = command_subparser.add_parser(
        "boot",
        help="apply the configurations at boot",
    )
    command_boot.add_argument(
        "boot_status",
        choices=["enable", "disable", "status"],
        help="specify the boot action to perform",
    )

    command_delete = command_subparser.add_parser(
        "delete",
        help="delete configurations",
    )

    args = parser.parse_args()

    if args.verbose:
        cpp_commands.enable_debug()
        logging.getLogger().setLevel(logging.DEBUG)

    # Execute the desired command
    res = ExitCode.FAILURE
    if args.command == "run":
        res = cpp_commands.run(args.device.encode())

    elif args.command == "configure":
        check_root()
        res = cpp_commands.configure(
            args.device.encode(),
            args.width,
            args.height,
            args.manual,
            args.emitters,
            args.limit,
            args.no_gui,
        )
        if res == ExitCode.SUCCESS:
            boot("enable")

    elif args.command == "tweak":
        check_root()
        res = cpp_commands.tweak(args.device.encode(), args.width, args.height)
        if res == ExitCode.SUCCESS:
            boot("enable")

    elif args.command == "test":
        res = cpp_commands.test(args.device.encode(), args.width, args.height)

    elif args.command == "boot":
        check_root()
        res = boot(args.boot_status)

    elif args.command == "delete":
        check_root()
        res = cpp_commands.delete_config(args.device.encode())

    else:
        parser.print_help()

    exit(res)
