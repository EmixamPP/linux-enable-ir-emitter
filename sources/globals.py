import os
import enum
import logging
import sys
import subprocess
import glob
from typing import List

SAVE_DRIVER_FOLDER_PATH = "/etc/linux-enable-ir-emitter/"

LOCAL_PATH = os.path.dirname(os.path.abspath(__file__))
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


def get_drivers_path(device: str = None) -> str:
    """Get the drivers path corresponding to all configured device
    or just to one specific

    Args:
        device: path to the a specific infrared camera
                None to get all drivers path.

    Returns:
        str: path to the driver(s)
    """
    driverName = "*"
    if device is not None:
        driverName = device[device.rfind("/") + 1:] + driverName
    return glob.glob(SAVE_DRIVER_FOLDER_PATH + driverName + ".driver")


def get_devices() -> List[str]:
    """Return all configured devices with the /dev/videoX form"""
    devices_path = []
    for driver in os.listdir(SAVE_DRIVER_FOLDER_PATH):
        path = "/dev/v4l/by-path/" + driver[:driver.rfind("_emitter")]
        devices_path.append(os.path.realpath(path))
    return devices_path


def get_index(device: str) -> str:
    """Get the index of the camera device

    Args:
        device: the infrared camera '/dev/videoX'

    Returns:
        device index (double quoted)
    """
    return subprocess.check_output("udevadm info -a %s | grep -m 1 -oP 'ATTR{index}==\\K.*'" % (device), shell=True).decode("utf-8").strip()


def get_kernels(device: str) -> str:
    """Get the parent kernels of the camera device

    Args:
        device: the infrared camera '/dev/videoX'

    Returns:
        device parent kernels (double quoted)
    """
    return subprocess.check_output("udevadm info -a %s | grep -m 1 -oP 'KERNELS==\\K.*'" % (device), shell=True).decode("utf-8").strip()
