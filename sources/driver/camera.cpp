#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <errno.h>
#include <vector>
#include <numeric>
#include <sys/ioctl.h>
#include <linux/usb/video.h>
#include <linux/uvcvideo.h>
using namespace std;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include <opencv2/videoio.hpp>
#include <opencv2/core/utils/logger.hpp>
#pragma GCC diagnostic pop

#include "logger.hpp"
#include "camera.hpp"

int array_gcd(const uint8_t *arr, uint16_t size)
{
    int result = arr[0];
    for (int i = 1; i < size; ++i)
    {
        result = gcd(arr[i], result);
        if (result == 1)
            return 1;
    }
    return result;
}

/**
 * @brief Obtain the id of any device path
 *
 * @param device path to the camera
 * @return the device id
 */
int Camera::deviceId(const char *device)
{

    char *devDevice = realpath(device, NULL);
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
    uint8_t *control = new uint8_t[instruction.getSize()];
    memcpy(control, instruction.getCurrent(), instruction.getSize() * sizeof(uint8_t));
    const struct uvc_xu_control_query query = {
        instruction.getUnit(),
        instruction.getSelector(),
        UVC_SET_CUR,
        instruction.getSize(),
        control,
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
int Camera::setUvcQuery(uint8_t unit, uint8_t selector, uint16_t controlSize, uint8_t *control) noexcept /* NOLINT(readability-non-const-parameter) */
{
    const struct uvc_xu_control_query query = {
        unit,
        selector,
        UVC_SET_CUR,
        controlSize,
        control,
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
int Camera::getUvcQuery(uint8_t query_type, uint8_t unit, uint8_t selector, uint16_t controlSize, uint8_t *control) noexcept /* NOLINT(readability-non-const-parameter) */
{
    const struct uvc_xu_control_query query = {
        unit,
        selector,
        query_type,
        controlSize,
        control,
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
        unit,
        selector,
        UVC_GET_LEN,
        2,
        len,
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
void CameraInstruction::logDebugCtrl(string prefixMsg, const uint8_t *control, uint16_t len) noexcept
{
    for (uint16_t i = 0; i < len; ++i)
        prefixMsg += " " + to_string((int)control[i]);
    Logger::debug(prefixMsg.c_str());
}

/**
 * @brief Compute the resolution control instruction composed of 0 or 1
 * by comparing two controls instruction
 * we assume isReachable(first, res, second, size) is true
 *
 * @param first the first instruction
 * @param second the second instruction
 * @param res the resolution instruction will be stored in it
 * @param size of the instructions
 */
void CameraInstruction::computeResCtrl(const uint8_t *first, const uint8_t *second, uint8_t *res, uint16_t size) noexcept
{
    int secondGcd = array_gcd(second, size);

    if (secondGcd > 1)
        for (unsigned i = 0; i < size; ++i)
            res[i] = (second[i] - first[i]) / secondGcd;
    else
        for (unsigned i = 0; i < size; ++i)
            res[i] = (uint8_t)second[i] != first[i];
}

/**
 * @brief Check if a resolution control allow to reach a control from another
 *
 * @param base instruction from which the resolution must be added
 * @param res the resolution control
 * @param toReach the instruction to reach
 * @param size of the instructions
 * @return true if reacheable, otherwise false
 */
bool CameraInstruction::isReachable(const uint8_t *base, const uint8_t *res, const uint8_t *toReach, uint16_t size) noexcept
{
    int it = 256;
    for (unsigned i = 0; i < size; ++i)
    {
        if (res[i] != 0)
        {
            int newit = (toReach[i] - base[i]) / res[i]; // # iterations required for that value
            if (newit < 0 || (newit != it && it != 256)) // negative iteration or not all value have the same # iterations
                return false;
            it = newit;
        }
    }

    return it != 256;
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
    if (camera.getUvcQuery(UVC_GET_CUR, unit, selector, ctrlSize, curCtrl) == 1)
        throw CameraInstructionException(camera.device, unit, selector);

    // ensure the control can be modified
    if (camera.setUvcQuery(unit, selector, ctrlSize, curCtrl) == 1)
        throw CameraInstructionException(camera.device, unit, selector);

    // try to get the maximum control value (it does not necessary exists)
    maxCtrl = new uint8_t[ctrlSize];
    if (camera.getUvcQuery(UVC_GET_MAX, unit, selector, ctrlSize, maxCtrl) == 1)
        throw CameraInstructionException(camera.device, unit, selector);

    // try get the minimum control value (it does not necessary exists)
    minCtrl = new uint8_t[ctrlSize];
    if (camera.getUvcQuery(UVC_GET_MIN, unit, selector, ctrlSize, minCtrl) == 1)
    {
        delete[] minCtrl;
        minCtrl = nullptr;
    }

    // try to get the resolution control value (it does not necessary exists)
    // and check if it is consistent
    resCtrl = new uint8_t[ctrlSize];
    if (camera.getUvcQuery(UVC_GET_RES, unit, selector, ctrlSize, resCtrl) == 1 ||
        !isReachable(curCtrl, resCtrl, maxCtrl, ctrlSize))
    {
        Logger::debug("Computing the resolution control.");

        if (minCtrl != nullptr)
        {
            computeResCtrl(minCtrl, curCtrl, resCtrl, ctrlSize);
            if (!isReachable(minCtrl, resCtrl, curCtrl, ctrlSize))
            {
                Logger::debug("Minimum not consistent, it will be ignored.");
                delete[] minCtrl;
                minCtrl = nullptr;
                computeResCtrl(curCtrl, maxCtrl, resCtrl, ctrlSize);
            }
        }
        else
            computeResCtrl(curCtrl, maxCtrl, resCtrl, ctrlSize);
    }

    logDebugCtrl("current:", curCtrl, ctrlSize);
    logDebugCtrl("maximum:", maxCtrl, ctrlSize);
    if (minCtrl != nullptr)
        logDebugCtrl("minimum:", minCtrl, ctrlSize);
    logDebugCtrl("resolution:", resCtrl, ctrlSize);
    Logger::debug(("unit: " + to_string((int)unit) + " selector: " + to_string((int)selector)).c_str());
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
 * @brief If a minimun control instruction exists
 * and is not already the current,
 * set the current control instruction with that value
 *
 * @return true if success, otherwise false
 */
bool CameraInstruction::trySetMinAsCur() noexcept
{
    if (minCtrl == nullptr || memcmp(curCtrl, minCtrl, ctrlSize * sizeof(uint8_t)) == 0)
        return false;

    memcpy(curCtrl, minCtrl, ctrlSize * sizeof(uint8_t));
    logDebugCtrl("new current:", curCtrl, ctrlSize);

    return true;
}

CameraException::CameraException(string device) : message("CRITICAL: Cannot access to " + device) {}

const char *CameraException::what() const noexcept
{
    return message.c_str();
}

CameraInstructionException::CameraInstructionException(string device, uint8_t unit, uint8_t selector)
    : message("ERROR: Impossible to obtain the instruction on " + device + " for unit: " + to_string((int)unit) + " selector:" + to_string((int)selector)) {}

const char *CameraInstructionException::what() const noexcept
{
    return message.c_str();
}