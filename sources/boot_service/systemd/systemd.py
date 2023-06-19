import logging
import subprocess

from globals import BOOT_SERVICE_NAME
from boot_service import BaseBootService


class Systemd(BaseBootService):
    def _enable(self) -> int:
        """Enable the service

        Returns:
            0: the service have been enabled successfully
            other value: Error with the boot service.
        """
        exit_code = subprocess.run(
            ["systemctl", "enable", "--now", BOOT_SERVICE_NAME], capture_output=True
        ).returncode

        if exit_code:
            logging.error("Error with the systemd boot service.")

        return exit_code

    def _disable(self) -> int:
        """Disable the service

        Returns:
            0: the service have been disabled successfully
            other value: The boot service does not exists.
        """
        exit_code = subprocess.run(
            ["systemctl", "disable", BOOT_SERVICE_NAME], capture_output=True
        ).returncode

        if exit_code:
            logging.error("The systemd boot service does not exists.")

        return exit_code

    def status(self) -> int:
        """Print the service status
        Returns:
            0: the service works fine
            other value: error with the boot service
        """
        exec = subprocess.run(
            ["systemctl", "status", BOOT_SERVICE_NAME], capture_output=True
        )

        if exec.returncode == 4:
            logging.error("The systemd boot service does not exists.")
        else:
            print(exec.stdout.decode("utf-8").strip())

        return exec.returncode
