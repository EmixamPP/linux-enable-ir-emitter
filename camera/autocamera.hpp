#pragma once

#include <string>
using namespace std;

#include "camera.hpp"

constexpr int MAGIC_REF_INTENSITY_VAR_COEF = 50;
constexpr unsigned CAPTURE_TIME_MS = 1000;
constexpr unsigned RETRY_CAPTURE = 2;

class AutoCamera : public Camera
{
private:
    unsigned capture_time_ms_;
    long long unsigned refIntesity_var_sum_;

    long long unsigned intensity_variation_sum();

public:
    AutoCamera() = delete;

    explicit AutoCamera(const string &device, int width = -1, int height = -1, unsigned capture_time_ms = CAPTURE_TIME_MS);

    ~AutoCamera() override = default;

    bool is_emitter_working() override;

    bool is_emitter_working_no_confirm();

    AutoCamera &operator=(const AutoCamera &) = delete;

    AutoCamera(const AutoCamera &) = delete;

    AutoCamera &operator=(AutoCamera &&other) = delete;

    AutoCamera(AutoCamera &&other) = delete;

    static shared_ptr<AutoCamera> FindGrayscaleCamera(int width = -1, int height = -1);
};
