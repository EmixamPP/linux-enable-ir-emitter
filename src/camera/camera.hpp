#pragma once

#include <linux/uvcvideo.h>

#include <memory>
#include <stop_token>
#include <string>
#include <vector>
using namespace std;

#include <opencv2/core/utils/logger.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

/**
 * @brief Exception thrown by `Camera` class.
 */
class CameraException final : public exception {
 private:
  string message_;

 public:
  CameraException(const string &message);

  const char *what() const noexcept override;
};

class CameraInstruction;

/**
 * @brief The `Camera` object allows to capture frames from a camera device for a specific amount,
 * duration or until requested, while displaying a video feedback (or not) to the user, and asking
 * him to determine if the ir emitter is working. In addition, it can automatically determine if the
 * camera is in grayscale. Futhermore, the object can perform UVC queries on the camera device to
 * control it. Lastly, the class can also gives the list of all /dev/videoX devices.
 */
class Camera {
 private:
  // Should the video feedback be not displayed
  bool no_gui_ = false;

  // File descriptor of the camera device
  int fd_ = -1;

  // Index of the camera device
  int index_;

  // Path of the camera device
  string device_;

  // Parameters for the OpenCV VideoCapture `cap_` object
  const vector<int> cap_para_;

  // OpenCV VideoCapture object
  const shared_ptr<cv::VideoCapture> cap_ = make_shared<cv::VideoCapture>();

  /**
   * @brief Open a file descriptor if not yet open.
   *
   * @throw CameraException if unable to open the camera device
   */
  void open_fd();

  /**
   * @brief Close the current file descriptor.
   */
  void close_fd() noexcept;

  /**
   * @brief Open a VideoCapture if not yet open
   *
   * @throw CameraException if unable to open the camera device
   */
  void open_cap();

  /**
   * @brief Close the current VideoCapture.
   */
  void close_cap() noexcept;

  /**
   * @brief Execute an uvc query on the device indicated by the file descriptor.
   *
   * @param query uvc query to execute
   *
   * @throw CameraException if unable to open the camera device
   *
   * @return 1 if error, otherwise 0
   **/
  int execute_uvc_query(const uvc_xu_control_query &query);

  /**
   * @brief Reads one frame.
   * Must be called between `open_cap()` and `close_cap()`, otherwise it will cause exceptions.
   *
   * @throw CameraException if unable to open the camera device
   *
   * @return the frame
   */
  cv::Mat read1_unsafe();

  /**
   * @brief Show a video feedback to the user
   * and asks if the emitter is working.
   * Must be called between `open_cap()` and `close_cap()`.
   *
   * @throw CameraException if unable to open the camera device
   *
   * @return true if yes, false if not
   */
  bool is_emitter_working_ask();

  /**
   * @brief Trigger the camera and asks if the emitter is working.
   *
   * @throw CameraException if unable to open the camera device
   *
   * @return true if yes, false if not
   */
  bool is_emitter_working_ask_no_gui();

 public:
  Camera() = delete;

  /**
   * @brief Construct a new Camera object.
   *
   * @param device path of the camera
   * @param width of the capture resolution
   * @param height of the capture resolution
   *
   * @throw CameraException if the device is invalid
   */
  explicit Camera(const string &device, int width = -1, int height = -1);

  /**
   * @brief Destroy the Camera object
   */
  virtual ~Camera();

  Camera &operator=(const Camera &) = delete;

  Camera(const Camera &) = delete;

  Camera &operator=(Camera &&other) = delete;

  Camera(Camera &&other) = delete;

  /**
   * @brief Get the device.
   *
   * @return device path
   */
  string device() const noexcept;

  /**
   * @brief Disable the video feedback for `is_emitter_working()`.
   */
  void disable_gui() noexcept;

  /**
   * @brief Show a video feedback until the stop is requested.
   * You should not use the camera object until the `request_stop()` is called.
   *
   * @throw CameraException if unable to open the camera device
   *
   * @return the stop source
   */
  std::stop_source play();

  /**
   * @brief Show a video feedback until the user exit by pressing any key.
   *
   * @throw CameraException if unable to open the camera device
   *
   */
  void play_forever();

  /**
   * @brief Reads one frame.
   *
   * @throw CameraException if unable to open the camera device
   *
   * @return the frame
   */
  cv::Mat read1();

  /**
   * @brief Read multiple frames.
   *
   * @param capture_time_ms for how long (ms) collect frames
   *
   * @throw CameraException if unable to open the camera device
   *
   * @return the frames
   */
  vector<cv::Mat> read_during(unsigned capture_time_ms);

  /**
   * @brief Check if the emitter is working by asking confirmation to the user.
   *
   * @throw CameraException if unable to open the camera device
   *
   * @return true if yes, false if not
   */
  virtual bool is_emitter_working();

  /**
   * @brief Determine if the camera is in grayscale.
   *
   * @throw CameraException if unable to open the camera device
   *
   * @return true if so, otherwise false.
   */
  bool is_gray_scale();

  /**
   * @brief Apply an instruction on the camera.
   *
   * @param instruction to apply
   *
   * @throw CameraException if unable to open the camera device
   *
   * @return true if success, otherwise false
   */
  bool apply(const CameraInstruction &instruction);

  /**
   * @brief Change the current uvc control value for the camera device
   *
   * @param unit extension unit ID
   * @param selector control selector
   * @param control control value
   *
   * @throw CameraException if unable to open the camera device
   *
   * @return 1 if error, otherwise 0
   */
  int uvc_set_query(uint8_t unit, uint8_t selector, vector<uint8_t> &control);

  /**
   * @brief Get the current, maximal, resolution or minimal value of the uvc control for the camera
   *device.
   *
   * @param query_type UVC_GET_MAX, UVC_GET_RES, UVC_GET_CUR or UVC_GET_MIN
   * @param unit extension unit ID
   * @param selector control selector
   * @param controlSize size of the uvc control
   * @param control control value
   *
   * @throw CameraException if unable to open the camera device
   *
   * @return 1 if error, otherwise 0
   */
  int uvc_get_query(uint8_t query_type, uint8_t unit, uint8_t selector, vector<uint8_t> &control);

  /**
   * @brief Get the size of the uvc control for the indicated device.
   *
   * @param unit extension unit ID
   * @param selector control selector
   *
   * @throw CameraException if unable to open the camera device
   *
   * @return size of the control, 0 if error
   */
  uint16_t uvc_len_query(uint8_t unit, uint8_t selector);

  /**
   * @brief Return all /dev/videoX devices
   *
   * @return path to the device
   */
  static vector<string> Devices();
};
