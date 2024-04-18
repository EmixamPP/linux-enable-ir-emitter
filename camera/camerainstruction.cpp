#include "camerainstruction.hpp"

#include <fstream>
#include <utility>
#include <linux/usb/video.h>
using namespace std;

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
    : unit_(unit), selector_(selector)
{
    // get the control instruction lenght
    const uint16_t ctrl_size = camera.uvc_len_query(unit, selector);
    if (ctrl_size == 0)
        throw CameraInstructionException(camera.device(), unit, selector);

    // get the current control value
    cur_ctrl_.resize(ctrl_size);
    init_ctrl_.resize(ctrl_size);
    if (camera.uvc_get_query(UVC_GET_CUR, unit, selector, cur_ctrl_) == 1)
        throw CameraInstructionException(camera.device(), unit, selector);
    init_ctrl_.assign(cur_ctrl_.begin(), cur_ctrl_.end());

    // ensure the control can be modified
    if (camera.uvc_set_query(unit, selector, cur_ctrl_) == 1)
        throw CameraInstructionException(camera.device(), unit, selector);

    // try to get the maximum control value (it does not necessary exists)
    max_ctrl_.resize(ctrl_size);
    if (camera.uvc_get_query(UVC_GET_MAX, unit, selector, max_ctrl_) == 1)
        max_ctrl_.resize(0);

    // try get the minimum control value (it does not necessary exists)
    min_ctrl_.resize(ctrl_size);
    if (camera.uvc_get_query(UVC_GET_MIN, unit, selector, min_ctrl_) == 1)
        min_ctrl_.resize(0);
}

/**
 * @brief Compute the next possible control value.
 *
 * @return true if the next value has been set,
 * false if the maximum control has already been set
 */
bool CameraInstruction::next() noexcept
{
    if (cur_ctrl_ == max_ctrl_)
        return false;

    for (size_t i = 0; i < cur_ctrl_.size(); ++i)
    {
        const uint16_t next_ctrl_i = static_cast<uint16_t>(cur_ctrl_[i] + 1);
        if (next_ctrl_i > max_ctrl_[i])
            cur_ctrl_[i] = min_ctrl_.empty() ? init_ctrl_[i] : min_ctrl_[i]; // simulate "overflow"
        else
        {
            cur_ctrl_[i] = static_cast<uint8_t>(next_ctrl_i);
            return true;
        }
    }

    // all are in overflow (should never arrive!)
    set_max_cur();
    return false;
}

/**
 * @brief Get the corruption status.
 *
 * @return true if the instruction is corrupted
 */
bool CameraInstruction::is_corrupted() const noexcept
{
    return corrupted_;
}

/**
 * @brief Get the unit of the instruction.
 *
 * @return unit
 */
uint8_t CameraInstruction::unit() const noexcept
{
    return unit_;
}

/**
 * @brief Get the selector of the instruction.
 *
 * @return selector
 */
uint8_t CameraInstruction::selector() const noexcept
{
    return selector_;
}

/**
 * @brief Get the current control value.
 *
 * @return current control value
 */
const vector<uint8_t> &CameraInstruction::cur() const noexcept
{
    return cur_ctrl_;
}

/**
 * @brief Get the maximum of the instruction.
 *
 * @return maximum control
 */
const vector<uint8_t> &CameraInstruction::max() const noexcept
{
    return max_ctrl_;
}

/**
 * @brief Get the minimum of the instruction.
 *
 * @return minimum control
 */
const vector<uint8_t> &CameraInstruction::min() const noexcept
{
    return min_ctrl_;
}

/**
 * @brief Get the initial control value.
 *
 * @return intial control value
 */
const vector<uint8_t> &CameraInstruction::init() const noexcept
{
    return init_ctrl_;
}

/**
 * @brief Changes the corrupted status of the instruction.
 *
 * @param is_corrupted status to set
 */
void CameraInstruction::set_corrupted(bool is_corrupted) noexcept
{
    corrupted_ = is_corrupted;
}

/**
 * @brief Sets a new current control value, if it is valid.
 *
 * @param cur control to set
 *
 * @return true if success, otherwise false
 */
bool CameraInstruction::set_cur(const vector<uint8_t> &cur) noexcept
{
    if (cur_ctrl_.size() != cur.size())
        return false;

    
    for (size_t i = 0; i < cur.size(); ++i)
    {
        if (!min_ctrl_.empty() && min_ctrl_[i] > cur[i])
            return false;
        else if (!max_ctrl_.empty() && max_ctrl_[i] < cur[i])
            return false;
    }

    cur_ctrl_.assign(cur.begin(), cur.end());

    return true;
}

/**
 * @brief If a minimum control instruction exists
 * and is not already the current,
 * sets it as the current control instruction with that value.
 *
 * @return true if success, otherwise false
 */
bool CameraInstruction::set_min_cur() noexcept
{
    if (min_ctrl_.empty() || cur_ctrl_ == min_ctrl_)
        return false;

    cur_ctrl_.assign(min_ctrl_.begin(), min_ctrl_.end());

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
bool CameraInstruction::set_max_cur() noexcept
{
    if (cur_ctrl_ == max_ctrl_)
        return false;
    else if (max_ctrl_.empty())
        cur_ctrl_.assign(cur_ctrl_.size(), 255);
    else
        cur_ctrl_.assign(max_ctrl_.begin(), max_ctrl_.end());

    return true;
}

/**
 * @brief Reset the current control
 * to the initial control value.
 */
void CameraInstruction::reset() noexcept
{
    cur_ctrl_.assign(init_ctrl_.begin(), init_ctrl_.end());
}

string to_string(const CameraInstruction &inst)
{
    string res = "unit: " + to_string(inst.unit());
    res += ", selector: " + to_string(inst.selector());
    res += ", control: " + to_string(inst.cur());
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
