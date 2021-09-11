#!/usr/bin/python3

import sys
import os
import argparse

from command import boot, fix, full, manual, quick, run, test
from globals import ExitCode


def _check_root():
    """Exit if the script isn't run as sudo
    """
    if os.getuid():
        print("Please run as root", file=sys.stderr)
        sys.exit(ExitCode.PERMISSION_DENIED)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Provides support for infrared cameras.",
        formatter_class=argparse.RawTextHelpFormatter,
        prog="linux-enable-ir-emitter",
        epilog="For help see : https://github.com/EmixamPP/linux-enable-ir-emitter/wiki"
    )

    parser.add_argument('-V', '--version', action='version', version='%(prog)s 2.1.0')
    parser.add_argument(
        "-p",
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

    command_quick.add_argument(
        "--skip-question", 
        help="skip all validations, may be misleading the camera configuration !",
        action='store_true',
        default=False
    )
    command_full.add_argument(
        "-f",
        metavar="file_path",
        help="use a .cap file to find the ir configuration",
        default=[None],
        nargs=1
    )
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
    if args.command == "run":
        exit_code = run.execute()
    elif args.command == "quick":
        _check_root()
        exit_code = quick.execute(args.p[0], args.skip_question)
    elif args.command == "full":
        _check_root()
        exit_code = full.execute(args.p[0], args.f[0])
    elif args.command == "manual":
        _check_root()
        exit_code = manual.execute(args.p[0])
    elif args.command == "boot":
        _check_root()
        exit_code = boot.execute(args.boot_status)
    elif args.command == "test":
        exit_code = test.execute()
    elif args.command == "fix":
        _check_root()
        exit_code = fix.execute(args.fix_target)
    else:
        parser.print_help()
    
    if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR:
        print("Cannot access to the camera ! Check the -p option or your other running processes.", file=sys.stderr)
    sys.exit(exit_code)
