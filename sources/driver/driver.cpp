#include "driver.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

Driver::Driver(const string &device, uint8_t unit, uint8_t selector, const vector<uint8_t> &control)
    : device(device), unit(unit), selector(selector), control(control) {}

/**
 * @brief Write the driver in a file
 *
 * @param driverFile path where the driver will be written
 * @param driver the driver to write
 *
 * @throw ifstream::failure Impossible to write the driver in driverFile
 */
void Driver::writeDriver(const string &driverFile, const unique_ptr<Driver> &driver)
{
    ofstream file(driverFile);
    if (!file.is_open())
        throw ifstream::failure("CRITICAL: Impossible to write the driver in " + driverFile);

    file << "device=" << driver->device << endl;
    file << "unit=" << static_cast<int>(driver->unit) << endl;
    file << "selector=" << static_cast<int>(driver->selector) << endl;
    for (unsigned i = 0; i < driver->control.size(); ++i)
        file << "control" << i << "=" << static_cast<int>(driver->control[i]) << endl;
}

/**
 * @brief Read the driver and return its values
 *
 * @param driverFile path where the driver is store
 *
 * @throw ifstream::failure Impossible to open the driver at driverFile
 
 * @return the driver
 */
unique_ptr<Driver> Driver::readDriver(const string &driverFile)
{
    ifstream file(driverFile);
    if (!file.is_open())
        throw ifstream::failure("CRITICAL: Impossible to open the driver at " + driverFile);

    stringstream contentstream;
    contentstream << file.rdbuf();
    string content = contentstream.str();  
    
    char device[128];
    uint8_t unit;
    uint8_t selector;
    uint8_t controli;
    vector<uint8_t> control;
    int res = 0;
    res = sscanf(content.c_str(), "device=%s", device);
    if (res == 0)
        throw runtime_error("CRITICAL: device is missing in the driver at " + driverFile);
    res = sscanf(content.c_str(), "unit=%hhu", &unit);
    if (res == 0)
        throw runtime_error("CRITICAL: unit is missing in the driver at " + driverFile);
    res = sscanf(content.c_str(), "selector=%hhu", &selector);
    if (res == 0)
        throw runtime_error("CRITICAL: selector is missing in the driver at " + driverFile);
    res = 1;
    for (unsigned i = 0; res == 1; ++i)
    {
        string key = "control" + to_string(i) + "=%hhu";
        res = sscanf(content.c_str(), key.c_str(), &controli);
        if (res == 1)
            control.push_back(controli);
    }
    if (control.empty())
        throw runtime_error("CRITICAL: control is missing in the driver at " + driverFile);

    return make_unique<Driver>(device, unit, selector, control);
}
