#pragma once

#include <vector>
using namespace std;

#include "camera/camera.hpp"
#include "camera/camerainstruction.hpp"

/**
 * @brief A `Finder` object is used to find an instruction which enable the ir emitter(s)
 * by changing the current instruction control value of a camera by an exhaustive search.
 */
class Finder {
  // Camera on which on which enable the ir emitter(s)
  shared_ptr<Camera> camera_;

  // Number of emitters on the camera
  const unsigned emitters_;

  // Skip an instruction pattern after a number of negative answer
  const unsigned neg_answer_limit_;

 public:
  Finder() = delete;

  /**
   * @brief Construct a new `Finder` object.
   *
   * @param camera on which try to find an instruction for the emitter
   * @param emitters number of emitters on the device
   * @param neg_answer_limit skip an instruction pattern after a number of negative
   * answer
   */
  explicit Finder(shared_ptr<Camera> camera, unsigned emitters, unsigned neg_answer_limit_);

  ~Finder() = default;

  Finder &operator=(const Finder &) = delete;

  Finder(const Finder &) = delete;

  Finder &operator=(Finder &&other) = delete;

  Finder(Finder &&other) = delete;

  /**
   * @brief Find an instruction which enable the ir emitter(s) by changing its value.
   *
   * @param instructions to test and modify, disable ones are ignored or will be
   * marked as such
   *
   * @throw CameraException
   *
   * @return true if success otherwise false
   */
  bool find(CameraInstructions &instructions);
};
