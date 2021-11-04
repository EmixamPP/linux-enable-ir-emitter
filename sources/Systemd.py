from configparser import ConfigParser
from globals import SYSTEMD_PATH, UDEV_RULE_PATH, SYSTEMD_NAME
import subprocess
import logging


"""DOCUMENTATION
- https://www.freedesktop.org/software/systemd/man/systemd.unit.html
    info 1: systemd service section
    info 2: exit code
- https://github.com/EmixamPP/linux-enable-ir-emitter/issues/1
    info 1: udev rule
    info 2: systemd service section
- https://stackoverflow.com/questions/47630139/camera-dev-video0-dependencies-in-systemd-service-ubuntu-16-04
    info 1: modprobe uvcvideo command
"""


class Systemd:
    def __init__(self, devices: list[str]) -> None:
        self.devices = devices
        self.service = self._initialize_service_file()
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
        return exit_code

    def enable(self) -> int:
        """Enable the service

        Returns:
            0: the service have been enabled successfully
            other value: Error with the boot service.
        """
        self._create_udev_rule()  
        self._create_service()

        exit_code = subprocess.run(["systemctl", "enable", SYSTEMD_NAME], capture_output=True).returncode
        
        if exit_code:
            logging.error("Error with the boot service.")
        return exit_code
    
    @staticmethod
    def status() -> int:
        """Print the service status

        Returns:
            0: the service works fine
            other value: error with the boot service
        """
        exec = subprocess.run(["systemctl", "status", SYSTEMD_NAME], capture_output=True)

        if exec.returncode == 4:  # 
            logging.error("The boot service does not exists.")
        else:
            print(exec.stdout.strip().decode('utf-8'))
        return exec.returncode
    
        
    def _create_service(self) -> None:
        """Create the service file at SYSTEMD_PATH"""
        with open(SYSTEMD_PATH, 'w') as service_file:
            self.service.write(service_file)

    def _create_udev_rule(self) -> None:
        """Create the rule file at UDEV_RULE_PATH"""
        with open(UDEV_RULE_PATH, 'w') as rule_file:
            for device in self.devices:
                rule = 'KERNEL=="{}", SYMLINK="{}", TAG+="systemd"'.format(device[5:], device[5:])
                rule_file.write(rule + "\n")

    def _add_device_to_service(self) -> None:
        """Add each device to self.service """
        for device in self.devices:
            target = " dev-{}.device".format(device[5:])
            self.service["Unit"]["After"] += target
            self.service["Unit"]["Requires"] += target
    
    @staticmethod
    def _initialize_service_file() -> ConfigParser:
        """Represent the systemd service file by a ConfigParser.
        Initialize the parser with the default values, which does not contain devices rules.

        Returns:
            ConfigParser: the intialized systemd service 
        """
        service = ConfigParser()
        service.optionxform = str

        service["Unit"] = {}
        service["Unit"]["Description"] = "enable the infrared emitter"
        service["Unit"]["Requires"] = "multi-user.target suspend.target hybrid-sleep.target hibernate.target suspend-then-hibernate.target"
        service["Unit"]["After"] = "multi-user.target suspend.target hybrid-sleep.target hibernate.target suspend-then-hibernate.target"

        service["Service"] = {}
        service["Service"]["Type"] = "oneshot"
        service["Service"]["ExecStartPre"] = "/sbin/modprobe uvcvideo"
        service["Service"]["ExecStart"] = "/usr/bin/linux-enable-ir-emitter run"

        service["Install"] = {}
        service["Install"]["WantedBy"] = "multi-user.target suspend.target hybrid-sleep.target hibernate.target suspend-then-hibernate.target"

        return service
