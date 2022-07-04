import subprocess
import logging
import os
from configparser import ConfigParser
from typing import List

from globals import SYSTEMD_PATH, UDEV_RULE_PATH, SYSTEMD_NAME, get_kernels, get_index


"""DOCUMENTATION
- https://www.freedesktop.org/software/systemd/man/systemd.unit.html
    info 1: systemd service type
    info 2: systemctl exit code
    info 3: systemd service dependencies
- https://stackoverflow.com/questions/47630139/camera-dev-video0-dependencies-in-systemd-service-ubuntu-16-04
    info 1: modprobe uvcvideo command
- https://wiki.archlinux.org/title/udev
    info 1: udev rule run script
"""

class Systemd:
    """Manage the boot service of linux-enable-ir-emitter"""
    def __init__(self, devices: List[str]) -> None:
        """Create a boot service for run the drivers

        Args:
            devices : devices for which a driver will be run
        """
        self.devices = devices

    @staticmethod
    def disable() -> int:
        """Disable the service

        Returns:
            0: the service have been disabled successfully
            other value: The boot service does not exists.
        """
        exit_code = subprocess.run(["systemctl", "disable", SYSTEMD_NAME], capture_output=True).returncode
        if exit_code:
            logging.error("The boot service does not exists.")
        else:
            os.remove(SYSTEMD_PATH)
            os.remove(UDEV_RULE_PATH)

        return exit_code

    def enable(self) -> int:
        """Enable the service

        Returns:
            0: the service have been enabled successfully
            other value: Error with the boot service.
        """
        self._create_udev()
        self._create_systemd()

        exit_code = subprocess.run(["udevadm", "control", "--reload-rules"], capture_output=True).returncode
        exit_code = exit_code + subprocess.run(["udevadm", "trigger"], capture_output=True).returncode
        if exit_code:
            logging.error("Error with the udev boot service.")

        exit_code = subprocess.run(["systemctl", "enable", "--now", SYSTEMD_NAME], capture_output=True).returncode
        if exit_code:
            logging.error("Error with the systemd boot service.")

        return exit_code

    @staticmethod
    def status() -> int:
        """Print the service status
        Returns:
            0: the service works fine
            other value: error with the boot service
        """
        exec = subprocess.run(["systemctl", "status", SYSTEMD_NAME], capture_output=True)

        if exec.returncode == 4:
            logging.error("The boot service does not exists.")
        else:
            print(exec.stdout.decode('utf-8').strip())
        return exec.returncode

    def _create_systemd(self) -> None:
        """Create the service file at SYSTEMD_PATH"""
        systemd = self._initialize_systemd()
        with open(SYSTEMD_PATH, 'w') as service_file:
            systemd.write(service_file)

    def _create_udev(self) -> None:
        """Create the rule file at UDEV_RULE_PATH"""
        with open(UDEV_RULE_PATH, 'w') as rule_file:
            for device in self.devices:
                kernels = get_kernels(device)
                index = get_index(device)

                rule1 = 'ACTION=="add|change", KERNELS==%s, ATTR{index}==%s, RUN+="/usr/bin/linux-enable-ir-emitter run"'%(kernels, index)
                rule2 = 'KERNELS==%s, ATTR{index}==%s, TAG+="systemd"'%(kernels, index) # create systemd .device for all device

                rule_file.write(rule1 + "\n")
                rule_file.write(rule2 + "\n")

    def _initialize_systemd(self) -> ConfigParser:
        """Represent the systemd service file by a ConfigParser.

        Returns:
            ConfigParser: the systemd service 
        """
        service = ConfigParser()
        service.optionxform = str

        service["Unit"] = {}
        service["Unit"]["Description"] = "enable the infrared emitter"
        service["Unit"]["Requires"] = ""
        service["Unit"]["After"] = "multi-user.target suspend.target hybrid-sleep.target hibernate.target suspend-then-hibernate.target"
        for device in self.devices:  # wait for all device
            dev = " dev-{}.device".format(device[5:])
            service["Unit"]["After"] += dev
            service["Unit"]["Requires"] += dev

        service["Service"] = {}
        service["Service"]["Type"] = "oneshot"
        service["Service"]["ExecStartPre"] = "/sbin/modprobe uvcvideo"
        service["Service"]["ExecStart"] = "/usr/bin/linux-enable-ir-emitter run"

        service["Install"] = {}
        service["Install"]["WantedBy"] = "multi-user.target suspend.target hybrid-sleep.target hibernate.target suspend-then-hibernate.target"

        return service
