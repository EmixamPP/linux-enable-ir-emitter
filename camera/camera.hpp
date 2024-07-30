#pragma once

#include <linux/uvcvideo.h>

#include <memory>
#include <string>
#include <vector>
using namespace std;

#include <opencv2/core/utils/logger.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

class CameraException final : public exception {
 private:
  string message_;

 public:
  CameraException(const string &message);

  const char *what() const noexcept override;
};

class CameraInstruction;

class Camera {
 private:
  bool no_gui_ = false;
  int fd_ = -1;
  int index_;
  string device_;
  const vector<int> cap_para_;
  const shared_ptr<cv::VideoCapture> cap_ = make_shared<cv::VideoCapture>();

  void open_fd();

  void close_fd() noexcept;

  void open_cap();

  void close_cap() noexcept;

  int execute_uvc_query(const uvc_xu_control_query &query);

  cv::Mat read1_unsafe();

  bool is_emitter_working_ask();

  bool is_emitter_working_ask_no_gui();

 public:
  Camera() = delete;

  explicit Camera(const string &device, int width = -1, int height = -1);

  virtual ~Camera();

  Camera &operator=(const Camera &) = delete;

  Camera(const Camera &) = delete;

  Camera &operator=(Camera &&other) = delete;

  Camera(Camera &&other) = delete;

  string device() const noexcept;

  void disable_gui() noexcept;

  function<void()> play();

  void play_forever();

  cv::Mat read1();

  vector<cv::Mat> read_during(unsigned capture_time_ms);

  virtual bool is_emitter_working();

  bool is_gray_scale();

  bool apply(const CameraInstruction &instruction);

  int uvc_set_query(uint8_t unit, uint8_t selector, vector<uint8_t> &control);

  int uvc_get_query(uint8_t query_type, uint8_t unit, uint8_t selector, vector<uint8_t> &control);

  uint16_t uvc_len_query(uint8_t unit, uint8_t selector);

  static vector<string> Devices();
};
