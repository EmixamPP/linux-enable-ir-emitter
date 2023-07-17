#include "driver.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

Driver::Driver(string device, uint8_t unit, uint8_t selector, const vector<uint8_t> &control)
    : device(device), unit(unit), selector(selector), control(control) {}

Driver::~Driver() {}

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
    file << "unit=" << static_cast<int>(driver->unit) << endl;
    file << "selector=" << static_cast<int>(driver->selector) << endl;
    for (unsigned i = 0; i < driver->control.size(); ++i)
        file << "control" << i << "=" << static_cast<int>(driver->control[i]) << endl;

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
        throw ifstream::failure("CRITICAL: Impossible to open the driver at " + driverFile);

    char device[128];
    uint8_t unit, selector, controli;
    vector<uint8_t> control;
    int res = 0;
    res = fscanf(file, " device=%s", device);
    if (res == 0)
        throw runtime_error("CRITICAL: device is missing in the driver at " + driverFile);
    res = fscanf(file, " unit=%hhu", &unit);
    if (res == 0)
        throw runtime_error("CRITICAL: unit is missing in the driver at " + driverFile);
    res = fscanf(file, " selector=%hhu", &selector);
    if (res == 0)
        throw runtime_error("CRITICAL: selector is missing in the driver at " + driverFile);
    res = 1;
    for (unsigned i = 0; res == 1; ++i)
    {
        string key = " control" + to_string(i) + "=%hhu";
        res = fscanf(file, key.c_str(), &controli);
        if (res == 1)
            control.push_back(controli);
    }
    if (control.size() == 0)
        throw runtime_error("CRITICAL: control is missing in the driver at " + driverFile);

    fclose(file);
    return new Driver(device, unit, selector, control);
}
