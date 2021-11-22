import os
import logging
import subprocess
import sys
from typing import NoReturn

from driver.Driver import Driver
from driver.DriverSerializer import DriverSerializer
from globals import SAVE_DRIVER_FILE_PATH, EDITOR_PATH, ExitCode


def execute() -> NoReturn:
    """Display the current driver in the default editor"""
    if not os.path.exists(SAVE_DRIVER_FILE_PATH):
        dummy_driver = Driver([0], 0, 0, "/dev/videoX")
        DriverSerializer.save_drivers([dummy_driver])

    try:
        subprocess.run([EDITOR_PATH, SAVE_DRIVER_FILE_PATH])
    except FileNotFoundError:
        logging.critical("No editor found, set the envion variable 'EDITOR' or install nano.")
        sys.exit(ExitCode.MISSING_DEPENDENCY)

    sys.exit(ExitCode.SUCCESS)
