import os
import yaml
import sys
import logging

from globals import SAVE_DRIVER_FILE_PATH, ExitCode
from driver.Driver import Driver


class DriverSerializer:
    @staticmethod
    def _deserialize_saved_drivers() -> list[Driver]:
        """Load all drivers saved in globals.SAVE_DRIVER_FILE_PATH
        No error catching

        Returns:
            List of saved driver
        """
        with open(SAVE_DRIVER_FILE_PATH, "r") as save_file:
            return list(yaml.load_all(save_file, Loader=yaml.Loader))

    @staticmethod
    def load_saved_drivers() -> Driver or None:
        """Load all drivers saved in globals.SAVE_DRIVER_FILE_PATH

        Returns:
            List of saved driver
            None if no driver is saved
        """
        try:
            if os.path.exists(SAVE_DRIVER_FILE_PATH):
                dummy_driver = Driver([0], 0, 0, '')
                deserialized = DriverSerializer._deserialize_saved_drivers()
                for driver in deserialized:
                    assert(isinstance(driver, Driver) and dir(dummy_driver) == dir(driver))
                return deserialized
            logging.error("No driver is currently saved.")
        except:
            logging.critical("The driver file is corrupted.")
            logging.info("Execute 'linux-enable-ir-emitter fix driver' to reset the file.")
            sys.exit(ExitCode.FAILURE)
    
    @staticmethod
    def save_drivers(driver_list: list[Driver]) -> None:
        """Save all drivers in globals.SAVE_DRIVER_FILE_PATH

        Args:
            driver_list: drivers to save
        """
        with open(SAVE_DRIVER_FILE_PATH, "w") as save_driver_file:
            save_driver_file.write("#Caution: any manual modification of this file may corrupt the operation of the program! You must therefore be very careful.\n")
            save_driver_file.write("#Please consult https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Manual-configuration before.\n")
            save_driver_file.write("#If you currupt the driver file: execute 'linux-enable-ir-emitter fix driver' to reset the file.\n\n")
            yaml.dump_all(driver_list, save_driver_file)
    
    @staticmethod
    def add_driver(driver: Driver) -> None:
        """Add a driver to file globals.SAVE_DRIVER_FILE_PATH

        Args:
            driver: driver to add
        """
        saved_drivers_list = None
        try:
            saved_drivers_list = DriverSerializer._deserialize_saved_drivers()
            for saved_driver in saved_drivers_list.copy():
                if saved_driver.device == driver.device:
                    saved_drivers_list.remove(saved_driver)
            saved_drivers_list.append(driver)
        except:  # if driver file corrupted or no driver saved: delete all saved drivers
            saved_drivers_list = [driver]
        DriverSerializer.save_drivers(saved_drivers_list)