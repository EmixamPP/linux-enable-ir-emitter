#include <algorithm>
#include <vector>
using namespace std;

#include "camera/camerainstruction.hpp"
#include "commands.hpp"
#include "logger.hpp"

ExitCode run(const optional<string> &device, int width, int height) {
  logger::debug("Executing run command.");

  auto devices = Configuration::ConfiguredDevices();

  if (devices.empty()) {
    logger::warn("No device has been configured.");
    return ExitCode::SUCCESS;
  }

  if (device.has_value()) devices = {device.value()};

  bool oneFailure = false;
  for (const auto &device : devices) {
    auto instructions = Configuration::Load(device);
    if (!instructions) {
      oneFailure = true;
      logger::error("Failed to load a configuration for {}.", device);
      continue;
    }

    Camera camera(device, width, height);

    try {
      for (const auto &instruction : instructions.value()) {
        if (!instruction.is_disable()) {
          logger::debug("Applying instruction {} on {}.", to_string(instruction), device);
          if (!camera.apply(instruction)) {
            logger::error("Failed to apply the instruction {}.", to_string(instruction));
            oneFailure = true;
          }
        }
      }
    } catch (const CameraException &e) {
      logger::critical(e.what());
      exit(ExitCode::FILE_DESCRIPTOR_ERROR);
    }
  }

  return oneFailure ? ExitCode::FAILURE : ExitCode::SUCCESS;
}
