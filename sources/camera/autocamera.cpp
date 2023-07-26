#include "autocamera.hpp"

#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <vector>
using namespace std;

#include "opencv.hpp"

/**
 * @brief Obtain the intensity variation sum of camera captures
 *
 * @return the intensity variation sum
 */
long long unsigned AutoCamera::intensityVariationSum()
{
    openCap();
    shared_ptr<cv::VideoCapture> cap = getCap();
    vector<unique_ptr<cv::Mat>> frames;

    // capture frames
    const auto stopTime = chrono::steady_clock::now() + chrono::seconds(captureTime);
    while (chrono::steady_clock::now() < stopTime)
    {
        auto frame = make_unique<cv::Mat>();
        cap->read(*frame);
        if (!frame->empty())
            frames.push_back(std::move(frame));
    }

    // compute lighting intensity
    vector<unique_ptr<vector<int>>> intensities;
    for (auto &frame : frames)
    {
        auto intensity = make_unique<vector<int>>();
        for (int r = 0; r < frame->rows; ++r)
            for (int c = 0; c < frame->cols; ++c)
            {
                const cv::Vec3b &pixel = frame->at<cv::Vec3b>(r, c);
                intensity->push_back(pixel[0] + pixel[1] + pixel[2]);
            }
        intensities.push_back(std::move(intensity));
    }

    // compute difference between each consecutive frame intensity
    vector<int> diffs;
    for (unsigned i = 0; i < intensities.size() - 1; ++i)
    {
        const auto &intensity1 = intensities[i];
        const auto &intensity2 = intensities[i + 1];
        int diff = 0;
        for (unsigned j = 0; j < intensity1->size(); ++j)
        {
            diff += intensity1->at(j) - intensity2->at(j);
        }
        diffs.push_back(diff);
    }

    // compute difference between each consecutive intensity difference
    // this is the variation in the lighting intensity of the fames
    // and sum them all
    long long unsigned sum = 0;
    for (unsigned i = 0; i < diffs.size() - 1; ++i)
        sum += static_cast<long long unsigned>(abs(diffs[i] - diffs[i + 1]));
   
    closeCap();
    return sum;
}

bool AutoCamera::isEmitterWorking()
{
    return intensityVariationSum() > refIntesityVarSum * MAGIC_REF_INTENSITY_VAR_COEF;
}

AutoCamera::AutoCamera(const string &device, unsigned captureTime) : Camera(device), captureTime(captureTime), refIntesityVarSum(intensityVariationSum()) {}
