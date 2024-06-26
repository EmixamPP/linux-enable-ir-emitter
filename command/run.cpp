#include "commands.hpp"

#include <algorithm>
#include <vector>
using namespace std;

#include <spdlog/spdlog.h>

#include "camera/camera.hpp"
#include "camera/camerainstruction.hpp"
#include "configuration.hpp"

/**
 * @brief Execute a configuration.
 *
 * @param device path of the camera, nothing to execute all configurations
 * @param width of the capture resolution
 * @param height of the capture resolution
 *
 * @return exit code
 */
ExitCode run(const optional<string> &device, int width, int height)
{
    spdlog::debug("Executing run command.");

    auto devices = Configuration::ConfiguredDevices();

    if (devices.empty())
    {
        spdlog::warn("No device has been configured.");
        return ExitCode::SUCCESS;
    }
    else if (device.has_value())
        devices = {device.value()};

    bool oneFailure = false;
    for (const auto &device : devices)
    {
        auto instructions = Configuration::Load(device);
        if (!instructions)
        {
            oneFailure = true;
            spdlog::error("Failed to load a configuration for {}.", device);
            continue;
        }

        Camera camera(device, width, height);

        try
        {
            for (const auto &instruction : instructions.value())
            {
                if (!instruction.is_disable())
                {
                    spdlog::debug("Applying instruction {} on {}.", to_string(instruction), device);
                    if (!camera.apply(instruction))
                    {
                        spdlog::error("Failed to apply the instruction {}.", to_string(instruction));
                        oneFailure = true;
                    }
                }
            }
        }
        catch (const CameraException &e)
        {
            spdlog::critical(e.what());
            exit(ExitCode::FILE_DESCRIPTOR_ERROR);
        }
    }

    return oneFailure ? ExitCode::FAILURE : ExitCode::SUCCESS;
}
