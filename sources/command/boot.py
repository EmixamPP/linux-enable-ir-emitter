import subprocess
import sys

from globals import SYSTEMD_NAME, ExitCode


def execute(boot_status):
    """Enable or disable the systemd service which activates the ir emitter

        args:
            boot_status (string): "enable" or "disable" or "status"

        Raises:
            Exception: boot status arg can only be equal to enable, disable or status
    """
    if boot_status in ("enable", "disable", "status"):
        subprocess.call("systemctl {} --now {}".format(boot_status, SYSTEMD_NAME), shell=True)
    else:
        raise Exception("boot status arg can only be equal to 'enable', 'disable' or 'status'")
    
    sys.exit(ExitCode.SUCCESS)
