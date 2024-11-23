#pragma once

#include <linux/uvcvideo.h>

#include <format>
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

#include "camerainstruction.hpp"

class FileDescriptor {
  // File descriptor
  int fd_;

 public:
  FileDescriptor(const string &device);
  ~FileDescriptor();
  operator int() const;
  FileDescriptor(const FileDescriptor &) = default;
  FileDescriptor &operator=(const FileDescriptor &) = default;
  FileDescriptor(FileDescriptor &&) = default;
  FileDescriptor &operator=(FileDescriptor &&) = default;
};

/**
 * @brief The `Camera` object allows to capture frames from a camera device for a specific amount,
 * duration or until requested, while displaying a video feedback (or not) to the user, and asking
 * him to determine if the ir emitter is working. In addition, it can automatically determine if the
 * camera is in grayscale. Futhermore, the object can perform UVC queries on the camera device to
 * control it. Lastly, the class can also gives the list of all /dev/videoX devices.
 */
class Camera {
  using Unit = CameraInstruction::Unit;
  using Selector = CameraInstruction::Selector;
  using Control = CameraInstruction::Control;

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
   * @throw Camera::Exception if unable to read a frame
   * @return the frame
   */
  cv::Mat read1();

  /**
   * @brief Show a frame (if `no_gui` is false)
   * and wait for `IMAGE_DELAY`ms or the user to press a key.
   * @throw Camera::Exception if unable to read a frame
   * @return the value of the key pressed by the user, -1 if no key pressed
   */
  int show_frame_and_wait();

 public:
  class Exception;
  using ExceptionPtr = shared_ptr<Exception>;

  /**
   * @brief Construct a new Camera object.
   * @param device path of the camera
   * @param width of the capture resolution (default: -1)
   * @param height of the capture resolution (default: -1)
   * @throw Camera::Exception if the device is invalid
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
   * @brief Disable the video feedback during `play()`.
   */
  void disable_gui() noexcept;

  /**
   * @brief Show a video feedback until the stop is requested or a key is pressed.
   * You should not use the camera object until the stop function is called.
   * @param eptr exception pointer to retrieve possible `Camera::Exception` if unable to read a
   * frame
   * @param key_exit if true, the user can also exit by pressing any key (default: false)
   * @return a request stop function if `key_exit` is false, otherwise a waiting function
   */
  [[nodiscard]] std::function<void()> play(ExceptionPtr &eptr, bool key_exit = false) noexcept;

  /**
   * @brief Read multiple frames.
   * @param capture_time_ms for how long (ms) collect frames
   * @throw Camera::Exception if unable to read a frame
   * @return the frames
   */
  vector<cv::Mat> read_during(unsigned capture_time_ms);

  /**
   * @brief Check if the emitter is working by asking confirmation to the user.
   * @throw Camera::Exception if unable to read a frame
   * @return true if yes, false if not
   */
  virtual bool is_emitter_working();

  /**
   * @brief Determine if the camera is in grayscale.
   * @throw Camera::Exception if unable to read a frame
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
  int uvc_set_query(Unit unit, Selector selector, Control &control) noexcept;

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
  int uvc_get_query(uint8_t query_type, Unit unit, Selector selector, Control &control) noexcept;

  /**
   * @brief Get the size of the uvc control for the indicated device.
   * @param unit extension unit ID
   * @param selector control selector
   * @return size of the control, 0 if error
   */
  uint16_t uvc_len_query(Unit unit, Selector selector) noexcept;

  /**
   * @brief Return all /dev/videoX devices
   * @return path to the device
   */
  static vector<string> Devices() noexcept;

  class Exception : public exception {
    string message;

   public:
    template <typename... Args>
    explicit Exception(const string &format, Args... args)
        : message(std::vformat(format, std::make_format_args(args...))) {}

    const char *what() const noexcept override { return message.c_str(); }
  };
};

using CameraPtr = shared_ptr<Camera>;
