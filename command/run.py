from typing import NoReturn

import logging
import subprocess

from globals import BIN_EXECUTE_DRIVER_PATH, ExitCode, get_drivers_path


def run(device: str | None) -> NoReturn:
    """Apply the driver associated to a device and exit.

    Args:
        device (str | None): path to the infrared camera. None to execute all driver.
    """

    paths = get_drivers_path(device)

    if len(paths) == 0:
        logging.critical("No driver for %s has been configured.", device)
        exit(ExitCode.FAILURE)

    general_exit_code = ExitCode.SUCCESS
    for driver in paths:
        exit_code = subprocess.call([BIN_EXECUTE_DRIVER_PATH, driver])
        if exit_code != ExitCode.SUCCESS:
            general_exit_code = exit_code
            logging.error("Impossible to execute the driver at %s.", driver)

    exit(general_exit_code)
