#pragma once

#include <vector>
using namespace std;

#include "camera/camera.hpp"
#include "camera/camerainstruction.hpp"

class Tweaker {
 private:
  shared_ptr<Camera> camera;

 public:
  Tweaker() = delete;

  explicit Tweaker(shared_ptr<Camera> camera);

  ~Tweaker() = default;

  Tweaker &operator=(const Tweaker &) = delete;

  Tweaker(const Tweaker &) = delete;

  Tweaker &operator=(Tweaker &&other) = delete;

  Tweaker(Tweaker &&other) = delete;

  void tweak(CameraInstructions &instructions);
};