import logging
import sys
from typing import NoReturn

from globals import ExitCode, exit_if_file_descriptor_error
from command import boot
from driver.DriverGenerator import DriverGenerator, DriverGeneratorError
from driver.DriverSerializer import DriverSerializer


def execute(device: str, neg_answer_limit: int) -> NoReturn:
    """Find a driver for the infrared camera

    Args:
        device: the infrared camera '/dev/videoX'
        neg_answer_limit: after k negative answer the pattern will be skiped. Use 256 for unlimited
    """

    driver_generator = DriverGenerator(device, neg_answer_limit)
    
    logging.info("Warning to do not kill the processes !")
    try:
        driver_generator.generate()
        if driver_generator.driver:
            DriverSerializer.add_driver(driver_generator.driver)
            driver_generator.driver.run()
            logging.info("The configuration is completed with success.")
            boot.execute("enable")

    except DriverGeneratorError as e:
        if e.error_code == DriverGeneratorError.DRIVER_ALREADY_EXIST:
            logging.error("Your infrared camera is already working, skipping the configuration.")
        exit_if_file_descriptor_error(e.error_code, device)

    logging.error("The configuration has failed.")
    logging.info("Do not hesitate to open an issue on GitHub ! https://github.com/EmixamPP/linux-enable-ir-emitter/wiki")
    sys.exit(ExitCode.FAILURE)
