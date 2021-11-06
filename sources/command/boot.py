import sys
import logging
from typing import NoReturn

from globals import ExitCode
from Systemd import Systemd
from driver.DriverSerializer import DriverSerializer


def execute(boot_status: str) -> NoReturn:
    """Enable or disable the systemd service which activates the ir emitter

        args:
            boot_status: "enable" or "disable" or "status"

        Raises:
            Exception: boot status arg can only be equal to enable, disable or status
    """

    if boot_status == "enable":
        drivers = DriverSerializer.load_saved_drivers()
        if not drivers: sys.exit(ExitCode.FAILURE)
        systemd = Systemd([driver.device for driver in drivers])

        exit_code = systemd.enable()
        if exit_code: sys.exit(ExitCode.FAILURE)
        logging.info("The boot service have been enabled, please reboot.")

    elif boot_status == "disable":
        exit_code = Systemd.disable()
        if exit_code: sys.exit(ExitCode.FAILURE)

    elif boot_status == "status":
        exit_code = Systemd.status()
        if exit_code: sys.exit(ExitCode.FAILURE)

    else:
        raise Exception("boot status arg can only be equal to 'enable', 'disable' or 'status'")
    
    sys.exit(ExitCode.SUCCESS)
