import logging
import subprocess
import sys

from globals import SAVE_CONFIG_FILE_PATH, ExitCode


def _fix_config():
    """Rest the configuration"""
    subprocess.run("rm -f " + SAVE_CONFIG_FILE_PATH, shell=True, capture_output=True)
    logging.info("The configuration file have been removed.")


def _fix_chicony():
    """Uninstall chicony-ir-toggle"""
    subprocess.run("rm -f /usr/local/bin/chicony-ir-toggle", shell=True, capture_output=True)
    subprocess.run("rm -f /lib/udev/rules.d/99-ir-led.rules", shell=True, capture_output=True)
    subprocess.run("rm -f /lib/systemd/system-sleep/ir-led.sh", shell=True, capture_output=True)
    logging.info("chicony-ir-toggle have been uninstall.")


def execute(target):
    """Fix well know problems

    Args:
        target (string): "config" or "chicony"

    Raises:
        Exception: fix target arg can only be equal to config or chicony
    """
    if target in ("config", "chicony"):
        eval("_fix_" + target + "()")
    else:
        raise Exception("fix target arg can only be equal to 'config' or 'chicony'")
    
    sys.exit(ExitCode.SUCCESS)
