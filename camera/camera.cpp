#include "camera.hpp"

#include <cerrno>
#include <fcntl.h>
#include <filesystem>
#include <linux/usb/video.h>
#include <regex>
#include <stdexcept>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
using namespace std;

#include "camerainstruction.hpp"
#include "utils/logger.hpp"

constexpr int OK_KEY = 121;
constexpr int NOK_KEY = 110;
constexpr int IMAGE_DELAY = 30;

const regex DEVICE_PATTERN("/dev/video([0-9]+)");

/**
 * @brief Open a file discriptor if not yet open
 *
 * @throw CameraException if unable to open the camera device
 */
void Camera::open_fd()
{
    close_cap();

    if (fd_ < 0)
    {
        errno = 0;
        fd_ = open(device_.c_str(), O_WRONLY);
        if (fd_ < 0 || errno)
            throw CameraException("Cannot access to " + device_);
    }
}

/**
 * @brief Close the current file descriptor
 */
void Camera::close_fd() noexcept
{
    if (fd_ != -1)
    {
        close(fd_);
        fd_ = -1;
    }
}

/**
 * @brief Open a VideoCapture if not yet open
 *
 * @throw CameraException if unable to open the camera device
 */
void Camera::open_cap()
{
    if (cap_->isOpened())
        return;

    if (!cap_->open(index_, cv::CAP_V4L, cap_para_))
        throw CameraException("OpenCV cannot access to " + device_);
}

/**
 * @brief Close the current VideoCapture
 */
void Camera::close_cap() noexcept
{
    if (cap_->isOpened())
        cap_->release();
}

/**
 * @brief Execute an uvc query on the device indicated by the file descriptor
 *
 * @param query uvc query to execute
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return 1 if error, otherwise 0
 **/
int Camera::execute_uvc_query(const uvc_xu_control_query &query)
{
    open_fd();
    errno = 0;
    const int result = ioctl(fd_, UVCIOC_CTRL_QUERY, &query);
    if (result == 1 || errno)
    {
        /* // ioctl debug not really useful for automated configuration generation since linux-enable-ir-emitter v3
        fprintf(stderr, "Ioctl error code: %d, errno: %d\n", result, errno);
        switch (errno) {
        case ENOENT:
             fprintf(stderr, "The device does not support the given control or the specified extension unit could not be found.\n");
             break;
        case ENOBUFS:
             fprintf(stderr, "The specified buffer size is incorrect (too big or too small).\n");
             break;
        case EINVAL:
             fprintf(stderr, "An invalid request code was passed.\n");
             break;
        case EBADRQC:
             fprintf(stderr, "The given request is not supported by the given control.\n");
             break;
        case EFAULT:
             fprintf(stderr, "The data pointer references an inaccessible memory area.\n");
             break;
        case EILSEQ:
             fprintf(stderr, "Illegal byte sequence.\n");
             break;
        }*/
        return 1;
    }
    return 0;
}

/**
 * @brief Show a video feedback to the user
 * and asks if the emitter is working.
 * Must be called between `open_cap()` and `close_cap()`.
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return true if yes, false if not
 */
bool Camera::is_emitter_working_ask()
{
    open_cap();

    cout << "Is the video flashing? Press Y or N in the window." << endl;
    int key = -1;
    while (key != OK_KEY && key != NOK_KEY)
    {
        cv::imshow("linux-enable-ir-emitter", read1_unsafe());
        key = cv::waitKey(IMAGE_DELAY);
    }
    Logger::debug(key == OK_KEY ? "Y pressed." : "N pressed.");

    cv::destroyAllWindows();
    close_cap();

    return key == OK_KEY;
}

/**
 * @brief Trigger the camera
 * and asks if the emitter is working.
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return true if yes, false if not
 */
bool Camera::is_emitter_working_ask_no_gui()
{
    open_cap();
    read1_unsafe();

    string answer;
    cout << "Is the ir emitter flashing (not just turn on) ? Yes/No ? ";
    cin >> answer;
    transform(answer.begin(), answer.end(), answer.begin(), [](char c)
              { return tolower(c); });

    while (answer != "yes" && answer != "y" && answer != "no" && answer != "n")
    {
        cout << "Yes/No ? ";
        cin >> answer;
        transform(answer.begin(), answer.end(), answer.begin(), [](char c)
                  { return tolower(c); });
    }

    close_cap();

    return answer == "yes" || answer == "y";
}

/**
 * @brief Construct a new Camera:: Camera object
 *
 * @param device path to the camera
 * @param width of the capture resolution
 * @param height of the capture resolution
 *
 * @throw CameraException if the device is invalid
 */
Camera::Camera(const string &device, int width, int height)
    : cap_para_({
          cv::CAP_PROP_FOURCC,
          cv::VideoWriter::fourcc('G', 'R', 'E', 'Y'),
          cv::CAP_PROP_FRAME_WIDTH,
          width,
          cv::CAP_PROP_FRAME_HEIGHT,
          height,
      })
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);

    if (!filesystem::exists(device))
        throw CameraException(device + " does not exists.");

    if (regex_match(device, DEVICE_PATTERN))
        device_ = device;
    else
        // try to obtain the /dev/videoX form by taking the caninical path
        device_ = filesystem::canonical(device).string();

    smatch match;
    if (regex_search(device_, match, DEVICE_PATTERN))
        index_ = stoi(match[1]);
    else
        throw CameraException("Impossible to obtain the index of " + device);
}

Camera::~Camera()
{
    close_fd();
    close_cap();
}

/**
 * @brief Get the device
 *
 * @return device path
 */
string Camera::device() const noexcept
{
    return device_;
}

/**
 * @brief Disable the video feedback for `is_emitter_working()`
 */
