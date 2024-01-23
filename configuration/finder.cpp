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
 * @param neg_answer_limit skip a patern after neg_answer_limit negative answer
 */
Finder::Finder(Camera &camera, unsigned emitters, unsigned neg_answer_limit)
    : camera_(camera), emitters_(emitters), neg_answer_limit_(neg_answer_limit) {}

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
        if (instruction.is_corrupted()) {
            Logger::debug("Corrupted instruction skipped:", to_string(instruction));
            continue;
        }

        try
        {
            instruction.set_min_cur();

            unsigned neg_answer_counter = 0;
            while (neg_answer_counter < neg_answer_limit_ && instruction.next())
            {
                if (neg_answer_counter == neg_answer_limit_ - 1)
                    instruction.set_max_cur();

                Logger::debug("Instruction applied:", to_string(instruction));

                if (camera_.apply(instruction) && camera_.is_emitter_working() && ++configured == emitters_)
                    return true; // all emitters are configured

                ++neg_answer_counter;
            }

            instruction.reset();
            camera_.apply(instruction);
        }
        catch (const CameraInstructionException &e)
        {
            continue;
        }
        catch (const CameraException &e)
        {
            Logger::error("Impossible to reset the camera.");
            Logger::info("Please shutdown your computer, then boot and retry.");
            instruction.reset();
            instruction.set_corrupted(true);
            throw e; // propagate to exit
        }
    }

    return false;
}
