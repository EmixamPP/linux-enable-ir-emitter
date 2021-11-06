import logging
import sys
import os
from typing import NoReturn

from globals import SAVE_DRIVER_FILE_PATH, ExitCode


def _fix_driver() -> ExitCode:
    """Reset the driver

    Returns:
        ExitCode.SUCCESS or ExitCode.FAILURE
    """
    try:
        os.remove(SAVE_DRIVER_FILE_PATH)
        logging.info("The driver file have been delete.")
        return ExitCode.SUCCESS
    except FileNotFoundError:
        logging.error("No driver file to delete.")
        return ExitCode.FAILURE


def _fix_chicony() -> ExitCode:
    """Uninstall chicony-ir-toggle
    
     Returns:
        ExitCode.SUCCESS or ExitCode.FAILURE
    """
    try:
        os.remove("/usr/local/bin/chicony-ir-toggle")
        os.remove("/lib/udev/rules.d/99-ir-led.rules")
        os.remove("/lib/systemd/system-sleep/ir-led.s")
        logging.info("chicony-ir-toggle have been uninstall.")
        return ExitCode.SUCCESS
    except FileNotFoundError:
        logging.error("chicony-ir-toggle is not installed.")
        return ExitCode.FAILURE
    


def execute(target: str) -> NoReturn:
    """Fix well know problems

    Args:
        target: "driver" or "chicony"

    Raises:
        Exception: fix target arg can only be equal to driver or chicony
    """
    if target in ("driver", "chicony"):
        sys.exit(eval("_fix_" + target + "()"))
    raise Exception("fix target arg can only be equal to 'driver' or 'chicony'")
