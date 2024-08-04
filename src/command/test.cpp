#include <spdlog/spdlog.h>

#include "camera/autocamera.hpp"
#include "commands.hpp"

ExitCode test(const optional<string> &device, int width, int height) {
  spdlog::debug("Executing test command.");

  try {
    auto camera = CreateCamera<Camera>(device, width, height);

    if (camera->is_gray_scale())
      spdlog::info("The camera {} is in grey scale. This is probably your infrared camera.",
                   camera->device());
    else
      spdlog::warn("The camera {} is not in grey scale. This is probably your regular camera.",
                   camera->device());

    camera->play_forever();
  } catch (const CameraException &e) {
    spdlog::critical(e.what());
    exit(ExitCode::FILE_DESCRIPTOR_ERROR);
  }

  return ExitCode::SUCCESS;
}
