import os
import time
import cv2 as cv
import logging

from globals import ExitCode, UVC_SET_QUERY_PATH


class Driver:
    def __init__(self, control, unit, selector, device):
        """Query a UVC XU control (UVC_SET_CUR querry)
           Which is intended to activate the infrared camera transmitter.

        Args:
            control (int list): Control value
            unit (int): Extension unit ID
            selector (int): Control selector
            device (str): the infrared camera e.g : "/dev/video2"
        """
        self._control = [int(i) for i in control]
        self._unit = int(unit)
        self._selector = int(selector)
        self._device = device

    @property
    def device(self):
        return self._device

    @property
    def control(self):
        return self._control

    @property
    def unit(self):
        return self._unit

    @property
    def selector(self):
        return self._selector
    
    @property
    def control_size(self):
        return len(self._control)
    
    @property
    def _control_str(self):
        """Convert the self.data list to a string sequence
        Returns:
            str: e.g. "1 3 3 0 0 0 0 0 0"
        """
        return ' '.join(str(value) for value in self.control)
    
    def run(self):
        """Execute the UVC_SET_CUR query

        Returns:
            ExitCode: ExitCode.SUCCESS
            ExitCode: ExitCode.FAILURE
            ExitCode: ExitCode.FILE_DESCRIPTOR_ERROR cannot access to the camera
        """
        # Subprocess does not work with systemd ! 
        command = "{} {} {} {} {} {}".format(UVC_SET_QUERY_PATH, self.device, self.unit, self.selector, self.control_size, self._control_str)
        if logging.getLogger().level != logging.DEBUG:
            command += " &> /dev/null"

        # The exit codes returned by os.system not correspond to those returned by the executed program.
        exit_code = os.system(command)
        if exit_code == 32256: 
            return ExitCode.FILE_DESCRIPTOR_ERROR
        elif exit_code == 256:
            return ExitCode.FAILURE
        return ExitCode.SUCCESS

    def triggerIr(self, duration=1.5):
        """Execute Driver.run() and try to trigger the ir emitter. 

        Args:
            duration (int|float): transmit for how long ? (seconds). Defaults to 1.5.

        Returns:
            ExitCode: ExitCode.SUCCESS
            ExitCode: ExitCode.FAILURE
            ExitCode: ExitCode.FILE_DESCRIPTOR_ERROR cannot access to the camera
        """
        exit_code = self.run()
        if exit_code == ExitCode.SUCCESS:
            device = cv.VideoCapture(self.device)
            if not device.isOpened():
                return ExitCode.FILE_DESCRIPTOR_ERROR
            device.read()
            time.sleep(duration)
            device.release()
        return exit_code

    def __eq__(self, to_compare):
        if not isinstance(to_compare, Driver):
            return False
        elif self.control != to_compare.control:
            return False
        elif self.unit != to_compare.unit:
            return False
        elif self.selector != to_compare.selector:
            return False
        elif self.device != to_compare.device:
            return False
        else:
            return True

    