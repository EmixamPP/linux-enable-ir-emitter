#include "configuration.hpp"

#include <fstream>
#include <filesystem>
#include <set>
#include <string>
using namespace std;

#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

#include "camera/camerainstruction.hpp"

const string V4L_PREFIX = "/dev/v4l/by-path/";

static void write_to_file(const CameraInstructions &instructions, const string &file_path)
{
    YAML::Node node(instructions);
    ofstream file(file_path);
    file << node << endl;
    file.close();
}

static optional<CameraInstructions> read_from_file(const string &file_path) noexcept
{
    try
    {
        YAML::Node node = YAML::LoadFile(file_path);
        return node.as<CameraInstructions>();
    }
    catch (const YAML::BadFile &e)
    {
        spdlog::debug("yaml error: {}.", e.what());
    }
    catch (...)
    {
        spdlog::warn("Error while reading the configuration file at {}.", file_path);
    }

    return {};
}

static optional<string> V4LNameOf(const string &device)
{
    set<string> names; // multiple names can exists

    try
    {
        auto device_path = filesystem::canonical(device);

        for (const auto &v4l : filesystem::directory_iterator(V4L_PREFIX))
            if (device_path == filesystem::canonical(v4l) && v4l.path().has_filename())
                names.emplace(v4l.path().filename());

        if (names.empty())
            return {};
    }
    catch (const filesystem::filesystem_error &e)
    {
        spdlog::debug(e.what());
    }

    return *names.begin();
}

static optional<string> PathOf(const string &device) noexcept
{
    auto v4l_name = V4LNameOf(device);
    if (v4l_name)
        return Configuration::SAVE_FOLDER_CONFIG_PATH_ + v4l_name.value();

    spdlog::error("Impossible to obtain the v4l name of {}.", device);
    return {};
}

/**
 * @brief Read the configuration file of a device.
 *
 * @param device path of the camera
 *
 * @return vector with the instructions of the configuration
 * @return no value if an error happened
 */
optional<CameraInstructions> Configuration::Load(const string &device, optional<string> path) noexcept
{
    if (!path)
        path = PathOf(device);

    if (path)
        return read_from_file(path.value());

    return {};
}

/**
 * @brief Read the configuration file of a device.
 *
 * @param device path of the camera
 *
 * @return vector with the instructions of the configuration
 * @return no value if an error happened
 */
optional<CameraInstructions> Configuration::LoadInit(const string &device) noexcept
{
    auto path = PathOf(device);
    if (path)
        path->append(".ini");

    return Configuration::Load(device, path);
}

/**
 * @brief Save the configuration of a device
 *
 * @param device path of the camera
 * @param instructions of the configuration
 * @param path path where to store the configuration, nothing for default one
 *
 * @return true if success otherwise false
 */
bool Configuration::Save(const string &device, const CameraInstructions &instructions, optional<string> path)
{
    if (!path)
        path = PathOf(device);

    if (path)
    {
        write_to_file(instructions, path.value());
        spdlog::debug("Configuration for {} saved here: {}.", device, path.value());
        return true;
    }

    return false;
}

/**
 * @brief Save the configuration of a device as initial
 *
 * @param device path of the camera
 * @param instructions of the configuration
 *
 * @return true if success otherwise false
 */
bool Configuration::SaveInit(const string &device, const CameraInstructions &instructions)
{
    auto path = PathOf(device);
    if (path)
        path->append(".ini");

    return Configuration::Save(device, instructions, path);
}

/**
 * @brief Get all the device configured, only based on the file name
 *
 * @return device path of the configured camera
 */
vector<string> Configuration::ConfiguredDevices() noexcept
{
    vector<string> devices;
    for (const auto &conf : filesystem::directory_iterator(SAVE_FOLDER_CONFIG_PATH_))
    {
        if (conf.path().filename().extension().compare(".ini") == 0)
            continue;

        auto device = V4L_PREFIX + conf.path().filename().string();
        devices.push_back(std::move(device));
    }
    return devices;
}
