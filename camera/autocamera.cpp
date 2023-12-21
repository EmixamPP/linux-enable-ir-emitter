#include "autocamera.hpp"

#include <chrono>
#include <cmath>
#include <memory>
#include <utility>
#include <vector>
using namespace std;

#include "opencv.hpp"
#include "globals.hpp"

/**
 * @brief Obtain the intensity variation sum of camera captures
 *
 * @return the intensity variation sum
 */
long long unsigned AutoCamera::intensityVariationSum()
{
    openCap();
    auto cap = getCap();
    vector<cv::Mat> frames;

    // capture frames
    const auto stopTime = chrono::steady_clock::now() + chrono::milliseconds(captureTimeMs);
    while (chrono::steady_clock::now() < stopTime)
    {
        cv::Mat frame;
        cap->read(frame);
        if (!frame.empty())
            frames.push_back(std::move(frame));
    }
    
    // compute lighting intensity
    vector<vector<int>> intensities;
    for (auto &frame : frames)
    {
        vector<int> intensity;
        for (int r = 0; r < frame.rows; ++r)
            for (int c = 0; c < frame.cols; ++c)
            {
                const cv::Vec3b &pixel = frame.at<cv::Vec3b>(r, c);
                intensity.push_back(pixel[0] + pixel[1] + pixel[2]);
            }
        intensities.push_back(std::move(intensity));
    }

    // compute difference between each consecutive frame intensity
    vector<int> diffs;
    for (size_t i = 0; i < intensities.size() - 1; ++i)
    {
        auto &intensity1 = intensities[i];
        auto &intensity2 = intensities[i + 1];
        int diff = 0;
        for (size_t j = 0; j < intensity1.size(); ++j)
            diff += intensity1.at(j) - intensity2.at(j);
        diffs.push_back(diff);
    }

    // compute difference between each consecutive intensity difference
    // this is the variation in the lighting intensity of the fames
    // and sum them all
    long long unsigned sum = 0;
    for (size_t i = 0; i < diffs.size() - 1; ++i)
        sum += static_cast<long long unsigned>(abs(diffs[i] - diffs[i + 1]));

    closeCap();
    return sum;
}

/**
 * @brief Check if the emitter is working,
 * without asking for manual confirmation
 *
 * @return true if yes, false if not
 */
bool AutoCamera::isEmitterWorkingNoConfirm()
{
    return intensityVariationSum() > refIntesityVarSum * MAGIC_REF_INTENSITY_VAR_COEF;
}

/**
 * @brief Check if the emitter is working,
 * if so, ask for manual confirmation
 *
 * @return true if yes, false if not
 */
bool AutoCamera::isEmitterWorking()
{
    return isEmitterWorkingNoConfirm() && Camera::isEmitterWorking();
}

AutoCamera::AutoCamera(const string &device, unsigned captureTimeMs) : Camera(device), captureTimeMs(captureTimeMs), refIntesityVarSum(intensityVariationSum()) {}

/**
 * @brief Find a grayscale camera.
 *
 * @return path to the graycale device,
 * nullptr if unable to find such device
 */
shared_ptr<AutoCamera> AutoCamera::findGrayscaleCamera()
{
    vector<string> v4lDevices = getV4LDevices();
    for (auto &device : v4lDevices)
    {
        try
        {
            shared_ptr<AutoCamera> camera = make_shared<AutoCamera>(device);
            if (camera->isGrayscale())
                return camera;
        }
        catch (const CameraException &e)
        { // ignore them
        }
    }

    return nullptr;
}