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
#include "globals.hpp"
#include "utils/logger.hpp"

constexpr int OK_KEY = 121;
constexpr int NOK_KEY = 110;
constexpr int IMAGE_DELAY = 30;

/**
 * @brief Open a file discriptor if not yet open
 *
 * @throw CameraException if unable to open the camera device
 */
void Camera::openFd()
{
    closeCap();

    if (fd < 0)
    {
        errno = 0;
        fd = open(device.c_str(), O_WRONLY);
        if (fd < 0 || errno)
            throw CameraException(device);
    }
}

/**
 * @brief Close the current file descriptor
 */
void Camera::closeFd() noexcept
{
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
}

/**
 * @brief Open a VideoCapture if not yet open
 *
 * @throw CameraException if unable to open the camera device
 */
void Camera::openCap()
{
    if (!cap->open(id, cv::CAP_V4L, capParams))
        throw CameraException(device);
}

/**
 * @brief Close the current VideoCapture
 */
void Camera::closeCap() noexcept
{
    if (cap->isOpened())
        cap->release();
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
int Camera::executeUvcQuery(const uvc_xu_control_query &query)
{
    openFd();
    errno = 0;
    const int result = ioctl(fd, UVCIOC_CTRL_QUERY, &query);
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
 * Must be called between `openCap()` and `closeCap()`.
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return true if yes, false if not
 */
bool Camera::isEmitterWorkingAsk()
{
    cv::Mat frame;
    int key = -1;

    cout << "Is the video flashing? Press Y or N in the window." << endl;
    while (key != OK_KEY && key != NOK_KEY)
    {
        cap->read(frame);
        cv::imshow("linux-enable-ir-emitter", frame);
        key = cv::waitKey(IMAGE_DELAY);
    }
    Logger::debug(key == OK_KEY ? "Y pressed." : "N pressed.");

    cv::destroyAllWindows();
    return key == OK_KEY;
}

/**
 * @brief Trigger the camera
 * and asks if the emitter is working.
 * Must be called between `openCap()` and `closeCap()`.
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return true if yes, false if not
 */
bool Camera::isEmitterWorkingAskNoGui()
{
    cv::Mat frame;
    cap->read(frame);

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
    Logger::debug(answer, " inputed.");

    return answer == "yes" || answer == "y";
}

/**
 * @brief Obtain the id of any device path
 *
 * @param device path to the camera
 *
 * @return the device id
 */
static int deviceId(const string &device)
{
    auto realPath = filesystem::canonical(device).string();
    regex pattern("/dev/video([0-9]+)");
    smatch match;
    if (!regex_search(realPath, match, pattern))
        Logger::critical(ExitCode::FAILURE, device + " is not of the regex form /dev/video[0-9]+");
    return stoi(match[1]);
}

/**
 * @brief Construct a new Camera:: Camera object
 *
 * @param device path to the camera
 * @param width of the capture resolution
 * @param height of the capture resolution
 */
Camera::Camera(const string &device, int width, int height)
    : capParams({
          cv::CAP_PROP_FOURCC,
          cv::VideoWriter::fourcc('G', 'R', 'E', 'Y'),
          cv::CAP_PROP_FRAME_WIDTH,
          width,
          cv::CAP_PROP_FRAME_HEIGHT,
          height,
      }),
      id(deviceId(device)),
      device(device)
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);
}

Camera::~Camera()
{
    closeFd();
    closeCap();
}

/**
 * @brief Disable the video feedback for `isEmitterWorking()`
 */
void Camera::disableGui()
{
    noGui = true;
}

/**
 * @brief Show a video feedback until the
 * stop funciton is called.
 * You should not use the camera object
 * until the stop function is called.
 *
 * @return a stop function
 */
function<void()> Camera::play()
{
    openCap();

    shared_ptr<bool> stop = make_shared<bool>(false);

    shared_ptr<thread> showVideo = make_shared<thread>(
        [this, stop]()
        {
            cv::Mat frame;
            while (!(*stop))
            {
                cap->read(frame);
                cv::imshow("linux-enable-ir-emitter", frame);
                cv::waitKey(IMAGE_DELAY);
            }
        });

    return [this, stop, showVideo]()
    {
        *stop = true;
        showVideo->join();
        closeCap();
        cv::destroyAllWindows();
    };
}

