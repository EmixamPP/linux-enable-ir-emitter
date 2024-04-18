#include "commands.hpp"

#include "camera/autocamera.hpp"
#include "utils/logger.hpp"

/**
 * @brief Test if the camera is in greyscale and if the emitter is working.
 * Also display a video feedback.
 *
 * @param device path to the infrared camera, nothing for automatic detection
 * @param width of the capture resolution
 * @param height of the capture resolution
 *
 * @return exit code
 */
ExitCode test(const optional<string> &device, int width, int height)
{
    Logger::debug("Executing test command.");

    try
    {
        auto camera = CreateCamera<Camera>(device, width, height);

        if (camera->is_gray_scale())
            Logger::info("The camera", camera->device(), "is in grey scale. This is probably your infrared camera.");
        else
            Logger::warning("The camera", camera->device(), "is not in grey scale. This is probably your regular camera.");

        camera->play_forever();
    }
    catch (const CameraException &e)
    {
        Logger::critical(ExitCode::FILE_DESCRIPTOR_ERROR, e.what());
    }

    return ExitCode::SUCCESS;
}
