import time
import subprocess
import cv2
import logging
import sys

from globals import UVC_LEN_QUERY_PATH, UVC_SET_QUERY_PATH, UVC_GET_QUERY_PATH, ExitCode, SAVE_CONFIG_FILE_PATH
from IrConfiguration import IrConfiguration


"""DOCUMENTATION
- https://www.kernel.org/doc/html/v5.14/userspace-api/media/drivers/uvcvideo.html
    info 1: All uvc queries are explained
    info 2: All possible units can be found by parsing the uvc descriptor
- https://www.mail-archive.com/search?l=linux-uvc-devel@lists.berlios.de&q=subject:%22Re%5C%3A+%5C%5BLinux%5C-uvc%5C-devel%5C%5D+UVC%22&o=newest&f=1
    info 1: There are 8 bits possible selector value and since the manufacturer does not provide a driver, it is impossible to know which one it is.
"""


def execute(device):
    if _isAllReadyConfigured(device):
        logging.error("Your infrared camera is already working, skipping the configuration.")
        sys.exit(ExitCode.FAILURE)

    logging.info("Warning to do not kill the processus !")
    for unit in _getUnitList(device):
        for selector in range(0, 256):
            # get the control instruction lenght
            control_size, exit_code = _getControlSize(device, unit, str(selector))
            if exit_code == ExitCode.FAILURE:
                continue
            _exitIfFileDescriptorError(exit_code, device)

            # get the current control instruction value
            current_control, exit_code = _getCurrentControl(device, unit, str(selector), control_size)
            if exit_code == ExitCode.FAILURE:
                continue
            _exitIfFileDescriptorError(exit_code, device)
            
            # get the max control instruction value
            max_control, exit_code = _getMaxControl(device, unit, str(selector), control_size)
            if exit_code == ExitCode.FAILURE or current_control == max_control:
                # or: cause maxControl isn't a possible instruction for enable the ir emitter
                continue
            _exitIfFileDescriptorError(exit_code, device)


            # try the max control instruction value
            ir_config = IrConfiguration(max_control, unit, selector, device)
            exit_code = ir_config.triggerIr()
            if exit_code == ExitCode.FAILURE:
                continue
            _exitIfFileDescriptorError(exit_code, device)

            logging.debug("unit: {}, selector: {}, curr control: {}, max control: {}".format(unit, selector, current_control, max_control))

            if _emitterIsWorking():
                ir_config.save(SAVE_CONFIG_FILE_PATH)
                logging.info("The configuration is completed with success. To activate the emitter at system boot execute 'linux-enable-ir-emitter boot enable'")
                sys.exit(ExitCode.SUCCESS)
            else:  # reset the control value
                command = [UVC_SET_QUERY_PATH, device, unit, str(selector), control_size] + current_control
                subprocess.run(command, capture_output=True)

    logging.error("The configuration has failed.")
    logging.info("Do not hesitate to open an issue on GitHub ! https://github.com/EmixamPP/linux-enable-ir-emitter")
    sys.exit(ExitCode.FAILURE)


def _isAllReadyConfigured(device):
    """Test if the emitter is all ready configured

    Args:
        device (str): the infrared camera '/dev/videoX'

    Returns:
        bool: true if the infrared emitter is working, otherwise false
    """
    capture = cv2.VideoCapture(int(device[-1]))
    capture.read()
    time.sleep(2)
    capture.release()
    return _emitterIsWorking()


def _emitterIsWorking():
    """Ask the question "Did you see the ir emitter flashing (not just turn on) ? Yes/No ? "

    Returns:
        bool: true if the user input yes, otherwise false
    """
    check = input("Did you see the ir emitter flashing (not just turn on) ? Yes/No ? ").lower()
    while (check not in ("yes", "y", "no", "n")):
        check = input("Yes/No ? ").lower()

    if check in ("yes", "y"):
        return True
    return False


def _getUnitList(device):
    """Return the list of extension unit ID

    Args:
        device (str): the infrared camera '/dev/videoX'

    Returns:
        str list: list of extension unit ID for the device
    """
    command = "find /sys/class/video4linux/video" + device[-1] + "/device/ -name vendor -exec cat {} +"
    vid = subprocess.check_output(command, shell=True).strip().decode("utf-8")
    command = "find /sys/class/video4linux/video" + device[-1] + "/device/ -name product -exec cat {} +"
    pid = subprocess.check_output(command, shell=True).strip().decode("utf-8")

    command = "lsusb -d {}:{} -v | grep bUnitID | grep -Eo '[0-9]+'".format(vid, pid)
    return subprocess.run(command, shell=True, capture_output=True).stdout.strip().decode("utf-8").split("\n")


def _getControlSize(device, unit, selector):
    """Execute the UVC LEN QUERY

    Args:
        device (str): the infrared camera '/dev/videoX'
        unit (str): extension unit ID
        selector (str): control selector

    Returns:
        tuple(str, int): the control size and the exit code of the query
    """
    exec = subprocess.run([UVC_LEN_QUERY_PATH, device, unit, selector], capture_output=True)
    return exec.stdout.strip().decode('utf-8'), exec.returncode


def _getCurrentControl(device, unit, selector, control_size):
    """Execute the UVC GET CURR QUERY

    Args:
        device (str): the infrared camera '/dev/videoX'
        unit (str): extension unit ID
        selector (str): control selector
        control_size: control size

    Returns:
        tuple(str list, int): the current control and the exit code of the query
    """
    exec = subprocess.run([UVC_GET_QUERY_PATH, "0", device, unit, selector, control_size], capture_output=True)
    return exec.stdout.strip().decode('utf-8').split(' '), exec.returncode


def _getMaxControl(device, unit, selector, control_size):
    """Execute the UVC GET MAX QUERY

    Args:
        device (str): the infrared camera '/dev/videoX'
        unit (str): extension unit ID
        selector (str): control selector
        control_size: control size

    Returns:
        tuple(str, int): the max control and the exit code of the query
    """
    exec = subprocess.run([UVC_GET_QUERY_PATH, "1", device, unit, selector, control_size], capture_output=True)
    return exec.stdout.strip().decode('utf-8').split(' '), exec.returncode


def _exitIfFileDescriptorError(exit_code, device):
    """Exit if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR

    Args:
        exit_code (ExitCode): the exit code to check
        device (str): the infrared camera '/dev/videoX'
    """
    if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR:
        logging.critical("Cannot access to %s.", device)
        sys.exit(ExitCode.FILE_DESCRIPTOR_ERROR)