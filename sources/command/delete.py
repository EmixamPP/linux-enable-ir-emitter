import os
import logging

from typing import NoReturn
from globals import ExitCode, get_drivers_path


def execute(device: str) -> NoReturn:
    """Remove the driver associated to a device, 
    without causing error if the driver does not exists

    Args:
        device: path to the infrared camera
                None to execute all driver.
    """
    try:
        for driver in get_drivers_path(device):
            os.remove(driver)
    except FileNotFoundError:
        pass # no driver for this device, but there is no need to send error message

    logging.info("The drivers have been deleted.")
    exit(ExitCode.SUCCESS)
