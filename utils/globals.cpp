#include "globals.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <signal.h>
using namespace std;

const string V4L_PATHS_DIR = "/dev/v4l/by-path/";

/**
 * @brief Obtains the name of a device
 *
 * @param device path to the infrared camera
 * @return devie name
 */
static string deviceNameOf(const string &device)
{
    size_t pos = device.rfind('/');
    return device.substr(pos + 1);
}

/**
 * @brief Gets the path corresponding to the configuration
 * of a device. It does not check if the path exists.
 *
 * @param device path to the infrared camera
 *
 * @return path to the file
 */
string ConfigPathOf(const string &device)
{
    return _SAVE_FOLDER_CONFIG_PATH + deviceNameOf(device);
}

/**
 * @brief Get the configurations path corresponding to all configured device
 * or just to one specific
 *
 * @param device path to the infrared camera
 * Empty string to get all configurations path
 *
 * @return path(s) to the configuration(s)
 */
vector<string> GetConfigPaths(const string &device)
{
    const string deviceName = deviceNameOf(device);
    vector<string> paths;
    for (const auto &entry : filesystem::directory_iterator(_SAVE_FOLDER_CONFIG_PATH))
    {
        string pathStr = entry.path();
        if (pathStr.find(deviceName) != string::npos)
            paths.push_back(entry.path());
    }

    return paths;
}

/**
 * @brief Gets the device corresponding to
 * the path of a configuration.
 *
 * @param configPath path of the configuration
 *
 * @return device path
 */
string DeviceOf(const string &configPath)
{
    return V4L_PATHS_DIR + deviceNameOf(configPath);
}

/**
 * @brief Return all v4l devices
 *
 * @return path to the v4l device
 */
vector<string> GetV4LDevices()
{
    vector<string> devices;
    auto paths = filesystem::directory_iterator(V4L_PATHS_DIR);
    for (const auto &entry : paths)
        devices.push_back(entry.path());

    return devices;
}

static void signalHandler(int signal)
{
    if (signal == SIGINT)
    {
        static int ctrlc_counter = 0;
        ++ctrlc_counter;
        if (ctrlc_counter == 2)
            exit(ExitCode::FAILURE);
        cout << " Ctrl-c again if you really want to, be careful this could break the camera." << endl;
    }
}

/**
 * @brief Catch ctrl-c signal one time.
 */
void CatchCtrlC()
{
    signal(SIGINT, signalHandler);
}
