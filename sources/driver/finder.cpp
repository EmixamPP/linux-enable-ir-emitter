#include "finder.hpp"

#include <vector>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
using namespace std;

#include "camera.hpp"
#include "logger.hpp"
#include "driver.hpp"

/**
 * @brief Execute shell command and return the ouput
 *
 * @param cmd command
 * @return output
 */
string *Finder::shellExec(const string cmd) noexcept
{
    char buffer[128];
    string *result = new string();
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
        *result = "error";
        return result;
    }

    while (fgets(buffer, 128 * sizeof(char), pipe.get()) != nullptr)
        *result += buffer;
    result->erase(result->end() - 1); // remove last \n
    return result;
}

/**
 * @brief Get all the extension unit ID of the camera device
 *
 * @param Camera the camera device
 *
 * @return list of units
 */
vector<uint8_t> *Finder::getUnits(const Camera &camera) noexcept
{
    const string *vid = Finder::shellExec("udevadm info " + string(camera.device) + " | grep -oP 'E: ID_VENDOR_ID=\\K.*'");
    const string *pid = Finder::shellExec("udevadm info " + string(camera.device) + " | grep -oP 'E: ID_MODEL_ID=\\K.*'");
    const string *units = Finder::shellExec("lsusb -d" + *vid + ":" + *pid + " -v | grep bUnitID | grep -Eo '[0-9]+'");
    auto *unitsList = new vector<uint8_t>;

    unsigned i = 0, j = 0;
    for (; j < units->length(); ++j)
        if (units->at(j) == '\n')
        {
            unitsList->push_back((uint8_t)stoi(units->substr(i, j - i)));
            i = j + 1;
        }
    unitsList->push_back((uint8_t)stoi(units->substr(i, j - i)));

    delete vid;
    delete pid;
    delete units;
    return unitsList;
}

/**
 * @brief Create a Driver from Instruction object
 *
 * @param instruction from which the driver has to be create
 * @return the driver
 */
Driver *Finder::createDriverFromInstruction(const CameraInstruction &instruction, uint8_t unit, uint8_t selector) const noexcept
{
    return new Driver(camera.device, unit, selector, instruction.getSize(), instruction.getCurrent());
}

/**
 * @brief Obtain the units and selectors to exclude
 *
 * @return exclude list
 */
vector<pair<uint8_t, uint8_t>> *Finder::getExcluded() noexcept
{
    auto *excludedList = new vector<pair<uint8_t, uint8_t>>;

    ifstream file(excludedPath);
    if (!file.is_open())
        return excludedList;

    while (file)
    {
        string line;
        getline(file, line);
        uint8_t unit, selector;
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
    file << (int)unit << " " << (int)selector << endl;
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
Finder::Finder(Camera &camera, unsigned emitters, unsigned negAnswerLimit, string excludedPath) noexcept
    : camera(camera), units(Finder::getUnits(camera)), emitters(emitters), negAnswerLimit(negAnswerLimit), excludedPath(excludedPath), excluded(nullptr)
{
    excluded = getExcluded();
};

Finder::~Finder()
{
    delete units;
    delete excluded;
}

/**
 * @brief Find a driver which enable the ir emitter
 *
 * @return the driver if success otherwise nullptr
 */
Driver **Finder::find()
{
    Driver **drivers = new Driver *[emitters];
    unsigned configuredEmitters = 0;

    for (const uint8_t unit : *units)
        for (int __selector = 0; __selector < 256; ++__selector)
        {
            const uint8_t selector = (uint8_t)__selector; // safe: 0 <= __selector <= 255
            if (isExcluded(unit, selector))
                continue;
            try
            {      
                CameraInstruction instruction(camera, unit, selector);
                CameraInstruction initInstruction = instruction; // copy for reset later

                if (!instruction.trySetMinAsCur()) // if no min instruction exists
                {
                    if (instruction.hasNext()) // start from the next one
                        instruction.next();
                    else
                        continue;
                }
                
                unsigned negAnswerCounter = 0;
                do
                {
                    if (camera.apply(instruction))
                    {
                        if (camera.isEmitterWorking())
                        {
                            drivers[configuredEmitters++] = createDriverFromInstruction(instruction, unit, selector);
                            if (configuredEmitters == emitters) // all emitters are configured
                                return drivers;
                        }
                        else
                            camera.apply(initInstruction); // reset
                    }
                    ++negAnswerCounter;
                } while (negAnswerCounter < negAnswerLimit && instruction.hasNext() && instruction.next());

                if (negAnswerCounter >= negAnswerLimit)
                    Logger::debug("Negative answer limit exceeded, skipping the pattern.");
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

    delete[] drivers;
    return nullptr;
}
