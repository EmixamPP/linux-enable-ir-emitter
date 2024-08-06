#include "autocamera.hpp"

#include <cmath>
#include <format>
#include <memory>
#include <utility>
#include <vector>
using namespace std;

#include <opencv2/core.hpp>

// Coefficient applied on `refIntensity_var_sum_`
// that determines the significance
constexpr int MAGIC_REF_INTENSITY_VAR_COEF = 50;

// How many times retry a frame capture in case of failure
// before increasing `capture_time_ms_` by `CAPTURE_TIME_MS`
constexpr unsigned RETRY_CAPTURE = 2;

/**
 * @brief Compute lighting intensity for each pixel of each frame
 *
 * @param frames the frames
 * @return the intensities
 */
static vector<vector<int>> compute_intensities(const vector<cv::Mat> &frames) {
  vector<vector<int>> intensities;
  for (const auto &frame : frames) {
    vector<int> intensity;
    for (int r = 0; r < frame.rows; ++r)
      for (int c = 0; c < frame.cols; ++c) {
        const cv::Vec3b &pixel = frame.at<cv::Vec3b>(r, c);
        intensity.push_back(pixel[0] + pixel[1] + pixel[2]);
      }
    intensities.push_back(std::move(intensity));
  }
  return intensities;
}

/**
 * @brief Compute the difference for each two consecutive frame lighting intensity
 *
 * @param intensities the frames intensities computed
 * @return the differences
 */
static vector<int> compute_intensities_diff(const vector<vector<int>> &intensities) {
  vector<int> diffs;
  for (size_t i = 1; i < intensities.size(); ++i) {
    const auto &intensity1 = intensities[i - 1];
    const auto &intensity2 = intensities[i];
    int diff = 0;
    for (size_t j = 0; j < intensity1.size(); ++j) diff += intensity1.at(j) - intensity2.at(j);
    diffs.push_back(diff);
  }
  return diffs;
}

/**
 * @brief Compute the sum of the absolute differences between each two consecutive frames
 * intensities difference
 *
 * @param diffs the frames intensities difference
 * @return the sum
 */
static long long unsigned compute_sum_intensities_variation(const vector<int> &diffs) {
  long long unsigned sum = 0;
  for (size_t i = 1; i < diffs.size(); ++i)
    sum += static_cast<long long unsigned>(abs(diffs[i - 1] - diffs[i]));

  return sum;
}

long long unsigned AutoCamera::intensity_variation_sum() {
  // capture frames
  vector<cv::Mat> frames = read_during(capture_time_ms_);

  auto retry = RETRY_CAPTURE;
  while (frames.empty() && retry-- != 0) {
    // double the capture time,
    // to give the camera more time to capture frames
    capture_time_ms_ += CAPTURE_TIME_MS;
    frames = read_during(capture_time_ms_);
  }

  if (retry == 0) throw CameraException(std::format("Unable to read frames from {}", device()));

  // compute lighting intensity for each pixel of each frame
  const vector<vector<int>> intensities = compute_intensities(frames);

  // compute difference between each consecutive frame intensity
  const vector<int> diffs = compute_intensities_diff(intensities);

  // compute difference between each consecutive intensity difference
  // this is the variation in the lighting intensity of the fames
  // and sum them all
  return compute_sum_intensities_variation(diffs);
}

bool AutoCamera::is_emitter_working_no_confirm() {
  return intensity_variation_sum() > refIntensity_var_sum_ * MAGIC_REF_INTENSITY_VAR_COEF;
}

bool AutoCamera::is_emitter_working() {
  return is_emitter_working_no_confirm() && Camera::is_emitter_working();
}

AutoCamera::AutoCamera(const string &device, int width, int height, unsigned capture_time_ms)
    : Camera(device, width, height),
      capture_time_ms_(capture_time_ms),
      refIntensity_var_sum_(intensity_variation_sum()) {}
