#include "globals.hpp"

#include <filesystem>
#include <string>
#include <vector>
using namespace std;

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
string configPathOf(const string &device)
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
vector<string> getConfigPaths(const string &device)
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
string deviceOf(const string &configPath)
{
    return "/dev/v4l/by-path/" + deviceNameOf(configPath);
}

/**
 * @brief Return all v4l devices
 *
 * @return path to the v4l device
 */
vector<string> getV4LDevices()
{
    vector<string> devices;
    for (const auto &entry : filesystem::directory_iterator("/dev/v4l/by-path"))
        devices.push_back(entry.path());

    return devices;
}
