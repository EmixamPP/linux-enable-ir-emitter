#include <algorithm>
#include <vector>

#include "commands.hpp"
using namespace std;

#include <spdlog/spdlog.h>

#include "camera/camerainstruction.hpp"

ExitCode run(const optional<string> &device, int width, int height) {
  spdlog::debug("Executing run command.");

  auto devices = Configuration::ConfiguredDevices();

  if (devices.empty()) {
    spdlog::warn("No device has been configured.");
    return ExitCode::SUCCESS;
  }

  if (device.has_value()) devices = {device.value()};

  bool oneFailure = false;
  for (const auto &device : devices) {
    auto instructions = Configuration::Load(device);
    if (!instructions) {
      oneFailure = true;
      spdlog::error("Failed to load a configuration for {}.", device);
      continue;
    }

    Camera camera(device, width, height);

    try {
      for (const auto &instruction : instructions.value()) {
        if (!instruction.is_disable()) {
          spdlog::debug("Applying instruction {} on {}.", to_string(instruction), device);
          if (!camera.apply(instruction)) {
            spdlog::error("Failed to apply the instruction {}.", to_string(instruction));
            oneFailure = true;
          }
        }
      }
    } catch (const CameraException &e) {
      spdlog::critical(e.what());
      exit(ExitCode::FILE_DESCRIPTOR_ERROR);
    }
  }

  return oneFailure ? ExitCode::FAILURE : ExitCode::SUCCESS;
}
