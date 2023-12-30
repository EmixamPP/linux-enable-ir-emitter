#include "camerainstruction.hpp"

#include <fstream>
#include <utility>
#include <linux/usb/video.h>
using namespace std;

#include <yaml-cpp/yaml.h>

#include "camera.hpp"
#include "utils/logger.hpp"

/**
 * @brief Construct a new CameraInstruction object.
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
    initCtrl.resize(ctrlSize);
    if (camera.getUvcQuery(UVC_GET_CUR, unit, selector, curCtrl) == 1)
        throw CameraInstructionException(camera.device, unit, selector);
    initCtrl.assign(curCtrl.begin(), curCtrl.end());

    // ensure the control can be modified
    if (camera.setUvcQuery(unit, selector, curCtrl) == 1)
        throw CameraInstructionException(camera.device, unit, selector);

    // try to get the maximum control value (it does not necessary exists)
    maxCtrl.resize(ctrlSize);
    if (camera.getUvcQuery(UVC_GET_MAX, unit, selector, maxCtrl) == 1)
        maxCtrl.resize(0);

    // try get the minimum control value (it does not necessary exists)
    minCtrl.resize(ctrlSize);
    if (camera.getUvcQuery(UVC_GET_MIN, unit, selector, minCtrl) == 1)
        minCtrl.resize(0);
}

/**
 * @brief Compute the next possible control value.
 *
 * @return true if the next value has been set,
 * false if the maximum control has already been set
 */
bool CameraInstruction::next() noexcept
{
    if (curCtrl == maxCtrl)
        return false;

    for (size_t i = 0; i < curCtrl.size(); ++i)
    {
        uint16_t nextCtrli = static_cast<uint16_t>(curCtrl[i] + 1);
        if (nextCtrli > maxCtrl[i])
            curCtrl[i] = minCtrl.empty() ? initCtrl[i] : minCtrl[i]; // simulate "overflow"
        else
        {
            curCtrl[i] = static_cast<uint8_t>(nextCtrli);
            return true;
        }
    }

    // all are in overflow (should never arrive!)
    setMaxAsCur();
    return false;
}

/**
 * @brief Get the corruption status.
 *
 * @return true if the instruction is corrupted
 */
bool CameraInstruction::isCorrupted() const noexcept
{
    return corrupted;
}

/**
 * @brief Get the unit of the instruction.
 *
 * @return unit
 */
uint8_t CameraInstruction::getUnit() const noexcept
{
    return unit;
}

/**
 * @brief Get the selector of the instruction.
 *
 * @return selector
 */
uint8_t CameraInstruction::getSelector() const noexcept
{
    return selector;
}

/**
 * @brief Get the current control value.
 *
 * @return current control value
 */
const vector<uint8_t> &CameraInstruction::getCur() const noexcept
{
    return curCtrl;
}

/**
 * @brief Get the maximum of the instruction.
 *
 * @return maximum control
 */
const vector<uint8_t> &CameraInstruction::getMax() const noexcept
{
    return maxCtrl;
}

/**
 * @brief Get the minimum of the instruction.
 *
 * @return minimum control
 */
const vector<uint8_t> &CameraInstruction::getMin() const noexcept
{
    return minCtrl;
}

/**
 * @brief Get the initial control value.
 *
 * @return intial control value
 */
const vector<uint8_t> &CameraInstruction::getInit() const noexcept
{
    return initCtrl;
}

/**
 * @brief Changes the corrupted status of the instruction.
 *
 * @param isCorrupted status to set
 */
void CameraInstruction::setCorrupted(bool isCorrupted) noexcept
{
    corrupted = isCorrupted;
}

/**
 * @brief Sets a new current control value, if it is valid.
 *
 * @param newCur control to set
 *
 * @return true if success, otherwise false
 */
bool CameraInstruction::setCur(const vector<uint8_t> &newCur) noexcept
{
    if (curCtrl.size() != newCur.size())
        return false;

    
    for (size_t i = 0; i < newCur.size(); ++i)
    {
        if (!minCtrl.empty() && minCtrl[i] > newCur[i])
            return false;
        else if (!maxCtrl.empty() && maxCtrl[i] < newCur[i])
            return false;
    }

    curCtrl.assign(newCur.begin(), newCur.end());

    return true;
}

/**
 * @brief If a minimum control instruction exists
 * and is not already the current,
 * sets it as the current control instruction with that value.
 *
 * @return true if success, otherwise false
 */
bool CameraInstruction::setMinAsCur() noexcept
{
    if (minCtrl.empty() || curCtrl == minCtrl)
        return false;

    curCtrl.assign(minCtrl.begin(), minCtrl.end());

    return true;
}

/**
 * @brief If a maximum control instruction exists
 * and is not already the current,
 * set it as the current control instruction with that value.
 * If no maximum control exists,
 * set the maximum value possible (i.e. 255).
 *
 * @return true if success, otherwise false
 */
bool CameraInstruction::setMaxAsCur() noexcept
{
    if (curCtrl == maxCtrl)
        return false;
    else if (maxCtrl.empty())
        curCtrl.assign(curCtrl.size(), 255);
    else
        curCtrl.assign(maxCtrl.begin(), maxCtrl.end());

    return true;
}

/**
 * @brief Reset the current control
 * to the initial control value.
 */
void CameraInstruction::reset() noexcept
{
    curCtrl.assign(initCtrl.begin(), initCtrl.end());
}

string to_string(const CameraInstruction &inst)
{
    string res = "unit: " + to_string(inst.getUnit());
    res += ", selector: " + to_string(inst.getSelector());
    res += ", control: " + to_string(inst.getCur());
    return res;
}

string to_string(const vector<uint8_t> &vec)
{
    string res;
    for (auto v : vec)
        res += " " + to_string(v);
    return res.empty() ? res : res.substr(1);
}

CameraInstructionException::CameraInstructionException(const string &device, uint8_t unit, uint8_t selector)
    : message("Impossible to obtain the instruction on " + device + " for unit: " + to_string(unit) + " selector:" + to_string(selector)) {}

const char *CameraInstructionException::what() const noexcept
{
    return message.c_str();
}
