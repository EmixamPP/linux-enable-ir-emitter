#include "commands.hpp"

#include <fstream>
#include <memory>
using namespace std;

#include "globals.hpp"
#include "camera/camera.hpp"
#include "camera/autocamera.hpp"
#include "camera/camerainstruction.hpp"
#include "configuration/finder.hpp"
#include "configuration/scanner.hpp"
#include "utils/logger.hpp"
#include "utils/configuration.hpp"

void enableDebug()
{
    Logger::enableDebug();
}

/**
 * @brief Finds a configuration for an infrared camera which enables its emitter(s).
 *
 * @param device path to the infrared camera, empty string for automatic detection
 * @param manual true for enabling the manual configuration
 * @param emitters number of emitters on the device
 * @param negAnswerLimit number of negative answer before the pattern is skiped. Use -1 for unlimited
 * @param noGui no gui video feedback
 *
 * @return exit code
 */
ExitCode configure(const char *device_char_p, bool manual, unsigned emitters, unsigned negAnswerLimit, bool noGui)
{
    const string device_string = string(device_char_p);

    Logger::info("Stand in front of and close to the camera and make sure the room is well lit.");
    Logger::info("Ensure to not use the camera during the execution."); // TODO catch ctrl-c
    Logger::info("Warning to do not kill the process !");

    shared_ptr<Camera> camera;
    if (manual)
    {
        if (device_string.empty())
            camera = Camera::findGrayscaleCamera();
        else
            camera = make_shared<Camera>(device_string);
    }
    else
    {
        if (device_string.empty())
            camera = AutoCamera::findGrayscaleCamera();
        else
            camera = make_shared<AutoCamera>(device_string);
    }

    if (noGui)
        camera->disableGui();

    if (camera == nullptr)
        Logger::critical(ExitCode::FAILURE, "Impossible to find an infrared camera.");

    Logger::info("Configuring the camera:", camera->device, ".");

    vector<CameraInstruction> instructions = Configuration::load(camera->device);
    if (instructions.empty())
    {
        Scanner scanner(*camera);
        instructions = scanner.scan();
    }

    Finder finder(*camera, emitters, negAnswerLimit);

    bool success = false;
    try
    {
        if (camera->Camera::isEmitterWorking())
        {
            Logger::error("Your emiter is already working, skipping the configuration.");
            return ExitCode::FAILURE;
        }

        success = finder.find(instructions);
    }
    catch (CameraException &e)
    {
        Configuration::save(camera->device, instructions);
        Logger::critical(ExitCode::FILE_DESCRIPTOR_ERROR, e.what());
    }

    Configuration::save(camera->device, instructions);

    if (!success)
    {
        Logger::error("The configuration has failed.");
        Logger::error("Please retry in manual mode by adding the '-m' option.");
        Logger::info("Do not hesitate to visit the GitHub !");
        Logger::info("https://github.com/EmixamPP/linux-enable-ir-emitter/blob/master/docs/README.md");
        return ExitCode::FAILURE;
    }

    Logger::info("The emitter has been successfully configured.");
    return ExitCode::SUCCESS;
}
