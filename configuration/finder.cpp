#include "finder.hpp"

#include <fstream>
#include <string>
#include <thread>
using namespace std;

#include "camera/camerainstruction.hpp"
#include "utils/logger.hpp"

/**
 * @brief Construct a new Finder:: Finder object
 *
 * @param camera on which try to find an instruction for the emitter
 * @param emitters number of emitters on the device
 * @param negAnswerLimit skip a patern after negAnswerLimit negative answer
 */
Finder::Finder(Camera &camera, unsigned emitters, unsigned negAnswerLimit)
    : camera(camera), emitters(emitters), negAnswerLimit(negAnswerLimit) {}

/**
 * @brief Find an instruction which enable the ir emitter(s)
 *
 * @param instructions to test, corrupted ones are ignored or will be marked as such
 * 
 * @throw CameraException
 *
 * @return a vector containing the intruction(s),
 * empty if the configuration failed
 */
bool Finder::find(vector<CameraInstruction> &intructions)
{
    unsigned configured = 0;

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

                Logger::debug("Instruction applied:", to_string(instruction));

                if (camera.apply(instruction) && camera.isEmitterWorking() && ++configured == emitters)
                    return true; // all emitters are configured

                ++negAnswerCounter;
            }

            instruction.reset();
            camera.apply(instruction);
        }
        catch (const CameraInstructionException &e)
        {
            continue;
        }
        catch (const CameraException &e)
        {
            Logger::error("Impossible to reset the camera.");
            Logger::info("Please shut down your computer, then boot and retry.");
            instruction.reset();
            instruction.setCorrupted(true);
            throw e; // propagate to exit
        }
    }

    return false;
}
