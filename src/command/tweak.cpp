#include "commands.hpp"
#include "configuration/tools.hpp"

ExitCode tweak(const optional<string> &device, int width, int height) {
  try {
    logger::debug("Executing tweak command.");

    auto camera = CreateCamera<Camera>(device, width, height);
    Configuration config(camera, true);

    logger::info("Tweaking the camera {}", camera->device());
    logger::info("Caution, you could break the camera.");
    Tools::Tweak(config);

    return ExitCode::SUCCESS;

  } catch (const std::exception &e) {
    logger::error(e.what());
  }
  return ExitCode::FAILURE;
}