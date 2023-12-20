#include "commands.hpp"

#include "camera/camera.hpp"
#include "configuration/scanner.hpp"
#include "configuration/tweaker.hpp"
#include "utils/configuration.hpp"
#include "utils/logger.hpp"

#include <memory>
#include <string>
using namespace std;

static shared_ptr<Camera> makeCamera(const string &device)
{
    shared_ptr<Camera> camera;

    if (device.empty())
    {
        camera = Camera::findGrayscaleCamera();
        if (camera == nullptr)
            Logger::critical(ExitCode::FAILURE, "Impossible to find an infrared camera.");
    }
    else
    {
        camera = make_shared<Camera>(device);
    }

    return camera;
}

ExitCode tweak(const char *device_char_p)
{
    shared_ptr<Camera> camera = makeCamera(string(device_char_p));

    Logger::info("Tweaking the camera:", camera->device, ".");

    vector<CameraInstruction> instructions = Configuration::load(camera->device);
    if (instructions.empty())
    {
        Scanner scanner(*camera);
        instructions = scanner.scan();
    }

    Tweaker tweaker(*camera);
    tweaker.tweak(instructions);

    Configuration::save(camera->device, instructions);

    return ExitCode::SUCCESS;
}