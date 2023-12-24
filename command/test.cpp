#include "commands.hpp"

#include <memory>
#include <string>
using namespace std;

#include "camera/autocamera.hpp"
#include "globals.hpp"
#include "utils/logger.hpp"

/**
 * @brief Test if the camera is in grayscale and if the emitter is working.
 * Also display a video feedback.
 *
 * @param path to the infrared camera, empty string for automatic detection
 *
 * @return exit code
 */
ExitCode test(const char *device_char_p, int width, int height)
{
    auto camera = makeCamera<Camera>(string(device_char_p), width, height);

    if (camera->isGrayscale())
        Logger::info("The camera", camera->device, "is in gray scale. This is probably your infrared camera.");
    else
        Logger::warning("The camera", camera->device, "is not in gray scale. This is probably your regular camera.");

    try
    {
        camera->playForever();
    }
    catch (const CameraException &e)
    {
        Logger::critical(ExitCode::FILE_DESCRIPTOR_ERROR, e.what());
    }

    return ExitCode::SUCCESS;
}
