#pragma once

#include <vector>
using namespace std;

#include "camera/camera.hpp"

class Tweaker
{
protected:
    Camera &camera;

public:
    Tweaker() = delete;

    explicit Tweaker(Camera &camera);

    ~Tweaker() = default;

    Tweaker &operator=(const Tweaker &) = default;

    Tweaker(const Tweaker &) = default;

    Tweaker &operator=(Tweaker &&other) = default;

    Tweaker(Tweaker &&other) = default;

    void tweak(vector<CameraInstruction> &intructions);
};