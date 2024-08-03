#pragma once

#include <vector>
using namespace std;

#include "camera/camera.hpp"
#include "camera/camerainstruction.hpp"

/**
 * @brief A `Scanner` object is used to scan the available `CameraInstruction`
 * of a given camera.
 */
class Scanner {
  // Camera on which scans for instructions
  shared_ptr<Camera> camera_;

 public:
  Scanner() = delete;

  /**
   * @brief Construct a new `Scanner` object
   *
   * @param camera on which scans the instructions
   */
  explicit Scanner(shared_ptr<Camera> camera);

  ~Scanner() = default;

  Scanner &operator=(const Scanner &) = delete;

  Scanner(const Scanner &) = delete;

  Scanner &operator=(Scanner &&other) = delete;

  Scanner(Scanner &&other) = delete;

  /**
   * @brief Scans the available camera instructions
   *
   * @return the list of instructions
   */
  CameraInstructions scan() noexcept;
};
