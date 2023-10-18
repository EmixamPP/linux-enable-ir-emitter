#include "globals.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>
using namespace std;

#include <iostream>
/**
 * @brief Get the drivers path corresponding to all configured device
 * or just to one specific
 *
 * @param device path to the a specific infrared camera.
 * Empty string to get all drivers path
 *
 * @return path(s) to the driver(s)
 */
shared_ptr<vector<string>> get_drivers_path(const string &device)
{
    string deviceName = "";
    if (!device.empty())
    {
        size_t pos = device.rfind("/");
        if (pos != string::npos)
            deviceName = device.substr(pos + 1);
    }

    auto drivers = make_shared<vector<string>>();
    for (auto &entry : filesystem::directory_iterator(SAVE_DRIVER_FOLDER_PATH))
    {
        string pathStr = entry.path();
        if (pathStr.find(deviceName) != string::npos)
            drivers->push_back(entry.path());
    }

    return drivers;
}

/**
 * @brief Return all v4l devices
 *
 * @return path to the v4l device
 */
shared_ptr<vector<string>> get_v4l_devices()
{
    auto devices = make_shared<vector<string>>();
    for (auto &entry : filesystem::directory_iterator("/dev/v4l/by-path"))
        devices->push_back(entry.path());

    return devices;
}
