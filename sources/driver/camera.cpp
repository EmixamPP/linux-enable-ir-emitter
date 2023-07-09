#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include "camera.hpp"

#include <vector>
#include <cstring>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/usb/video.h>
#include <linux/uvcvideo.h>
using namespace std;

#include <opencv2/videoio.hpp>
#include <opencv2/core/utils/logger.hpp>

#include "camerainstruction.hpp"
#include "../utils/logger.hpp"

/**
 * @brief Obtain the id of any device path
 *
 * @param device path to the camera
 *
 * @throw runtime_error if unable to obtain the /dev/videoX path
 *
 * @return the device id
 */
int Camera::deviceId(string device)
{
    char *devDevice = realpath(device.c_str(), NULL);
    int id;
    if (devDevice == NULL || sscanf(devDevice, "/dev/video%d", &id) != 1)
    {
        delete devDevice;
        throw runtime_error("CRITICAL: Unable to obtain the /dev/videoX path");
    }
    delete devDevice;
    return id;
}

/**
 * @brief Open a file discriptor if not yet open
 *
 * @throw CameraException if unable to open the camera device
 */
void Camera::openFd()
{
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

Camera::Camera(string device)
    : id(Camera::deviceId(device)), device(device)
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);
}

Camera::~Camera()
{
    closeFd();
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
bool Camera::apply(const CameraInstruction &instruction) noexcept
{
    const struct uvc_xu_control_query query = {
        instruction.getUnit(),
        instruction.getSelector(),
        UVC_SET_CUR,
        (uint16_t) instruction.getCurrent().size(),
        const_cast<uint8_t*>(instruction.getCurrent().data()), // const_cast safe; this is a set query
    };
    return executeUvcQuery(query) == 0;
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
    closeFd();
    cv::VideoCapture cap;
    cv::Mat frame;
    if (!cap.open(id, cv::CAP_V4L2) || !cap.read(frame))
        throw CameraException(device);

    string answer;
    cout << "Is the ir emitter flashing (not just turn on)? Yes/No? ";
    cin >> answer;

    while (answer.empty() || (answer[0] != 'y' && answer[0] != 'Y' && answer[0] != 'n' && answer[0] != 'N'))
    {
        cout << "Yes/No? ";
        cin >> answer;
    }

    cap.release();
    frame.release();
    return answer[0] == 'y' || answer[0] == 'Y';
}

/**
 * @brief Execute an uvc query on the device indicated by the file descriptor
 *
 * @param query uvc query to execute
 *
 * @return 1 if error, otherwise 0
 **/
int Camera::executeUvcQuery(const struct uvc_xu_control_query &query) noexcept
{
    openFd();
    errno = 0;
    int result = ioctl(fd, UVCIOC_CTRL_QUERY, &query);
    if (result || errno)
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
 * @return 1 if error, otherwise 0
 **/
int Camera::setUvcQuery(uint8_t unit, uint8_t selector, vector<uint8_t> &control) noexcept
{
    const struct uvc_xu_control_query query = {
        unit,
        selector,
        UVC_SET_CUR,
        (uint16_t) control.size(),
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
 * @return 1 if error, otherwise 0
 **/
int Camera::getUvcQuery(uint8_t query_type, uint8_t unit, uint8_t selector, vector<uint8_t> &control) noexcept
{
    const struct uvc_xu_control_query query = {
        unit,
        selector,
        query_type,
        (uint16_t) control.size(),
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
 * @return size of the control, 0 if error
 **/
uint16_t Camera::lenUvcQuery(uint8_t unit, uint8_t selector) noexcept
{
    uint8_t len[2] = {0x00, 0x00};
    const struct uvc_xu_control_query query = {
        unit,
        selector,
        UVC_GET_LEN,
        2,
        len,
    };

    if (executeUvcQuery(query))
        return 0;

    // UVC_GET_LEN is in little-endian
    return (uint16_t)(len[0] + len[1] * 16);
}

CameraException::CameraException(string device) : message("CRITICAL: Cannot access to " + device) {}

const char *CameraException::what() const noexcept
{
    return message.c_str();
}
