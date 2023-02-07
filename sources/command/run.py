import logging
import subprocess
from typing import NoReturn

from globals import ExitCode, EXECUTE_DRIVER_PATH, get_drivers_path


def execute(device: str) -> NoReturn:
    """Apply the driver associated to a device 

    Args:
        device: path to the infrared camera
                None to execute all driver.
    """
    paths = get_drivers_path(device)

    if len(paths) == 0:
        logging.critical("No driver for %s has been configured.", device)
        exit(ExitCode.FAILURE)            

    general_exit_code = ExitCode.SUCCESS
    for driver in paths:
        exit_code = subprocess.call([EXECUTE_DRIVER_PATH, driver])
        if exit_code != ExitCode.SUCCESS:
            general_exit_code = exit_code
            logging.error("Impossible to execute the driver at %s.", driver)

    exit(general_exit_code)
