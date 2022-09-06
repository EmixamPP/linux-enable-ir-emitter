#!/usr/bin/python3

import argparse
import logging
import os

from command import boot, run, configure, delete
from globals import ExitCode, check_root, get_all_devices


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
        version="%(prog)s 4.1.5\nDevelopped by Maxime Dirksen - EmixamPP\nMIT License",
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

    if args.verbose:  # enable verbosity
        logging.getLogger().setLevel(logging.DEBUG)

    device = None
    if args.command == "configure" or (args.device and args.command in ("run", "delete")):  # determine the device if needed
        # In case of configuration: use the specified device otherwise the default one (i.e. /dev/video2)
        # In case of run or delete: use the specified device
        device = os.path.realpath(args.device[0]) if args.device else "/dev/video2"  
        
        # check if the device exists
        available_devices = get_all_devices()
        if not len(available_devices):
            logging.critical("No camera device recognized by the system.")
            exit(ExitCode.FAILURE)
        elif device not in available_devices:
            logging.critical("The device {} does not exists.".format(device))
            logging.info("Please choose among this list: {}".format(' '.join(available_devices)))
            exit(ExitCode.FAILURE)

    # Execute the desired command
    if args.command == "run":
        run.execute(device)

    elif args.command == "configure":
        check_root()
        configure.execute(device, args.limit[0])

    elif args.command == "boot":
        check_root()
        boot.execute(args.boot_status)
    
    elif args.command == "delete":
        check_root()
        delete.execute(device)

    else:
        parser.print_help()
