import os
import enum
import logging
import sys
import subprocess
import glob
import re
import importlib
from typing import List, Any

SAVE_DRIVER_FOLDER_PATH = "@SAVE_DRIVER_FOLDER_PATH@"

BIN_EXECUTE_DRIVER_PATH = "@BIN_EXECUTE_DRIVER_PATH@"
BIN_DRIVER_GENERATOR_PATH = "@BIN_DRIVER_GENERATOR_PATH@"

UDEV_RULE_PATH = "@UDEV_RULE_PATH@"
BOOT_SERVICE_NAME = "@BOOT_SERVICE_NAME@"
BOOT_SERVICE_MANAGER = "@BOOT_SERVICE_MANAGER@"


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


def get_boot_service_constructor() -> Any:
    """Get the installed boot service constructor

    Returns:
        Any: Systemd or Openrc
        None: no boot service installed
    """
    try:
        module = importlib.import_module(
            f"boot_service.{BOOT_SERVICE_MANAGER}")
        return getattr(module, BOOT_SERVICE_MANAGER)
    except Exception as e:
        print(e)
        return None


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
    path = os.path.join(SAVE_DRIVER_FOLDER_PATH, driverName + ".driver")
    return glob.glob(path)


def get_devices() -> List[str]:
    """Return all configured devices"""
    devices_path = []
    for driver in os.listdir(SAVE_DRIVER_FOLDER_PATH):
        if re.match(r".*_emitter[0-9]+.driver", driver):
            path = "/dev/v4l/by-path/" + driver[:driver.rfind("_emitter")]
            devices_path.append(path)
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
