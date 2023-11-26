#include "configuration.hpp"

#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include <yaml-cpp/yaml.h>

#include "camera/camerainstruction.hpp"
#include "globals.hpp"

static void writeToFile(const vector<CameraInstruction> &instructions, const string &filePath)
{
    YAML::Node node(instructions);
    ofstream file(filePath);
    file << node << endl;
    file.close();
}

static vector<CameraInstruction> readFromFile(const string &filePath)
{
    try
    {
        YAML::Node node = YAML::LoadFile(filePath);
        return node.as<vector<CameraInstruction>>();
    }
    catch (YAML::BadFile &file)
    {
        // TODO better handeling
    }
    return vector<CameraInstruction>();
}

void Configuration::save(const string &device, const vector<CameraInstruction> &instructions)
{   
    string path = configPathOf(device);
    writeToFile(instructions, path);
}

vector<CameraInstruction> Configuration::load(const string &device)
{
    string path = configPathOf(device);
    return readFromFile(path);
}
