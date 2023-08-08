#include "camerainstruction.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <linux/usb/video.h>
using namespace std;

#include "opencv.hpp"

#include "camera.hpp"
#include "../utils/logger.hpp"

/**
 * @brief Print the control value in the debug log
 *
 * @param prefixMasg what show before the control
 * @param control control value
 */
void CameraInstruction::logDebugCtrl(const string &prefixMsg, const vector<uint8_t> &control) noexcept
{
    string msg = prefixMsg;
    for (const uint8_t &i : control)
        msg += " " + to_string(i);
    Logger::debug(msg);
}

/**
 * @brief Construct a new CameraInstruction object
 *
 * @param camera on which find the control instruction
 * @param unit of the instruction
 * @param selector of the instruction
 *
 * @throw CameraInstructionException if information are missing for controlling the device
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
    {
        Logger::debug("Using default maximum control.");
        maxCtrl.assign(ctrlSize, 255);
    }

    // try get the minimum control value (it does not necessary exists)
    minCtrl.resize(ctrlSize);
    if (camera.getUvcQuery(UVC_GET_MIN, unit, selector, minCtrl) == 1)
    {
        Logger::debug("Using current as minimum control.");
        minCtrl.assign(curCtrl.begin(), curCtrl.end());
    }

    Logger::debug(("unit: " + to_string(unit) + " selector: " + to_string(selector)));
    logDebugCtrl("current:", curCtrl);
    logDebugCtrl("minimum:", minCtrl);
    logDebugCtrl("maximum:", maxCtrl);
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
    : unit(unit), selector(selector), curCtrl(control) {}

/**
 * @brief Compute the next possible control value
 *
 * @return true if the next value has been set,
 * false if the maximum control has already been set
 */
bool CameraInstruction::next() noexcept
{
    for (unsigned i = 0; i < curCtrl.size(); ++i)
    {
        uint16_t nextCtrli = static_cast<uint16_t>(curCtrl[i] + 1);
        if (nextCtrli > maxCtrl[i])
            curCtrl[i] = minCtrl[i]; // simulate "overflow"
        else 
        {
            curCtrl[i] = static_cast<uint8_t>(nextCtrli);
            logDebugCtrl("new current:", curCtrl);
            return true;
        }   
    }

    setMaxAsCur();

    return false;
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
bool CameraInstruction::setMinAsCur() noexcept
{
    if (minCtrl.empty() || curCtrl == minCtrl)
        return false;

    curCtrl.assign(minCtrl.begin(), minCtrl.end());
    logDebugCtrl("new current:", curCtrl);

    return true;
}

/**
 * @brief If a maximum control instruction
 * is not already the current,
 * set the current control instruction with that value
 *
 * @return true if success, otherwise false
 */
bool CameraInstruction::setMaxAsCur() noexcept
{
    if (curCtrl == maxCtrl)
        return false;

    curCtrl.assign(maxCtrl.begin(), maxCtrl.end());
    logDebugCtrl("new current:", curCtrl);

    return true;
}

CameraInstructionException::CameraInstructionException(const string &device, uint8_t unit, uint8_t selector)
    : message("ERROR: Impossible to obtain the instruction on " + device + " for unit: " + to_string(unit) + " selector:" + to_string(selector)) {}

const char *CameraInstructionException::what() const noexcept
{
    return message.c_str();
}
