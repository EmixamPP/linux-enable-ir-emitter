#include "finder.hpp"

#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
using namespace std;

#include "../camera/camera.hpp"
#include "../camera/camerainstruction.hpp"
#include "driver.hpp"
#include "../utils/logger.hpp"

/**
 * @brief Create a Driver from Instruction object
 *
 * @param instruction from which the driver has to be create
 *
 * @return the driver
 */
unique_ptr<Driver> Finder::createDriverFromInstruction(const CameraInstruction &instruction, uint8_t unit, uint8_t selector) const noexcept
{
    return make_unique<Driver>(camera.device, unit, selector, instruction.getCurrent());
}

/**
 * @brief Obtain the units and selectors to exclude
 *
 * @return exclude list
 */
unique_ptr<vector<pair<uint8_t, uint8_t>>> Finder::getExcluded() noexcept
{
    auto excludedList = make_unique<vector<pair<uint8_t, uint8_t>>>();

    ifstream file(excludedPath);
    if (!file.is_open())
        return excludedList;

    while (!file.eof())
    {
        string line;
        getline(file, line);
        uint8_t unit;
        uint8_t selector;
        sscanf(line.c_str(), "%hhu %hhu", &unit, &selector);
        excludedList->push_back(pair<uint8_t, uint8_t>(unit, selector));
    }

    file.close();
    return excludedList;
}

/**
 * @brief Check if an unit and selector are excluded
 *
 * @param unit to check
 * @param selector to select
 *
 * @return true if they are excluded, otherwise false
 */
bool Finder::isExcluded(uint8_t unit, uint8_t selector) const noexcept
{
    for (auto unitSelector : *excluded)
        if (unitSelector.first == unit && unitSelector.second == selector)
            return true;
    return false;
}

/**
 * @brief Add an unit and selector to the exclude list (do not modify the attribute)
 *
 * @param unit to exclude
 * @param selector to exclude
 */
void Finder::addToExclusion(uint8_t unit, uint8_t selector) noexcept
{
    ofstream file(excludedPath, std::ofstream::out | std::ofstream::app);
    if (!file.is_open())
        return;
    file << unit << " " << selector << endl;
    file.close();
}

/**
 * @brief Construct a new Finder:: Finder object
 *
 * @param camera on which try to find a driver for the emitter
 * @param emitters number of emitters on the device
 * @param negAnswerLimit skip a patern after negAnswerLimit negative answer
 * @param excludedPath path where write unit and selector to exclude from the search
 */
Finder::Finder(Camera &camera, unsigned emitters, unsigned negAnswerLimit, const string &excludedPath)
    : camera(camera), emitters(emitters), negAnswerLimit(negAnswerLimit), excludedPath(excludedPath)
{
    for (unsigned unit = 0; unit < 256; ++unit)
        units.push_back(static_cast<uint8_t>(unit));

    excluded = getExcluded();
}

/**
 * @brief Find a driver which enable the ir emitter(s)
 *
 * @return a vector containing the driver(s),
 * empty if the configuration failed
 */
unique_ptr<vector<unique_ptr<Driver>>> Finder::find()
{
    auto drivers = make_unique<vector<unique_ptr<Driver>>>();

    for (const uint8_t unit : units)
        for (unsigned __selector = 0; __selector < 256; ++__selector)
        {
            const uint8_t selector = static_cast<uint8_t>(__selector); // safe: 0 <= __selector <= 255
            if (isExcluded(unit, selector))
                continue;
            try
            {
                CameraInstruction instruction(camera, unit, selector);
                const CameraInstruction initInstruction = instruction; // copy for reset later
                
                instruction.setMinAsCur();
                unsigned negAnswerCounter = 0;
                while (negAnswerCounter < negAnswerLimit && instruction.next())
                {   
                    if (negAnswerCounter == negAnswerLimit - 1)
                        instruction.setMaxAsCur();

                    if (camera.apply(instruction) && camera.isEmitterWorking())
                    {
                        drivers->push_back(createDriverFromInstruction(instruction, unit, selector));
                        if (drivers->size() == emitters) // all emitters are configured
                            return drivers;
                    }
                    ++negAnswerCounter;
                }

                camera.apply(initInstruction);
                Logger::debug("");
            }
            catch (CameraInstructionException &e)
            {
                continue;
            }
            catch (CameraException &e)
            {
                Logger::error("Impossible to reset the camera.");
                Logger::info("Please shut down your computer, then boot and retry.");
                addToExclusion(unit, selector);
                throw e; // propagate to exit
            }
        }

    return drivers;
}
