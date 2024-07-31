#pragma once

#include <string>
#include <vector>
using namespace std;

#include <yaml-cpp/yaml.h>

class Camera;

/**
 * @brief A `CameraInstruction` object represents an instruction that can be used to control a
 * camera device by setting the a control value. Given a camera, an unit and a selector,
 * the object will automatically get the current control value, the maximum and minimum one.
 * Based on these two last, the object can compute the next possible control value.
 *
 */
class CameraInstruction {
  // Unit used on the camera to identify the control
  uint8_t unit_;

  // Selector used on the camera to identify the control
  uint8_t selector_;

  // Flag to mark the control as disable
  bool disable_;

  // Current control value
  vector<uint8_t> cur_ctrl_;

  // Initial control value
  vector<uint8_t> init_ctrl_;

  // Maximum control value
  vector<uint8_t> max_ctrl_;

  // Minimum control value
  vector<uint8_t> min_ctrl_;

 public:
  CameraInstruction() = default;

  /**
   * @brief Construct a new CameraInstruction object.
   *
   * @param camera on which find the control instruction
   * @param unit of the instruction
   * @param selector of the instruction
   *
   * @throw CameraInstructionException if information are missing for controlling
   * the device
   */
  explicit CameraInstruction(Camera &camera, uint8_t unit, uint8_t selector, bool disable = false);

  ~CameraInstruction() = default;

  CameraInstruction &operator=(const CameraInstruction &) = default;

  CameraInstruction(const CameraInstruction &) = default;

  CameraInstruction &operator=(CameraInstruction &&) = default;

  CameraInstruction(CameraInstruction &&) = default;

  /**
   * @brief Compute the next possible control value.
   *
   * @return true if the next value has been set,
   * false if the maximum control has already been set
   */
  bool next() noexcept;

  /**
   * @brief Get the disable status.
   *
   * @return true if the instruction is disable
   */
  bool is_disable() const noexcept;

  /**
   * @brief Get the unit of the instruction.
   *
   * @return unit
   */
  uint8_t unit() const noexcept;

  /**
   * @brief Get the selector of the instruction.
   *
   * @return selector
   */
  uint8_t selector() const noexcept;

  /**
   * @brief Get the current control value.
   *
   * @return minimum control
   */
  const vector<uint8_t> &cur() const noexcept;

  /**
   * @brief Get the maximum of the instruction.
   *
   * @return minimum control
   */
  const vector<uint8_t> &max() const noexcept;

  /**
   * @brief Get the minimum of the instruction.
   *
   * @return maximum control
   */
  const vector<uint8_t> &min() const noexcept;

  /**
   * @brief Get the initial control value.
   *
   * @return initial control value
   */
  const vector<uint8_t> &init() const noexcept;

  /**
   * @brief Changes the disable status of the instruction.
   *
   * @param is_disable status to set
   */
  void set_disable(bool is_disable) noexcept;

  /**
   * @brief Sets a new current control value, if it is valid.
   *
   * @param cur control to set
   *
   * @return true if success, otherwise false
   */
  bool set_cur(const vector<uint8_t> &cur) noexcept;

  /**
   * @brief If a minimum control instruction exists and is not already the current, sets it as the
   * current control instruction with that value.
   *
   * @return true if success, otherwise false
   */
  bool set_min_cur() noexcept;

  /**
   * @brief If a maximum control instruction exists and is not already the current, set it as the
   * current control instruction with that value. If no maximum control exists, set the maximum
   * value possible (i.e. UINT8_MAX).
   *
   * @return true if success, otherwise false
   */
  bool set_max_cur() noexcept;

  /**
   * @brief Reset the current control to the initial control value.
   */
  void reset() noexcept;

  friend struct YAML::convert<CameraInstruction>;
};

string to_string(const CameraInstruction &inst);

string to_string(const vector<uint8_t> &vec);

using CameraInstructions = vector<CameraInstruction>;

/**
 * @brief Exception thrown by the `CameraInstruction`.
 *
 */
class CameraInstructionException : public exception {
 private:
  string message;

 public:
  explicit CameraInstructionException(const string &device, uint8_t unit, uint8_t selector);

  const char *what() const noexcept override;
};

/**
 * @brief Convert a `CameraInstruction` object to a YAML node.
 * Useful to serialize/deserialize the object, in order to be stored in a file.
 */
namespace YAML {
template <>
struct convert<CameraInstruction> {
  static Node encode(const CameraInstruction &obj) {
    Node node;
    node["disable"] = obj.disable_;
    node["unit"] = static_cast<int>(obj.unit_);
    node["selector"] = static_cast<int>(obj.selector_);

    for (const auto &v : obj.cur_ctrl_) node["current"].push_back(static_cast<int>(v));

    if (obj.cur_ctrl_ != obj.init_ctrl_)
      for (const auto &v : obj.init_ctrl_) node["initial"].push_back(static_cast<int>(v));

    for (const auto &v : obj.max_ctrl_) node["maximum"].push_back(static_cast<int>(v));

    for (const auto &v : obj.min_ctrl_) node["minimum"].push_back(static_cast<int>(v));

    return node;
  }

  static bool decode(const Node &node, CameraInstruction &obj) {
    try {
      obj.disable_ = node["disable"].as<bool>();
      obj.unit_ = node["unit"].as<uint8_t>();
      obj.selector_ = node["selector"].as<uint8_t>();

      obj.cur_ctrl_ = node["current"].as<vector<uint8_t>>();

      if (node["initial"])
        obj.init_ctrl_ = node["initial"].as<vector<uint8_t>>();
      else
        obj.init_ctrl_.assign(obj.cur_ctrl_.begin(), obj.cur_ctrl_.end());

      if (node["maximum"]) obj.max_ctrl_ = node["maximum"].as<vector<uint8_t>>();

      if (node["minimum"]) obj.min_ctrl_ = node["minimum"].as<vector<uint8_t>>();
    } catch (...) {
      return false;
    }

    return true;
  }
};
}  // namespace YAML
