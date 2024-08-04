#pragma once

#include <vector>
using namespace std;

#include "camera/camera.hpp"
#include "camera/camerainstruction.hpp"

/**
 * @brief A `Tweaker` object is used to tweak the possible `CameraInstructions` of a camera.
 * Very useful for exploring the possibility of a camera, and to manually find a way to
 * enable the ir emitter of a camera. This object will during the tweaking process, display
 * continuously a video feedback, in order to help the user to find the effect of the instruction
 * tweaked.
 */
class Tweaker {
  // Camera on which to tweak instructions
  shared_ptr<Camera> camera;

 public:
  Tweaker() = delete;

  /**
   * @brief Construct a new `Tweaker` object.
   *
   * @param camera on which to tweak instructions
   */
  explicit Tweaker(shared_ptr<Camera> camera);

  ~Tweaker() = default;

  Tweaker &operator=(const Tweaker &) = delete;

  Tweaker(const Tweaker &) = delete;

  Tweaker &operator=(Tweaker &&other) = delete;

  Tweaker(Tweaker &&other) = delete;

  /**
   * @brief Allow the user to tweak the instruction of its camera.
   *
   * @param instructions the instructions to tweak
   *
   * @throw CameraException
   */
  void tweak(CameraInstructions &instructions);
};