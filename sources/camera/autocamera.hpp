#ifndef AUTOCAMERA_HPP
#define AUTOCAMERA_HPP

#include <string>
using namespace std;

#include "camera.hpp"

class AutoCamera : public Camera
{
private:
    unsigned captureTime;

public:
    AutoCamera() = delete;

    explicit AutoCamera(const string &device, unsigned captureTime = 2);

    ~AutoCamera() override = default;

    bool isEmitterWorking() override;

    AutoCamera &operator=(const AutoCamera &) = delete;

    AutoCamera(const AutoCamera &) = delete;

    AutoCamera &operator=(AutoCamera &&other) = delete;

    AutoCamera(AutoCamera && other) = delete;
};

#endif