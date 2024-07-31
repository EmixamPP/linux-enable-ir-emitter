#include <fstream>
#include <memory>
using namespace std;

#include <spdlog/spdlog.h>

#include "camera/autocamera.hpp"
#include "camera/camerainstruction.hpp"
#include "commands.hpp"
#include "configuration/finder.hpp"
#include "configuration/scanner.hpp"

ExitCode configure(const optional<string> &device, int width, int height, bool manual,
                   unsigned emitters, unsigned neg_answer_limit, bool no_gui) {
  spdlog::debug("Executing configure command.");
  spdlog::info("Stand in front of and close to the camera and make sure the room is well lit.");
  spdlog::info("Ensure to not use the camera during the execution.");

  bool success = false;
  try {
    shared_ptr<Camera> camera;
    if (manual)
      camera = CreateCamera<Camera>(device, width, height, no_gui);
    else
      camera = CreateCamera<AutoCamera>(device, width, height, no_gui);

    spdlog::info("Configuring the camera {}.", camera->device());

    auto instructions = Configuration::LoadInit(camera->device());
    if (!instructions) {
      spdlog::debug("No previous configuration found.");
      Scanner scanner(camera);
      instructions = scanner.scan();
      Configuration::SaveInit(camera->device(), instructions.value());
    } else
      spdlog::debug("Previous configuration found.");

    Finder finder(camera, emitters, neg_answer_limit);

    if (camera->Camera::is_emitter_working()) {
      spdlog::error("The emitter is already working, skipping the configuration.");
      return ExitCode::FAILURE;
    }

    success = finder.find(instructions.value());
    success = success && Configuration::Save(camera->device(), instructions.value());
  } catch (const CameraException &e) {
    spdlog::critical(e.what());
    exit(ExitCode::FILE_DESCRIPTOR_ERROR);
  }

  if (!success) {
    spdlog::error("The configuration failed.");
    spdlog::info("Please retry in manual mode by adding the '-m' option.");
    spdlog::info("Do not hesitate to visit the GitHub!");
    spdlog::info("https://github.com/EmixamPP/linux-enable-ir-emitter/blob/master/docs/README.md");
    return ExitCode::FAILURE;
  }

  spdlog::info("The infrared camera has been successfully configured.");
  return ExitCode::SUCCESS;
}
