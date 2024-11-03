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
struct CameraInstruction {
  using Unit = uint8_t;
  using Selector = uint8_t;
  using Control = vector<uint8_t>;

 private:
  // Unit used on the camera to identify the control
  Unit unit_;

  // Selector used on the camera to identify the control
  Selector selector_;

  // Flag to mark the control as disable
  bool disable_;

  // Current control value
  Control cur_;

  // Initial control value
  Control init_;

  // Maximum control value
  Control max_;

  // Minimum control value
  Control min_;

  // the control block to modify (8 bits value)
  unsigned ctrl_block_;

 public:
  /**
   * @brief Construct a new empty CameraInstruction object.
   * Values for unit, selector and controls needs to be set.
   */
  CameraInstruction() = default;

  /**
   * @brief Construct a new CameraInstruction object.
   * @param camera on which find the control instruction
   * @param unit of the instruction
   * @param selector of the instruction
   * @param disable if the instruction considered disabled
   * @throw CameraInstructionException if an error occurs
   */
  explicit CameraInstruction(Camera &camera, Unit unit, Selector selector, bool disable = false);

  /**
   * @brief Construct a new CameraInstruction object.
   * @param unit of the instruction
   * @param selector of the instruction
   * @param cur current control
   * @param init initial control
   * @param max maximum control
   * @param min minimum control
   * @param disable if the instruction considered disabled
   */
  explicit CameraInstruction(Unit unit, Selector selector, Control cur, Control init, Control max,
                             Control min, bool disable);

  /**
   * @brief Get the unit of the instruction.
   * @return unit
   */
  uint8_t unit() const noexcept;

  /**
   * @brief Get the selector of the instruction.
   * @return selector
   */
  uint8_t selector() const noexcept;

  /**
   * @brief Get the current control.
   * @return minimum control
   */
  const Control &cur() const noexcept;

  /**
   * @brief Sets a new current control, if they have the same size.
   * @param cur control to set
   * @throw CameraInstructionException if `disable()` is true
   * @return true if success, otherwise false
   */
  bool set_cur(Control cur);

  /**
   * @brief Get the maximum of the instruction.
   * @return minimum control
   */
  const Control &max() const noexcept;

  /**
   * @brief If a maximum control exists,
   * set it as the current control with that value.
   * If no maximum control exists, set the maximum possible (i.e. UINT8_MAX).
   * @throw CameraInstructionException if `disable()` is true
   * @return true if success, otherwise false
   */
  bool set_max_cur();

  /**
   * @brief Get the minimum of the instruction.
   * @return maximum control
   */
  const Control &min() const noexcept;

  /**
   * @brief If a minimum control exists,
   * sets it as the current control with that value.
   * @throw CameraInstructionException if `disable()` is true
   * @return true if success, otherwise false
   */
  bool set_min_cur();

  /**
   * @brief Get the initial control.
   * @return initial control
   */
  const Control &init() const noexcept;

  /**
   * @brief Get the disable status.
   * @return true if the instruction is disable
   */
  bool is_disable() const noexcept;

  /**
   * @brief Changes the disable status of the instruction.
   * @param is_disable status to set
   */
  void set_disable(bool is_disable) noexcept;

  /**
   * @brief Reset the current control to the initial control value.
   * @throw CameraInstructionException if `disable()` is true
   */
  void reset();

  /**
   * @brief Compute the next possible control,
   * it is incremented individually for each control block,
   * no combination will be done, for that use `set_cur()`
   * @return true if a next value has been set,
   * false if all control block have been incremented up to their `maximum()` or UINT8_MAX
   * @throw CameraInstructionException if `disable()` is true
   */
  bool next();
};

string to_string(const CameraInstruction &inst);

string to_string(const CameraInstruction::Control &vec);

string to_string(uint8_t v);

using CameraInstructions = vector<CameraInstruction>;

/**
 * @brief Exception thrown by the `CameraInstruction`.
 *
 */
class CameraInstructionException : public exception {
 private:
  string message;

 public:
  explicit CameraInstructionException(const CameraInstruction &inst, const string &error);

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
    node["disable"] = obj.is_disable();
    node["unit"] = static_cast<int>(obj.unit());
    node["selector"] = static_cast<int>(obj.selector());
    for (const auto &v : obj.cur()) node["current"].push_back(static_cast<int>(v));
    if (obj.cur() != obj.init())
      for (const auto &v : obj.init()) node["initial"].push_back(static_cast<int>(v));
    for (const auto &v : obj.max()) node["maximum"].push_back(static_cast<int>(v));
    for (const auto &v : obj.min()) node["minimum"].push_back(static_cast<int>(v));
    return node;
  }

  static bool decode(const Node &node, CameraInstruction &obj) {
    try {
      auto disable = node["disable"].as<bool>();
      auto unit = node["unit"].as<CameraInstruction::Unit>();
      auto selector = node["selector"].as<CameraInstruction::Selector>();
      auto cur = node["current"].as<CameraInstruction::Control>();
      auto init = cur;
      auto max = CameraInstruction::Control();
      auto min = CameraInstruction::Control();
      if (node["initial"]) init = node["initial"].as<CameraInstruction::Control>();
      if (node["maximum"]) max = node["maximum"].as<CameraInstruction::Control>();
      if (node["minimum"]) min = node["minimum"].as<CameraInstruction::Control>();
      obj = CameraInstruction(unit, selector, std::move(cur), std::move(init), std::move(max),
                              std::move(min), disable);
    } catch (...) {
      return false;
    }

    return true;
  }
};
}  // namespace YAML
