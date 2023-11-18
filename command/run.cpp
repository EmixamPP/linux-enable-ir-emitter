#include "commands.hpp"

#include "camera/camera.hpp"
#include "camera/camerainstruction.hpp"
#include "utils/logger.hpp"
#include "utils/serializer.hpp"

/**
 * @brief Execute a configuration.
 *
 * @param device path to the infrared camera, empty string to execute all configurations
 *
 * @return exit code
 */
ExitCode run(const char *device)
{

    vector<string> paths = getConfigPaths(device);

    if (paths.empty())
        Logger::critical(ExitCode::FAILURE, "No configuration for", device, "has been configured.");

    ExitCode code = ExitCode::SUCCESS;
    for (auto &configPath : paths)
    {
        vector<CameraInstruction> configuration = Serializer::readConfigFromFile(configPath);
        string device = deviceOf(configPath);

        Camera camera(device);
        try
        {   
            for (auto &instruction : configuration)
            {
                if (!camera.apply(instruction))
                {
                    Logger::error("Failed to apply the configuration of", device);
                    code = ExitCode::FAILURE;
                }
            }
        }
        catch (CameraException &e)
        {
            Logger::critical(ExitCode::FILE_DESCRIPTOR_ERROR, e.what());
        }
    }

    return code;
}
