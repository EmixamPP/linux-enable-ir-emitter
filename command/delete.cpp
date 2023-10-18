#include "commands.hpp"

#include <filesystem>
#include <string>
using namespace std;

#include "globals.hpp"
#include "../utils/logger.hpp"

/**
 * @brief Delete the driver associated to a device,
 * without causing error if the driver does not exists.
 *
 * @param path to the infrared camera, empty string to execute all driver
 *
 * @return exit code
 */
ExitCode delete_driver(const char* device)
{
    auto drivers = get_drivers_path(device);
    for (auto &driver : *drivers)
        filesystem::remove(driver);

    Logger::info("The drivers have been deleted.");
    return ExitCode::SUCCESS;
}
