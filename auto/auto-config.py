#!/usr/bin/env python3

from yaml import load, FullLoader
from os import system
from cv2 import VideoCapture


def create_systemd(command):
    file_content = "[Unit]\nDescription=enable ir emitter\nAfter=multi-user.target suspend.target hibernate.target hybrid-sleep.target suspend-then-hibernate.target\n\n[Service]\nType=oneshot\nExecStart=/usr/local/bin/{}\n\n[Install]\nWantedBy=multi-user.target suspend.target hibernate.target hybrid-sleep.target suspend-then-hibernate.target\n".format(
        command[2:])

    file = open("enable-ir-emitter.service", "w")
    file.write(file_content)
    file.close()

    system("sudo cp enable-ir-emitter /usr/local/bin")
    system("sudo cp enable-ir-emitter.service /etc/systemd/system")
    system("sudo systemctl enable --now enable-ir-emitter")


def capture(video_path_number, time):
    """Start a video capture.
    video_path_number -- the integer part of /dev/videoX. e.g.: 2 for /dev/video2 
    time -- for how long ? (seconds)
    """
    capture = VideoCapture(video_path_number)
    capture.read()
    system("sleep " + str(time))
    capture.release()


if __name__ == "__main__":
    system("gcc enable-ir-emitter-template.c -o enable-ir-emitter")

    with open("config.yaml") as config_file:
        config_list = load(config_file, Loader=FullLoader)

    i = 0
    for config in config_list:
        print("Configuration #{} ...".format(i))

        data = config["data"]
        data_size = data.count(" ") + 1
        unit = config["unit"]
        selector = config["selector"]

        command = "./enable-ir-emitter -dataSize {} -data {} -unit {} -selector {}".format(data_size, data, unit, selector)
        res = system(command)

        if not res:
            user_input = input("A configuration has been found, do you want to test now ? Yes/No ? ").lower()
            if user_input in ("yes", "y"):
                capture(2, 3)
            else:
                print("Please test if the emitter of your infrared camera works by yourself.")

            user_input = input("Does it work ? Yes/No ? ").lower()

            if user_input in ("yes", "y"):
                user_input = input("Do you want to automatically activate the emitter at system startup ? Yes/No ? ").lower()
                if user_input in ("yes", "y"):
                    print("Creation of the systemd service ... (administrator commands will be executed)")
                    create_systemd(command)
                else:
                    print("No problem, here is the command: ", command)
                break

        i += 1
