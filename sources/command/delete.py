import os
import logging

from typing import NoReturn
from globals import SAVE_DRIVER_FOLDER_PATH, ExitCode, driver_name


def execute(device: str) -> NoReturn:
    """Remove the driver associated to a device, 
    without causing error if the driver deos not exists

    Args:
        device: path to the infrared camera, /dev/videoX
                None to execute all driver.
    """
    if device:
        drivers = [driver_name(device)]
    else:
        drivers = os.listdir(SAVE_DRIVER_FOLDER_PATH)

    try:
        for driver in drivers:
            os.remove(SAVE_DRIVER_FOLDER_PATH + driver)
    except FileNotFoundError:
        pass # there exist no driver for device, but there is no need to send error message

    logging.info("The drivers have been deleted.")
    exit(ExitCode.SUCCESS)
