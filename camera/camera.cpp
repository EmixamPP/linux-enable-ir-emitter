#include "camera.hpp"

#include <cerrno>
#include <fcntl.h>
#include <linux/usb/video.h>
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
 * @brief Get the file descriptor previously opened
 *
 * @return the fd
 */
int Camera::getFd() const noexcept
{
    return fd;
}

/**
 * @brief Get the VideoCapture previsouly opened
 *
 * @return the cap
 */
shared_ptr<cv::VideoCapture> Camera::getCap() const noexcept
{
    return cap;
}

/**
 * @brief Obtain the id of any device path
 *
 * @param device path to the camera
 *
 * @return the device id
 */
int Camera::deviceId(const string &device)
{
    unique_ptr<char[], decltype(&free)> devDevice(realpath(device.c_str(), nullptr), &free);
    int id = 0;
    if (devDevice == nullptr || sscanf(devDevice.get(), "/dev/video%d", &id) != 1)
        Logger::critical(ExitCode::FAILURE, "Unable to obtain the /dev/videoX path");
    return id;
}

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
    closeFd();

    if (!cap->isOpened())
    {
        bool isOpened = cap->open(id, cv::CAP_V4L2);
        if (width > 0)
            cap->set(cv::CAP_PROP_FRAME_WIDTH, width);
        if (height > 0)
            cap->set(cv::CAP_PROP_FRAME_HEIGHT, height);
        if (!isOpened)
            throw CameraException(device);
    }
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
 * @brief Construct a new Camera:: Camera object
 *
 * @param device path to the camera
 */
Camera::Camera(const string &device, int width, int height)
    : id(Camera::deviceId(device)),  width(width), height(height), device(device)
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);
}

Camera::~Camera()
{
    closeFd();
    closeCap();
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
 * @brief Disable the video feedback for `isEmitterWorking()`
 */
void Camera::disableGui()
{
    noGui = true;
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

    bool res;
    if (noGui)
        res = isEmitterWorkingAskNoGui();
    else
        res = isEmitterWorkingAsk();

    closeCap();

    return res;
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
 * @brief Determine if the camera is in grayscale.
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
 * @brief Find a grayscale camera.
 *
 * @return path to the graycale device,
 * nullptr if unable to find such device
 */
shared_ptr<Camera> Camera::findGrayscaleCamera(int width, int height)
{
    vector<string> v4lDevices = get_V4L_devices();
    for (auto &device : v4lDevices)
    {
        shared_ptr<Camera> camera = make_shared<Camera>(device, width, height);
        try
        {
            if (camera->isGrayscale())
                return camera;
        }
        catch (const CameraException &e)
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