/**
 * @brief Show a video feedback until the user exit
 * by pressing any key
 */
void Camera::playForever()
{
    openCap();
    cv::Mat frame;
    int key = -1;

    cout << "Press any key in the window to close" << endl;

    while (key == -1)
    {
        cap->read(frame);
        cv::imshow("linux-enable-ir-emitter", frame);
        key = cv::waitKey(IMAGE_DELAY);
    }

    closeCap();
    cv::destroyAllWindows();
}

/**
 * @brief Read one frame
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return the frame
 */
cv::Mat Camera::read1()
{
    openCap();
    cv::Mat frame;
    cap->read(frame);
    closeCap();
    return frame;
}

/**
 * @brief Read multiple frames
 *
 * @param captureTimeMs for how long (ms) collect frames
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return the frames
 */
vector<cv::Mat> Camera::readDuring(unsigned captureTimeMs)
{
    openCap();
    vector<cv::Mat> frames;
    const auto stopTime = chrono::steady_clock::now() + chrono::milliseconds(captureTimeMs);
    while (chrono::steady_clock::now() < stopTime)
    {
        cv::Mat frame;
        cap->read(frame);
        if (!frame.empty())
            frames.push_back(move(frame));
    }
    closeCap();
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
bool Camera::isEmitterWorking()
{
    openCap();
    bool res = noGui ? isEmitterWorkingAskNoGui() : isEmitterWorkingAsk();
    closeCap();
    return res;
}

/**
 * @brief Determine if the camera is in greyscale.
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return true if so, otheriwse false.
 */
bool Camera::isGrayscale()
{
    cv::Mat frame = read1();

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
        instruction.getUnit(),
        instruction.getSelector(),
        UVC_SET_CUR,
        static_cast<uint16_t>(instruction.getCur().size()),
        const_cast<uint8_t *>(instruction.getCur().data()), // const_cast safe; this is a set query
    };
    return executeUvcQuery(query) == 0;
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
int Camera::setUvcQuery(uint8_t unit, uint8_t selector, vector<uint8_t> &control)
{
    const uvc_xu_control_query query = {
        unit,
        selector,
        UVC_SET_CUR,
        static_cast<uint16_t>(control.size()),
        control.data(),
    };

    return executeUvcQuery(query);
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
int Camera::getUvcQuery(uint8_t query_type, uint8_t unit, uint8_t selector, vector<uint8_t> &control)
{
    const uvc_xu_control_query query = {
        unit,
        selector,
        query_type,
        static_cast<uint16_t>(control.size()),
        control.data(),
    };

    return executeUvcQuery(query);
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
uint16_t Camera::lenUvcQuery(uint8_t unit, uint8_t selector)
{
    uint8_t data[2] = {0x00, 0x00};
    const uvc_xu_control_query query = {
        unit,
        selector,
        UVC_GET_LEN,
        2,
        data,
    };

    if (executeUvcQuery(query) == 1)
        return 0;

    uint16_t len = 0;
    memcpy(&len, query.data, 2);
    return len;
}

/**
 * @brief Find a greyscale camera.
 *
 * @return path to the greycale device,
 * nullptr if unable to find such device
 */
shared_ptr<Camera> Camera::findGrayscaleCamera(int width, int height)
{
    vector<string> v4lDevices = get_V4L_devices();
    for (auto &device : v4lDevices)
    {
        Logger::debug("Checking if", device, "is a greyscale camera.");
        try
        {
            auto camera = make_shared<Camera>(device, width, height);
            if (camera->isGrayscale())
            {
                Logger::debug(device, "is a greyscale camera.");
                return camera;
            }
        }
        catch (const CameraException &)
        { // ignore
        }
    }

    return nullptr;
}

CameraException::CameraException(const string &device) : message("Cannot access to " + device) {}

const char *CameraException::what() const noexcept
{
    return message.c_str();
}
