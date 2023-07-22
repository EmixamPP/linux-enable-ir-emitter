#include "camera.hpp"

#include <cstdint>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <linux/usb/video.h>
#include <linux/uvcvideo.h>
#include <memory>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>
using namespace std;

#include "opencv.hpp"

#include "camerainstruction.hpp"
#include "../utils/logger.hpp"

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
 * @throw runtime_error if unable to obtain the /dev/videoX path
 *
 * @return the device id
 */
int Camera::deviceId(const string &device)
{
    std::unique_ptr<char[], decltype(&free)> devDevice(realpath(device.c_str(), nullptr), &free);
    int id;
    if (devDevice == nullptr || sscanf(devDevice.get(), "/dev/video%d", &id) != 1)
        throw runtime_error("CRITICAL: Unable to obtain the /dev/videoX path");
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

Camera::Camera(const string &device)
    : id(Camera::deviceId(device)), device(device)
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);
}

Camera::~Camera()
{
    closeFd();
    closeCap();
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
        static_cast<uint16_t>(instruction.getCurrent().size()),
        const_cast<uint8_t *>(instruction.getCurrent().data()), // const_cast safe; this is a set query
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
unique_ptr<cv::Mat> Camera::read1()
{
    openCap();
    auto frame = make_unique<cv::Mat>();
    cap->read(*frame);
    closeCap();
    return frame;
}

/**
 * @brief Check if the emitter is working
 *
 * @throw CameraException if unable to open the camera device
 *
 * @return true if yes, false if not
 */
bool Camera::isEmitterWorking()
{
    openCap();
    cv::Mat frame;
    cap->read(frame);

    string answer;
    cout << "Is the ir emitter flashing (not just turn on)? Yes/No? ";
    cin >> answer;

    while (answer.empty() || (answer[0] != 'y' && answer[0] != 'Y' && answer[0] != 'n' && answer[0] != 'N'))
    {
        cout << "Yes/No? ";
        cin >> answer;
    }

    closeCap();
    return answer[0] == 'y' || answer[0] == 'Y';
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
        /* // ioctl debug not really useful for automated driver generation since linux-enable-ir-emitter v3
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
    uint8_t len[2] = {0x00, 0x00};
    const uvc_xu_control_query query = {
        unit,
        selector,
        UVC_GET_LEN,
        2,
        len,
    };

    if (executeUvcQuery(query) == 1)
        return 0;

    // UVC_GET_LEN is in little-endian
    return static_cast<uint16_t>(len[0] + len[1] * 16);
}

CameraException::CameraException(const string &device) : message("CRITICAL: Cannot access to " + device) {}

const char *CameraException::what() const noexcept
{
    return message.c_str();
}
