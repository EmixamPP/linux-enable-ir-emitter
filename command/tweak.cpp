#include "commands.hpp"

#include "camera/camera.hpp"
#include "configuration/scanner.hpp"
#include "configuration/tweaker.hpp"
#include "configuration.hpp"
#include "utils/logger.hpp"

/**
 * @brief Let the user modify the configuration of a camera
 *
 * @param device path to the infrared camera, nothing for automatic detection
 * @param width of the capture resolution
 * @param height of the capture resolution
 *
 * @return exit code
 */
ExitCode tweak(const optional<string> &device, int width, int height)
{
    Logger::debug("Executing tweak command.");

    bool saved = false;
    try
    {
        auto camera = CreateCamera<Camera>(device, width, height);

        Logger::info("Tweaking the camera", camera->device());
        Logger::info("Caution, you could break the camera.");

        auto instructions = Configuration::Load(camera->device());
        if (!instructions)
        {
            Logger::debug("No previous configuration found.");
            Scanner scanner(camera);
            instructions = scanner.scan();
            Configuration::Save(camera->device(), instructions.value());
        }
        else
            Logger::debug("Previous configuration found.");

        Tweaker tweaker(camera);

        tweaker.tweak(instructions.value());

        saved = Configuration::Save(camera->device(), instructions.value());
    }
    catch (const CameraException &e)
    {
        Logger::critical(ExitCode::FILE_DESCRIPTOR_ERROR, e.what());
    }

    return saved ? ExitCode::SUCCESS : ExitCode::FAILURE;
}