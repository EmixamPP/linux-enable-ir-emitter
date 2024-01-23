#include "commands.hpp"

#include <memory>
#include <string>
using namespace std;

#include "camera/autocamera.hpp"
#include "globals.hpp"
#include "utils/logger.hpp"

/**
 * @brief Test if the camera is in greyscale and if the emitter is working.
 * Also display a video feedback.
 *
 * @param device_char_p path to the infrared camera, empty string for automatic detection
 * @param width of the capture resolution
 * @param height of the capture resolution
 *
 * @return exit code
 */
ExitCode test(const char *device_char_p, int width, int height)
{
    Logger::debug("Executing test command.");

    auto camera = MakeCamera<Camera>(string(device_char_p), width, height);

    if (camera->is_gray_scale())
        Logger::info("The camera", camera->device, "is in grey scale. This is probably your infrared camera.");
    else
        Logger::warning("The camera", camera->device, "is not in grey scale. This is probably your regular camera.");

    try
    {
        camera->play_forever();
    }
    catch (const CameraException &e)
    {
        Logger::critical(ExitCode::FILE_DESCRIPTOR_ERROR, e.what());
    }

    return ExitCode::SUCCESS;
}
