import logging
import sys

from driver.DriverSerializer import DriverSerializer
from globals import ExitCode, exit_if_file_descriptor_error


def execute(trigger=False) -> None:
    """Apply all saved drivers

    Args:
        trigger: and try to trigger the ir emitter. Defaults to False.
    """
    driver_list = DriverSerializer.load_saved_drivers()
    if not driver_list: sys.exit(ExitCode.FAILURE)

    for driver in driver_list:
        exit_code = driver.run() if not trigger else driver.trigger_ir()
        
        exit_if_file_descriptor_error(exit_code, driver.device)
        if exit_code != ExitCode.SUCCESS: 
            logging.critical("Bad driver for %s.", driver.device)
            sys.exit(exit_code)
        
    sys.exit(ExitCode.SUCCESS)
    