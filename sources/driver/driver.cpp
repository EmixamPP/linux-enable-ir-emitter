#include "driver.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <cstring>
using namespace std;


Driver::Driver(string device, uint8_t unit, uint8_t selector, uint16_t size, const uint8_t *control)
    : device(device), unit(unit), selector(selector), size(size), control(new uint8_t[size])
{
    memcpy(this->control, control, size * sizeof(uint8_t));
}

Driver::~Driver()
{
    delete[] control;
}


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
    file << "size=" << (int)driver->size << endl;
    for (unsigned i = 0; i < driver->size; ++i)
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
    fscanf(file, " device=%s*", device);
    fscanf(file, " unit=%hhd", &unit);
    fscanf(file, " selector=%hhd", &selector);
    fscanf(file, " size=%hd", &size);
    uint8_t *control = new uint8_t[size];
    for (unsigned i = 0; i < size; ++i)
    {
        string key = " control" + to_string(i) + "=%d";
        fscanf(file, key.c_str(), &control[i]);
    }

    fclose(file);
    Driver *driver = new Driver(device, unit, selector, size, control);
    delete[] control;
    return driver;
}
