#include "commands.hpp"

#include <fstream>
#include <memory>
using namespace std;

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

static shared_ptr<Camera> makeCamera(const string &device, bool manual, bool noGui)
{
    shared_ptr<Camera> camera;
    if (manual)
    {
        if (device.empty())
            camera = Camera::findGrayscaleCamera();
        else
            camera = make_shared<Camera>(device);
    }
    else
    {
        if (device.empty())
            camera = AutoCamera::findGrayscaleCamera();
        else
            camera = make_shared<AutoCamera>(device);
    }

    if (camera == nullptr)
        Logger::critical(ExitCode::FAILURE, "Impossible to find an infrared camera.");

    if (noGui)
        camera->disableGui();
    
    return camera;
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
    catch_ctrl_c();

    Logger::info("Stand in front of and close to the camera and make sure the room is well lit.");
    Logger::info("Ensure to not use the camera during the execution."); // TODO catch ctrl-c
    Logger::info("Warning to do not kill the process !");

    shared_ptr<Camera> camera = makeCamera(string(device_char_p), manual, noGui);
    Logger::info("Configuring the camera:", camera->device);

    auto instructions = Configuration::load(camera->device);
    if (!instructions)
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
            Logger::error("The emiter is already working, skipping the configuration.");
            return ExitCode::FAILURE;
        }

        success = finder.find(instructions.value());
    }
    catch (const CameraException &e)
    {
        Configuration::save(camera->device, instructions.value());
        Logger::critical(ExitCode::FILE_DESCRIPTOR_ERROR, e.what());
    }

    Configuration::save(camera->device, instructions.value());

    if (!success)
    {
        Logger::error("The configuration has failed.");
        Logger::info("Please retry in manual mode by adding the '-m' option.");
        Logger::info("Do not hesitate to visit the GitHub !");
        Logger::info("https://github.com/EmixamPP/linux-enable-ir-emitter/blob/master/docs/README.md");
        return ExitCode::FAILURE;
    }

    Logger::info("The emitter has been successfully configured.");
    return ExitCode::SUCCESS;
}
