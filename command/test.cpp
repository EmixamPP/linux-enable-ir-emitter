#include "commands.hpp"

#include <spdlog/spdlog.h>

#include "camera/autocamera.hpp"

/**
 * @brief Test if the camera is in grayscale and if the emitter is working.
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
    spdlog::debug("Executing test command.");

    try
    {
        auto camera = CreateCamera<Camera>(device, width, height);

        if (camera->is_gray_scale())
            spdlog::info("The camera {} is in grey scale. This is probably your infrared camera.", camera->device());
        else
            spdlog::warn("The camera {} is not in grey scale. This is probably your regular camera.", camera->device());

        camera->play_forever();
    }
    catch (const CameraException &e)
    {
        spdlog::critical(e.what());
        exit(ExitCode::FILE_DESCRIPTOR_ERROR);
    }

    return ExitCode::SUCCESS;
}
