#include <fcntl.h>
#include <linux/usb/video.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <filesystem>
#include <format>
#include <functional>
#include <regex>
#include <stdexcept>
#include <thread>
using namespace std;

#include "camera.hpp"
#include "camerainstruction.hpp"
#include "logger.hpp"

// Value associated the keyboard key for OK
constexpr int OK_KEY = 121;
// Value associated to the keyboard key for NOT OK
constexpr int NOK_KEY = 110;

// Delay between each image capture of the camera in ms
constexpr int IMAGE_DELAY = 30;

// Regex pattern checking if the device is valid
const regex DEVICE_PATTERN("/dev/video([0-9]+)");

FileDescriptor::FileDescriptor(const string &device) : fd_(open(device.c_str(), O_WRONLY)) {
  if (fd_ < 0) throw Camera::Exception("Cannot access {}.", device);
}

FileDescriptor::~FileDescriptor() { close(fd_); }

FileDescriptor::operator int() const { return fd_; }

int Camera::execute_uvc_query(const uvc_xu_control_query &query) noexcept {
  auto res = ioctl(fd_, UVCIOC_CTRL_QUERY, &query);
  if (res != 0) {
    // ioctl debug not really useful for automated configuration generation since
    // linux-enable-ir-emitter v3
    // fprintf(stderr, "Ioctl error code: %d, errno: %d\n", res, errno);
    // switch (errno) {
    //   case ENOENT:
    //     fprintf(stderr,
    //             "The device does not support the given control or the specified extension unit "
    //             "could not be found.\n");
    //     break;
    //   case ENOBUFS:
    //     fprintf(stderr, "The specified buffer size is incorrect (too big or too small).\n");
    //     break;
    //   case EINVAL:
    //     fprintf(stderr, "An invalid request code was passed.\n");
    //     break;
    //   case EBADRQC:
    //     fprintf(stderr, "The given request is not supported by the given control.\n");
    //     break;
    //   case EFAULT:
    //     fprintf(stderr, "The data pointer references an inaccessible memory area.\n");
    //     break;
    //   case EILSEQ:
    //     fprintf(stderr, "Illegal byte sequence.\n");
    //     break;
    // }
    return 1;
  }
  return 0;
}

int Camera::show_frame_and_wait() {
  cv::Mat frame = read1();
  if (!no_gui_) {
    cv::imshow("linux-enable-ir-emitter", frame);
  }
  return cv::waitKey(IMAGE_DELAY);
}

static auto device_index(const string &device) {
  smatch match;
  if (regex_search(device, match, DEVICE_PATTERN)) return stoi(match[1]);
  throw Camera::Exception("Impossible to obtain the index of {}.", device);
}

static auto cannonical_device(const string &device) {
  try {
    return filesystem::canonical(device);
  } catch (const std::exception &e) {
    throw Camera::Exception("Impossible to obtain the canonical path of {}.", device);
  }
}

Camera::Camera(const string &_device, int width, int height)
    : device_(cannonical_device(_device)),
      fd_(device_),
      cap_(device_index(device_), cv::CAP_V4L,
           {
               cv::CAP_PROP_FOURCC,
               cv::VideoWriter::fourcc('G', 'R', 'E', 'Y'),
               cv::CAP_PROP_FRAME_WIDTH,
               width,
               cv::CAP_PROP_FRAME_HEIGHT,
               height,
           }) {
  cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);
}

string Camera::device() const noexcept { return device_; }

void Camera::disable_gui() noexcept { no_gui_ = true; }

std::function<void()> Camera::play(ExceptionPtr &eptr, bool key_exit) noexcept {
  auto show_video =
      std::make_shared<jthread>([&eptr, key_exit, this](const std::stop_token &stop) mutable {
        try {
          while (!stop.stop_requested()) {
            if (show_frame_and_wait() != -1 && key_exit) {
              break;
            }
          }
        } catch (Exception &e) {
          eptr = std::make_shared<Exception>(std::move(e));
        }
        if (!no_gui_) cv::destroyAllWindows();
      });

  auto stop = [show_video, key_exit]() {
    if (!key_exit) show_video->request_stop();
    show_video->join();
  };

  return stop;
}

cv::Mat Camera::read1() {
  cv::Mat frame;
  auto retry = IMAGE_DELAY;
  while (frame.empty() && --retry != 0) cap_.read(frame);
  if (retry == 0) throw Camera::Exception("Unable to read a frame from {}.", device_);
  return frame;
}

vector<cv::Mat> Camera::read_during(unsigned capture_time_ms) {
  vector<cv::Mat> frames;
  const auto stop_time = chrono::steady_clock::now() + chrono::milliseconds(capture_time_ms);
  while (chrono::steady_clock::now() < stop_time) frames.push_back(read1());
  return frames;
}

bool Camera::is_emitter_working() {
  ExceptionPtr eptr;
  auto stop = play(eptr);

  string answer;
  cout << "Is the ir emitter flashing (not just turn on)? Yes/No? ";
  cin >> answer;
  transform(answer.begin(), answer.end(), answer.begin(), [](char c) { return tolower(c); });
  while (answer != "yes" && answer != "y" && answer != "no" && answer != "n") {
    cout << "Yes/No? ";
    cin >> answer;
    transform(answer.begin(), answer.end(), answer.begin(), [](char c) { return tolower(c); });
  }

  stop();
  if (eptr) throw *eptr;
  return answer == "yes" || answer == "y";
}

bool Camera::is_gray_scale() {
  const cv::Mat frame = read1();
  if (frame.channels() != 3) return false;
  for (int r = 0; r < frame.rows; ++r) {
    for (int c = 0; c < frame.cols; ++c) {
      const cv::Vec3b &pixel = frame.at<cv::Vec3b>(r, c);
      if (pixel[0] != pixel[1] || pixel[0] != pixel[2]) return false;
    }
  }
  return true;
}

bool Camera::apply(const CameraInstruction &instruction) noexcept {
  const uvc_xu_control_query query = {
      instruction.unit(),
      instruction.selector(),
      UVC_SET_CUR,
      static_cast<uint16_t>(instruction.cur().size()),
      const_cast<uint8_t *>(instruction.cur().data()),  // const_cast safe; this is a set query
  };
  return execute_uvc_query(query) == 0;
}

int Camera::uvc_set_query(Unit unit, Selector selector, Control &control) noexcept {
  const uvc_xu_control_query query = {
      unit, selector, UVC_SET_CUR, static_cast<uint16_t>(control.size()), control.data(),
  };

  return execute_uvc_query(query);
}

int Camera::uvc_get_query(uint8_t query_type, Unit unit, Selector selector,
                          Control &control) noexcept {
  const uvc_xu_control_query query = {
      unit, selector, query_type, static_cast<uint16_t>(control.size()), control.data(),
  };

  return execute_uvc_query(query);
}

uint16_t Camera::uvc_len_query(Unit unit, Selector selector) noexcept {
  std::array<uint8_t, 2> data;
  const uvc_xu_control_query query = {
      unit, selector, UVC_GET_LEN, 2, data.data(),
  };

  if (execute_uvc_query(query) == 1) return 0;

  uint16_t len = 0;
  memcpy(&len, query.data, 2);
  return len;
}

vector<string> Camera::Devices() noexcept {
  vector<string> devices;
  auto paths = filesystem::directory_iterator("/dev");
  for (const auto &entry : paths) {
    auto dev_path = entry.path().string();
    if (regex_match(dev_path, DEVICE_PATTERN)) devices.push_back(dev_path);
  }
  return devices;
}
