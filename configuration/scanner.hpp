#pragma once

#include <vector>
using namespace std;

#include "camera/camera.hpp"
#include "camera/camerainstruction.hpp"

class Scanner
{
private:
    Camera &camera_;

public:
    Scanner() = delete;

    explicit Scanner(Camera &camera);

    ~Scanner() = default;

    Scanner &operator=(const Scanner &) = delete;

    Scanner(const Scanner &) = delete;

    Scanner &operator=(Scanner &&other) = delete;

    Scanner(Scanner &&other) = delete;

    CameraInstructions scan() noexcept;
};
