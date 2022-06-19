#!/usr/bin/python3

import argparse
import logging
import re
import os
import subprocess

from command import boot, run, configure, delete
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
        version="%(prog)s 4.0.0\nDevelopped by Maxime Dirksen - EmixamPP\nMIT License",
        help="show version information and exit"
    )
    parser.add_argument(
        "-d", "--device",
        metavar="device",
        help="specify the infrared camera, by default is '/dev/video2'",
        nargs=1
    )
    command_subparser = parser.add_subparsers(dest='command')
    command_run = command_subparser.add_parser("run", help="apply drivers")
    command_configure = command_subparser.add_parser("configure", help="generate ir emitter's driver")
    command_configure.add_argument(
        "-l", "--limit",
        metavar="k",
        help="after k negative answer the pattern will be skiped, by default is 5. Use 256 for unlimited",
        default=[5],
        type=int,
        nargs=1
    )
    command_delete = command_subparser.add_parser("delete", help="delete drivers")
    command_boot = command_subparser.add_parser("boot", help="enable ir at boot")
    command_boot.add_argument(
        "boot_status", 
        choices=["enable", "disable", "status"], 
        help="specify the boot action to perform"
    )

    args = parser.parse_args()

    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)
       
    device = args.device[0] if args.device else "/dev/video2"
    if not re.fullmatch("/dev/video[0-9]+", device):
        device = os.path.realpath(device) # try to get the path pattern /dev/videoX
    
    available_devices = subprocess.run(["ls /dev/video*"], shell=True, capture_output=True)
    if available_devices.returncode:
        logging.critical("No camera device recognized by the system.")
        exit(ExitCode.FAILURE)
    available_devices = available_devices.stdout.decode("utf-8").replace("\n", " ")
    if device not in available_devices.split():
        logging.critical("The device {} does not exists.".format(device))
        logging.info("Please choose among this list: {}".format(available_devices))
        exit(ExitCode.FAILURE)

    if args.command == "run":
        from command import run
        if not args.device: # use specified device otherwise all(=None)
            device = None
        run.execute(device)

    elif args.command == "configure":
        from command import configure
        check_root()
        configure.execute(device, args.limit[0])

    elif args.command == "boot":
        check_root()
        boot.execute(args.boot_status)
    
    elif args.command == "delete":
        check_root()
        if not args.device: # use specified device otherwise all(=None)
            device = None
        delete.execute(device)

    else:
        parser.print_help()
