import logging
from typing import NoReturn

from globals import ExitCode, get_devices, get_boot_service_constructor


def boot(boot_status: str) -> NoReturn:
    """Enable or disable the boot service which
    activates the ir emitter for all configured device

    args:
        boot_status: "enable" or "disable" or "status"

    Raises:
        Exception: boot status arg can only be equal to
        enable, disable or status
    """
    assert boot_status in ["enable", "disable", "status"]

    boot_service_constructor = get_boot_service_constructor()
    if boot_service_constructor is None:
        logging.critical("No supported boot service are installed.")
        exit(ExitCode.FAILURE)

    devices = get_devices()
    boot_service = boot_service_constructor(devices)

    if boot_status == "enable":
        if not len(devices):
            logging.critical("No driver have been configured.")
            exit(ExitCode.FAILURE)
        if boot_service.enable():
            exit(ExitCode.FAILURE)
        logging.info("The boot service have been enabled.")

    elif boot_status == "disable":
        if boot_service.disable():
            exit(ExitCode.FAILURE)
        logging.info("The boot service have been disabled.")

    elif boot_status == "status":
        if boot_service.status():
            exit(ExitCode.FAILURE)

    exit(ExitCode.SUCCESS)
