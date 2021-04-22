from json import load
from os import system


def create_systemd(command):
    print("Creation of the service ...")
    file_content = """[Unit]
Description=enable ir emitter
After=multi-user.target suspend.target hibernate.target hybrid-sleep.target suspend-then-hibernate.target

[Service]
ExecStart=/usr/local/bin/{}

[Install]
WantedBy=multi-user.target suspend.target hibernate.target hybrid-sleep.target suspend-then-hibernate.target
""".format(command[2:])

    file = open("enable-ir-emitter.service", "w")
    file.write(file_content)
    file.close()

    system("sudo cp enable-ir-emitter /usr/local/bin")
    system("sudo systemctl enable enable-ir-emitter && sudo systemctl start enable-ir-emitter")


if __name__ == "__main__":
    print("Compilation of the C script ...")
    system("gcc auto-enable-ir-emitter.c -o enable-ir-emitter")

    print("Trying all know infrared camera configuration ...")
    with open("config.json") as config_file:
        config_list = load(config_file)

    i = 0
    for config in config_list:
        print("Configuration #{} ...".format(i))

        data = config["data"]
        data_size = data.count(" ") + 1
        unit = config["unit"]
        selector = config["selector"]

        command = "./enable-ir-emitter -dataSize {} -data {} -unit {} -selector {}".format(data_size, data, unit, selector)
        res = system(command)

        check = ""
        if not res:
            while check not in ("yes", "no", "y", "n"):
                check = input("A configuration has been found, please test if the emitter of your infrared camera works.\nDoes it work? Yes/No ? ").lower()

            if check in ("yes", "y"):
                check = ""
                while check not in ("yes", "no", "y", "n"):
                    check = input("Do you want to automatically activate the emitter at system startup ? Yes/No ? ").lower()

                if check in ("yes", "y"):
                    print("Creating the script with systemd ... (administrator commands will be executed)")
                    create_systemd(command)
                else:
                    print("No problem, here is the command: ", command)
                break

        print("Configuration #{} does not work".format(i))
        i += 1
