#include "commands.hpp"

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
ExitCode test(const char* device_char_p)
{   
    const string device_string = string(device_char_p);
    
    shared_ptr<AutoCamera> camera;
    if (device_string.empty())
    {
        camera = AutoCamera::findGrayscaleCamera();
        if (camera == nullptr)
            Logger::critical(ExitCode::FAILURE, "Impossible to find an infrared camera.");
    }
    else
        camera = make_shared<AutoCamera>(device_string);

    if (camera->isGrayscale())
        Logger::info("The camera", camera->device, "is in gray scale. This is probably your infrared camera.");
    else
        Logger::error("The camera", camera->device, "is not in gray scale. This is probably your regular camera.");

    try
    {
        camera->play();
    }
    catch (CameraException &e)
    {
        Logger::critical(ExitCode::FILE_DESCRIPTOR_ERROR, e.what());
    }

    return ExitCode::SUCCESS;
}
