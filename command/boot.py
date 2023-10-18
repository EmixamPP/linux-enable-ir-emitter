import logging

from utils import ExitCode, get_boot_service_constructor, get_devices


def boot(boot_status: str) -> ExitCode:
    """Enable or disable the boot service which
    activates the ir emitter for all configured device,
    and exit.

    args:
        boot_status (str): "enable" or "disable" or "status".

    Raises:
        Exception (AssertionError): boot_status not in the proposition.

    Returns:
        int: exit code.
    """
    assert boot_status in ["enable", "disable", "status"]

    boot_service_constructor = get_boot_service_constructor()
    if boot_service_constructor is None:
        logging.critical("No supported boot service are installed.")
        return ExitCode.FAILURE

    devices = get_devices()
    boot_service = boot_service_constructor(devices)

    if boot_status == "enable":
        if not len(devices):
            logging.critical("No driver have been configured.")
            return ExitCode.FAILURE
        if boot_service.enable():
            return ExitCode.FAILURE
        logging.info("The boot service have been enabled.")

    elif boot_status == "disable":
        if boot_service.disable():
            return ExitCode.FAILURE
        logging.info("The boot service have been disabled.")

    elif boot_status == "status":
        if boot_service.status():
            return ExitCode.FAILURE

    return ExitCode.SUCCESS
