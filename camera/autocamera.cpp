#include "autocamera.hpp"

#include <cmath>
#include <memory>
#include <utility>
#include <vector>
using namespace std;

#include "opencv.hpp"
#include "globals.hpp"
#include "utils/logger.hpp"

static vector<vector<int>> computeIntensities(const vector<cv::Mat> &frames)
{
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
        intensities.push_back(move(intensity));
    }
    return intensities;
}

static vector<int> computeIntesitiesDiff(const vector<vector<int>> &intensities)
{
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
    return diffs;
}

static long long unsigned computeSumOfIntensitiesVariation(const vector<int> &diffs)
{
    long long unsigned sum = 0;
    for (size_t i = 0; i < diffs.size() - 1; ++i)
        sum += static_cast<long long unsigned>(abs(diffs[i] - diffs[i + 1]));

    return sum;
}

/**
 * @brief Obtain the intensity variation sum of camera captures
 *
 * @return the intensity variation sum
 */
long long unsigned AutoCamera::intensityVariationSum()
{
    // capture frames
    const vector<cv::Mat> frames = readDuring(captureTimeMs);

    // compute lighting intensity for each pixel of each frame
    const vector<vector<int>> intensities = computeIntensities(frames);

    // compute difference between each consecutive frame intensity
    const vector<int> diffs = computeIntesitiesDiff(intensities);

    // compute difference between each consecutive intensity difference
    // this is the variation in the lighting intensity of the fames
    // and sum them all
    return computeSumOfIntensitiesVariation(diffs);
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

AutoCamera::AutoCamera(const string &device, int width, int height, unsigned captureTimeMs)
    : Camera(device, width, height), captureTimeMs(captureTimeMs), refIntesityVarSum(intensityVariationSum()) {}

/**
 * @brief Find a greyscale camera.
 *
 * @return path to the greycale device,
 * nullptr if unable to find such device
 */
shared_ptr<AutoCamera> AutoCamera::findGrayscaleCamera(int width, int height)
{
    vector<string> v4lDevices = get_V4L_devices();
    for (auto &device : v4lDevices)
    {
        Logger::debug("Checking if", device, "is a greyscale camera.");
        try
        {
            auto camera = make_shared<AutoCamera>(device, width, height);
            if (camera->isGrayscale())
            {
                Logger::debug(device, "is a greyscale camera.");
                return camera;
            }
        }
        catch (const CameraException &)
        { // ignore them
        }
    }

    return nullptr;
}