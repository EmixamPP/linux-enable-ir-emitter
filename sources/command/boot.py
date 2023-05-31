import logging
from typing import NoReturn

from globals import ExitCode, get_devices
from systemd import Systemd
from openrc import Openrc


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
    systemd_mode = (
        subprocess.run(["which", "systemctl"], capture_output=True).returncode == 0
    )

    if boot_status == "enable":
        devices = get_devices()
        if not len(devices):
            logging.critical("No driver have been configured.")
            exit(ExitCode.FAILURE)

        if systemd_mode and Systemd(devices).enabled():
            exit(ExitCode.FAILURE)
        elif Openrc(devices).enabled():
            exit(ExitCode.FAILURE)
        logging.info("The boot service have been enabled.")

    elif boot_status == "disable":
        if systemd_mode and Systemd.disable():
            exit(ExitCode.FAILURE)
        elif Openrc.disable():
            exit(ExitCode.FAILURE)
        logging.info("The boot service have been disabled.")

    elif boot_status == "status":
        if systemd_mode and Systemd.status():
            exit(ExitCode.FAILURE)
        elif Openrc.status():
            exit(ExitCode.FAILURE)

    exit(ExitCode.SUCCESS)