void Camera::disable_gui() noexcept
{
    no_gui_ = true;
}

/**
 * @brief Show a video feedback until the
 * stop funciton is called.
 * You should not use the camera object
 * until the stop function is called
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return a stop function
 */
function<void()> Camera::play()
{
    open_cap();

    shared_ptr<bool> stop = make_shared<bool>(false);

    shared_ptr<thread> show_video = make_shared<thread>(
        [this, stop]()
        {
            cv::Mat frame;
            while (!(*stop))
            {
                cv::imshow("linux-enable-ir-emitter", read1_unsafe());
                cv::waitKey(IMAGE_DELAY);
            }
        });

    return [this, stop, show_video]()
    {
        *stop = true;
        show_video->join();
        cv::destroyAllWindows();
        close_cap();
    };
}

/**
 * @brief Show a video feedback until the user exit
 * by pressing any key
 *
 * @throw CameraException if unable to open the camera device
 *
 */
void Camera::play_forever()
{
    open_cap();

    cv::Mat frame;
    int key = -1;

    cout << "Press any key in the window to close" << endl;

    while (key == -1)
    {
        cv::imshow("linux-enable-ir-emitter", read1_unsafe());
        key = cv::waitKey(IMAGE_DELAY);
    }

    cv::destroyAllWindows();

    close_cap();
}

/**
 * @brief Reads one frame
 * Must be called between `open_cap()` and `close_cap()`,
 * otherwise it will cause exceptions.
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return the frame
 */
cv::Mat Camera::read1_unsafe()
{
    cv::Mat frame;
    auto retry = IMAGE_DELAY;
    while (frame.empty() && retry-- != 0)
        cap_->read(frame);

    if (retry == 0)
        throw CameraException("Unable to read a frame from " + device_);

    return frame;
}

/**
 * @brief Reads one frame
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return the frame
 */
cv::Mat Camera::read1()
{
    open_cap();
    auto frame = read1_unsafe();
    close_cap();
    return frame;
}

/**
 * @brief Read multiple frames
 *
 * @param capture_time_ms for how long (ms) collect frames
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return the frames
 */
vector<cv::Mat> Camera::read_during(unsigned capture_time_ms)
{
    open_cap();

    vector<cv::Mat> frames;
    const auto stop_time = chrono::steady_clock::now() + chrono::milliseconds(capture_time_ms);
    while (chrono::steady_clock::now() < stop_time)
        frames.push_back(read1_unsafe());

    close_cap();

    return frames;
}

/**
 * @brief Check if the emitter is working
 * by asking confirmation to the user
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return true if yes, false if not
 */
bool Camera::is_emitter_working()
{
    bool res = no_gui_ ? is_emitter_working_ask_no_gui() : is_emitter_working_ask();
    return res;
}

/**
 * @brief Determine if the camera is in greyscale.
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return true if so, otheriwse false.
 */
bool Camera::is_gray_scale()
{
    const cv::Mat frame = read1();

    if (frame.channels() != 3)
        return false;

    for (int r = 0; r < frame.rows; ++r)
        for (int c = 0; c < frame.cols; ++c)
        {
            const cv::Vec3b &pixel = frame.at<cv::Vec3b>(r, c);
            if (pixel[0] != pixel[1] || pixel[0] != pixel[2])
                return false;
        }

    return true;
}

/**
 * @brief Apply an instruction on the camera
 *
 * @param instruction to apply
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return true if success, otherwise false
 */
bool Camera::apply(const CameraInstruction &instruction)
{
    const uvc_xu_control_query query = {
        instruction.unit(),
        instruction.selector(),
        UVC_SET_CUR,
        static_cast<uint16_t>(instruction.cur().size()),
        const_cast<uint8_t *>(instruction.cur().data()), // const_cast safe; this is a set query
    };
    return execute_uvc_query(query) == 0;
}

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
 **/
int Camera::uvc_set_query(uint8_t unit, uint8_t selector, vector<uint8_t> &control)
{
    const uvc_xu_control_query query = {
        unit,
        selector,
        UVC_SET_CUR,
        static_cast<uint16_t>(control.size()),
        control.data(),
    };

    return execute_uvc_query(query);
}

/**
 * @brief Get the current, maximale, resolution or minimale value of the uvc control for the camera device
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
 **/
int Camera::uvc_get_query(uint8_t query_type, uint8_t unit, uint8_t selector, vector<uint8_t> &control)
{
    const uvc_xu_control_query query = {
        unit,
        selector,
        query_type,
        static_cast<uint16_t>(control.size()),
        control.data(),
    };

    return execute_uvc_query(query);
}

/**
 * @brief Get the size of the uvc control for the indicated device.
 *
 * @param unit extension unit ID
 * @param selector control selector
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return size of the control, 0 if error
 **/
uint16_t Camera::uvc_len_query(uint8_t unit, uint8_t selector)
{
    uint8_t data[2] = {0x00, 0x00};
    const uvc_xu_control_query query = {
        unit,
        selector,
        UVC_GET_LEN,
        2,
        data,
    };

    if (execute_uvc_query(query) == 1)
        return 0;

    uint16_t len = 0;
    memcpy(&len, query.data, 2);
    return len;
}

/**
 * @brief Return all /dev/videoX devices
 *
 * @return path to the device
 */
vector<string> Camera::Devices()
{
    vector<string> devices;
    auto paths = filesystem::directory_iterator("/dev");
    for (const auto &entry : paths)
    {
        auto dev_path = entry.path().string();
        if (regex_match(dev_path, DEVICE_PATTERN))
            devices.push_back(dev_path);
    }
    return devices;
}

CameraException::CameraException(const string &message) : message_(message) {}

const char *CameraException::what() const noexcept
{
    return message_.c_str();
}
