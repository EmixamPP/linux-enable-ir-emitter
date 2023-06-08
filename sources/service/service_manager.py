import os
import subprocess

from .base_service_manager import BaseServiceManager
from .openrc.openrc import Openrc
from .systemd.systemd import Systemd


def _get_boot_service_manager():
    try:
        with open("/proc/1/comm", "r") as f:
            comm = f.read().strip()

        if comm == "systemd":
            return "systemd"
        elif comm == "init":
            try:
                subprocess.run(
                    ["rc-status", "--version"],
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                )
                return "openrc"
            except FileNotFoundError:
                pass
    except FileNotFoundError:
        pass

    return "unknown"


boot_service_name = _get_boot_service_manager()


def get_service_manager(devices) -> BaseServiceManager:
    if boot_service_name == "systemd":
        return Systemd(devices)
    elif boot_service_name == "openrc":
        return Openrc(devices)
