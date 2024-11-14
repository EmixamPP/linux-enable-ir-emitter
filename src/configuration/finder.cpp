#include <atomic>
#include <csignal>
#include <fstream>
#include <iomanip>
#include <string>
using namespace std;

#include "camera/camerainstruction.hpp"
#include "logger.hpp"
#include "tools.hpp"

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

bool Tools::Find(Configuration &config, unsigned emitters, unsigned neg_answer_limit) {
  catch_ctrl_c();

  unsigned configured = 0;

  unsigned p = 0;  // progression
  for (auto &instruction : config) {
    if (instruction.is_disable()) {
      logger::debug("Disable instruction skipped: {}.", to_string(instruction));
      continue;
    }

    try {
      instruction.set_min_cur();

      unsigned neg_answer_counter = 0;
      while (!force_exit && neg_answer_counter < neg_answer_limit && instruction.next()) {
        cout << '\r' << setw(PROGRESSION_LINE_SIZE) << ' ' << '\r';  // wipe previous
        cout << "Searching, please be patient" << string(p++ % PROGRESSION_NBR_POINTS, '.') << '\r' << flush;

        if (neg_answer_counter == neg_answer_limit - 1) instruction.set_max_cur();

        logger::debug("Instruction applied: {}.", to_string(instruction));

        if (config.camera->apply(instruction) && config.camera->is_emitter_working()) {
          logger::debug("The instruction makes emitter flash.");
          if (++configured == emitters) {
            logger::debug("All emitters are configured.");
            return true;
          }
        } else
          logger::debug("The instruction does not enable the emitter.");

        ++neg_answer_counter;
      }

      instruction.reset();
      logger::debug("Reseting to the instruction: {}.", to_string(instruction));
      if (!config.camera->apply(instruction)) {
        logger::error("Impossible to reset the instruction: {}.", to_string(instruction));
        logger::info("Please shutdown your computer, then boot and retry.");
        instruction.set_disable(true);
        return false;
      }

      if (force_exit) return false;
    } catch (const Camera::Exception &e) {
      logger::error(e.what());
      logger::info("Please shutdown your computer, then boot and retry.");
      instruction.reset();
      instruction.set_disable(true);
      return false;
    }
  }

  return false;
}
