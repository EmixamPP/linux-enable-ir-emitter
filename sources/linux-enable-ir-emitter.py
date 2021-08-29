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

parser.add_argument('-V', '--version', action='version', version='%(prog)s 2.1.0')

parser.add_argument(
    "-p", "--video_path",
    metavar="video_path",
    help="specify the path to the infrared camera, by default is '/dev/video2'",
    default=["/dev/video2"],
    nargs=1
)

command_subparser = parser.add_subparsers(dest='command')
command_run = command_subparser.add_parser("run", help="run the actual config")
command_quick = command_subparser.add_parser("quick", help="quick ir configuration")
command_full = command_subparser.add_parser("full", help="full ir configuration")
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
video_path = args.video_path[0]

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
    command.boot(args.boot_status)
elif args.command == "test":
    command.test()
elif args.command == "fix":
    _check_sudo()
    command.fix(args.fix_target)
else:
    parser.print_help()
