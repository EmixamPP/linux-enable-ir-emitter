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
Finder::Finder(shared_ptr<Camera> camera, unsigned emitters, unsigned neg_answer_limit)
    : camera_(camera), emitters_(emitters), neg_answer_limit_(neg_answer_limit) {}

/**
 * @brief Find an instruction which enable the ir emitter(s)
 * by changing its value.
 *
 * @param instructions to test and modify, corrupted ones are ignored or will be marked as such
 *
 * @throw CameraException
 *
 * @return true if success otherwise false
 */
bool Finder::find(CameraInstructions &intructions)
{
    unsigned configured = 0;

    for (auto &instruction : intructions)
    {
        if (instruction.is_corrupted())
        {
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

                if (camera_->apply(instruction))
                {
                    if (camera_->is_emitter_working())
                    {
                        Logger::debug("The instruction makes emitter flash.");
                        if (++configured == emitters_)
                        {
                            Logger::debug("All emitters are configured.");
                            return true;
                        }
                    }
                }
                else
                    Logger::debug("The instruction is not valid.");

                ++neg_answer_counter;
            }

            instruction.reset();
            Logger::debug("Reseting to the instruction:", to_string(instruction));
            camera_->apply(instruction);
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
