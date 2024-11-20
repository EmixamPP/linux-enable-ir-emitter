#include "commands.hpp"

ExitCode run(const optional<string> &device, int width, int height) {
  auto res = ExitCode::SUCCESS;
  try {
    logger::debug("Executing run command.");

    Configurations confs;
    if (device.has_value()) {
      CameraPtr camera = CreateCamera<Camera>(device, width, height);
      confs.push_back(Configuration(camera, false));
    } else {
      confs = Configuration::ConfiguredDevices();
    }

    if (confs.empty()) {
      logger::warn("No device has been configured.");
    }

    for (const auto &conf : confs) {
      for (const auto &instruction : conf) {
        if (instruction.status() == CameraInstruction::Status::START) {
          logger::debug("Applying instruction {} on {}.", to_string(instruction),
                        conf.camera->device());
          if (!conf.camera->apply(instruction)) {
            logger::warn("Failed to apply the instruction.");
            res = ExitCode::FAILURE;
          }
        }
      }
    }

  } catch (const std::exception &e) {
    logger::error(e.what());
  }
  return res;
}
