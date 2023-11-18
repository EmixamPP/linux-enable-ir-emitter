#pragma once

#include <vector>
using namespace std;

#include "camera/camerainstruction.hpp"

namespace Serializer
{
    void writeScanToFile(const vector<CameraInstruction> &instructions, const string &deviceName);
    void writeConfigToFile(const vector<CameraInstruction> &instructions, const string &deviceName);
    vector<CameraInstruction> readScanFromFile(const string &deviceName);
    vector<CameraInstruction> readConfigFromFile(const string &deviceName);

}