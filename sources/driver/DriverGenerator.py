import subprocess
import logging
import cv2 as cv
import time
import re

from driver.Driver import Driver
from globals import ExitCode, UVC_GET_QUERY_PATH, UVC_LEN_QUERY_PATH


"""DOCUMENTATION
- https://www.kernel.org/doc/html/v5.14/userspace-api/media/drivers/uvcvideo.html
    info 1: uvc queries are explained
    info 2: units can be found by parsing the uvc descriptor
- https://www.mail-archive.com/search?l=linux-uvc-devel@lists.berlios.de&q=subject:%22Re%5C%3A+%5C%5BLinux%5C-uvc%5C-devel%5C%5D+UVC%22&o=newest&f=1
    info 1: selector is on 8 bits and since the manufacturer does not provide a driver, it is impossible to know which value it is.
"""


class DriverGenerator:
    def __init__(self, device: str, neg_answer_limit: int) -> None:
        """Try to find a driver for an infrared camera
        Args:
            device: the infrared camera '/dev/videoX'
            neg_answer_limit: after k negative answer the pattern will be skiped. Use 256 for unlimited
        """
        self._device = device
        self._neg_answer_limit = neg_answer_limit
        self._driver = None
    
    @property
    def device(self) -> str:
        return self._device
    
    @property
    def driver(self) -> Driver or None:
        """
        Returns:
            The working driver for DriverGenerator.device
            None if no working driver
        """
        return self._driver

    @property
    def device_number(self) -> str:
        start_pos = re.search("[0-9]", self.device).start()
        return self.device[start_pos:]

    def generate(self) -> None:
        """Try to find a driver DriverGenerator.device

        Raises:
            DriverGeneratorError: error_code:DriverGeneratorError.DRIVER_ALREADY_EXIST
            DriverGeneratorError: error_code:ExitCode.FILE_DESCRIPTOR_ERROR
        """
        if self.emitter_is_working():
            raise DriverGeneratorError("a driver already exists", DriverGeneratorError.DRIVER_ALREADY_EXIST)

        for unit in self._units:
            for selector in range(0, 256):
                selector = str(selector)

                # get the control instruction lenght
                control_size = self._control_size(unit, selector)
                if not control_size:
                    continue

                # get the current control instruction value
                curr_control = self._curr_control(unit, selector, control_size)
                if not curr_control:
                    continue
                initial_driver = Driver(curr_control, unit, selector, self._device)

                exit_code = self.execute_driver(initial_driver)
                if exit_code != ExitCode.SUCCESS:
                    # if an unit and selector exists but can't be modified
                    continue

                # get the max control instruction value
                max_control = self._max_control(unit, selector, control_size)
                if not max_control or curr_control == max_control:
                    # or: because maxControl isn't a possible instruction for enable the ir emitter
                    continue

                res_control = self._res_control(unit, selector, control_size, curr_control, max_control)

                logging.debug("unit: {}, selector: {}, curr control: {}, max control: {}, res control: {}".format(
                    unit, selector, curr_control, max_control, res_control))

                # try to find the right control instruction
                next_control = curr_control
                neg_answer_counter = 0
                while(next_control and neg_answer_counter < self._neg_answer_limit):
                    next_control = self._next_curr_control(next_control, res_control, max_control)
                    if not next_control:
                        continue

                    logging.debug("control: {}".format(next_control))
                    driver = Driver(next_control, unit, selector, self._device)

                    if self.driver_is_working(driver):
                        self._driver = driver
                        return
                    neg_answer_counter += 1
    
                if neg_answer_counter > self._neg_answer_limit:
                    logging.debug("Negative answer limit exceeded, skipping the pattern.")
            
                # reset the control
                self.execute_driver(initial_driver)

    def driver_is_working(self, driver: Driver) -> bool:
        """Apply the driver and execute DriverGenerator.emitterIsWorking()

        Args:
            driver (Driver): driver to test

        Returns:
            true if the user input yes, otherwise false
        """
        exit_code = self.execute_driver(driver)
        if exit_code != ExitCode.SUCCESS:
            return False    
        return self.emitter_is_working()   

    def emitter_is_working(self):
        """Trigger the infrared emitter and ask the question:
         "Did you see the ir emitter flashing (not just turn on) ? Yes/No ? "

        Returns:
            bool: true if the user input yes, otherwise false
        """
        device = cv.VideoCapture(self.device)
        if not device.isOpened():
            self._raise_if_file_descriptor_error(ExitCode.FILE_DESCRIPTOR_ERROR)
        device.read()
        time.sleep(1.5)
        device.release()

        check = input("Did you see the ir emitter flashing (not just turn on) ? Yes/No ? ").lower()
        while (check not in ("yes", "y", "no", "n")):
            check = input("Yes/No ? ").lower()
        return check in ("yes", "y")

    def execute_driver(self, driver: Driver) -> bool:
        """Execute a driver

        Args:
            driver: driver to execute
        
        Raises:
            DriverGeneratorError: error_code:ExitCode.FILE_DESCRIPTOR_ERROR

        Returns:
            ExitCode returned by the driver execution
        """
        # debug print are disabled because it is not relevent while automatic configuration
        init_log_level = logging.getLogger().level
        logging.getLogger().setLevel(logging.INFO)
        exit_code = driver.run()
        logging.getLogger().setLevel(init_log_level)
        self._raise_if_file_descriptor_error(exit_code)
        return exit_code

    @property
    def _units(self) -> list[str]:
        """Return the list of extension unit ID

        Returns:
            list of extension unit ID for the device
        """
        command = "find /sys/class/video4linux/video" + \
            self.device_number + "/device/ -name vendor -exec cat {} +"
        vid = subprocess.check_output(
            command, shell=True).strip().decode("utf-8")
        command = "find /sys/class/video4linux/video" + \
            self.device_number + "/device/ -name product -exec cat {} +"
        pid = subprocess.check_output(command, shell=True).strip().decode("utf-8")

        command = "lsusb -d {}:{} -v | grep bUnitID | grep -Eo '[0-9]+'".format(vid, pid)
        return subprocess.run(command, shell=True, capture_output=True).stdout.strip().decode("utf-8").split("\n")

    def _control_size(self, unit: str, selector: str) -> str or None:
        """Execute the UVC LEN QUERY

        Args:
            unit: extension unit ID
            selector: control selector

        Raises:
            DriverGeneratorError: Cannot access to {self.device}

        Returns:
            The control size and the exit code of the query
            None if error
        """
        exec = subprocess.run([UVC_LEN_QUERY_PATH, self._device, unit, selector], capture_output=True)
        exit_code = exec.returncode
        self._raise_if_file_descriptor_error(exit_code)
        return exec.stdout.strip().decode('utf-8') if exit_code == ExitCode.SUCCESS else None

    def _curr_control(self, unit: str, selector: str, control_size: str) -> list[str] or None:
        """Execute the UVC GET CURR QUERY

        Args:
            unit: extension unit ID
            selector: control selector
            control_size: control size

         Raises:
            DriverGeneratorError: Cannot access to {self.device}

        Returns:
            The current control
            None if error
        """
        exec = subprocess.run([UVC_GET_QUERY_PATH, "0", self._device, unit, selector, control_size], capture_output=True)
        exit_code = exec.returncode
        self._raise_if_file_descriptor_error(exit_code)
        return exec.stdout.strip().decode('utf-8').split(' ') if exit_code == ExitCode.SUCCESS else None

    def _max_control(self, unit: str, selector: str, control_size: str) -> list[str]:
        """Execute the UVC GET MAX QUERY

        Args:
            unit: extension unit ID
            selector: control selector
            control_size: control size

        Returns:
            The maximum control
            None if error
        """
        exec = subprocess.run([UVC_GET_QUERY_PATH, "1", self._device, unit, selector, control_size], capture_output=True)
        exit_code = exec.returncode
        self._raise_if_file_descriptor_error(exit_code)
        return exec.stdout.strip().decode('utf-8').split(' ') if exit_code == ExitCode.SUCCESS else None

    def _res_control(self, unit: str, selector: str, control_size: str, curr_control: list[str], max_control: list[str]) -> list[str]:
        """Execute the UVC GET RES QUERY

        Args:
            unit: extension unit ID
            selector: control selector
            control_size: control size
            curr_control: current control
            max_control: maximum control

        Returns:
            The resolution control
        """
        exec = subprocess.run([UVC_GET_QUERY_PATH, "2", self._device, unit, selector, control_size], capture_output=True)
        exit_code = exec.returncode
        self._raise_if_file_descriptor_error(exit_code)
        if exit_code == ExitCode.SUCCESS:
            return exec.stdout.strip().decode('utf-8').split(' ')

        # try to find the resolution control by substitution, it may result in a false ressolution control
        return [str(int(c1 != c2)) for c1, c2 in zip(curr_control, max_control)]

    def _next_curr_control(self, curr_control: list[str], res_control: list[str], max_control: list[str]) -> list[str] or None:
        """Compute the next possible control instruction

        Args:
            curr_control: last executed control
            res_control: resolution control
            max_control: maximum control

        Returns:
            The next possible control
            None if no more possible instruction
        """
        new_current_control = []
        for c1, c2, c3 in zip(curr_control, res_control, max_control):
            new_c1 = int(c1) + int(c2)
            if new_c1 > int(c3):
                return
            else:
                new_current_control.append(str(new_c1))
        return new_current_control

    def _raise_if_file_descriptor_error(self, exit_code: ExitCode) -> None:
        """ Raise error if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR 
            otherwise do nothing 

        Args:
            exit_code: the exit code to check

        Raises:
            DriverGeneratorError: Cannot access to {self.device}
        """
        if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR:
            raise DriverGeneratorError(
                "Cannot access to {}.".format(self._device), exit_code)


class DriverGeneratorError(Exception):
    DRIVER_ALREADY_EXIST = -1

    def __init__(self, message: object, error_code: int or ExitCode) -> None:
        super().__init__(message)
        self.message = message
        self.error_code = error_code

    def __str__(self) -> str:
        return "DriverGeneratorError, raison: {}, error code: {}".format(self.message, self.error_code)
