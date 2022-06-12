import logging
import subprocess
from typing import NoReturn

from globals import ExitCode, EXECUTE_DRIVER_PATH, get_driver_path, get_drivers_path


def execute(device: str) -> NoReturn:
    """Apply the driver associated to a device 

    Args:
        device: path to the infrared camera, /dev/videoX
                None to execute all driver.
    """
    drivers = get_drivers_path()
    if not len(drivers):
        logging.critical("No driver has been configured.")
        exit(ExitCode.FAILURE)

    if device:  # only keep the specified device
        driver = get_driver_path(device)
        if driver in drivers:
            drivers = [driver]
        else:
            logging.critical("No driver for %s has been configured.", device)
            exit(ExitCode.FAILURE)

    general_exit_code = ExitCode.SUCCESS
    for driver in drivers:
        exit_code = subprocess.call([EXECUTE_DRIVER_PATH, driver])
        if exit_code != ExitCode.SUCCESS:
            general_exit_code = exit_code
            logging.error("Impossible to execute the driver at %s.", driver)

    exit(general_exit_code)
