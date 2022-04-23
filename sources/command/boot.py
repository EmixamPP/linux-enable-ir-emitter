import sys
import logging
from typing import NoReturn
import os

from globals import ExitCode, SAVE_DRIVER_FOLDER_PATH, device_name
from Systemd import Systemd


def execute(boot_status: str) -> NoReturn:
    """Enable or disable the systemd service which activates the ir emitter

        args:
            boot_status: "enable" or "disable" or "status"
python-opencv python-yaml
        Raises:
            Exception: boot status arg can only be equal to enable, disable or status
    """
    assert(boot_status in ["enable", "disable", "status"])

    if boot_status == "enable":
        devices = [device_name(driver) for driver in os.listdir(SAVE_DRIVER_FOLDER_PATH)]
        if not len(devices):
            logging.critical("No driver have been configured.")
            sys.exit(ExitCode.FAILURE)

        systemd = Systemd(devices)
        exit_code = systemd.enable()
        if exit_code: sys.exit(ExitCode.FAILURE)
        logging.info("The boot service have been enabled, please reboot.")

    elif boot_status == "disable":
        exit_code = Systemd.disable()
        if exit_code: sys.exit(ExitCode.FAILURE)

    elif boot_status == "status":
        exit_code = Systemd.status()
        if exit_code: sys.exit(ExitCode.FAILURE)
    
    sys.exit(ExitCode.SUCCESS)
