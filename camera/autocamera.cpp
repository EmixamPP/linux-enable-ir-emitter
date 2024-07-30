#include "autocamera.hpp"

#include <cmath>
#include <memory>
#include <utility>
#include <vector>
using namespace std;

#include <spdlog/fmt/fmt.h>

#include <opencv2/core.hpp>

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

/**
 * @brief Obtain the intensity variation sum of camera captures
 *
 * @throw CameraException if unable to read frames
 *
 * @return the intensity variation sum
 */
long long unsigned AutoCamera::intensity_variation_sum() {
  // capture frames
  vector<cv::Mat> frames = read_during(capture_time_ms_);

  auto retry = RETRY_CAPTURE;
  while (frames.empty() && retry-- != 0) {
    // extend capture time,
    // to give the camera more time to capture frames
    capture_time_ms_ += CAPTURE_TIME_MS;
    frames = read_during(capture_time_ms_);
  }

  if (retry == 0) throw CameraException(fmt::format("Unable to read frames from {}", device()));

  // compute lighting intensity for each pixel of each frame
  const vector<vector<int>> intensities = compute_intensities(frames);

  // compute difference between each consecutive frame intensity
  const vector<int> diffs = compute_intensities_diff(intensities);

  // compute difference between each consecutive intensity difference
  // this is the variation in the lighting intensity of the fames
  // and sum them all
  return compute_sum_intensities_variation(diffs);
}

/**
 * @brief Check if the emitter is working,
 * without asking for manual confirmation
 *
 * @throw CameraException if unable to read frames
 *
 * @return true if yes, false if not
 */
bool AutoCamera::is_emitter_working_no_confirm() {
  return intensity_variation_sum() > refIntensity_var_sum_ * MAGIC_REF_INTENSITY_VAR_COEF;
}

/**
 * @brief Check if the emitter is working,
 * if so, ask for manual confirmation
 *
 * @throw CameraException if unable to read frames
 *
 * @return true if yes, false if not
 */
bool AutoCamera::is_emitter_working() {
  return is_emitter_working_no_confirm() && Camera::is_emitter_working();
}

/**
 * @brief Construct a new AutoCamera object
 * The difference with the regular Camera object is that this one
 * can automatically determine if the ir emitter is working or not.
 *
 * @param device path of the camera
 * @param width of the capture resolution
 * @param height of the capture resolution
 * @param capture_time_ms duration of the capture
 */
AutoCamera::AutoCamera(const string &device, int width, int height, unsigned capture_time_ms)
    : Camera(device, width, height),
      capture_time_ms_(capture_time_ms),
      refIntensity_var_sum_(intensity_variation_sum()) {}
