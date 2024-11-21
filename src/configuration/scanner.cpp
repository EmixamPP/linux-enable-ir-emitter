#include <iomanip>
#include <iostream>
using namespace std;

#include "camera/camerainstruction.hpp"
#include "tools.hpp"
#include "utils/logger.hpp"

CameraInstructions Tools::Scan(const CameraPtr &camera) noexcept {
  CameraInstructions instructions;

  for (unsigned _unit = 0; _unit < UINT8_MAX + 1; ++_unit) {
    const uint8_t unit = static_cast<uint8_t>(_unit);  // safe: 0 <= _units <= UINT8_MAX
    for (unsigned _selector = 0; _selector < UINT8_MAX + 1; ++_selector) {
      const uint8_t selector =
          static_cast<uint8_t>(_selector);  // safe: 0 <= _selector <= UINT8_MAX

      try {
        instructions.push_back(CameraInstruction(*camera, unit, selector));
      } catch (const CameraInstruction::Exception &) {
      }
    }
  }

  return instructions;
}
