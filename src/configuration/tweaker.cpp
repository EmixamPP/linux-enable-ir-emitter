#include <iostream>
#include <optional>
#include <sstream>
using namespace std;

#include "camera/camerainstruction.hpp"
#include "logger.hpp"
#include "tools.hpp"

constexpr unsigned WAIT_FOR_ERROR_MS = 500;

/**
 * @brief Ask the user to choose an instruction to tweak
 * @param instructions instructions list
 * @return index of the instruction choosen in the list
 */
static size_t ask_for_choice(const CameraInstructions &instructions) {
  size_t i = 0;
  for (; i < instructions.size(); ++i) {
    const auto &inst = instructions.at(i);

    cout << i << ") " << to_string(inst);
    if (inst.is_disable()) cout << " [DISABLE]";
    cout << endl;
  }
  cout << i << ") exit" << endl;

  size_t choice = instructions.size() + 1;
  while (choice > instructions.size()) {
    cout << "Choose an instruction to tweak: ";
    cin >> choice;
  }
  return choice;
}

/**
 * @brief Ask the user to input a new value for the current instruction
 * or to enable/disable the instruction.
 * @param inst te instruction to tweak
 * @return the input value for the instruction, it may be invalid
 * or nothing if the instruction has been disabled/enabled.
 */
static optional<CameraInstruction::Control> ask_for_new_cur(CameraInstruction &inst) {
  cout << "minimum: " << to_string(inst.min()) << endl;
  cout << "maximum: " << to_string(inst.max()) << endl;
  cout << "initial: " << to_string(inst.init()) << endl;
  cout << "current: " << to_string(inst.cur()) << endl;
  if (inst.is_disable())
    cout << "status: " << "disable" << endl;
  else
    cout << "status: " << "enable" << endl;
  cout << "Input a new current control or status (disable/enable) or q to return: ";

  string new_cur_str;
  getline(cin >> ws, new_cur_str);

  if (new_cur_str == "enable") {
    inst.set_disable(false);
    return {};
  }

  if (new_cur_str == "disable") {
    inst.set_disable(true);
    return {};
  }

  CameraInstruction::Control new_cur;
  istringstream iss(new_cur_str);
  try {
    std::transform(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(),
                   std::back_inserter(new_cur),
                   [](const std::string &val) { return static_cast<uint8_t>(std::stoi(val)); });
  } catch (...) {
    return {};
  }

  return new_cur;
}

void Tools::Tweak(Configuration &config) {
  Camera::ExceptionPtr eptr;
  auto video_feedback_stop = config.camera->play(eptr);

  while (true) {
    size_t choice = ask_for_choice(config.instructions());
    if (choice == config.instructions().size()) {
      break;
    }
    auto &inst = config.instructions().at(choice);

    auto prev_cur = inst.cur();
    auto new_cur = ask_for_new_cur(inst);

    if (!new_cur.has_value()) continue;

    try {
      if (!inst.set_cur(new_cur.value())) {
        logger::warn("Invalid value for the control.");
        continue;
      }
      config.camera->apply(inst);

      // wait in case of error to grab frames
      this_thread::sleep_for(chrono::milliseconds(500));
      if (eptr) {
        logger::error(eptr->what());
        video_feedback_stop();
        logger::info("Please shutdown your computer, then boot and retry.");
        inst.set_cur(prev_cur);
        inst.set_disable(true);
        return;
      }

    } catch (const CameraInstruction::Exception &e) {
      logger::warn(e.what());
    }
  }

  video_feedback_stop();
}