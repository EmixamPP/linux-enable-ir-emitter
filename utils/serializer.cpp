#include "serializer.hpp"

#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include <yaml-cpp/yaml.h>

#include "globals.hpp"
#include "camera/camerainstruction.hpp"

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
    }
    return vector<CameraInstruction>();
}

void Serializer::writeScanToFile(const vector<CameraInstruction> &instructions, const string &deviceName)
{
    string path = scanPathOf(deviceName);
    writeToFile(instructions, path);
}

void Serializer::writeConfigToFile(const vector<CameraInstruction> &instructions, const string &deviceName)
{
    string path = configPathOf(deviceName);
    writeToFile(instructions, path);
}

vector<CameraInstruction> Serializer::readScanFromFile(const string &deviceName)
{
    string path = scanPathOf(deviceName);
    return readFromFile(path);
}

vector<CameraInstruction> Serializer::readConfigFromFile(const string &deviceName)
{
    string path = configPathOf(deviceName);
    return readFromFile(path);
}
