#pragma once

#include <string>
using namespace std;

#include "camera.hpp"

// Default duration of the video capture in ms
constexpr unsigned CAPTURE_TIME_MS = 1000;

/**
 * @brief `AutoCamera` specialize `Camera`, to be able to
 * determine if the ir emitter is working thanks to a short video capture.
 */
class AutoCamera final : public Camera {
  // Duration of the video capture
  unsigned capture_time_ms_;

  // Lighting intensities differences variation sum
  // used as reference
  const long long unsigned refIntensity_var_sum_;

  /**
   * @brief Obtain the intensity variation sum of camera captures
   * @throw Camera::Exception if unable to read frames
   * @return the intensity variation sum
   */
  long long unsigned intensity_variation_sum();

 public:
  /**
   * @brief Construct a new AutoCamera object
   * The difference with the regular Camera object is that this one
   * can automatically determine if the ir emitter is working or not.
   * @param device path of the camera
   * @param width of the capture resolution
   * @param height of the capture resolution
   * @param capture_time_ms duration of the video capture
   */
  explicit AutoCamera(const string &device, int width = -1, int height = -1,
                      unsigned capture_time_ms = CAPTURE_TIME_MS);

  ~AutoCamera() override = default;

  /**
   * @brief Check if the emitter is working,
   * if so, ask for manual confirmation
   * @throw Camera::Exception if unable to read frames
   * @return true if yes, false if not
   */
  bool is_emitter_working() override;

  /**
   * @brief Check if the emitter is working,
   * without asking for manual confirmation
   * @throw Camera::Exception if unable to read frames
   * @return true if yes, false if not
   */
  bool is_emitter_working_no_confirm();

  AutoCamera &operator=(const AutoCamera &) = delete;
  AutoCamera(const AutoCamera &) = delete;
  AutoCamera &operator=(AutoCamera &&other) = delete;
  AutoCamera(AutoCamera &&other) = delete;
};
