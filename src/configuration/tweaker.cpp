#include <iostream>
#include <optional>
#include <sstream>
using namespace std;

#include "camera/camerainstruction.hpp"
#include "logger.hpp"
#include "tweaker.hpp"

Tweaker::Tweaker(shared_ptr<Camera> camera) : camera(std::move(camera)) {}

/**
 * @brief Ask the user to choose an instruction to tweak
 *
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
 *
 * @param inst te instruction to tweak
 * @return the input value for the instruction, it may be invalid
 * or nothing if the instruction has been disabled/enabled.
 */
static optional<vector<uint8_t>> ask_for_new_cur(CameraInstruction &inst) {
  cout << "minimum: " << to_string(inst.min()) << endl;
  cout << "maximum: " << to_string(inst.max()) << endl;
  cout << "initial: " << to_string(inst.init()) << endl;
  cout << "current: " << to_string(inst.cur()) << endl;
  if (inst.is_disable())
    cout << "status: "
         << "disable" << endl;
  else
    cout << "status: "
         << "enable" << endl;
  cout << "new current or status: ";

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

  istringstream iss(new_cur_str);
  auto new_cur_parsed =
      vector<int32_t>(istream_iterator<int32_t>{iss}, istream_iterator<int32_t>());

  vector<uint8_t> new_cur(new_cur_parsed.size());
  for (auto v : new_cur_parsed) new_cur.push_back(static_cast<uint8_t>(v));

  return new_cur;
}

void Tweaker::tweak(CameraInstructions &instructions) {
  while (true) {
    auto video_feedback = camera->play();

    size_t choice = ask_for_choice(instructions);
    if (choice == instructions.size()) {
      video_feedback.request_stop();
      break;
    }
    auto &inst = instructions.at(choice);

    auto prev_cur = inst.cur();
    auto new_cur = ask_for_new_cur(inst);

    video_feedback.request_stop();

    if (!new_cur.has_value()) continue;

    if (!inst.set_cur(new_cur.value())) {
      logger::warn("Invalid value for the instruction.");
      continue;
    }

    try {
      camera->apply(inst);
    } catch (const CameraException &e) {
      logger::info("Please shutdown your computer, then boot and retry.");
      inst.set_cur(prev_cur);
      inst.set_disable(true);
      throw e;  // propagate to exit
    }
  }
}