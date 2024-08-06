#include <linux/usb/video.h>

#include <format>
#include <fstream>
#include <utility>
using namespace std;

#include "camera.hpp"
#include "camerainstruction.hpp"

CameraInstruction::CameraInstruction(Camera &camera, uint8_t unit, uint8_t selector, bool disable)
    : unit_(unit), selector_(selector), disable_(disable) {
  // get the control instruction length
  const uint16_t ctrl_size = camera.uvc_len_query(unit, selector);
  if (ctrl_size == 0) throw CameraInstructionException(camera.device(), unit, selector);

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
  if (camera.uvc_get_query(UVC_GET_MAX, unit, selector, max_ctrl_) == 1) max_ctrl_.resize(0);

  // try get the minimum control value (it does not necessary exists)
  min_ctrl_.resize(ctrl_size);
  if (camera.uvc_get_query(UVC_GET_MIN, unit, selector, min_ctrl_) == 1) min_ctrl_.resize(0);
}

bool CameraInstruction::next() noexcept {
  if (cur_ctrl_ == max_ctrl_) return false;

  for (size_t i = 0; i < cur_ctrl_.size(); ++i) {
    const uint16_t next_ctrl_i = static_cast<uint16_t>(cur_ctrl_[i] + 1);
    if (next_ctrl_i > max_ctrl_[i])
      cur_ctrl_[i] = min_ctrl_.empty() ? init_ctrl_[i] : min_ctrl_[i];  // simulate "overflow"
    else {
      cur_ctrl_[i] = static_cast<uint8_t>(next_ctrl_i);
      return true;
    }
  }

  // all are in overflow (should never arrive!)
  set_max_cur();
  return false;
}

bool CameraInstruction::is_disable() const noexcept { return disable_; }

uint8_t CameraInstruction::unit() const noexcept { return unit_; }

uint8_t CameraInstruction::selector() const noexcept { return selector_; }

const vector<uint8_t> &CameraInstruction::cur() const noexcept { return cur_ctrl_; }

const vector<uint8_t> &CameraInstruction::max() const noexcept { return max_ctrl_; }

const vector<uint8_t> &CameraInstruction::min() const noexcept { return min_ctrl_; }

const vector<uint8_t> &CameraInstruction::init() const noexcept { return init_ctrl_; }

void CameraInstruction::set_disable(bool is_disable) noexcept { disable_ = is_disable; }

bool CameraInstruction::set_cur(const vector<uint8_t> &cur) noexcept {
  if (cur_ctrl_.size() != cur.size()) return false;

  for (size_t i = 0; i < cur.size(); ++i) {
    if ((!min_ctrl_.empty() && min_ctrl_[i] > cur[i]) ||
        (!max_ctrl_.empty() && max_ctrl_[i] < cur[i]))
      return false;
  }

  cur_ctrl_.assign(cur.begin(), cur.end());

  return true;
}

bool CameraInstruction::set_min_cur() noexcept {
  if (min_ctrl_.empty() || cur_ctrl_ == min_ctrl_) return false;

  cur_ctrl_.assign(min_ctrl_.begin(), min_ctrl_.end());

  return true;
}

bool CameraInstruction::set_max_cur() noexcept {
  if (cur_ctrl_ == max_ctrl_) return false;

  if (max_ctrl_.empty())
    cur_ctrl_.assign(cur_ctrl_.size(), UINT8_MAX);
  else
    cur_ctrl_.assign(max_ctrl_.begin(), max_ctrl_.end());

  return true;
}

void CameraInstruction::reset() noexcept { cur_ctrl_.assign(init_ctrl_.begin(), init_ctrl_.end()); }

string to_string(const CameraInstruction &inst) {
  return std::format("unit: {}, selector: {}, control: {}",
                     to_string(static_cast<int>(inst.unit())),
                     to_string(static_cast<int>(inst.selector())), to_string(inst.cur()));
}

string to_string(const vector<uint8_t> &vec) {
  string str;
  for (size_t i = 0; i < vec.size(); ++i) {
    str += to_string(static_cast<int>(vec[i]));
    if (i + 1 < vec.size()) str += " ";
  }
  return str;
}

CameraInstructionException::CameraInstructionException(const string &device, uint8_t unit,
                                                       uint8_t selector)
    : message(std::format("Impossible to obtain the instruction on {} for unit: {} selector: {}.",
                          device, to_string(static_cast<int>(unit)),
                          to_string(static_cast<int>(selector)))) {}

const char *CameraInstructionException::what() const noexcept { return message.c_str(); }
