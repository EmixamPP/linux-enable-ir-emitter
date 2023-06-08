import logging
import os
import subprocess
from typing import List

from globals import SYSTEMD_NAME

from ..base_service_manager import BaseServiceManager

"""DOCUMENTATION
- https://www.freedesktop.org/software/systemd/man/systemd.unit.html
    info 1: systemd service type
    info 2: systemctl exit code
    info 3: systemd service dependencies
"""


class Systemd(BaseServiceManager):
    @staticmethod
    def _enable() -> int:
        """Enable the service

        Returns:
            0: the service have been enabled successfully
            other value: Error with the boot service.
        """
        exit_code = subprocess.run(
            ["systemctl", "enable", "--now", SYSTEMD_NAME], capture_output=True
        ).returncode
        if exit_code:
            logging.error("Error with the systemd boot service.")

        return exit_code

    @staticmethod
    def _disable() -> int:
        """Disable the service

        Returns:
            0: the service have been disabled successfully
            other value: The boot service does not exists.
        """
        return subprocess.run(
            ["systemctl", "disable", SYSTEMD_NAME], capture_output=True
        ).returncode

    @staticmethod
    def status() -> int:
        """Print the service status
        Returns:
            0: the service works fine
            other value: error with the boot service
        """
        exec = subprocess.run(
            ["systemctl", "status", SYSTEMD_NAME], capture_output=True
        )

        if exec.returncode == 4:
            logging.error("The boot service does not exists.")
        else:
            print(exec.stdout.decode("utf-8").strip())
        return exec.returncode
