#include "commands.hpp"

#include "camera/camera.hpp"
#include "configuration/scanner.hpp"
#include "configuration/tweaker.hpp"
#include "utils/configuration.hpp"
#include "utils/logger.hpp"

#include <memory>
#include <string>
using namespace std;

ExitCode tweak(const char *device_char_p, int width, int height)
{
    Logger::debug("Executing tweak command.");

    catch_ctrl_c();

    auto camera = makeCamera<Camera>(string(device_char_p), width, height);

    Logger::info("Tweaking the camera:", camera->device);
    Logger::info("Caution, you could break the camera.");

    auto instructions = Configuration::load(camera->device);
    if (!instructions)
    {
        Logger::debug("No previous configuration found.");
        Scanner scanner(*camera);
        instructions = scanner.scan();
    }
    else
        Logger::debug("Previous configuration found.");

    Tweaker tweaker(*camera);
    try
    {
        tweaker.tweak(instructions.value());
    }
    catch (const CameraException &e)
    {
        Configuration::save(camera->device, instructions.value());
        Logger::critical(ExitCode::FILE_DESCRIPTOR_ERROR, e.what());
    }

    Configuration::save(camera->device, instructions.value());

    return ExitCode::SUCCESS;
}