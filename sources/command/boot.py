import logging
from typing import NoReturn

from globals import ExitCode, get_devices
from Systemd import Systemd


def execute(boot_status: str) -> NoReturn:
    """Enable or disable the systemd service which 
        activates the ir emitter for all configured device

        args:
            boot_status: "enable" or "disable" or "status"
            device: path to the infrared camera, /dev/videoX
                None to execute all driver.

        Raises:
            Exception: boot status arg can only be equal to 
            enable, disable or status
    """
    assert(boot_status in ["enable", "disable", "status"])

    if boot_status == "enable":
        devices = get_devices()
        if not len(devices):
            logging.critical("No driver have been configured.")
            exit(ExitCode.FAILURE)

        systemd = Systemd(devices)
        if systemd.enable():
            exit(ExitCode.FAILURE)
        logging.info("The boot service have been enabled.")

    elif boot_status == "disable":
        if Systemd.disable():
            exit(ExitCode.FAILURE)
        logging.info("The boot service have been disabled.")

    elif boot_status == "status":
        if Systemd.status():
            exit(ExitCode.FAILURE)

    exit(ExitCode.SUCCESS)
