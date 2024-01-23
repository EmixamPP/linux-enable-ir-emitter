#pragma once

#include <optional>
#include <vector>
using namespace std;

#include "camera/camerainstruction.hpp"

namespace Configuration
{
    void Save(const string &device, const vector<CameraInstruction> &instructions);
    optional<vector<CameraInstruction>> Load(const string &device);
}