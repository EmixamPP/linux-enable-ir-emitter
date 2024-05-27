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
#include "configuration.hpp"

/**
 * @brief Finds a configuration for an infrared camera which enables its emitter(s).
 *
 * @param device path of the infrared camera, nothing for automatic detection
 * @param width of the capture resolution
 * @param height of the capture resolution
 * @param manual true for enabling the manual configuration
 * @param emitters number of emitters on the device
 * @param neg_answer_limit number of negative answer before the pattern is skiped. Use -1 for unlimited
 * @param no_gui no gui video feedback
 *
 * @return exit code
 */
ExitCode configure(const optional<string> &device, int width, int height,
                   bool manual, unsigned emitters, unsigned neg_answer_limit, bool no_gui)
{
    Logger::debug("Executing configure command.");

    Logger::info("Stand in front of and close to the camera and make sure the room is well lit.");
    Logger::info("Ensure to not use the camera during the execution.");
    Logger::info("Do not kill the process, unless you really need to, and only use ctrl-c.");

    bool success = false;
    try
    {
        shared_ptr<Camera> camera;
        if (manual)
            camera = CreateCamera<Camera>(device, width, height, no_gui);
        else
            camera = CreateCamera<AutoCamera>(device, width, height, no_gui);

        Logger::info("Configuring the camera", camera->device());

        auto instructions = Configuration::LoadInit(camera->device());
        if (!instructions)
        {
            Logger::debug("No previous configuration found.");
            Scanner scanner(camera);
            instructions = scanner.scan();
            Configuration::SaveInit(camera->device(), instructions.value());
        }
        else
            Logger::debug("Previous configuration found.");

        Finder finder(camera, emitters, neg_answer_limit);

        if (camera->Camera::is_emitter_working())
        {
            Logger::error("The emiter is already working, skipping the configuration.");
            return ExitCode::FAILURE;
        }

        success = finder.find(instructions.value());
        success = success && Configuration::Save(camera->device(), instructions.value());
    }
    catch (const CameraException &e)
    {
        Logger::critical(ExitCode::FILE_DESCRIPTOR_ERROR, e.what());
    }

    if (!success)
    {
        Logger::error("The configuration failed.");
        Logger::info("Please retry in manual mode by adding the '-m' option.");
        Logger::info("Do not hesitate to visit the GitHub!");
        Logger::info("https://github.com/EmixamPP/linux-enable-ir-emitter/blob/master/docs/README.md");
        return ExitCode::FAILURE;
    }

    Logger::info("The infrared camera has been successfully configured.");
    return ExitCode::SUCCESS;
}
