#include "driver.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

Driver::Driver(string device, uint8_t unit, uint8_t selector, const vector<uint8_t> &control)
    : device(device), unit(unit), selector(selector), control(control) {}

/**
 * @brief Write the driver in a file
 *
 * @param driverFile path where the driver will be written
 * @param driver the driver to write
 *
 * @throw ifstream::failure Impossible to write the driver in driverFile
 */
void writeDriver(string driverFile, const Driver *driver)
{
    ofstream file(driverFile);
    if (!file.is_open())
        throw ifstream::failure("CRITICAL: Impossible to write the driver in " + driverFile);

    file << "device=" << driver->device << endl;
    file << "unit=" << (int)driver->unit << endl;
    file << "selector=" << (int)driver->selector << endl;
    for (unsigned i = 0; i < driver->control.size(); ++i)
        file << "control" << i << "=" << (int)driver->control[i] << endl;

    file.close();
}

/**
 * @brief Read the driver and return its values
 *
 * @param driverFile path where the driver is store
 *
 * @throw ifstream::failure Impossible to open the driver at driverFile
 *
 * @return the driver
 */
Driver *readDriver(string driverFile)
{
    FILE *file = fopen(driverFile.c_str(), "r");
    if (!file)
        throw ifstream::failure("CRITICAL: Impossible to open the driver at " + string(driverFile));

    char device[128];
    uint8_t unit, selector;
    uint16_t size;
    int count = 0;
    count += fscanf(file, " device=%s*", device);
    count += fscanf(file, " unit=%hhu", &unit);
    count += fscanf(file, " selector=%hhu", &selector);
    count += fscanf(file, " size=%hu", &size);
    vector<uint8_t> control(size);
    for (unsigned i = 0; i < size; ++i)
    {
        string key = " control" + to_string(i) + "=%d";
        count += fscanf(file, key.c_str(), &control[i]);
    }

    if (count != 4 + size)
        throw runtime_error("CRITICAL: The driver at " + string(driverFile) + " is corrupted");

    fclose(file);
    return new Driver(device, unit, selector, control);
}
