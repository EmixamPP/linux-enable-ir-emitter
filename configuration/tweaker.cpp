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

static size_t askForChoice(const vector<CameraInstruction> &instructions)
{
    size_t i = 0;
    for (; i < instructions.size(); ++i)
    {
        const auto &inst = instructions.at(i);

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
    cout << "minimum: " << to_string(inst.getMin()) << endl;
    cout << "maximum: " << to_string(inst.getMax()) << endl;
    cout << "initial: " << to_string(inst.getInit()) << endl;
    cout << "current: " << to_string(inst.getCur()) << endl;
    cout << "new current: ";

    string newCurStr;
    getline(cin >> ws, newCurStr);
    
    istringstream iss(newCurStr);
    auto newCurParsed = vector<int32_t>(istream_iterator<int32_t>{iss}, istream_iterator<int32_t>());

    vector<uint8_t> newCur;
    for (auto v : newCurParsed)
        newCur.push_back(static_cast<uint8_t>(v));

    return newCur;
}

/**
 * @brief Allow the user to tweak the instruction of its camera
 *
 * @param intructions to tweak, corrupted ones are ignored or will be marked as such
 *
 * @throw CameraException
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
            Logger::warning("Invalid value for the instruction.");
            continue;
        }

        try
        {
            camera.apply(inst);
        }
        catch (const CameraException &e)
        {
            Logger::info("Please shut down your computer, then boot and retry.");
            inst.setCur(prevCur);
            inst.setCorrupted(true);
            throw e; // propagate to exit
        }
    }
}