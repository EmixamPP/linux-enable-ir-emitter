#include "camerainstruction.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <linux/usb/video.h>
using namespace std;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include <opencv2/videoio.hpp>
#include <opencv2/core/utils/logger.hpp>
#pragma GCC diagnostic pop

#include "camera.hpp"
#include "../utils/math.hpp"
#include "../utils/logger.hpp"

/**
 * @brief Print the control value in the debug log
 *
 * @param prefixMasg what show before the control
 * @param control control value
 */
void CameraInstruction::logDebugCtrl(string prefixMsg, const vector<uint8_t> &control) noexcept
{
    for (auto &i : control)
        prefixMsg += " " + to_string((int)i);
    if (!prefixMsg.empty())
        Logger::debug(prefixMsg);
}

/**
 * @brief Compute the resolution control instruction composed of 0 or 1
 * by comparing two controls instruction
 * we assume isReachable(first, res, second, size) is true
 *
 * @param first the first instruction
 * @param second the second instruction
 * @param res the resolution instruction will be stored in it
 */
void CameraInstruction::computeResCtrl(const vector<uint8_t> &first, const vector<uint8_t> &second, vector<uint8_t> &res) noexcept
{
    int secondGcd = array_gcd(second);

    if (secondGcd > 1)
        for (unsigned i = 0; i < first.size(); ++i)
            res[i] = (second[i] - first[i]) / secondGcd;
    else
        for (unsigned i = 0; i < first.size(); ++i)
            res[i] = (uint8_t)second[i] != first[i];
}

/**
 * @brief Check if a resolution control allow to reach a control from another
 *
 * @param base instruction from which the resolution must be added
 * @param res the resolution control
 * @param toReach the instruction to reach
 *
 * @return true if reacheable, otherwise false
 */
bool CameraInstruction::isReachable(const vector<uint8_t> &base, const vector<uint8_t> &res, const vector<uint8_t> &toReach) noexcept
{
    int it = 256;
    for (unsigned i = 0; i < base.size(); ++i)
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
    uint16_t ctrlSize = camera.lenUvcQuery(unit, selector);
    if (ctrlSize == 0)
        throw CameraInstructionException(camera.device, unit, selector);

    // get the current control value
    curCtrl.resize(ctrlSize);
    if (camera.getUvcQuery(UVC_GET_CUR, unit, selector, curCtrl) == 1)
        throw CameraInstructionException(camera.device, unit, selector);

    // ensure the control can be modified
    if (camera.setUvcQuery(unit, selector, curCtrl) == 1)
        throw CameraInstructionException(camera.device, unit, selector);

    // try to get the maximum control value (it does not necessary exists)
    maxCtrl.resize(ctrlSize);
    if (camera.getUvcQuery(UVC_GET_MAX, unit, selector, maxCtrl) == 1)
        throw CameraInstructionException(camera.device, unit, selector);

    // try get the minimum control value (it does not necessary exists)
    minCtrl.resize(ctrlSize);
    if (camera.getUvcQuery(UVC_GET_MIN, unit, selector, minCtrl) == 1)
        minCtrl.resize(0);

    // try to get the resolution control value (it does not necessary exists)
    // and check if it is consistent
    resCtrl.resize(ctrlSize);
    if (camera.getUvcQuery(UVC_GET_RES, unit, selector, resCtrl) == 1 ||
        !isReachable(curCtrl, resCtrl, maxCtrl))
    {
        Logger::debug("Computing the resolution control.");

        if (!minCtrl.empty())
        {
            computeResCtrl(minCtrl, curCtrl, resCtrl);
            if (!isReachable(minCtrl, resCtrl, curCtrl))
            {
                Logger::debug("Minimum not consistent, it will be ignored.");
                minCtrl.resize(0);
                computeResCtrl(curCtrl, maxCtrl, resCtrl);
            }
        }
        else
            computeResCtrl(curCtrl, maxCtrl, resCtrl);
    }

    logDebugCtrl("current:", curCtrl);
    logDebugCtrl("maximum:", maxCtrl);
    logDebugCtrl("minimum:", minCtrl);
    logDebugCtrl("resolution:", resCtrl);
    Logger::debug(("unit: " + to_string((int)unit) + " selector: " + to_string((int)selector)));
}

/**
 * @brief Construct a new Camera Instruction object
 * Cannot compute next control instruction, and do not check if the instruction is valid
 *
 * @param unit of the instruction
 * @param selector of the instruction
 * @param control instruction
 */
CameraInstruction::CameraInstruction(uint8_t unit, uint8_t selector, const vector<uint8_t> &control)
    : unit(unit), selector(selector), curCtrl(control){}

CameraInstruction &CameraInstruction::operator=(const CameraInstruction &other)
{
    unit = other.unit;
    selector = other.selector;
    curCtrl = other.curCtrl;
    maxCtrl = other.maxCtrl;
    resCtrl = other.resCtrl;
    minCtrl = other.minCtrl;
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

    for (unsigned i = 0; i < curCtrl.size(); ++i)
    {
        int nextCtrl = curCtrl[i] + resCtrl[i]; // int to avoid overflow
        curCtrl[i] = (uint8_t)nextCtrl;
        if (nextCtrl > maxCtrl[i]) // not allow to exceed maxCtrl
        {
            curCtrl.assign(maxCtrl.begin(), maxCtrl.end());
            break;
        }
    }

    logDebugCtrl("new current:", curCtrl);
    return true;
}

/**
 * @brief Check if a next control value can be computed
 *
 * @return true yes, otherwise false
 */
bool CameraInstruction::hasNext() const noexcept
{
    return curCtrl != maxCtrl;
}

/**
 * @brief Get the current control value
 *
 * @return current control value
 */
const vector<uint8_t> &CameraInstruction::getCurrent() const noexcept
{
    return curCtrl;
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
    if (minCtrl.empty() || curCtrl == minCtrl)
        return false;

    curCtrl.assign(minCtrl.begin(), minCtrl.end());
    logDebugCtrl("new current:", curCtrl);

    return true;
}

CameraInstructionException::CameraInstructionException(string device, uint8_t unit, uint8_t selector)
    : message("ERROR: Impossible to obtain the instruction on " + device + " for unit: " + to_string((int)unit) + " selector:" + to_string((int)selector)) {}

const char *CameraInstructionException::what() const noexcept
{
    return message.c_str();
}
