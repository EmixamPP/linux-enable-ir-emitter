import os
import enum
import logging
import sys
import re

SAVE_DRIVER_FOLDER_PATH = "/etc/linux-enable-ir-emitter/"

LOCAL_PATH = path = os.path.dirname(os.path.abspath(__file__))
UVC_DIR_PATH = LOCAL_PATH + "/driver/"
EXECUTE_DRIVER_PATH = UVC_DIR_PATH + "execute-driver"
DRIVER_GENERATOR_PATH = UVC_DIR_PATH + "driver-generator"

SYSTEMD_NAME = "linux-enable-ir-emitter.service"
SYSTEMD_PATH = "/usr/lib/systemd/system/" + SYSTEMD_NAME
UDEV_RULE_NAME = "99-linux-enable-ir-emitter.rules"
UDEV_RULE_PATH = "/etc/udev/rules.d/" + UDEV_RULE_NAME


class ExitCode(enum.IntEnum):
    SUCCESS = 0
    FAILURE = 1
    FILE_DESCRIPTOR_ERROR = 126
    ROOT_REQUIRED = 2


def check_root() -> None:
    """Exit if the script isn't run as root"""
    if os.getuid():
        logging.critical("Please run as root.")
        sys.exit(ExitCode.ROOT_REQUIRED)


def driver_name(device: str) -> str:
    """Get the theoretical name for a device 
    The function do not check if the driver exists

    Args:
        device (str): path to the infrared camera, /dev/videoX

    Returns:
        str: driver name, videoX
    """
    assert(re.fullmatch("/dev/video[0-9]+", device))
    return device[device.rindex("v"):]


def device_name(driver: str) -> str:
    """Get the theoretical name for a driver 
    The function do not check if the driver exists

    Args:
        driver (str): driver name, videoX

    Returns:
        str: device name, /dev/videoX
    """
    assert(re.fullmatch("video[0-9]+", driver))
    return "/dev/" + driver