import logging
import os
import subprocess
from abc import ABCMeta, abstractmethod

from globals import UDEV_RULE_PATH, get_index, get_kernels


class BaseBootService(metaclass=ABCMeta):
    """Manage the boot service of linux-enable-ir-emitter"""

    def __init__(self, devices: list[str]) -> None:
        """Create a boot service for run the drivers.

        Args:
            devices (list[str]): devices for which a driver will be run.
        """
        self.devices: list[str] = devices

    @abstractmethod
    def _enable(self) -> int:
        """Enable the service.

        Returns:
            int: 0 if the service have been enabled successfully.
            Otherwise, error with the boot service.
        """

    @abstractmethod
    def _disable(self) -> int:
        """Disable the service.

        Returns:
            int: 0 if the service have been disabled successfully.
            Otherwise, error with the boot service.
        """

    @abstractmethod
    def status(self) -> int:
        """Print the service status

        Returns:
            int: 0 if the service works fine.
            Otherwise, error with the boot service.
        """

    def enable(self) -> int:
        """Enable the service.

        Returns:
            int: 0 if the service have been enabled successfully.
            Otherwise, error with the boot service.
        """
        self._create_udev()

        exit_code = subprocess.call(
            ["udevadm", "control", "--reload-rules"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        exit_code += subprocess.call(
            ["udevadm", "trigger"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

        if exit_code:
            logging.error("Error with the udev boot service.")

        exit_code += self._enable()
        return exit_code

    def disable(self) -> int:
        """Disable the service.

        Returns:
            int: 0 if the service have been disabled successfully.
            Otherwise, error with the boot service.
        """
        try:
            os.remove(UDEV_RULE_PATH)
            exit_code = 0
        except:
            logging.error("The udev boot service does not exists.")
            exit_code = 1

        exit_code += self._disable()
        return exit_code

    def _create_udev(self) -> None:
        """Create the rule file at UDEV_RULE_PATH"""
        with open(UDEV_RULE_PATH, "w") as rule_file:
            for device in self.devices:
                kernels = get_kernels(device)
                index = get_index(device)

                rule = (
                    'ACTION=="add|change", KERNELS==%s, ATTR{index}==%s, RUN+="/usr/bin/linux-enable-ir-emitter run"\n'
                    % (kernels, index)
                )
                rule_file.write(rule)
