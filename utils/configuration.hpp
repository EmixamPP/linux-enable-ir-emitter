#pragma once

#include <optional>
#include <vector>
using namespace std;

#include "camera/camerainstruction.hpp"

namespace Configuration
{
    void save(const string &device, const vector<CameraInstruction> &instructions);
    optional<vector<CameraInstruction>> load(const string &device);
}