import logging
import subprocess

from globals import BOOT_SERVICE_NAME
from boot_service import BaseBootService


class Openrc(BaseBootService):
    def _enable(self) -> int:
        """Enable the service

        Returns:
            0: the service have been enabled successfully
            other value: Error with the boot service.
        """
        exit_code = subprocess.run(
            ["rc-update", "add", BOOT_SERVICE_NAME, "default"], capture_output=True
        ).returncode
        exit_code += subprocess.run(
            ["rc-service", BOOT_SERVICE_NAME, "start"], capture_output=True
        ).returncode

        if exit_code:
            logging.error("Error with the openrc boot service.")

        return exit_code

    def _disable(self) -> int:
        """Disable the service

        Returns:
            0: the service have been disabled successfully
            other value: The boot service does not exists.
        """
        exit_code = subprocess.run(
            ["rc-update", "del", BOOT_SERVICE_NAME, "default"], capture_output=True
        ).returncode

        if exit_code:
            logging.error("The openrc boot service does not exists.")

        return exit_code

    def status(self) -> int:
        """Print the service status
        Returns:
            0: the service works fine
            other value: error with the boot service
        """
        exec = subprocess.run(
            ["rc-status", "-a"], capture_output=True
        )

        output = exec.stdout.decode("utf-8").strip()
        for line in output.split("\n"):
            if BOOT_SERVICE_NAME in line:
                return 0

        logging.error("The openrc boot service does not exists.")
        return exec.returncode
