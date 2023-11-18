#include "globals.hpp"

#include <filesystem>
#include <string>
#include <vector>
using namespace std;

#include <iostream>

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
    size_t pos = configPath.rfind('/');
    return "/dev/v4l/by-path/" + configPath.substr(pos + 1);
}

string deviceNameOf(const string &device)
{
    size_t pos = device.rfind('/');
    return device.substr(pos + 1);
}

/**
 * @brief Gets the path corresponding to the scan
 * of a device.
 *
 * @param deviceName name of the device
 *
 * @return path to the file
 */
string scanPathOf(const string &deviceName)
{
    return _SAVE_FOLDER_SCAN_PATH + deviceName;
}

/**
 * @brief Gets the path corresponding to the configuration
 * of a device.
 *
 * @param deviceName name of the device
 *
 * @return path to the file
 */
string configPathOf(const string &deviceName){
    return _SAVE_FOLDER_CONFIG_PATH + deviceName;
}

/**
 * @brief Get the configurations path corresponding to all configured device
 * or just to one specific
 *
 * @param deviceName name of a specific device
 * Empty string to get all configurations path
 *
 * @return path(s) to the configuration(s)
 */
vector<string> getConfigPaths(const string &deviceName)
{
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
