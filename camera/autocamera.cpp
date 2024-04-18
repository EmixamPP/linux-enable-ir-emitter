#include "autocamera.hpp"

#include <cmath>
#include <memory>
#include <utility>
#include <vector>
using namespace std;

#include <opencv2/core.hpp>

#include "utils/logger.hpp"

static vector<vector<int>> compute_intensities(const vector<cv::Mat> &frames)
{
    vector<vector<int>> intensities;
    for (const auto &frame : frames)
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
    return intensities;
}

static vector<int> compute_intesities_diff(const vector<vector<int>> &intensities)
{
    vector<int> diffs;
    for (size_t i = 0; i < intensities.size() - 1; ++i)
    {
        const auto &intensity1 = intensities[i];
        const auto &intensity2 = intensities[i + 1];
        int diff = 0;
        for (size_t j = 0; j < intensity1.size(); ++j)
            diff += intensity1.at(j) - intensity2.at(j);
        diffs.push_back(diff);
    }
    return diffs;
}

static long long unsigned compute_sum_intensities_variation(const vector<int> &diffs)
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
long long unsigned AutoCamera::intensity_variation_sum()
{
    // capture frames
    const vector<cv::Mat> frames = read_during(capture_time_ms_);

    // compute lighting intensity for each pixel of each frame
    const vector<vector<int>> intensities = compute_intensities(frames);

    // compute difference between each consecutive frame intensity
    const vector<int> diffs = compute_intesities_diff(intensities);

    // compute difference between each consecutive intensity difference
    // this is the variation in the lighting intensity of the fames
    // and sum them all
    return compute_sum_intensities_variation(diffs);
}

/**
 * @brief Check if the emitter is working,
 * without asking for manual confirmation
 *
 * @return true if yes, false if not
 */
bool AutoCamera::is_emitter_working_no_confirm()
{
    return intensity_variation_sum() > refIntesity_var_sum_ * MAGIC_REF_INTENSITY_VAR_COEF;
}

/**
 * @brief Check if the emitter is working,
 * if so, ask for manual confirmation
 *
 * @return true if yes, false if not
 */
bool AutoCamera::is_emitter_working()
{
    return is_emitter_working_no_confirm() && Camera::is_emitter_working();
}

AutoCamera::AutoCamera(const string &device, int width, int height, unsigned capture_time_ms)
    : Camera(device, width, height), capture_time_ms_(capture_time_ms), refIntesity_var_sum_(intensity_variation_sum()) {}
