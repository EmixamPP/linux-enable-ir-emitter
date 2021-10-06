import subprocess
import logging
import cv2 as cv
import numpy
from scipy import stats

from Driver import Driver
from globals import ExitCode, UVC_GET_QUERY_PATH, UVC_LEN_QUERY_PATH


class DriverGenerator:
    def __init__(self, device, neg_answer_limit, manual_check):
        """Try to find a driver for an infrared camera
        Args:
            device (str): the infrared camera '/dev/videoX'
            neg_answer_limit (int): after k negative answer the pattern will be skiped. Use 256 for unlimited
            manual_check (bool): true for manual checking
        """
        self._device = device
        self._neg_answer_limit = neg_answer_limit
        self._manual_check = manual_check

        self._driver = None
    
    @property
    def device(self):
        """
        Returns:
            device (str): the infrared camera '/dev/videoX'
        """
        return self._device
    
    @property
    def driver(self):
        """
        Returns:
            Driver: working driver for DriverGenerator.device
            None: no working driver
        """
        return self._driver

    @property
    def deviceNumber(self):
        return int(self._device[-1])

    def generate(self):
        """Try to find a driver DriverGenerator.device

        Raises:
            DriverGeneratorError: error_code:DriverGeneratorError.DRIVER_ALREADY_EXIST
            DriverGeneratorError: error_code:ExitCode.FILE_DESCRIPTOR_ERROR
        """
        self.captureFrames()
        if self._askQuestionIsWorking():
            raise DriverGeneratorError("a driver already exists", DriverGeneratorError.DRIVER_ALREADY_EXIST)

        candidate_drivers = []
        for unit in self._units:
            for selector in range(0, 256):
                selector = str(selector)

                # get the control instruction lenght
                control_size = self._controlSize(unit, selector)
                if not control_size:
                    continue

                # get the current control instruction value
                curr_control = self._currControl(unit, selector, control_size)
                if not curr_control:
                    continue
                inital_driver = Driver(
                    curr_control, unit, selector, self._device)

                # get the max control instruction value
                max_control = self._maxControl(unit, selector, control_size)
                if not max_control or curr_control == max_control:
                    # or: cause maxControl isn't a possible instruction for enable the ir emitter
                    continue

                res_control = self._resControl(
                    unit, selector, control_size, curr_control, max_control)

                logging.debug("unit: {}, selector: {}, curr control: {}, max control: {}, res control: {}".format(
                    unit, selector, curr_control, max_control, res_control))

                # try to find the right control instruction
                next_control = curr_control
                neg_answer_counter = 0
                while(next_control and neg_answer_counter <= self._neg_answer_limit):
                    next_control = self._nextCurrControl(next_control, res_control, max_control)
                    if not next_control:
                        continue

                    logging.debug("control: {}".format(next_control))
                    driver = Driver(next_control, unit, selector, self._device)

                    isWorking, mean_var = self.emitterIsWorking(driver)
                    if isWorking:
                        if self._manual_check:
                            self._driver = driver
                            return
                        candidate_drivers.append((driver, mean_var))

                    neg_answer_counter += 1
                    # reset the control
                    inital_driver.run()
    
                if neg_answer_counter >= self._neg_answer_limit:
                    logging.debug("Negative answer limit exceeded, skipping the pattern.")

        self._driver = max(candidate_drivers, key=lambda driver:driver[1])[0]

    def captureFrames(self, sample_size=30):
        """Capture frames and compute their variance

        Raises:
            DriverGeneratorError: Cannot access to {self.device}

        Args:
            sample_size (int): number of images to capture, default is 30

        Returns:
            int list: 30 frame variances
        """
        device = cv.VideoCapture(self.deviceNumber)
        if not device.isOpened():
            self._raiseIfFileDescritonError(ExitCode.FILE_DESCRIPTOR_ERROR)

        # the variance of a frame gives an indicator of the shades of grey
        # when the emitter is triggered, strong shades are visible due to the return of infrared
        frames = [numpy.var(device.read()[1]) for _ in range(sample_size)]

        device.release()
        return frames

    def emitterIsWorking(self, driver):
        """Execute the driver and check if the infrared emitter work

        Raises:
            DriverGeneratorError: Cannot access to {self.device}

        Args:
            driver (Driver): driver to test

        Returns:
            tuple(bool, int): true if work and mean variance of frames captured after applied the driver
        """
        # debug print are disabled during the test because it is not relevent while automatic configuration
        init_log_level = logging.getLogger().level
        logging.getLogger().setLevel(logging.INFO)
        if self._manual_check:
            isWorking = self.manualEmitterIsWorking(driver)
            mean_var = numpy.inf
        else:
            isWorking, mean_var = self.automaticEmitterIsWorking(driver)
        logging.getLogger().setLevel(init_log_level)
        return isWorking, mean_var

    def manualEmitterIsWorking(self, driver):
        """Execute the driver and ask if the ir flashing

        Args:
            driver (Driver): driver to test

        Returns:
            bool: true if the user input yes, otherwise false
        """
        exit_code = driver.triggerIr()
        self._raiseIfFileDescritonError(exit_code)
        if exit_code == ExitCode.FAILURE:
            return False    
        return self._askQuestionIsWorking()   

    def _askQuestionIsWorking(self):
        """Ask the question "Did you see the ir emitter flashing (not just turn on) ? Yes/No ? "

        Returns:
            bool: true if the user input yes, otherwise false
        """
        check = input("Did you see the ir emitter flashing (not just turn on) ? Yes/No ? ").lower()
        while (check not in ("yes", "y", "no", "n")):
            check = input("Yes/No ? ").lower()
        return check in ("yes", "y")

    def automaticEmitterIsWorking(self, driver):
        """Execute the driver and check automaticaly if the infrared emitter work

        Args:
            driver (Driver): driver to test

        Returns:
            tuple(bool, int): true if work and mean variance of frames captured after applied the driver
        """
        before = self.captureFrames()

        exit_code = driver.run()
        self._raiseIfFileDescritonError(exit_code)
        if exit_code == ExitCode.FAILURE:
            return False, numpy.inf

        after = self.captureFrames()

        p = stats.ttest_ind(before, after)[1]/2
        return p < 0.01, numpy.mean(after)

    @property
    def _units(self):
        """Return the list of extension unit ID

        Returns:
            str list: list of extension unit ID for the device
        """
        command = "find /sys/class/video4linux/video" + \
            self._device[-1] + "/device/ -name vendor -exec cat {} +"
        vid = subprocess.check_output(
            command, shell=True).strip().decode("utf-8")
        command = "find /sys/class/video4linux/video" + \
            self._device[-1] + "/device/ -name product -exec cat {} +"
        pid = subprocess.check_output(
            command, shell=True).strip().decode("utf-8")

        command = "lsusb -d {}:{} -v | grep bUnitID | grep -Eo '[0-9]+'".format(vid, pid)
        return subprocess.run(command, shell=True, capture_output=True).stdout.strip().decode("utf-8").split("\n")

    def _controlSize(self, unit, selector):
        """Execute the UVC LEN QUERY

        Args:
            unit (str): extension unit ID
            selector (str): control selector

        Raises:
            DriverGeneratorError: Cannot access to {self.device}

        Returns:
            int: the control size and the exit code of the query
            None: error
        """
        exec = subprocess.run([UVC_LEN_QUERY_PATH, self._device, unit, selector], capture_output=True)
        exit_code = exec.returncode
        self._raiseIfFileDescritonError(exit_code)
        return exec.stdout.strip().decode('utf-8') if exit_code == ExitCode.SUCCESS else None

    def _currControl(self, unit, selector, control_size):
        """Execute the UVC GET CURR QUERY

        Args:
            unit (str): extension unit ID
            selector (str): control selector
            control_size (str): control size

         Raises:
            DriverGeneratorError: Cannot access to {self.device}

        Returns:
            str list: the current control
            None: error
        """
        exec = subprocess.run([UVC_GET_QUERY_PATH, "0", self._device, unit, selector, control_size], capture_output=True)
        exit_code = exec.returncode
        self._raiseIfFileDescritonError(exit_code)
        return exec.stdout.strip().decode('utf-8').split(' ') if exit_code == ExitCode.SUCCESS else None

    def _maxControl(self, unit, selector, control_size):
        """Execute the UVC GET MAX QUERY

        Args:
            unit (str): extension unit ID
            selector (str): control selector
            control_size (str): control size

        Returns:
            str list: the maximum control
            None: error
        """
        exec = subprocess.run([UVC_GET_QUERY_PATH, "1", self._device, unit, selector, control_size], capture_output=True)
        exit_code = exec.returncode
        self._raiseIfFileDescritonError(exit_code)
        return exec.stdout.strip().decode('utf-8').split(' ') if exit_code == ExitCode.SUCCESS else None

    def _resControl(self, unit, selector, control_size, curr_control, max_control):
        """Execute the UVC GET RES QUERY

        Args:
            unit (str): extension unit ID
            selector (str): control selector
            control_size (str): control size
            curr_control (str list): current control
            max_control (str list): maximum control

        Returns:
            str list: the resolution control
        """
        exec = subprocess.run([UVC_GET_QUERY_PATH, "2", self._device, unit, selector, control_size], capture_output=True)
        exit_code = exec.returncode
        self._raiseIfFileDescritonError(exit_code)
        if exit_code == ExitCode.SUCCESS:
            return exec.stdout.strip().decode('utf-8').split(' ')

        # try to find the resolution control by substitution, it may result in a false ressolution control
        return [str(int(c1 != c2)) for c1, c2 in zip(curr_control, max_control)]

    def _nextCurrControl(self, curr_control, res_control, max_control):
        """Compute the next possible control instruction

        Args:
            curr_control (str list): last executed control
            res_control (str list): resolution control
            max_control (str list): maximum control

        Returns:
            str list: the next possible control
            None: no more possible instruction
        """
        new_current_control = []
        for c1, c2, c3 in zip(curr_control, res_control, max_control):
            new_c1 = int(c1) + int(c2)
            if new_c1 > int(c3):
                return
            else:
                new_current_control.append(str(new_c1))
        return new_current_control

    def _raiseIfFileDescritonError(self, exit_code):
        """ Raise error if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR 
            otherwise do nothing 

        Args:
            exit_code (ExitCode): the exit code to check

        Raises:
            DriverGeneratorError: Cannot access to {self.device}
        """
        if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR:
            raise DriverGeneratorError(
                "Cannot access to {}.".format(self._device), exit_code)


class DriverGeneratorError(Exception):
    DRIVER_ALREADY_EXIST = -1

    def __init__(self, message, error_code):
        super().__init__(message)
        self.message = message
        self.error_code = error_code

    def __str__(self):
        return "DriverGeneratorError, raison: {}, error code: {}".format(self.message, self.error_code)
