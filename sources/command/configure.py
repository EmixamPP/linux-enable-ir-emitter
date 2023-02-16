import logging
from typing import NoReturn
import subprocess

from globals import ExitCode, BIN_DRIVER_GENERATOR_PATH, SAVE_DRIVER_FOLDER_PATH, BIN_ENV
from command import boot


def configure(device: str, emitters: int, neg_answer_limit: int) -> NoReturn:
    """Find a driver for the infrared camera

    Args:
        device: path to the infrared camera
        emitters: number of emitters on the device
        neg_answer_limit: after k negative answer the pattern will be skiped. Use 256 for unlimited
    """
    logging.info("Ensure to not use the camera during the execution.")
    logging.info("Warning to do not kill the process !")

    log_level = int(logging.getLogger().level == logging.DEBUG)
    exit_code = subprocess.call([BIN_DRIVER_GENERATOR_PATH, device, str(emitters), str(neg_answer_limit), SAVE_DRIVER_FOLDER_PATH, str(log_level)], env=BIN_ENV)

    if exit_code != ExitCode.SUCCESS:
        logging.error("The configuration has failed.")
        logging.info("Do not hesitate to visit the GitHub ! https://github.com/EmixamPP/linux-enable-ir-emitter/wiki")
    else:
        logging.info("The driver has been successfully generated.")
        boot("enable")

    exit(exit_code)
