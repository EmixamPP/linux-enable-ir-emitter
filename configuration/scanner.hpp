#pragma once

#include <vector>
using namespace std;

#include "camera/camera.hpp"
#include "camera/camerainstruction.hpp"

class Scanner {
 private:
  shared_ptr<Camera> camera_;

 public:
  Scanner() = delete;

  explicit Scanner(shared_ptr<Camera> camera);

  ~Scanner() = default;

  Scanner &operator=(const Scanner &) = delete;

  Scanner(const Scanner &) = delete;

  Scanner &operator=(Scanner &&other) = delete;

  Scanner(Scanner &&other) = delete;

  CameraInstructions scan() noexcept;
};
