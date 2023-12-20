#include "tweaker.hpp"

#include <iostream>
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

static size_t askForChoice(const vector<CameraInstruction> &instructions)
{
    size_t i = 0;
    for (; i < instructions.size(); ++i)
    {
        auto &inst = instructions.at(i);

        if (inst.isCorrupted())
            continue;

        cout << i << ") " << to_string(inst) << endl;
    }
    cout << i << ") exit" << endl;

    size_t choice;
    cout << "Choose an instruction to tweak: ";
    cin >> choice;
    while (choice > instructions.size())
    {
        cout << "Choose an instruction to tweak: ";
        cin >> choice;
    }
    return choice;
}

static vector<uint8_t> askForNewCur(const CameraInstruction &inst)
{
    auto &prevCur = inst.getCur();
    auto &min = inst.getMin();
    auto &max = inst.getMax();

    string newCurStr;
    cout << "minimum: " << to_string(min) << ", maximum: " << to_string(max);
    cout << " initial: " << to_string(inst.getInit()) << ", current: " << to_string(prevCur) << std::endl;
    cout << "new current: ";
    getline(cin >> ws, newCurStr);

    vector<uint8_t> newCur;
    for (size_t i = 0; i < newCurStr.size(); i += 2)
        newCur.push_back(static_cast<uint8_t>(newCurStr[i] - '0'));

    return newCur;
}

/**
 * @brief Allow the user to tweak the instruction of its camera
 *
 * @param intructions to tweak, corrupted ones are ignored or will be marked as such
 */
void Tweaker::tweak(vector<CameraInstruction> &instructions)
{
    while (true)
    {
        auto stopFeedback = camera.play();

        size_t choice = askForChoice(instructions);
        if (choice == instructions.size())
        {
            stopFeedback();
            break;
        }
        auto &inst = instructions.at(choice);

        auto prevCur = inst.getCur();
        auto newCur = askForNewCur(inst);

        stopFeedback();

        if (!inst.setCur(newCur))
        {
            Logger::error("Invalid value for the instruction.");
            continue;
        }

        try
        {
            camera.apply(inst);
        }
        catch (CameraException &e)
        {
            inst.setCur(prevCur);
            inst.setCorrupted(true);
            throw e; // propagate to exit
        }
    }
}