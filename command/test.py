from typing import NoReturn

import subprocess
import logging

from globals import (
    BIN_IS_GRAY_CAMERA_PATH,
    BIN_IS_EMITTER_WORKING_PATH,
    BIN_VIDEO_FEEDBACK_PATH,
    find_grayscale_camera,
    ExitCode,
)


def test(device: str | None) -> NoReturn:
    """Test if the camera is in grayscale and if the emitter is working.
    Also display a video feedback, then exit.

    Args:
        device (str | None): path to the camera, None for automatic detection
    """
    if device is None:
        device = find_grayscale_camera()

    exit_code = subprocess.call(
        [BIN_IS_GRAY_CAMERA_PATH, device],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR:
        exit(exit_code)
    elif exit_code == ExitCode.SUCCESS:
        logging.info(
            "The camera is in gray scale. This is probably your infrared camera."
        )
    else:
        logging.error(
            "The camera is not in gray scale. This is probably your regular camera."
        )

    exit_code = subprocess.call(
        [BIN_IS_EMITTER_WORKING_PATH, device],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR:
        exit(exit_code)
    elif exit_code == ExitCode.SUCCESS:
        logging.info("The infrared emitter is working.")
    else:
        logging.error("The infrared emitter is not working.")

    exit_code = subprocess.call(
        [BIN_VIDEO_FEEDBACK_PATH, device],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR:
        exit(exit_code)

    exit(ExitCode.SUCCESS)
