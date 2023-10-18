#ifndef AUTOCAMERA_HPP
#define AUTOCAMERA_HPP

#include <string>
using namespace std;

#include "camera.hpp"

constexpr int MAGIC_REF_INTENSITY_VAR_COEF = 50;

class AutoCamera : public Camera
{
private:
    unsigned captureTimeMs;
    long long unsigned refIntesityVarSum;

    long long unsigned intensityVariationSum();

public:
    AutoCamera() = delete;

    explicit AutoCamera(const string &device, unsigned captureTimeMs = 1000);

    ~AutoCamera() override = default;

    bool isEmitterWorking() override;

    bool isEmitterWorkingNoConfirm();

    AutoCamera &operator=(const AutoCamera &) = delete;

    AutoCamera(const AutoCamera &) = delete;

    AutoCamera &operator=(AutoCamera &&other) = delete;

    AutoCamera(AutoCamera &&other) = delete;

    static shared_ptr<AutoCamera> findGrayscaleCamera();
};

#endif