#include "camera.hpp"

#include <iostream>
#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <errno.h>
#include <vector>
#include <sys/ioctl.h>
#include <linux/usb/video.h>
#include <linux/uvcvideo.h>
using namespace std;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wconversion"
#include <opencv2/videoio.hpp>
#include <opencv2/core/utils/logger.hpp>
#pragma GCC diagnostic pop

#include "logger.hpp"

/**
 * @brief Obtain the id of any device path
 *
 * @param device path to the camera
 * @return the device id
 */
int Camera::deviceId(const char *device) noexcept
{

    char devDevice[16];
    realpath(device, devDevice);
    int id;
    sscanf(devDevice, "/dev/video%d", &id);
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
 *
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
    : id(Camera::deviceId(device.c_str())), device(device)
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
bool Camera::apply(CameraInstruction instruction) noexcept
{
    uint8_t control[instruction.getSize()] = {};
    memcpy(control, instruction.getCurrent(), instruction.getSize() * sizeof(uint8_t));
    const struct uvc_xu_control_query query = {
        .unit = instruction.getUnit(),
        .selector = instruction.getSelector(),
        .query = UVC_SET_CUR,
        .size = instruction.getSize(),
        .data = control,
    };
    return executeUvcQuery(&query) == 0;
}

/**
 * @brief Check if the emitter is working
 *
 * @param deviceID id of the camera device
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
    cout << "Is the ir emitter flashing (not just turn on) ? Yes/No ? ";
    cin >> answer;
    while (answer != "yes" && answer != "y" && answer != "Yes" && answer != "Y" && answer != "no" && answer != "n" && answer != "No" && answer != "N")
    {
        cout << "Yes/No ? ";
        cin >> answer;
    }

    cap.release();
    frame.release();
    return answer == "yes" || answer == "y" || answer == "Yes" || answer == "Y";
}

/**
 * @brief Execute an uvc query on the device indicated by the file descriptor
 *
 * @param query uvc query to execute
 *
 * @return 1 if error, otherwise 0
 **/
int Camera::executeUvcQuery(const struct uvc_xu_control_query *query) noexcept
{
    openFd();
    errno = 0;
    int result = ioctl(fd, UVCIOC_CTRL_QUERY, query);
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
 * @param controlSize size of the uvc control
 * @param control control value
 *
 * @return 1 if error, otherwise 0
 **/
int Camera::setUvcQuery(uint8_t unit, uint8_t selector, uint16_t controlSize, uint8_t *control) noexcept
{
    const struct uvc_xu_control_query query = {
        .unit = unit,
        .selector = selector,
        .query = UVC_SET_CUR,
        .size = controlSize,
        .data = control,
    };

    return executeUvcQuery(&query);
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
int Camera::getUvcQuery(uint8_t query_type, uint8_t unit, uint8_t selector, uint16_t controlSize, uint8_t *control) noexcept
{
    const struct uvc_xu_control_query query = {
        .unit = unit,
        .selector = selector,
        .query = query_type,
        .size = controlSize,
        .data = control,
    };

    return executeUvcQuery(&query);
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
        .unit = unit,
        .selector = selector,
        .query = UVC_GET_LEN,
        .size = 2,
        .data = len,
    };

    if (executeUvcQuery(&query))
        return 0;

    return (uint16_t)(len[0] + len[1] * 16); // UVC_GET_LEN is in little-endian
}

/**
 * @brief Print the control value in the debug log
 *
 * @param prefixMasg what show before the control
 * @param control control value
 * @param len size of the control value
 */
void CameraInstruction::logDebugCtrl(string prefixMsg, const uint8_t *control, const uint16_t len) noexcept
{
    for (uint16_t i = 0; i < len; ++i)
        prefixMsg += " " + to_string((int)control[i]);
    Logger::debug(prefixMsg.c_str());
}

/**
 * @brief Check if minCtrl is coherent w.r.t. to maxCtrl
 *
 * @return false if minCtrl is nullptr or not coherent, otherwise true
 */
bool CameraInstruction::isMinConsistent() noexcept
{
    if (minCtrl == nullptr)
        return false;

    for (unsigned i = 0; i < ctrlSize; ++i)
        if (minCtrl[i] > maxCtrl[i])
            return false;

    return true;
}

/**
 * @brief Construct a new CameraInstruction object
 * And find the first control instruction as current one
 *
 * @param camera on which find the control instruction
 * @param unit of the instruction
 * @param selector of the instruction
 *
 * @throw CameraInstructionException if unit + selector are invalid or if the instruction cannot be modfied
 */
CameraInstruction::CameraInstruction(Camera &camera, uint8_t unit, uint8_t selector)
    : unit(unit), selector(selector)
{
    // get the control instruction lenght
    ctrlSize = camera.lenUvcQuery(unit, selector);
    if (ctrlSize == 0)
        throw CameraInstructionException(camera.device, unit, selector);

    // get the current control value
    curCtrl = new uint8_t[ctrlSize];
    if (camera.getUvcQuery(UVC_GET_CUR, unit, selector, ctrlSize, curCtrl))
        throw CameraInstructionException(camera.device, unit, selector);
    logDebugCtrl("current:", curCtrl, ctrlSize);

    // try to get the maximum control value (the value does not necessary exists)
    maxCtrl = new uint8_t[ctrlSize];
    if (camera.getUvcQuery(UVC_GET_MAX, unit, selector, ctrlSize, maxCtrl))
        memset(maxCtrl, 255, ctrlSize * sizeof(uint8_t)); // use the 255 array
    logDebugCtrl("maximum:", maxCtrl, ctrlSize);

    // try get the minimum control value (the value does not necessary exists)
    minCtrl = new uint8_t[ctrlSize];
    if (camera.getUvcQuery(UVC_GET_MIN, unit, selector, ctrlSize, minCtrl))
        logDebugCtrl("minimum:", minCtrl, ctrlSize);
    else
    {
        delete[] minCtrl;
        minCtrl = nullptr;
    }

    // try get the resolution control value (the value does not necessary exists)
    resCtrl = new uint8_t[ctrlSize];
    if (camera.getUvcQuery(UVC_GET_RES, unit, selector, ctrlSize, resCtrl))
    {
        Logger::debug("Computing the resolution control.");
        for (unsigned i = 0; i < ctrlSize; ++i)
        {
            // step of 0 or 1
            if (isMinConsistent())
                resCtrl[i] = (uint8_t)minCtrl[i] != maxCtrl[i];
            else
                resCtrl[i] = (uint8_t)curCtrl[i] != maxCtrl[i];
        }
    }
    logDebugCtrl("resolution:", resCtrl, ctrlSize);
}

/**
 * @brief Construct a new Camera Instruction object
 * Cannot compute next control instruction, and do not check if the instruction is valid
 *
 * @param unit of the instruction
 * @param selector of the instruction
 * @param control instruction
 */
CameraInstruction::CameraInstruction(uint8_t unit, uint8_t selector, uint8_t *control, uint16_t size)
    : unit(unit), selector(selector), ctrlSize(size), curCtrl(new uint8_t[size])
{
    memcpy(curCtrl, control, size * sizeof(uint8_t));
}

CameraInstruction::~CameraInstruction()
{
    delete[] curCtrl;
    delete[] maxCtrl;
    delete[] minCtrl;
    delete[] resCtrl;
}

CameraInstruction &CameraInstruction::operator=(const CameraInstruction &other)
{
    unit = other.unit;
    selector = other.selector;
    ctrlSize = other.ctrlSize;
    curCtrl = new uint8_t[ctrlSize];
    memcpy(curCtrl, other.curCtrl, ctrlSize * sizeof(uint8_t));
    if (other.maxCtrl != nullptr)
    {
        maxCtrl = new uint8_t[ctrlSize];
        memcpy(maxCtrl, other.maxCtrl, ctrlSize * sizeof(uint8_t));
    }
    else
        maxCtrl = nullptr;
    if (other.resCtrl != nullptr)
    {
        resCtrl = new uint8_t[ctrlSize];
        memcpy(resCtrl, other.resCtrl, ctrlSize * sizeof(uint8_t));
    }
    else
        resCtrl = nullptr;
    if (other.minCtrl != nullptr)
    {
        minCtrl = new uint8_t[ctrlSize];
        memcpy(minCtrl, other.minCtrl, ctrlSize * sizeof(uint8_t));
    }
    else
        minCtrl = nullptr;
    return *this;
}

CameraInstruction::CameraInstruction(const CameraInstruction &other)
{
    operator=(other);
}

/**
 * @brief Compute the next possible control value
 * @return true
 */
bool CameraInstruction::next()
{
    if (!hasNext())
        throw range_error("CRITICAL: Maximal instruction already reached.");

    for (unsigned i = 0; i < ctrlSize; ++i)
    {
        int nextCtrl = curCtrl[i] + resCtrl[i]; // int to avoid overflow
        curCtrl[i] = (uint8_t)nextCtrl;
        if (nextCtrl > maxCtrl[i]) // not allow to exceed maxCtrl
        {
            memcpy(curCtrl, maxCtrl, ctrlSize * sizeof(uint8_t));
            break;
        }
    }

    logDebugCtrl("new current:", curCtrl, ctrlSize);
    return true;
}

/**
 * @brief Check if a next control value can be computed
 *
 * @return true yes, otherwise false
 */
bool CameraInstruction::hasNext() const noexcept
{
    return maxCtrl != nullptr && memcmp(curCtrl, maxCtrl, ctrlSize * sizeof(uint8_t)) != 0;
}

/**
 * @brief Get the current control value
 *
 * @return const uint8_t const* current control value
 */
const uint8_t *CameraInstruction::getCurrent() const noexcept
{
    return curCtrl;
}

/**
 * @brief Get the size of the current control
 *
 * @return size of the current control
 */
uint16_t CameraInstruction::getSize() const noexcept
{
    return ctrlSize;
}

/**
 * @brief Get the unit of the instruction
 *
 * @return unit
 */
uint8_t CameraInstruction::getUnit() const noexcept
{
    return unit;
}

/**
 * @brief Get the selector of the instruction
 *
 * @return selector
 */
uint8_t CameraInstruction::getSelector() const noexcept
{
    return selector;
}

/**
 * @brief If a minimun control instruction exists and is consistent,
 * set the current control instruction with that value
 *
 * @return true if success, otherwise false
 */
bool CameraInstruction::trySetMinAsCur() noexcept
{
    if (!isMinConsistent())
        return false;

    memcpy(curCtrl, minCtrl, ctrlSize * sizeof(uint8_t));
    logDebugCtrl("new current:", curCtrl, ctrlSize);

    return true;
}

CameraException::CameraException(string device) : message("CRITICAL: Cannot access to " + device) {}

const char *CameraException::what()
{
    return message.c_str();
}

CameraInstructionException::CameraInstructionException(string device, uint8_t unit, uint8_t selector) : message("ERROR: Impossible to obtain the instruction on " + device + " for unit: " + to_string((int)unit) + " selector:" + to_string((int)selector)) {}

const char *CameraInstructionException::what()
{
    return message.c_str();
}
