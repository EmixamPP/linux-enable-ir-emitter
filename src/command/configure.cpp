#include <fstream>
#include <memory>
using namespace std;

#include "camera/autocamera.hpp"
#include "commands.hpp"
#include "configuration/tools.hpp"

ExitCode configure(const optional<string> &device, int width, int height, bool manual,
                   unsigned emitters, unsigned neg_answer_limit, bool no_gui) {
  auto res = ExitCode::FAILURE;
  try {
    logger::debug("Executing configure command.");
    logger::info("Stand in front of and close to the camera and make sure the room is well lit.");
    logger::info("Ensure to not use the camera during the execution.");

    CameraPtr camera;
    if (manual)
      camera = CreateCamera<Camera>(device, width, height, no_gui);
    else
      camera = CreateCamera<AutoCamera>(device, width, height, no_gui);
    logger::info("Configuring the camera {}.", camera->device());

    if (camera->Camera::is_emitter_working()) {
      logger::error("The emitter is already working, skipping the configuration.");
      return ExitCode::FAILURE;
    }

    Configuration config(camera, true);
    if (!Tools::Find(config, emitters, neg_answer_limit)) {
      logger::info("Please retry in manual mode by adding the '-m' option.");
    } else {
      logger::info("The infrared camera has been successfully configured.");
      res = ExitCode::SUCCESS;
    }

  } catch (const std::exception &e) {
    logger::error(e.what());
  }

  if (res != ExitCode::SUCCESS) {
    logger::error("The configuration failed.");
    logger::info("Do not hesitate to visit the GitHub!");
    logger::info("https://github.com/EmixamPP/linux-enable-ir-emitter/blob/master/docs/README.md");
  }
  return res;
}
