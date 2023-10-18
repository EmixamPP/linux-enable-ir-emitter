from __future__ import annotations
from typing import TYPE_CHECKING, Type

if TYPE_CHECKING:
    from boot_service.systemd import Systemd
    from boot_service.openrc import Openrc

import enum
import importlib
import logging
import os
import re
import subprocess
import sys

SAVE_DRIVER_FOLDER_PATH = "@SAVE_DRIVER_FOLDER_PATH@"
CPP_COMMANDS_LIB_PATH = "@CPP_COMMANDS_LIB_PATH@"

UDEV_RULE_PATH = "@UDEV_RULE_PATH@"
BOOT_SERVICE_NAME = "@BOOT_SERVICE_NAME@"
BOOT_SERVICE_MANAGER = "@BOOT_SERVICE_MANAGER@"


class ExitCode(enum.IntEnum):
    SUCCESS = 0
    FAILURE = 1
    FILE_DESCRIPTOR_ERROR = 126
    ROOT_REQUIRED = 2


def check_root() -> None:
    """Exit if the script isn't run as root."""
    if os.getuid():
        logging.critical("Please run as root.")
        sys.exit(ExitCode.ROOT_REQUIRED)


def get_boot_service_constructor() -> Type[Systemd] | Type[Openrc] | None:
    """Get the installed boot service constructor.

    Returns:
        Systemd | Openrc | None: Systemd or Openrc class constructor.
        None if no boot service is installed.
    """
    try:
        module = importlib.import_module(f"boot_service.{BOOT_SERVICE_MANAGER}")
        return getattr(module, BOOT_SERVICE_MANAGER)
    except:
        return None


def get_devices() -> list[str]:
    """Return all configured devices."""
    devices_path = []
    for driver in os.listdir(SAVE_DRIVER_FOLDER_PATH):
        if re.match(r".*_emitter[0-9]+.driver", driver):
            path = "/dev/v4l/by-path/" + driver[: driver.rfind("_emitter")]
            devices_path.append(path)
    return devices_path


def get_index(device: str) -> str:
    """Get the index of the camera device.

    Args:
        device (str): the infrared camera '/dev/videoX'.

    Returns:
        str: device index (double quoted).
    """
    return subprocess.check_output(
        "udevadm info -a %s | grep -m 1 -oP 'ATTR{index}==\\K.*'" % (device),
        shell=True,
        text=True,
    ).strip()


def get_kernels(device: str) -> str:
    """Get the parent kernels of the camera device.

    Args:
        device (str): the infrared camera '/dev/videoX'.

    Returns:
        str: device parent kernels (double quoted).
    """
    return subprocess.check_output(
        "udevadm info -a %s | grep -m 1 -oP 'KERNELS==\\K.*'" % (device),
        shell=True,
        text=True,
    ).strip()
