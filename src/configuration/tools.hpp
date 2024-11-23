#pragma once

#include "configuration.hpp"

namespace Tools {
/**
 * @brief Find an instruction which enable the ir emitter(s) by changing
 * the current controls of the camera by a semi-exhaustive search.
 * @param config the configuration of the camera that contains the instructions test and modify,
 * disable ones are ignored or will be marked as such
 * @param emitters number of emitters on the device
 * @param neg_answer_limit skip an instruction pattern after a number of negative answer
 * @throw Camera::Exception
 * @return true if success otherwise false
 */
bool Find(Configuration &config, unsigned emitters, unsigned neg_answer_limit);

/**
 * @brief Allow the user to tweak the instruction of its camera.
 * Very useful for exploring the possibility of a camera, and to manually find a way to
 * enable the ir emitter of a camera. This object will during the tweaking process, display
 * continuously a video feedback, in o>rder to help the user to find the effect of the instruction
 * tweaked.
 * @param config the configuration of the camera that contains the instructions modify
 * @param instructions the instructions to tweak
 * @throw Camera::Exception
 */
void Tweak(Configuration &config);

/**
 * @brief Scans the available camera instructions
 * @param camera camera on which scans the instructions
 * @return the list of instructions
 */
CameraInstructions Scan(const CameraPtr &camera) noexcept;
}  // namespace Tools
