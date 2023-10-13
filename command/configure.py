from typing import NoReturn

import logging
import subprocess

from command import boot
from globals import (
    BIN_GENERATE_DRIVER_PATH,
    SAVE_DRIVER_FOLDER_PATH,
    find_grayscale_camera,
    ExitCode,
)


def configure(
    device: str | None, manual: bool, emitters: int, neg_answer_limit: int
) -> NoReturn:
    """Find a driver for the infrared camera and exit.

    Args:
        device (str | None): path to the infrared camera, None for automatic detection
        manual (bool): true for enabling the manual configuration
        emitters (int): number of emitters on the device
        neg_answer_limit (int): number of negative answer before the pattern is skiped. Use -1 for unlimited
    """
    logging.info(
        "Stand in front of and close to the camera and make sure the room is well lit."
    )
    logging.info("Ensure to not use the camera during the execution.")
    logging.info("Warning to do not kill the process !")

    if device is None:
        device = find_grayscale_camera()

    logging.info(f"Configuring the camera: {device}.")
    log_level = int(logging.getLogger().level == logging.DEBUG)
    exit_code = subprocess.call(
        [
            BIN_GENERATE_DRIVER_PATH,
            device,
            str(emitters),
            str(neg_answer_limit),
            SAVE_DRIVER_FOLDER_PATH,
            str(log_level),
            str(int(manual)),
        ]
    )

    if exit_code != ExitCode.SUCCESS:
        logging.error("The configuration has failed.")
        logging.error("Please retry in manual mode by adding the '-m' option.")
        logging.info("Do not hesitate to visit the GitHub !")
        logging.info(
            "https://github.com/EmixamPP/linux-enable-ir-emitter/blob/master/docs/README.md"
        )
    else:
        logging.info("The driver has been successfully generated.")
        boot("enable")

    exit(exit_code)
