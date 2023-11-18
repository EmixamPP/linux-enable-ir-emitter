#pragma once

#include <cstdint>
#include <string>
#include <vector>
using namespace std;

#include "camera/camera.hpp"

class Scanner
{
protected:
    Camera &camera;

public:
    Scanner() = delete;

    explicit Scanner(Camera &camera);

    ~Scanner() = default;

    Scanner &operator=(const Scanner &) = default;

    Scanner(const Scanner &) = default;

    Scanner &operator=(Scanner &&other) = default;

    Scanner(Scanner &&other) = default;

    vector<CameraInstruction> scan() noexcept;
};
