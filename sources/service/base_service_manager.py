from abc import ABCMeta, abstractmethod
from typing import List

from globals import UDEV_RULE_PATH, get_index, get_kernels

"""DOCUMENTATION
- Boot service manager control
    Systemd
    Openrc
- https://stackoverflow.com/questions/47630139/camera-dev-video0-dependencies-in-systemd-service-ubuntu-16-04
    info 1: modprobe uvcvideo command
- https://wiki.archlinux.org/title/udev
    info 1: udev rule run script
"""


class BaseServiceManager(metaclass=ABCMeta):
    """Manage the boot service of linux-enable-ir-emitter"""

    def __init__(self, devices: List[str]) -> None:
        """Create a boot service for run the drivers

        Args:
            devices : devices for which a driver will be run
        """
        self.devices = devices

    @abstractmethod
    @staticmethod
    def _enable() -> int:
        """Enable the service

        Returns:
            0: the service have been enabled successfully
            other value: Error with the boot service.
        """

    @abstractmethod
    @staticmethod
    def _disable() -> int:
        """Disable the service

        Returns:
            0: the service have been disabled successfully
            other value: The boot service does not exists.
        """

    @abstractmethod
    @staticmethod
    def status() -> int:
        """Print the service status
        Returns:
            0: the service works fine
            other value: error with the boot service
        """

    def enable(self) -> int:
        self._create_udev(self.devices)

        exit_code = subprocess.run(
            ["udevadm", "control", "--reload-rules"], capture_output=True
        ).returncode
        exit_code = (
            exit_code
            + subprocess.run(["udevadm", "trigger"], capture_output=True).returncode
        )
        if exit_code:
            logging.error("Error with the udev boot service.")

        exit_code = self._enable()
        return exit_code

    @staticmethod
    def disable() -> int:
        exit_code = BaseServiceManager._disable()
        if exit_code:
            logging.error("The boot service does not exists.")
        else:
            os.remove(UDEV_RULE_PATH)

    @staticmethod
    def _create_udev(devices) -> None:
        """Create the rule file at UDEV_RULE_PATH"""
        with open(UDEV_RULE_PATH, "w") as rule_file:
            for device in devices:
                kernels = get_kernels(device)
                index = get_index(device)

                rule = (
                    'ACTION=="add|change", KERNELS==%s, ATTR{index}==%s, RUN+="/usr/bin/linux-enable-ir-emitter run"\n'
                    % (kernels, index)
                )
                rule_file.write(rule)
