import re

from globals import SAVE_DRIVER_FOLDER_PATH

OLD_DRIVER_PATH = "/etc/linux-enable-ir-emitter.yaml"

def write_new_driver(control: list[str], device: str, unit: str, selector: str) -> None:
    """Migrate old driver file structure to the new one, 
    if one of the parameter is not correct, the driver will written

    Args:
        control: control value
        device: path to the infrared camera /dev/videoX
        unit: extension unit ID
        selector: control selector
    """
    if (re.fullmatch("/dev/video[0-9]+", device) and len(control) and unit and selector): 
        with open(SAVE_DRIVER_FOLDER_PATH + device[5:], 'w') as f:
            f.write("device=%s\n" %(device))
            f.write("unit=%s\n" %(unit))
            f.write("selector=%s\n" %(selector))
            f.write("size=%d\n" %(len(control)))   
            for i in range(len(control)):
                f.write("control%d=%s\n" %(i, control[i]))

if __name__ == "__main__":
    control = []
    device = unit = selector = None
    for line in open(OLD_DRIVER_PATH, "r"):
        line = line.strip()
        if "---" == line[:3]:  # many drivers has been configured
            write_new_driver(control, device, unit, selector)
            control = []
            device = unit = selector = None
        if "- " == line[:2]:
            control.append(line[2:])
        elif "_device: " == line[:9]:
            device = line[9:]
        elif "_selector: " == line[:11]:
            selector = line[11:]
        elif "_unit: " == line[:7]:
            unit = line[7:]

    write_new_driver(control, device, unit, selector)
