import os

from globals import ExitCode, SYSTEMD_NAME


def execute(boot_status):
    """Enable or disable the systemd service which activates the ir emitter

        args:
            boot_status (string): "enable" or "disable" or "status"

        Raises:
            Exception: boot status arg can only be equal to enable, disable or status
        
        Returns:
            ExitCode: always ExitCode.SUCCESS
    """
    if boot_status in ("enable", "disable", "status"):
        os.system("systemctl {} --now {}".format(boot_status, SYSTEMD_NAME))
    else:
        raise Exception("boot status arg can only be equal to 'enable', 'disable' or 'status'")
    
    return ExitCode.SUCCESS
