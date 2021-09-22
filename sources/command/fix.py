import logging
import sys
import os

from globals import SAVE_CONFIG_FILE_PATH, ExitCode


def _fix_config():
    """Reset the configuration

    Returns:
        ExitCode: SUCCESS or FAILURE
    """
    try:
        os.remove(SAVE_CONFIG_FILE_PATH)
        logging.info("The configuration file have been delete.")
        return ExitCode.SUCCESS
    except FileNotFoundError:
        logging.error("No configuration file to delete.")
        return ExitCode.FAILURE


def _fix_chicony():
    """Uninstall chicony-ir-toggle
    
     Returns:
        ExitCode: SUCCESS or FAILURE
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
    


def execute(target):
    """Fix well know problems

    Args:
        target (str): "config" or "chicony"

    Raises:
        Exception: fix target arg can only be equal to config or chicony
    """
    if target in ("config", "chicony"):
        sys.exit(eval("_fix_" + target + "()"))
    raise Exception("fix target arg can only be equal to 'config' or 'chicony'")
