import logging
import subprocess

from globals import BOOT_SERVICE_NAME
from boot_service import BaseBootService

class Openrc(BaseBootService):
    @staticmethod
    def _enable() -> int:
        """Enable the service

        Returns:
            0: the service have been enabled successfully
            other value: Error with the boot service.
        """
        exit_code = subprocess.run(
            ["rc-update", "add", BOOT_SERVICE_NAME, "default"], capture_output=True
        ).returncode
        exit_code = (
            exit_code
            + subprocess.run(
                ["rc-service", BOOT_SERVICE_NAME, "start"], capture_output=True
            ).returncode
        )
        if exit_code:
            logging.error("Error with the openrc boot service.")

        return exit_code

    @staticmethod
    def _disable() -> int:
        """Disable the service

        Returns:
            0: the service have been disabled successfully
            other value: The boot service does not exists.
        """
        return subprocess.run(
            ["rc-update", "del", BOOT_SERVICE_NAME, "default"], capture_output=True
        ).returncode

    @staticmethod
    def status() -> int:
        """Print the service status
        Returns:
            0: the service works fine
            other value: error with the boot service
        """

        service_name = "linux-enable-ir-emitter"
        exec= subprocess.run(["rc-status", "-a"], stdout=subprocess.PIPE, text=True)
        output = exec.stdout

        for line in output.split("\n"):
            if service_name in line:
                return 0

        logging.error("The boot service does not exists.")
        return exec.returncode
