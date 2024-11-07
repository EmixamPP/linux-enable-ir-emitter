#include "camera/autocamera.hpp"
#include "commands.hpp"
#include "logger.hpp"

ExitCode test(const optional<string> &device, int width, int height) {
  logger::debug("Executing test command.");

  try {
    auto camera = CreateCamera<Camera>(device, width, height);

    if (camera->is_gray_scale())
      logger::info("The camera {} is in grey scale. This is probably your infrared camera.",
                   camera->device());
    else
      logger::warn("The camera {} is not in grey scale. This is probably your regular camera.",
                   camera->device());

    camera->play_wait().get();

  } catch (const CameraException &e) {
    logger::critical(e.what());
    exit(ExitCode::FILE_DESCRIPTOR_ERROR);
  }

  return ExitCode::SUCCESS;
}
