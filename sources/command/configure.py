import time
import subprocess
import cv2
import logging
import sys

from globals import UVC_LEN_QUERY_PATH, UVC_SET_QUERY_PATH, UVC_GET_QUERY_PATH, ExitCode
from IrConfiguration import IrConfiguration
from IrConfigurationSerializer import IrConfigurationSerializer


"""DOCUMENTATION
- https://www.kernel.org/doc/html/v5.14/userspace-api/media/drivers/uvcvideo.html
    info 1: uvc queries are explained
    info 2: units can be found by parsing the uvc descriptor
- https://www.mail-archive.com/search?l=linux-uvc-devel@lists.berlios.de&q=subject:%22Re%5C%3A+%5C%5BLinux%5C-uvc%5C-devel%5C%5D+UVC%22&o=newest&f=1
    info 1: selector is on 8 bits and since the manufacturer does not provide a driver, it is impossible to know which value it is.
"""


def execute(device, neg_answer_limit):
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
            exitIfFileDescriptorError(exit_code, device)

            # get the current control instruction value
            current_control, exit_code = _getCurrentControl(device, unit, str(selector), control_size)
            if exit_code == ExitCode.FAILURE:
                continue
            exitIfFileDescriptorError(exit_code, device)
            
            # get the max control instruction value
            max_control, exit_code = _getMaxControl(device, unit, str(selector), control_size)
            if exit_code == ExitCode.FAILURE or current_control == max_control:
                # or: cause maxControl isn't a possible instruction for enable the ir emitter
                continue
            exitIfFileDescriptorError(exit_code, device)

            res_control, exit_code = _getResControl(device, unit, str(selector), control_size, current_control, max_control)
            exitIfFileDescriptorError(exit_code, device)
            logging.debug("unit: {}, selector: {}, curr control: {}, max control: {}, res control: {}".format(unit, selector, current_control, max_control, res_control))

            # try to find the right control instruction
            next_control = current_control
            neg_answer_counter = 0
            while(next_control):
                next_control = _getNextCurrControl(next_control, res_control, max_control)
                if not next_control:
                    continue
                ir_config = IrConfiguration(next_control, unit, selector, device)

                # debug print are disabled during the test cause it is not relevent while automatic configuration
                init_log_level  = logging.getLogger().level
                logging.getLogger().setLevel(logging.INFO)  
                exit_code = ir_config.triggerIr()
                logging.getLogger().setLevel(init_log_level)

                if exit_code == ExitCode.FAILURE:
                    continue
                exitIfFileDescriptorError(exit_code, device)
                logging.debug("control: {}".format(next_control))

                if _emitterIsWorking():
                    IrConfigurationSerializer.add_config(ir_config)
                    logging.info("The configuration is completed with success. To activate the emitter at system boot execute 'linux-enable-ir-emitter boot enable'")
                    sys.exit(ExitCode.SUCCESS)
                elif neg_answer_counter + 1 >= neg_answer_limit:  
                    logging.debug("Negative answer limit exceeded, skipping the pattern.")
                    break # this control pattern seems to don't work
                neg_answer_counter += 1
            
            # reset the control
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

    return check in ("yes", "y")


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
        tuple(str list, int): the maximum control and the exit code of the query
    """
    exec = subprocess.run([UVC_GET_QUERY_PATH, "1", device, unit, selector, control_size], capture_output=True)
    return exec.stdout.strip().decode('utf-8').split(' '), exec.returncode


def _getResControl(device, unit, selector, control_size, curr_control, max_control):
    """Execute the UVC GET RES QUERY

    Args:
        device (str): the infrared camera '/dev/videoX'
        unit (str): extension unit ID
        selector (str): control selector
        control_size: control size
        curr_control (list of int|str): current control
        max_control (list of int|str): maximum control

    Returns:
        tuple(str list, int): the maximum control and the exit code of the query (but never FAILURE)
    """
    exec = subprocess.run([UVC_GET_QUERY_PATH, "2", device, unit, selector, control_size], capture_output=True)
    res_control, exit_code = exec.stdout.strip().decode('utf-8').split(' '), exec.returncode
    if exit_code != ExitCode.FAILURE:
        return res_control, exit_code
    
    # try to find the resolution control by substitution, it may result in a false ressolution control
    return [int(c1 != c2) for c1, c2 in zip(curr_control, max_control)], ExitCode.SUCCESS


def _getNextCurrControl(curr_control, res_control, max_control):
    """Compute the next possible control instruction

    Args:
        curr_control (list of int|str): last executed control
        res_control (list of int|str): resolution control
        max_control (list of int|str): maximum control

    Returns:
        int list: the next possible control
        None: no more possible instruction
    """
    new_current_control = []
    for c1, c2, c3 in zip(curr_control, res_control, max_control):
        new_c1 = int(c1) + int(c2)
        if new_c1 > int(c3):
            return
        else:
            new_current_control.append(new_c1)
    return new_current_control


def exitIfFileDescriptorError(exit_code, device):
    """Exit if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR

    Args:
        exit_code (ExitCode): the exit code to check
        device (str): the infrared camera '/dev/videoX'
    """
    if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR:
        logging.critical("Cannot access to %s.", device)
        sys.exit(ExitCode.FILE_DESCRIPTOR_ERROR)
