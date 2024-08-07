#include "camera/camera.hpp"
#include "commands.hpp"
#include "configuration.hpp"
#include "configuration/scanner.hpp"
#include "configuration/tweaker.hpp"
#include "logger.hpp"

ExitCode tweak(const optional<string> &device, int width, int height) {
  logger::debug("Executing tweak command.");

  bool saved = false;
  try {
    auto camera = CreateCamera<Camera>(device, width, height);

    logger::info("Tweaking the camera {}", camera->device());
    logger::info("Caution, you could break the camera.");

    auto instructions = Configuration::Load(camera->device());
    if (!instructions) {
      logger::debug("No previous configuration found.");
      Scanner scanner(camera);
      instructions = scanner.scan();
      Configuration::Save(camera->device(), instructions.value());
    } else
      logger::debug("Previous configuration found.");

    Tweaker tweaker(camera);

    tweaker.tweak(instructions.value());

    saved = Configuration::Save(camera->device(), instructions.value());
  } catch (const CameraException &e) {
    logger::critical(e.what());
    exit(ExitCode::FILE_DESCRIPTOR_ERROR);
  }

  return saved ? ExitCode::SUCCESS : ExitCode::FAILURE;
}