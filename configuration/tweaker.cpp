#include "tweaker.hpp"

#include <iostream>
#include <sstream>
using namespace std;

#include "camera/camerainstruction.hpp"
#include "utils/logger.hpp"

/**
 * @brief Construct a new Tweaker:: Tweaker object
 *
 * @param camera on which to tweak instructions
 */
Tweaker::Tweaker(Camera &camera) : camera(camera)
{
}

static size_t ask_for_choice(const CameraInstructions &instructions)
{
    size_t i = 0;
    for (; i < instructions.size(); ++i)
    {
        const auto &inst = instructions.at(i);

        if (inst.is_corrupted())
            continue;

        cout << i << ") " << to_string(inst) << endl;
    }
    cout << i << ") exit" << endl;

    size_t choice = instructions.size() + 1;
    while (choice > instructions.size())
    {
        cout << "Choose an instruction to tweak: ";
        cin >> choice;
    }
    return choice;
}

static vector<uint8_t> ask_for_new_cur(const CameraInstruction &inst)
{
    cout << "minimum: " << to_string(inst.min()) << endl;
    cout << "maximum: " << to_string(inst.max()) << endl;
    cout << "initial: " << to_string(inst.init()) << endl;
    cout << "current: " << to_string(inst.cur()) << endl;
    cout << "new current: ";

    string new_cur_str;
    getline(cin >> ws, new_cur_str);
    
    istringstream iss(new_cur_str);
    auto new_cur_parsed = vector<int32_t>(istream_iterator<int32_t>{iss}, istream_iterator<int32_t>());

    vector<uint8_t> new_cur;
    for (auto v : new_cur_parsed)
        new_cur.push_back(static_cast<uint8_t>(v));

    return new_cur;
}

/**
 * @brief Allow the user to tweak the instruction of its camera
 *
 * @param intructions to tweak, corrupted ones are ignored or will be marked as such
 *
 * @throw CameraException
 */
void Tweaker::tweak(CameraInstructions &instructions)
{
    while (true)
    {
        auto stop_feedback = camera.play();

        size_t choice = ask_for_choice(instructions);
        if (choice == instructions.size())
        {
            stop_feedback();
            break;
        }
        auto &inst = instructions.at(choice);

        auto prev_cur = inst.cur();
        auto new_cur = ask_for_new_cur(inst);

        stop_feedback();

        if (!inst.set_cur(new_cur))
        {
            Logger::warning("Invalid value for the instruction.");
            continue;
        }

        try
        {
            camera.apply(inst);
        }
        catch (const CameraException &e)
        {
            Logger::info("Please shutdown your computer, then boot and retry.");
            inst.set_cur(prev_cur);
            inst.set_corrupted(true);
            throw e; // propagate to exit
        }
    }
}