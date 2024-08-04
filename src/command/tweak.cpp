#include <spdlog/spdlog.h>

#include "camera/camera.hpp"
#include "commands.hpp"
#include "configuration.hpp"
#include "configuration/scanner.hpp"
#include "configuration/tweaker.hpp"

ExitCode tweak(const optional<string> &device, int width, int height) {
  spdlog::debug("Executing tweak command.");

  bool saved = false;
  try {
    auto camera = CreateCamera<Camera>(device, width, height);

    spdlog::info("Tweaking the camera {}", camera->device());
    spdlog::info("Caution, you could break the camera.");

    auto instructions = Configuration::Load(camera->device());
    if (!instructions) {
      spdlog::debug("No previous configuration found.");
      Scanner scanner(camera);
      instructions = scanner.scan();
      Configuration::Save(camera->device(), instructions.value());
    } else
      spdlog::debug("Previous configuration found.");

    Tweaker tweaker(camera);

    tweaker.tweak(instructions.value());

    saved = Configuration::Save(camera->device(), instructions.value());
  } catch (const CameraException &e) {
    spdlog::critical(e.what());
    exit(ExitCode::FILE_DESCRIPTOR_ERROR);
  }

  return saved ? ExitCode::SUCCESS : ExitCode::FAILURE;
}