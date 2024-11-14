#include "commands.hpp"

ExitCode test(const optional<string> &device, int width, int height) {
  try {
    logger::debug("Executing test command.");

    auto camera = CreateCamera<Camera>(device, width, height);
    if (camera->is_gray_scale())
      logger::info("The camera {} is in grey scale. This is probably your infrared camera.",
                   camera->device());
    else
      logger::warn("The camera {} is not in grey scale. This is probably your regular camera.",
                   camera->device());

    Camera::ExceptionPtr eptr;
    auto wait = camera->play(eptr, true);
    wait();
    if (eptr) {
      logger::error(eptr->what());
      return ExitCode::FAILURE;
    }

    return ExitCode::SUCCESS;

  } catch (const Camera::Exception &e) {
    logger::error(e.what());
  }
  return ExitCode::FAILURE;
}
