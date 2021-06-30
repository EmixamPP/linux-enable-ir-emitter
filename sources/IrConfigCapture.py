import os
import pyshark

from sources.IrConfiguration import IrConfiguration


class IrConfigCapture():
    def __init__(self, video_path):
        """Captures every bus packet that may have been used to activate the infrared emitter.

        Args:
            video_path (string): Path to the infrared camera e.g : "/dev/video2"
        """
        interfaces = ["usbmon1", "usbmon2", "usbmon3", "usbmon4"]
        self._capture = pyshark.LiveCapture(interface=interfaces, display_filter="usb.transfer_type==0x02 && usb.bmRequestType==0x21")
        self._stop_flag = False
        self._config_list = []
        self._video_path = video_path

    def start(self, time):
        """Start the packets capture, the script will be paused for [time] sec

        Args:
            time (int): sniff the camera bus during [time] sec
        """
        os.system("modprobe usbmon")
        self._capture.sniff(timeout=time)
        for pkt in self._capture:
            config = self._pkt_to_config(pkt)
            if config:
                self._config_list.append(config)

    def _pkt_to_config(self, pkt):
        """Convert a bus packet to an infrared configuration

        Args:
            pkt (pyshark.packet.data): packet

        Returns:
            IrConfiguration: the converted packet
            None: impossible conversion
        """
        try:
            # unit : first or two first windex symbols (after 0x)
            windex = hex(int(pkt.usb_setup_windex))  # convert into hex
            windex = int(windex[:3], 16) if len(windex) % 2 else int(windex[:4], 16)
            # selector : two first wvalue symbols (after 0x)
            wvalue = hex(int(pkt.usb_setup_wvalue, 16))  # already in hex but remove usless 0
            wvalue = int(wvalue[:3], 16) if len(wvalue) % 2 else int(wvalue[:4], 16)
            # data : each two symbol separated by ":"
            data = pkt.usb_data_fragment.split(":")
            data = [int(i, 16) for i in data]
            return IrConfiguration(data, windex, wvalue, self._video_path)
        except:
            return None

    @property
    def config_list(self):
        """Every possible infrared configuration captured

        Returns:
            list: of IrConfiguration object, can be empty
        """
        return self._config_list
