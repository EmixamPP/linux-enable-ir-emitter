#include "commands.hpp"

#include <vector>
using namespace std;

#include "camera/camera.hpp"
#include "camera/camerainstruction.hpp"
#include "utils/logger.hpp"
#include "utils/configuration.hpp"

/**
 * @brief Execute a configuration.
 *
 * @param device path to the infrared camera, empty string to execute all configurations
 *
 * @return exit code
 */
ExitCode run(const char *device)
{
    Logger::debug("Executing run command.");

    vector<string> paths = GetConfigPaths(device);

    if (paths.empty())
        Logger::critical(ExitCode::FAILURE, "No configuration has been found.");

    bool oneFailure = false;
    for (auto &path : paths)
    {
        auto instructions = Configuration::Load(path);
        string device = DeviceOf(path);
        Camera camera(device);

        try
        {
            for (auto &instruction : instructions.value())
            {
                Logger::debug("Applying instruction", to_string(instruction), "on", device);
                if (!camera.apply(instruction))
                {
                    Logger::warning("Failed to apply the instruction", to_string(instruction));
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
