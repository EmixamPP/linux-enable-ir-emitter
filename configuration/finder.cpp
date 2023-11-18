#include "finder.hpp"

#include <fstream>
#include <string>
#include <thread>
#include <vector>
using namespace std;

#include "camera/camera.hpp"
#include "camera/camerainstruction.hpp"
#include "utils/logger.hpp"

/**
 * @brief Construct a new Finder:: Finder object
 *
 * @param camera on which try to find an instruction for the emitter
 * @param emitters number of emitters on the device
 * @param negAnswerLimit skip a patern after negAnswerLimit negative answer
 * @param intructions instructions to test, corrupted ones are ignored or will be marked as such
 */
Finder::Finder(Camera &camera, unsigned emitters, unsigned negAnswerLimit, vector<CameraInstruction> &intructions)
    : camera(camera), emitters(emitters), negAnswerLimit(negAnswerLimit), intructions(intructions) {}

/**
 * @brief Find an instruction which enable the ir emitter(s)
 *
 * @return a vector containing the intruction(s),
 * empty if the configuration failed
 */
vector<CameraInstruction> Finder::find()
{
    vector<CameraInstruction> configuration;

    for (auto &instruction : intructions)
    {
        if (instruction.isCorrupted())
            continue;

        try
        {
            instruction.setMinAsCur();

            unsigned negAnswerCounter = 0;
            while (negAnswerCounter < negAnswerLimit && instruction.next())
            {
                if (negAnswerCounter == negAnswerLimit - 1)
                    instruction.setMaxAsCur();
                
                Logger::debug("Instruction applied:", string(instruction));

                if (camera.apply(instruction) && camera.isEmitterWorking())
                {
                    configuration.push_back(instruction);
                    if (configuration.size() == emitters) // all emitters are configured
                        return configuration;
                }
                ++negAnswerCounter;
            }

            instruction.reset();
            camera.apply(instruction);
        }
        catch (CameraInstructionException &e)
        {
            continue;
        }
        catch (CameraException &e)
        {
            Logger::error("Impossible to reset the camera.");
            Logger::info("Please shut down your computer, then boot and retry.");
            instruction.reset();
            instruction.setCorrupted(true);
            throw e; // propagate to exit
        }
    }

    return configuration;
}
