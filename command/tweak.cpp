#include "commands.hpp"

#include "camera/camera.hpp"
#include "configuration/scanner.hpp"
#include "configuration/tweaker.hpp"
#include "configuration.hpp"
#include "utils/logger.hpp"

#include <memory>
#include <string>
using namespace std;

ExitCode tweak(const string &device, int width, int height)
{
    Logger::debug("Executing tweak command.");

    CatchCtrlC();

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