import logging
import subprocess

from boot_service import BaseBootService
from utils. import BOOT_SERVICE_NAME


class Openrc(BaseBootService):
    def _enable(self) -> int:
        exit_code = subprocess.call(
            ["rc-update", "add", BOOT_SERVICE_NAME, "default"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        exit_code += subprocess.call(
            ["rc-service", BOOT_SERVICE_NAME, "start"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

        if exit_code:
            logging.error("Error with the openrc boot service.")

        return exit_code

    def _disable(self) -> int:
        exit_code = subprocess.call(
            ["rc-update", "del", BOOT_SERVICE_NAME, "default"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

        if exit_code:
            logging.error("The openrc boot service does not exists.")

        return exit_code

    def status(self) -> int:
        exec = subprocess.run(
            ["rc-status", "-a"],
            capture_output=True,
            text=True,
        )

        for line in exec.stdout.split("\n"):
            if BOOT_SERVICE_NAME in line:
                return 0

        logging.error("The openrc boot service does not exists.")
        return exec.returncode
