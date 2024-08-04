#include "configuration.hpp"

#include <filesystem>
#include <fstream>
#include <set>
#include <string>
using namespace std;

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include "camera/camerainstruction.hpp"

// Prefix path of a v4l device
const string V4L_PREFIX = "/dev/v4l/by-path/";

/**
 * @brief Write the instructions to a file.
 *
 * @param instructions the instructions to write
 * @param file_path the path of the file
 */
static void write_to_file(const CameraInstructions &instructions, const string &file_path) {
  YAML::Node node(instructions);
  ofstream file(file_path);
  file << node << endl;
  file.close();
}

/**
 * @brief Read the instructions from a file.
 *
 * @param file_path the path of the file
 * @return the instructions
 */
static optional<CameraInstructions> read_from_file(const string &file_path) noexcept {
  try {
    YAML::Node node = YAML::LoadFile(file_path);
    return node.as<CameraInstructions>();
  } catch (const YAML::BadFile &e) {
    spdlog::debug("yaml error: {}.", e.what());
  } catch (...) {
    spdlog::warn("Error while reading the configuration file at {}.", file_path);
  }

  return {};
}

/**
 * @brief Get the v4l name of a device.
 *
 * @param device path of the camera
 * @return the v4l name
 */
static optional<string> V4LNameOf(const string &device) {
  set<string> names;  // multiple names can exists

  try {
    auto device_path = filesystem::canonical(device);

    for (const auto &v4l : filesystem::directory_iterator(V4L_PREFIX))
      if (device_path == filesystem::canonical(v4l) && v4l.path().has_filename())
        names.emplace(v4l.path().filename());

    if (names.empty()) return {};
  } catch (const filesystem::filesystem_error &e) {
    spdlog::debug(e.what());
  }

  return *names.begin();
}

/**
 * @brief Get the theoretical path of the configuration file.
 * It does not check if the configuration exists.
 *
 * @param device path of the camera
 * @return the path of the configuration file
 */
static optional<string> PathOf(const string &device) noexcept {
  auto v4l_name = V4LNameOf(device);
  if (v4l_name) return Configuration::SAVE_FOLDER_CONFIG_PATH_ + v4l_name.value();

  spdlog::error("Impossible to obtain the v4l name of {}.", device);
  return {};
}

optional<CameraInstructions> Configuration::Load(const string &device,
                                                 optional<string> path) noexcept {
  if (!path) path = PathOf(device);

  if (path) return read_from_file(path.value());

  return {};
}

optional<CameraInstructions> Configuration::LoadInit(const string &device) noexcept {
  auto path = PathOf(device);
  if (path) path->append(".ini");

  return Configuration::Load(device, path);
}

bool Configuration::Save(const string &device, const CameraInstructions &instructions,
                         optional<string> path) {
  if (!path) path = PathOf(device);

  if (path) {
    write_to_file(instructions, path.value());
    spdlog::debug("Configuration for {} saved here: {}.", device, path.value());
    return true;
  }

  return false;
}

bool Configuration::SaveInit(const string &device, const CameraInstructions &instructions) {
  auto path = PathOf(device);
  if (path) path->append(".ini");

  return Configuration::Save(device, instructions, path);
}

vector<string> Configuration::ConfiguredDevices() noexcept {
  vector<string> devices;
  for (const auto &conf : filesystem::directory_iterator(SAVE_FOLDER_CONFIG_PATH_)) {
    if (conf.path().filename().extension().compare(".ini") == 0) continue;

    auto device = V4L_PREFIX + conf.path().filename().string();
    devices.push_back(std::move(device));
  }
  return devices;
}
