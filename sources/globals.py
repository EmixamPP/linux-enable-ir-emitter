import os
import enum
import logging
import sys
import subprocess


class ExitCode(enum.IntEnum):
    SUCCESS = 0
    FAILURE = 1
    FILE_DESCRIPTOR_ERROR = 126
    MISSING_DEPENDENCY = 3
    ROOT_REQUIRED = 2


def exit_if_file_descriptor_error(exit_code: ExitCode, device: str) -> None:
    """Exit if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR

    Args:
        exit_code: the exit code to check
        device: the infrared camera '/dev/videoX'
    """
    if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR:
        logging.critical("Cannot access to %s.", device)
        sys.exit(ExitCode.FILE_DESCRIPTOR_ERROR)


def check_root() -> None:
    """Exit if the script isn't run as root"""
    if os.getuid():
        logging.critical("Please run as root.")
        sys.exit(ExitCode.ROOT_REQUIRED)


LOCAL_PATH = path = os.path.dirname(os.path.abspath(__file__))


def _driver_file_path() -> str:
    # old version, ensure compatibility with 3.0.0
    old_version_path = LOCAL_PATH + "/irConfig.yaml"
    path = "/etc/linux-enable-ir-emitter.yaml"
    if os.path.exists(old_version_path):
        check_root()
        with open(path, "w") as new, open(old_version_path) as old:
            line = old.readline()
            while(line):
                if line == "!!python/object:IrConfiguration.IrConfiguration\n":
                    new.write(line.replace("IrConfiguration.IrConfiguration", "driver.Driver.Driver"))
                if line == "_data:\n":
                    new.write("_control:\n")
                elif "'0x" in line:
                    new.write(line.replace("'", ""))
                elif line[:11] == "_videoPath:":
                    new.write(line.replace("videoPath", "device"))
                else:
                    new.write(line)
                line = old.readline()
        os.remove(old_version_path)
    return path


SAVE_DRIVER_FILE_PATH = _driver_file_path()

UVC_DIR_PATH = LOCAL_PATH + "/driver/uvc/"
UVC_LEN_QUERY_PATH = UVC_DIR_PATH + "len_query"
UVC_GET_QUERY_PATH = UVC_DIR_PATH + "get_query"
UVC_SET_QUERY_PATH = UVC_DIR_PATH + "set_query"

SYSTEMD_NAME = "linux-enable-ir-emitter.service"
SYSTEMD_PATH = "/usr/lib/systemd/system/" + SYSTEMD_NAME
UDEV_RULE_NAME = "99-linux-enable-ir-emitter.rules"
UDEV_RULE_PATH = "/etc/udev/rules.d/" + UDEV_RULE_NAME

EDITOR_PATH = os.environ["EDITOR"] if "EDITOR" in os.environ else "/usr/bin/nano"


def get_vid(device_number):
    command = "find /sys/class/video4linux/video" + \
        device_number + "/device/ -name vendor -exec cat {} +"
    return subprocess.check_output(command, shell=True).strip().decode("utf-8")


def get_pid(device_number):
    command = "find /sys/class/video4linux/video" + \
        device_number + "/device/ -name product -exec cat {} +"
    return subprocess.check_output(command, shell=True).strip().decode("utf-8")
