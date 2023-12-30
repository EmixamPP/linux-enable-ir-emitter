#include "configuration.hpp"

#include <fstream>
#include <string>
using namespace std;

#include <yaml-cpp/yaml.h>

#include "globals.hpp"
#include "camera/camerainstruction.hpp"
#include "utils/logger.hpp"

static void writeToFile(const vector<CameraInstruction> &instructions, const string &filePath)
{
    YAML::Node node(instructions);
    ofstream file(filePath);
    file << node << endl;
    file.close();
}

static optional<vector<CameraInstruction>> readFromFile(const string &filePath)
{
    try
    {
        YAML::Node node = YAML::LoadFile(filePath);
        return node.as<vector<CameraInstruction>>();
    }
    catch (const YAML::BadFile &)
    {
    }
    catch (...)
    {
        Logger::error("Error while reading the configuration file at", filePath);
    }

    return {};
}

void Configuration::save(const string &device, const vector<CameraInstruction> &instructions)
{
    string path = config_path_of(device);
    writeToFile(instructions, path);
}

/**
 * @brief Read the configuration file of a device.
 *
 * @param device path to the infrared camera
 *
 * @return vector with the instructions of the configuration
 * @return no value if an error happened
 */
optional<vector<CameraInstruction>> Configuration::load(const string &device)
{
    string path = config_path_of(device);
    return readFromFile(path);
}
