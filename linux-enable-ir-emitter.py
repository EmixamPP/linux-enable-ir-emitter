#!/usr/bin/python3

import sys
import os
import argparse

parser = argparse.ArgumentParser(
    description="Provides support for infrared cameras.",
    formatter_class=argparse.RawTextHelpFormatter,
    prog="linux-enable-ir-emitter",
    epilog="For help see : https://github.com/EmixamPP/linux-enable-ir-emitter")

parser.add_argument(
    "command",
    help="can be one of the following : run, quick, full, manual, boot :\n\trun the actual config\n\tquick ir configuration\n\tfull ir configuration\n\tmanual ir configuration\n\tenable ir at boot",
    metavar="command",
    choices=["run", "quick", "full", "manual", "boot"]
)

parser.add_argument(
    "boot_status",
    metavar="boot_status",
    help="can be one of the following : enable, disable",
    choices=["enable", "disable"],
    nargs="?"
)

args = parser.parse_args()

if args.command == "run":
    print("run")
elif args.command == "quick":
    print('quick')
elif args.command == "full":
    print("full")
elif args.command == "manual":
    print("manual")
elif args.command == "boot":
    if os.getenv("SUDO_USER") is None:
        print("Please run as root")
        sys.exit(1)
    elif args.boot_status is None:
        print("Required [boot_status] argument")
    else:
        print("boot")
