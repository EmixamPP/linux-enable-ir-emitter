import logging
import sys

from globals import ExitCode, exitIfFileDescriptorError
from DriverGenerator import DriverGenerator, DriverGeneratorError
from DriverSerializer import DriverSerializer


"""DOCUMENTATION
- https://www.kernel.org/doc/html/v5.14/userspace-api/media/drivers/uvcvideo.html
    info 1: uvc queries are explained
    info 2: units can be found by parsing the uvc descriptor
- https://www.mail-archive.com/search?l=linux-uvc-devel@lists.berlios.de&q=subject:%22Re%5C%3A+%5C%5BLinux%5C-uvc%5C-devel%5C%5D+UVC%22&o=newest&f=1
    info 1: selector is on 8 bits and since the manufacturer does not provide a driver, it is impossible to know which value it is.
"""


def execute(device, neg_answer_limit, manual_check):
    """Find a driver for the infrared camera

    Args:
        device (str): the infrared camera '/dev/videoX'
        neg_answer_limit (int): after k negative answer the pattern will be skiped. Use 256 for unlimited
        manual_check (bool): true for manual checking, false for automatic checking
    """

    driver_generator = DriverGenerator(device, neg_answer_limit, manual_check)
    
    logging.info("Warning to do not kill the processus !")
    try:
        driver_generator.generate()
        if driver_generator.driver:
            DriverSerializer.add_driver(driver_generator.driver)
            driver_generator.driver.run()
            logging.info("The configuration is completed with success. To activate the emitter at system boot execute 'linux-enable-ir-emitter boot enable'")
            return

    except DriverGeneratorError as e:
        if e.error_code == DriverGeneratorError.DRIVER_ALREADY_EXIST:
            logging.error("Your infrared camera is already working, skipping the configuration.")
        exitIfFileDescriptorError(e.error_code, device)

    logging.error("The configuration has failed.")
    logging.info("Do not hesitate to open an issue on GitHub ! https://github.com/EmixamPP/linux-enable-ir-emitter")
    sys.exit(ExitCode.FAILURE)
