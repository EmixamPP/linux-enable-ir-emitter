#!/usr/bin/python3

import sys
import os
import argparse

import command


def _check_sudo():
    """Exit if the script isn't run as sudo
    """
    if os.getuid():
        print("Please run as root", file=sys.stderr)
        sys.exit(1)


parser = argparse.ArgumentParser(
    description="Provides support for infrared cameras.",
    formatter_class=argparse.RawTextHelpFormatter,
    prog="linux-enable-ir-emitter",
    epilog="For help see : https://github.com/EmixamPP/linux-enable-ir-emitter/wiki"
)

parser.add_argument(
    "command",
    help="""can be one of the following : run, quick, full, manual, boot, test :
  run the actual config
  quick ir configuration
  full ir configuration
  manual ir configuration
  enable ir at boot
  try to trigger the ir emitter""",
    metavar="command",
    choices=["run", "quick", "full", "manual", "boot", "test"]
)

parser.add_argument(
    "boot_status",
    metavar="boot_status",
    help="can be one of the following : enable, disable",
    choices=["enable", "disable"],
    nargs="?"
)

parser.add_argument(
    "-p", "--video_path",
    metavar="video_path",
    help="specify the path to the infrared camera, by default is '/dev/video2'",
    nargs=1
)

args = parser.parse_args()
video_path = args.video_path[0] if args.video_path else "/dev/video2"

if args.command == "run":
    command.run()
elif args.command == "quick":
    _check_sudo()
    command.quick(video_path)
elif args.command == "full":
    _check_sudo()
    command.full(video_path)
elif args.command == "manual":
    _check_sudo()
    command.manual(video_path)
elif args.command == "boot":
    _check_sudo()
    if args.boot_status is None:
        print("Required [boot_status] argument", file=sys.stderr)
        sys.exit(1)
    else:
        command.boot(args.boot_status)
elif args.command == "test":
    command.test()
else:
    parser.print_help()
