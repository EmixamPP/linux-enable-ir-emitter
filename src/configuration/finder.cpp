#include <atomic>
#include <csignal>
#include <fstream>
#include <iomanip>
#include <string>
using namespace std;

#include "camera/camerainstruction.hpp"
#include "tools.hpp"
#include "utils/logger.hpp"

// Progression line size
constexpr unsigned PROGRESSION_LINE_SIZE = 40;

// Number of points on the progression line
constexpr unsigned PROGRESSION_NBR_POINTS = 5;

// Flag to force exit the process
std::atomic<bool> force_exit = false;

/**
 * @brief Catch ctrl-c and set `force_exit` to true
 */
static void catch_ctrl_c() {
  signal(SIGINT, [](int signal) {
    if (signal == SIGINT) {
      logger::info("The process will exit as soon as possible.");
      force_exit = true;
    }
  });
}

/**
 * @brief Search a control that enables the emitter for a given instruction
 * @param camera the camera on which apply the instruction
 * @param instruction the instruction on which the search
 * @param neg_answer_limit the number of negative answers before stopping the search
 * @return true if a control value enabling the emitter is not found, otherwise false
 */
static bool search_ir_control(CameraPtr &camera, CameraInstruction &instruction,
                              unsigned neg_answer_limit) {
  // try to set the minimum control value
  if (!instruction.set_min_cur()) {
    // if not reset to the initial known value for the instruction
    instruction.reset();
  }

  unsigned neg_answer_counter = 0;
  while (!force_exit && neg_answer_counter < neg_answer_limit && instruction.next()) {
    cout << '\r' << setw(PROGRESSION_LINE_SIZE) << ' ' << '\r';  // wipe previous
    cout << "Searching, please be patient"
         << string(neg_answer_counter % PROGRESSION_NBR_POINTS, '.') << '\r' << flush;

    if (neg_answer_counter == neg_answer_limit - 1) instruction.set_max_cur();

    logger::debug("Instruction applied: {}.", to_string(instruction));

    if (camera->apply(instruction) && camera->is_emitter_working()) {
      logger::debug("The instruction makes emitter flash.");
      return true;
    }

    logger::debug("The instruction does not enable the emitter.");
    ++neg_answer_counter;
  }

  instruction.reset();
  logger::debug("Reseting to the instruction: {}.", to_string(instruction));
  if (!camera->apply(instruction)) {
    throw Camera::Exception("Impossible to reset the instruction: {}.", to_string(instruction));
  }
  return false;
}

bool Tools::Find(Configuration &config, unsigned emitters, unsigned neg_answer_limit) {
  catch_ctrl_c();

  unsigned configured = 0;
  for (auto &instruction : config) {
    if (instruction.status() == CameraInstruction::Status::DISABLE) {
      logger::debug("Disable instruction skipped: {}.", to_string(instruction));
      continue;
    }

    try {
      if (search_ir_control(config.camera, instruction, neg_answer_limit)) {
        instruction.set_status(CameraInstruction::Status::START);
        ++configured;
      }

      if (configured == emitters) {
        logger::debug("All emitters are configured.");
        return true;
      }

      if (force_exit) break;
    } catch (const Camera::Exception &e) {
      logger::error(e.what());
      logger::info("Please shutdown your computer, then boot and retry.");
      instruction.reset();
      instruction.set_status(CameraInstruction::Status::DISABLE);
      break;
    }
  }

  return false;
}
