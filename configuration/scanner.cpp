#include "scanner.hpp"

#include <iomanip>
#include <iostream>
using namespace std;

#include <spdlog/spdlog.h>

#include "camera/camerainstruction.hpp"


/**
 * @brief Construct a new Scanner:: Scanner object
 *
 * @param camera on which scans the instructions
 */
Scanner::Scanner(shared_ptr<Camera> camera) : camera_(camera) {}

/**
 * @brief Scans the available camera instructions
 *
 * @return the list of instructions
 */
CameraInstructions Scanner::scan() noexcept
{
    CameraInstructions instructions;

    for (unsigned _unit = 0; _unit < 256; ++_unit)
    {
        const uint8_t unit = static_cast<uint8_t>(_unit); // safe: 0 <= _units <= 255
        for (unsigned _selector = 0; _selector < 256; ++_selector)
        {
            const uint8_t selector = static_cast<uint8_t>(_selector); // safe: 0 <= _selector <= 255

            try
            {
                instructions.push_back(CameraInstruction(*camera_, unit, selector));
            }
            catch (const CameraInstructionException &)
            {
            }
        }
    }
    cout << endl;

    return instructions;
}
