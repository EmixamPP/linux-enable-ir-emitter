import os

from globals import ExitCode, SAVE_CONFIG_FILE_PATH


def _fix_config():
    """Rest the configuration"""
    os.system("rm -f " + SAVE_CONFIG_FILE_PATH)
    print("The configuration file have been removed.")


def _fix_chicony():
    """Uninstall chicony-ir-toggle"""
    os.system("rm -f /usr/local/bin/chicony-ir-toggle")
    os.system("rm -f /lib/udev/rules.d/99-ir-led.rules")
    os.system("rm -f /lib/systemd/system-sleep/ir-led.sh")
    print("chicony-ir-toggle have been uninstall.")


def execute(target):
    """Fix well know problems

    Args:
        target (string): "config" or "chicony"

    Raises:
        Exception: fix target arg can only be equal to config or chicony
    
    Returns:
        ExitCode: always ExitCode.SUCCESS
    """
    if target in ("config", "chicony"):
        eval("_fix_" + target + "()")
    else:
        raise Exception("fix target arg can only be equal to 'config' or 'chicony'")
    
    return ExitCode.SUCCESS