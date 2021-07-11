import os
import yaml
import cv2

local_path = path = os.path.dirname(os.path.abspath(__file__))
bin_path = config_file_path = local_path + "/enable-ir-emitter"
save_config_file_path = os.path.expanduser("~") + "/.irConfig.yaml"


class IrConfiguration:
    def __init__(self, data, unit, selector, videoPath):
        """Query a UVC XU control (UVC_SET_CUR querry)
           Which is intended to activate the infrared camera transmitter.

        Args:
            data (int list): Control value
            unit (int): Extension unit ID
            selector (int): Control selector
            videoPath (str): Path to the infrared camera e.g : "/dev/video2"
        """
        self._data = [hex(i) for i in data]
        self._unit = hex(unit)
        self._selector = hex(selector)
        self._videoPath = videoPath

    @property
    def videoPath(self):
        return self._videoPath

    @property
    def data(self):
        return self._data

    @property
    def unit(self):
        return self._unit

    @property
    def selector(self):
        return self._selector

    @videoPath.setter
    def videoPath(self, videoPath):
        self._videoPath = videoPath

    @data.setter
    def data(self, data):
        self._data = data

    @unit.setter
    def unit(self, unit):
        self._unit = unit

    @selector.setter
    def selector(self, selector):
        self._selector = selector

    def _data_string(self):
        """Convert the self.data list to a string sequence

        Returns:
            str: e.g. "0x01 0x03 0x02 0x00 0x00 0x00 0x00 0x00 0x00"
        """
        data_str = str(self.data)  # => "[..., ..., ...]"
        data_str = data_str.replace(",", "")  # => "[... ... ...]"
        data_str = data_str[1:-1]  # => "... ... ..."
        return data_str

    def run(self):
        """Execute the UVC_SET_CUR querry

        Returns:
            int: 256 if error, else 0
        """
        command = "{} {} {} {} {} {}".format(
            bin_path, self.videoPath, self.unit, self.selector, len(self.data), self._data_string())
        return os.system(command)

    def trigger_ir(self, time=3):
        """Try to trigger the ir emitter

        Args:
            time (int): transmit for how long ? (seconds). Defaults to 3.
        """
        self.run()
        capture = cv2.VideoCapture(int(self.videoPath[-1]))
        capture.read()
        os.system("sleep " + str(time))
        capture.release()

    def save(self, save_dir_path):
        """Save the configuration in a irConfig.yaml file

        Args:
            save_dir_path (str): directory in which the file will be saved
        """
        with open(save_config_file_path, "w") as save_config_file:
            save_config_file.write(
                "#Caution: any manual modification of this file may corrupt the operation of the program! You must therefore be very careful.\n")
            save_config_file.write(
                "#Please consult https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Manual-configuration before.\n\n")
            yaml.dump(self, save_config_file)

    @staticmethod
    def load(saved_path_file):
        """Creates an IrConfiguration object from a saved configuration file

        Args:
            saved_path_file (str): path to the irConfig.yaml

        Returns:
            IrConfiguration: the object created
        """
        with open(saved_path_file, "r") as save_file:
            object_deserialized = yaml.load(save_file, Loader=yaml.Loader)
        return object_deserialized

    def __eq__(self, to_compare):
        if not isinstance(to_compare, IrConfiguration):
            return False
        elif self.data != to_compare.data:
            return False
        elif self.unit != to_compare.unit:
            return False
        elif self.selector != to_compare.selector:
            return False
        else:
            return True
