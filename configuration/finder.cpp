#include "finder.hpp"

#include <iomanip>
#include <fstream>
#include <signal.h>
#include <string>
#include <thread>
using namespace std;

#include <spdlog/spdlog.h>

#include "camera/camerainstruction.hpp"

/**
 * @brief Construct a new Finder:: Finder object
 *
 * @param camera on which try to find an instruction for the emitter
 * @param emitters number of emitters on the device
 * @param neg_answer_limit skip a pattern after neg_answer_limit negative answer
 */
Finder::Finder(shared_ptr<Camera> camera, unsigned emitters, unsigned neg_answer_limit)
    : camera_(camera), emitters_(emitters), neg_answer_limit_(neg_answer_limit) {}

bool force_exit = false;
/**
 * @brief Find an instruction which enable the ir emitter(s)
 * by changing its value.
 *
 * @param instructions to test and modify, disable ones are ignored or will be marked as such
 *
 * @throw CameraException
 *
 * @return true if success otherwise false
 */
bool Finder::find(CameraInstructions &instructions)
{
    signal(SIGINT, [](int signal)
           { if (signal == SIGINT) { 
            spdlog::info("The process will exit as soon as possible.");
            force_exit = true;} });

    unsigned configured = 0;

    unsigned p = 0; // progression
    for (auto &instruction : instructions)
    {
        if (instruction.is_disable())
        {
            spdlog::debug("Disable instruction skipped: {}.", to_string(instruction));
            continue;
        }

        try
        {
            instruction.set_min_cur();

            unsigned neg_answer_counter = 0;
            while (neg_answer_counter < neg_answer_limit_ && instruction.next())
            {
                cout << '\r' << setw(20) << ' ' << '\r'; // wipe previous
                cout << "Searching" << string(++p % 5, '.') << '\r' << flush;

                if (force_exit)
                    break;

                if (neg_answer_counter == neg_answer_limit_ - 1)
                    instruction.set_max_cur();

                spdlog::debug("Instruction applied: {}.", to_string(instruction));

                if (camera_->apply(instruction))
                {
                    if (camera_->is_emitter_working())
                    {
                        spdlog::debug("The instruction makes emitter flash.");
                        if (++configured == emitters_)
                        {
                            spdlog::debug("All emitters are configured.");
                            return true;
                        }
                    }
                }
                else
                    spdlog::debug("The instruction is not valid.");

                ++neg_answer_counter;
            }

            instruction.reset();
            spdlog::debug("Reseting to the instruction: {}.", to_string(instruction));
            camera_->apply(instruction);

            if (force_exit)
                return false;
        }
        catch (const CameraInstructionException &e)
        {
            continue;
        }
        catch (const CameraException &e)
        {
            spdlog::error("Impossible to reset the camera.");
            spdlog::info("Please shutdown your computer, then boot and retry.");
            instruction.reset();
            instruction.set_disable(true);
            throw e; // propagate to exit
        }
    }

    return false;
}
