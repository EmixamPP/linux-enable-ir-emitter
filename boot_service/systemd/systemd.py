import logging
import subprocess

from boot_service import BaseBootService
from utils import BOOT_SERVICE_NAME


class Systemd(BaseBootService):
    def _enable(self) -> int:
        exit_code = subprocess.call(
            ["systemctl", "enable", "--now", BOOT_SERVICE_NAME],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

        if exit_code:
            logging.error("Error with the systemd boot service.")

        return exit_code

    def _disable(self) -> int:
        exit_code = subprocess.call(
            ["systemctl", "disable", BOOT_SERVICE_NAME],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

        if exit_code:
            logging.error("The systemd boot service does not exists.")

        return exit_code

    def status(self) -> int:
        exec = subprocess.run(
            ["systemctl", "status", BOOT_SERVICE_NAME],
            capture_output=True,
            text=True,
        )

        if exec.returncode == 4:
            logging.error("The systemd boot service does not exists.")
        else:
            print(exec.stdout.strip())

        return exec.returncode
