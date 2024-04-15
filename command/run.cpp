#include "commands.hpp"

#include <algorithm>
#include <vector>
using namespace std;

#include "camera/camera.hpp"
#include "camera/camerainstruction.hpp"
#include "utils/logger.hpp"
#include "configuration.hpp"

/**
 * @brief Execute a configuration.
 *
 * @param device path to the camera, empty string to execute all configurations
 *
 * @return exit code
 */
ExitCode run(const string &device)
{
    Logger::debug("Executing run command.");

    auto devices = Configuration::ConfiguredDevices();

    if (devices.empty()) {
        Logger::warning("No device has been configured.");
        return ExitCode::SUCCESS;
    }
    else if (!device.empty())
        devices = {device};

    bool oneFailure = false;
    for (const auto &device : devices)
    {
        auto instructions = Configuration::Load(device);
        if (!instructions)
        {
            oneFailure = true;
            Logger::error("Failed to load a configuration for", device);
            continue;
        }

        Camera camera(device);

        try
        {
            for (const auto &instruction : instructions.value())
            {
                Logger::debug("Applying instruction", to_string(instruction), "on", device);
                if (!camera.apply(instruction))
                {
                    Logger::error("Failed to apply the instruction", to_string(instruction));
                    oneFailure = true;
                }
            }
        }
        catch (const CameraException &e)
        {
            Logger::critical(ExitCode::FILE_DESCRIPTOR_ERROR, e.what());
        }
    }

    return oneFailure ? ExitCode::FAILURE : ExitCode::SUCCESS;
}
