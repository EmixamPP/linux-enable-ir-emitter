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

    Tweaker &operator=(const Tweaker &) = delete;

    Tweaker(const Tweaker &) = delete;

    Tweaker &operator=(Tweaker &&other) = delete;

    Tweaker(Tweaker &&other) = delete;

    void tweak(vector<CameraInstruction> &instructions);
};