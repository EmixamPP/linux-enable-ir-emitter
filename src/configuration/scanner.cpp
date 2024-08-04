#include "scanner.hpp"

#include <iomanip>
#include <iostream>
using namespace std;

#include <spdlog/spdlog.h>

#include "camera/camerainstruction.hpp"

Scanner::Scanner(shared_ptr<Camera> camera) : camera_(std::move(camera)) {}

CameraInstructions Scanner::scan() noexcept {
  CameraInstructions instructions;

  for (unsigned _unit = 0; _unit < UINT8_MAX + 1; ++_unit) {
    const uint8_t unit = static_cast<uint8_t>(_unit);  // safe: 0 <= _units <= UINT8_MAX
    for (unsigned _selector = 0; _selector < UINT8_MAX + 1; ++_selector) {
      const uint8_t selector =
          static_cast<uint8_t>(_selector);  // safe: 0 <= _selector <= UINT8_MAX

      try {
        instructions.push_back(CameraInstruction(*camera_, unit, selector));
      } catch (const CameraInstructionException &) {
      }
    }
  }
  cout << endl;

  return instructions;
}
