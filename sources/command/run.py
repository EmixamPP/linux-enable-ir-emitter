import logging
import os
import subprocess
from typing import NoReturn

from globals import ExitCode, SAVE_DRIVER_FOLDER_PATH, EXECUTE_DRIVER_PATH, driver_name


def execute(device: str) -> NoReturn:
    """Apply the driver associated to a device 

    Args:
        device: path to the infrared camera, /dev/videoX
                None to execute all driver.
    """
    drivers = os.listdir(SAVE_DRIVER_FOLDER_PATH)
    if not len(drivers):
        logging.critical("No driver has been configured.")
        exit(ExitCode.FAILURE)

    if device:
        driver = driver_name(device)
        if driver in drivers:
            drivers = [driver]
        else:
            logging.critical("No driver for %s has been configured.", device)
            exit(ExitCode.FAILURE)

    general_exit_code = ExitCode.SUCCESS
    for driver in drivers:
        driver_path = SAVE_DRIVER_FOLDER_PATH + driver
        exit_code = subprocess.call([EXECUTE_DRIVER_PATH, driver_path])
        if exit_code != ExitCode.SUCCESS:
            general_exit_code = exit_code
            logging.error("Impossible to execute the driver at %s.", driver_path)

    exit(general_exit_code)
