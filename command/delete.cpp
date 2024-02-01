#include "commands.hpp"

#include <filesystem>
#include <string>
#include <vector>
using namespace std;

#include "utils/logger.hpp"
#include "configuration.hpp"

/**
 * @brief Deletes the config associated to a device,
 * without causing error if the config does not exists.
 *
 * @param device path to the camera,
 * empty to delete all configs
 *
 * @return exit code
 */
ExitCode delete_config(const char *device)
{
    Logger::debug("Executing delete command.");

    if (string(device).empty())
    {
        auto devices = Configuration::ConfiguredDevices();
        for (const auto &device: devices)
            Configuration::Delete(device);
    }
    else
        Configuration::Delete(device);

    Logger::info("The configurations have been deleted.");
    return ExitCode::SUCCESS;
}
