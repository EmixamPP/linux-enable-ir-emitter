import subprocess
import logging
import os
from configparser import ConfigParser
from typing import List

from globals import SYSTEMD_PATH, UDEV_RULE_PATH, SYSTEMD_NAME


"""DOCUMENTATION
- https://www.freedesktop.org/software/systemd/man/systemd.unit.html
    info 1: systemd service type
    info 2: systemctl exit code
    info 3: systemd service dependencies
- https://github.com/EmixamPP/linux-enable-ir-emitter/issues/1
    info 1: systemd wait for /dev/video
    info 2: systemd service dependencies
- https://stackoverflow.com/questions/47630139/camera-dev-video0-dependencies-in-systemd-service-ubuntu-16-04
    info 1: modprobe uvcvideo command
- https://wiki.archlinux.org/title/udev
    info 1: udev rule run script
"""


def get_vid(device: str):
    """Get the vendor id of the camera device

    Args:
        device: the infrared camera '/dev/videoX'

    Returns:
        str: vendor id
    """
    return subprocess.check_output("udevadm info {} | grep -oP 'E: ID_VENDOR_ID=\\K.*'".format(device), shell=True).strip().decode("utf-8")


def get_pid(device: str):
    """Get the product id of the camera device

    Args:
        device: the infrared camera '/dev/videoX'

    Returns:
        str: product id
    """
    return subprocess.check_output("udevadm info {} | grep -oP 'E: ID_MODEL_ID=\\K.*'".format(device), shell=True).strip().decode("utf-8")


class Systemd:
    def __init__(self, devices: List[str]) -> None:
        self.devices = devices
        self.service = self._initialize_systemd_file()
        self._add_device_to_service()

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
        self._create_udev_rule()
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
            print(exec.stdout.strip().decode('utf-8'))
        return exec.returncode

    def _create_systemd(self) -> None:
        """Create the service file at SYSTEMD_PATH"""
        with open(SYSTEMD_PATH, 'w') as service_file:
            self.service.write(service_file)

    def _create_udev_rule(self) -> None:
        """Create the rule file at UDEV_RULE_PATH"""
        with open(UDEV_RULE_PATH, 'w') as rule_file:
            for device in self.devices:
                vid = get_vid(device)
                pid = get_pid(device)

                rule1 = 'KERNEL=="{}", SYMLINK="{}", TAG+="systemd"'.format(device[5:], device[5:])
                rule2 = 'ACTION=="add|change", ATTRS{idVendor}=="%s", ATTRS{idProduct}=="%s", RUN+="/usr/bin/linux-enable-ir-emitter run"' %(vid, pid)

                rule_file.write(rule1 + "\n")
                rule_file.write(rule2 + "\n")

    def _add_device_to_service(self) -> None:
        """Add each device to self.service """
        for device in self.devices:
            target = " dev-{}.device".format(device[5:])
            self.service["Unit"]["After"] += target
            self.service["Unit"]["Requires"] += target

    @staticmethod
    def _initialize_systemd_file() -> ConfigParser:
        """Represent the systemd service file by a ConfigParser.
        Initialize the parser with the default values, which does not contain devices rules.

        Returns:
            ConfigParser: the intialized systemd service 
        """
        service = ConfigParser()
        service.optionxform = str

        service["Unit"] = {}
        service["Unit"]["Description"] = "enable the infrared emitter"
        service["Unit"]["Requires"] = ""
        service["Unit"]["After"] = "multi-user.target suspend.target hybrid-sleep.target hibernate.target suspend-then-hibernate.target"

        service["Service"] = {}
        service["Service"]["Type"] = "oneshot"
        service["Service"]["ExecStartPre"] = "/sbin/modprobe uvcvideo"
        service["Service"]["ExecStart"] = "/usr/bin/linux-enable-ir-emitter run"

        service["Install"] = {}
        service["Install"]["WantedBy"] = "multi-user.target suspend.target hybrid-sleep.target hibernate.target suspend-then-hibernate.target"

        return service
