import os
import logging

from typing import NoReturn
from globals import ExitCode, get_drivers_path, get_driver_path


def execute(device: str) -> NoReturn:
    """Remove the driver associated to a device, 
    without causing error if the driver does not exists

    Args:
        device: path to the infrared camera, /dev/videoX
                None to execute all driver.
    """
    if device:
        drivers = [get_driver_path(device)]
    else:
        drivers = get_drivers_path()

    try:
        for driver in drivers:
            os.remove(driver)
    except FileNotFoundError:
        pass # no driver for this device, but there is no need to send error message

    logging.info("The drivers have been deleted.")
    exit(ExitCode.SUCCESS)
