#include <linux/usb/video.h>

#include <format>
using namespace std;

#include "camera.hpp"
#include "camerainstruction.hpp"

#define NOT_DISABLE_OR_THROW()    \
  if (status_ == Status::DISABLE) \
  throw Exception("The instruction unit {} selector {} is disabled.", unit_, selector_)

CameraInstruction::CameraInstruction(Camera &camera, Unit unit, Selector selector, Status status)
    : unit_(unit), selector_(selector), status_(status), ctrl_block_(0) {
  // get the control instruction length
  const uint16_t ctrl_size = camera.uvc_len_query(unit, selector);
  if (ctrl_size == 0)
    throw Exception("Impossible to query instruction size for unit {} and selector {} on {}", unit,
                    selector, camera.device());

  // get the current control
  cur_.resize(ctrl_size);
  auto res = camera.uvc_get_query(UVC_GET_CUR, unit, selector, cur_);
  if (res == 1)
    throw Exception("Impossible to fetch current instruction for unit {} and selector {} on {}",
                    unit, selector, camera.device());

  // ensure the control can be modified
  res = camera.uvc_set_query(unit, selector, cur_);
  if (res == 1)
    throw Exception("Impossible to set current instruction for unit {} and selector {} on {}", unit,
                    selector, camera.device());

  // save the current as initial
  init_.assign(cur_.begin(), cur_.end());

  // try to get the maximum control (it does not necessary exists)
  max_.resize(ctrl_size);
  if (camera.uvc_get_query(UVC_GET_MAX, unit, selector, max_) != 0) min_.resize(0);

  // try get the minimum control (it does not necessary exists)
  min_.resize(ctrl_size);
  if (camera.uvc_get_query(UVC_GET_MIN, unit, selector, min_) != 0) min_.resize(0);
}

CameraInstruction::CameraInstruction(Unit unit, Selector selector, Control cur, Control init,
                                     Control max, Control min, Status status)
    : unit_(unit),
      selector_(selector),
      status_(status),
      cur_(std::move(cur)),
      init_(std::move(init)),
      max_(std::move(max)),
      min_(std::move(min)),
      ctrl_block_(0) {}

uint8_t CameraInstruction::unit() const noexcept { return unit_; }

uint8_t CameraInstruction::selector() const noexcept { return selector_; }

auto CameraInstruction::cur() const noexcept -> const Control & { return cur_; }

bool CameraInstruction::set_cur(Control cur) {
  NOT_DISABLE_OR_THROW();
  if (cur_.size() != cur.size()) return false;

  // check if each control block is between min and max included
  for (size_t i = 0; i < cur.size(); ++i) {
    if ((!min_.empty() && min_[i] > cur[i]) || (!max_.empty() && max_[i] < cur[i])) return false;
  }
  cur_ = std::move(cur);
  return true;
}

auto CameraInstruction::max() const noexcept -> const Control & { return max_; }

bool CameraInstruction::set_max_cur() {
  NOT_DISABLE_OR_THROW();
  if (max_.empty())
    cur_.assign(cur_.size(), UINT8_MAX);
  else
    cur_.assign(max_.begin(), max_.end());
  return true;
}

auto CameraInstruction::min() const noexcept -> const Control & { return min_; }

bool CameraInstruction::set_min_cur() {
  NOT_DISABLE_OR_THROW();
  if (min_.empty()) return false;
  cur_.assign(min_.begin(), min_.end());
  return true;
}

auto CameraInstruction::init() const noexcept -> const Control & { return init_; }

auto CameraInstruction::status() const noexcept -> Status { return status_; }

void CameraInstruction::set_status(Status status) noexcept { status_ = status; }

void CameraInstruction::reset() {
  NOT_DISABLE_OR_THROW();
  cur_.assign(init_.begin(), init_.end());
}

bool CameraInstruction::next() {
  NOT_DISABLE_OR_THROW();

  // while not all control block have been modified
  while (ctrl_block_ < cur_.size()) {
    // next value for the control block i; 8 bits (cast to detect overflow)
    const unsigned next_ctrl_i = static_cast<unsigned>(cur_[ctrl_block_] + 1);
    // maximum value for the control block i
    const unsigned max_ctrl_i = max_.empty() ? UINT8_MAX : max_[ctrl_block_];

    // maximum exceed or overflow occurred
    if (next_ctrl_i > max_ctrl_i) {
      // reset to initial
      cur_[ctrl_block_] = init_[ctrl_block_];
      // move to the next control block
      ++ctrl_block_;
      continue;
    }

    // modify this control block and return
    cur_[ctrl_block_] = static_cast<uint8_t>(next_ctrl_i);
    return true;
  }

  // all control block have been incremented
  return false;
}

string to_string(const CameraInstruction &inst) {
  return std::format("unit: {}, selector: {}, control: {}", to_string(inst.unit()),
                     to_string(inst.selector()), to_string(inst.cur()));
}

string to_string(const CameraInstruction::Control &vec) {
  string str;
  for (size_t i = 0; i < vec.size(); ++i) {
    str += to_string(vec[i]);
    if (i + 1 < vec.size()) str += " ";
  }
  return str;
}

string to_string(uint8_t v) { return to_string(static_cast<int>(v)); }

string to_string(CameraInstruction::Status status) {
  switch (status) {
    case CameraInstruction::Status::START:
      return "start";
    case CameraInstruction::Status::IDLE:
      return "idle";
    case CameraInstruction::Status::DISABLE:
      return "disable";
    default:
      throw CameraInstruction::Exception("Unknown status value {}",
                                         to_string(static_cast<int>(status)));
  }
}

CameraInstruction::Status from_string(const std::string &status_str) {
  if (status_str == "start") return CameraInstruction::Status::START;
  if (status_str == "idle") return CameraInstruction::Status::IDLE;
  if (status_str == "disable") return CameraInstruction::Status::DISABLE;
  throw CameraInstruction::Exception("Unknown status string " + status_str);
}