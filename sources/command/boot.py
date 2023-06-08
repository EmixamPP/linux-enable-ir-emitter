import logging
from typing import NoReturn

from globals import ExitCode, get_devices
from openrc import Openrc
from systemd import Systemd

from ..service.service_manager import get_service_manager


def boot(boot_status: str) -> NoReturn:
    """Enable or disable the systemd service which
    activates the ir emitter for all configured device

    args:
        boot_status: "enable" or "disable" or "status"

    Raises:
        Exception: boot status arg can only be equal to
        enable, disable or status
    """
    assert boot_status in ["enable", "disable", "status"]

    devices = get_devices()
    service_manager = get_service_manager(devices)
    if service_manager is None:
        exit(ExitCode.FAILURE)

    if boot_status == "enable":
        if not len(devices):
            logging.critical("No driver have been configured.")
            exit(ExitCode.FAILURE)
        if service_manager.enable():
            exit(ExitCode.FAILURE)
        logging.info("The boot service have been enabled.")

    elif boot_status == "disable":
        if service_manager.disable():
            exit(ExitCode.FAILURE)
        logging.info("The boot service have been disabled.")

    elif boot_status == "status":
        if service_manager.status():
            exit(ExitCode.FAILURE)

    exit(ExitCode.SUCCESS)
