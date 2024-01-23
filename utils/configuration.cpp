#include "configuration.hpp"

#include <fstream>
#include <string>
using namespace std;

#include <yaml-cpp/yaml.h>

#include "globals.hpp"
#include "camera/camerainstruction.hpp"
#include "utils/logger.hpp"

static void write_to_file(const vector<CameraInstruction> &instructions, const string &file_path)
{
    YAML::Node node(instructions);
    ofstream file(file_path);
    file << node << endl;
    file.close();
}

static optional<vector<CameraInstruction>> read_from_file(const string &file_path)
{
    try
    {
        YAML::Node node = YAML::LoadFile(file_path);
        return node.as<vector<CameraInstruction>>();
    }
    catch (const YAML::BadFile &)
    {
    }
    catch (...)
    {
        Logger::warning("Error while reading the configuration file at", file_path);
    }

    return {};
}

/**
 * @brief Save the configuration file of a device.
 *
 * @param device path to the infrared camera
 * @param instructions of the configuration
 */
void Configuration::Save(const string &device, const vector<CameraInstruction> &instructions)
{
    string path = ConfigPathOf(device);
    write_to_file(instructions, path);
}

/**
 * @brief Read the configuration file of a device.
 *
 * @param device path to the infrared camera
 *
 * @return vector with the instructions of the configuration
 * @return no value if an error happened
 */
optional<vector<CameraInstruction>> Configuration::Load(const string &device)
{
    string path = ConfigPathOf(device);
    return read_from_file(path);
}
