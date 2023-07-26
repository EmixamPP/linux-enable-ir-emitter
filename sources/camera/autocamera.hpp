#ifndef AUTOCAMERA_HPP
#define AUTOCAMERA_HPP

#include <string>
using namespace std;

#include "camera.hpp"

constexpr int MAGIC_REF_INTENSITY_VAR_COEF = 50;

class AutoCamera : public Camera
{
private:
    unsigned captureTime;
    long long unsigned refIntesityVarSum;

    long long unsigned intensityVariationSum();

public:
    AutoCamera() = delete;

    explicit AutoCamera(const string &device, unsigned captureTime = 1);

    ~AutoCamera() override = default;

    bool isEmitterWorking() override;

    AutoCamera &operator=(const AutoCamera &) = delete;

    AutoCamera(const AutoCamera &) = delete;

    AutoCamera &operator=(AutoCamera &&other) = delete;

    AutoCamera(AutoCamera && other) = delete;
};

#endif