#include <fstream>
#include <memory>
using namespace std;

#include "camera/autocamera.hpp"
#include "camera/camerainstruction.hpp"
#include "commands.hpp"
#include "configuration/finder.hpp"
#include "configuration/scanner.hpp"
#include "logger.hpp"

ExitCode configure(const optional<string> &device, int width, int height, bool manual,
                   unsigned emitters, unsigned neg_answer_limit, bool no_gui) {
  logger::debug("Executing configure command.");
  logger::info("Stand in front of and close to the camera and make sure the room is well lit.");
  logger::info("Ensure to not use the camera during the execution.");

  bool success = false;
  try {
    shared_ptr<Camera> camera;
    if (manual)
      camera = CreateCamera<Camera>(device, width, height, no_gui);
    else
      camera = CreateCamera<AutoCamera>(device, width, height, no_gui);

    logger::info("Configuring the camera {}.", camera->device());

    auto instructions = Configuration::LoadInit(camera->device());
    if (!instructions) {
      logger::debug("No previous configuration found.");
      Scanner scanner(camera);
      instructions = scanner.scan();
      Configuration::SaveInit(camera->device(), instructions.value());
    } else
      logger::debug("Previous configuration found.");

    Finder finder(camera, emitters, neg_answer_limit);

    if (camera->Camera::is_emitter_working()) {
      logger::error("The emitter is already working, skipping the configuration.");
      return ExitCode::FAILURE;
    }

    success = finder.find(instructions.value());
    success = success && Configuration::Save(camera->device(), instructions.value());
  } catch (const CameraException &e) {
    logger::critical(e.what());
    exit(ExitCode::FILE_DESCRIPTOR_ERROR);
  }

  if (!success) {
    logger::error("The configuration failed.");
    logger::info("Please retry in manual mode by adding the '-m' option.");
    logger::info("Do not hesitate to visit the GitHub!");
    logger::info("https://github.com/EmixamPP/linux-enable-ir-emitter/blob/master/docs/README.md");
    return ExitCode::FAILURE;
  }

  logger::info("The infrared camera has been successfully configured.");
  return ExitCode::SUCCESS;
}
