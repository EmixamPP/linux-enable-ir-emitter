#pragma once

#include <linux/uvcvideo.h>

#include <future>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>
#include <vector>
using namespace std;

#include <opencv2/core/utils/logger.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

struct CameraInstruction;

class FileDescriptor {
  // File descriptor
  int fd_;

 public:
  FileDescriptor(const string &device);
  ~FileDescriptor();
  operator int() const;
  FileDescriptor(const FileDescriptor &) = delete;
  FileDescriptor &operator=(const FileDescriptor &) = delete;
  FileDescriptor(FileDescriptor &&) = delete;
  FileDescriptor &operator=(FileDescriptor &&) = delete;
};

/**
 * @brief The `Camera` object allows to capture frames from a camera device for a specific amount,
 * duration or until requested, while displaying a video feedback (or not) to the user, and asking
 * him to determine if the ir emitter is working. In addition, it can automatically determine if the
 * camera is in grayscale. Futhermore, the object can perform UVC queries on the camera device to
 * control it. Lastly, the class can also gives the list of all /dev/videoX devices.
 */
class Camera {
  // Should the video feedback be not displayed
  bool no_gui_ = false;

  // Path of the camera device
  string device_;

  // File descriptor of the camera device
  FileDescriptor fd_;

  // OpenCV video capture of the camera device
  cv::VideoCapture cap_;

  /**
   * @brief Execute an uvc query on the device indicated by the file descriptor.
   * @param query uvc query to execute
   * @return 1 if error, otherwise 0
   **/
  int execute_uvc_query(const uvc_xu_control_query &query) noexcept;

  /**
   * @brief Reads one frame.
   * @throw CameraException if unable to read a frame
   * @return the frame
   */
  cv::Mat read1();

  /**
   * @brief Show a video feedback to the user
   * and asks if the emitter is working.
   * Must be called between `open_cap()` and `close_cap()`.
   * @throw CameraException if unable to read a frame
   * @return true if yes, false if not
   */
  bool is_emitter_working_ask();

  /**
   * @brief Trigger the camera and asks if the emitter is working.
   * @throw CameraException if unable to read a frame
   * @return true if yes, false if not
   */
  bool is_emitter_working_ask_no_gui();

 public:
  /**
   * @brief Construct a new Camera object.
   * @param device path of the camera
   * @param width of the capture resolution (default: -1)
   * @param height of the capture resolution (default: -1)
   * @throw CameraException if the device is invalid
   */
  explicit Camera(const string &device, int width = -1, int height = -1);

  virtual ~Camera() = default;

  Camera &operator=(const Camera &) = delete;

  Camera(const Camera &) = delete;

  Camera &operator=(Camera &&other) = delete;

  Camera(Camera &&other) = delete;

  /**
   * @brief Get the device.
   * @return device path
   */
  string device() const noexcept;

  /**
   * @brief Disable the video feedback for `is_emitter_working()`.
   */
  void disable_gui() noexcept;

  /**
   * @brief Show a video feedback until the stop is requested.
   * You should not use the camera object until the stop function is called.
   * @throw CameraException if unable to read a frame
   * @return the stop function that returns the key pressed by the user
   */
  [[nodiscard]] std::function<void()> play();

  /**
   * @brief Show a video feedback until the use press a key.
   * You should not use the camera object until the stop function is called.
   * @param yn_key_only if true, the user can only exit by pressing Y or N key, but not others (default: false)
   * @throw CameraException if unable to read a frame
   * @return a promise that will be resolved by the value of the key pressed
   */
  [[nodiscard]] std::future<int> play_wait(bool yn_key_only = false);

  /**
   * @brief Read multiple frames.
   * @param capture_time_ms for how long (ms) collect frames
   * @throw CameraException if unable to read a frame
   * @return the frames
   */
  vector<cv::Mat> read_during(unsigned capture_time_ms);

  /**
   * @brief Check if the emitter is working by asking confirmation to the user.
   * @throw CameraException if unable to read a frame
   * @return true if yes, false if not
   */
  virtual bool is_emitter_working();

  /**
   * @brief Determine if the camera is in grayscale.
   * @throw CameraException if unable to read a frame
   * @return true if so, otherwise false.
   */
  bool is_gray_scale();

  /**
   * @brief Apply an instruction on the camera.
   * @param instruction to apply
   * @return true if success, otherwise false
   */
  bool apply(const CameraInstruction &instruction) noexcept;

  /**
   * @brief Change the current uvc control value for the camera device
   * @param unit extension unit ID
   * @param selector control selector
   * @param control control value
   * @return 1 if error, otherwise 0
   */
  int uvc_set_query(uint8_t unit, uint8_t selector, vector<uint8_t> &control) noexcept;

  /**
   * @brief Get the current, maximal, resolution or minimal value of the uvc control for the camera
   *device.
   * @param query_type UVC_GET_MAX, UVC_GET_RES, UVC_GET_CUR or UVC_GET_MIN
   * @param unit extension unit ID
   * @param selector control selector
   * @param controlSize size of the uvc control
   * @param control control value
   * @return 1 if error, otherwise 0
   */
  int uvc_get_query(uint8_t query_type, uint8_t unit, uint8_t selector,
                    vector<uint8_t> &control) noexcept;

  /**
   * @brief Get the size of the uvc control for the indicated device.
   * @param unit extension unit ID
   * @param selector control selector
   * @return size of the control, 0 if error
   */
  uint16_t uvc_len_query(uint8_t unit, uint8_t selector) noexcept;

  /**
   * @brief Return all /dev/videoX devices
   * @return path to the device
   */
  static vector<string> Devices() noexcept;
};

using CameraPtr = shared_ptr<Camera>;

/**
 * @brief Exception thrown by `Camera` class.
 */
class CameraException final : public exception {
 private:
  string message_;

 public:
  CameraException(const string &device, const string &error);

  const char *what() const noexcept override;
};