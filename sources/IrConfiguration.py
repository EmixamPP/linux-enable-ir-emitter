import os
import time
import yaml
import sys
import cv2
import logging

from globals import ExitCode, UVC_SET_QUERY_PATH, SAVE_CONFIG_FILE_PATH


class IrConfiguration:
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
        # The exit codes returned by os.system not correspond to those returned by the executed program.
        command = "{} {} {} {} {} {}".format(UVC_SET_QUERY_PATH, self.device, self.unit, self.selector, self.control_size, self._control_str)
        if logging.getLogger().level != logging.DEBUG:
            command += " &> /dev/null"

        exit_code = os.system(command)
        if exit_code == 32256: 
            return ExitCode.FILE_DESCRIPTOR_ERROR
        elif exit_code == 256:
            return ExitCode.FAILURE
        return ExitCode.SUCCESS

    def triggerIr(self, duration=2):
        """Execute the UVC_SET_CUR query and try to trigger the ir emitter. 

        Args:
            duration (int): transmit for how long ? (seconds). Defaults to 2.

        Returns:
            ExitCode: ExitCode.SUCCESS
            ExitCode: ExitCode.FAILURE
            ExitCode: ExitCode.FILE_DESCRIPTOR_ERROR cannot access to the camera
        """
        exit_code = self.run()
        if exit_code == ExitCode.SUCCESS:
            capture = cv2.VideoCapture(int(self.device[-1]))
            capture.read()
            time.sleep(duration)
            capture.release()
        return exit_code

    def save(self, save_config_file_path):
        """Save the configuration in a .yaml file

        Args:
            save_config_file_path (str): file in which the config will be saved
        """
        with open(save_config_file_path, "w") as save_config_file:
            save_config_file.write("#Caution: any manual modification of this file may corrupt the operation of the program! You must therefore be very careful.\n")
            save_config_file.write("#Please consult https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Manual-configuration before.\n")
            save_config_file.write("#If you currupt the config file: execute 'linux-enable-ir-emitter fix config' to reset the file.\n\n")
            yaml.dump(self, save_config_file)

    @staticmethod
    def load(saved_path_file):
        """Creates an IrConfiguration object from a saved configuration file

        Args:
            saved_path_file (str): path to the .yaml config file
        
        Raises:
            yaml.YAMLError: the file cannot be deserialized
            AssertError: the file does not contain all the expected attributes
    
        Returns:
            IrConfiguration: the object created
        """
        with open(saved_path_file, "r") as save_file:
            object_deserialized = yaml.load(save_file, Loader=yaml.Loader)

        dummy_config = IrConfiguration([0], 0, 0, '')
        assert(dir(dummy_config) == dir(object_deserialized))

        return object_deserialized

    def __eq__(self, to_compare):
        if not isinstance(to_compare, IrConfiguration):
            return False
        elif self.control != to_compare.control:
            return False
        elif self.unit != to_compare.unit:
            return False
        elif self.selector != to_compare.selector:
            return False
        else:
            return True


def load_saved_config():
    """Load the ir configuration saved. Exit if a error occur

    Returns:
        IrConfiguration: the saved configuration
        None: if no configuration is saved
    """
    try:
        if os.path.exists(SAVE_CONFIG_FILE_PATH):
            return IrConfiguration.load(SAVE_CONFIG_FILE_PATH)
        else:
            logging.error("No configuration is currently saved.")
            return
    except:
        logging.critical("The config file is corrupted.")
        logging.info("Execute 'linux-enable-ir-emitter fix config' to reset the file.")
        sys.exit(ExitCode.FAILURE)