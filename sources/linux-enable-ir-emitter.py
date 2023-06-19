#!/usr/bin/python3

import argparse
import logging
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
        version="%(prog)s @version@\nDevelopped by Maxime Dirksen - EmixamPP\nMIT License",
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
    command_configure = command_subparser.add_parser("configure", help="generate ir emitter driver")
    command_configure.add_argument(
        "-e", "--emitters",
        metavar="<count>",
        help="the number of emitters on the device, by default is 1",
        default=[1],
        type=int,
        nargs=1
    )
    command_configure.add_argument(
        "-l", "--limit",
        metavar="<count>",
        help="the number of negative answer before the pattern is skiped, by default is 5. Use 256 for unlimited",
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
    # Determine the device if needed
    # In case of configuration: use the specified device otherwise the default one (i.e. /dev/video2)
    # In case of run or delete: use the specified device
    if args.command == "configure" or (args.device and args.command in ("run", "delete")):
        device = args.device[0] if args.device else "/dev/video2"
        # Find the v4l path
        v4l_device = subprocess.run(f"find -L /dev/v4l/by-path -samefile {device}", shell=True, capture_output=True)
        v4l_device = v4l_device.stdout.decode('utf-8').strip()
        if (len(v4l_device) == 0):
            logging.critical(f"The device {device} does not exists or is not a supported v4l camera.")
            exit(ExitCode.FAILURE)
        device = v4l_device
        
    # Execute the desired command
    if args.command == "run":
        run(device)

    elif args.command == "configure":
        check_root()
        configure(device, args.emitters[0], args.limit[0])

    elif args.command == "boot":
        check_root()
        boot(args.boot_status)
    
    elif args.command == "delete":
        check_root()
        delete(device)

    else:
        parser.print_help()
