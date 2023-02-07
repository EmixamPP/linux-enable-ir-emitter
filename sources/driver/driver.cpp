#include "driver.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;


/**
 * @brief Write the driver in a file
 *
 * @param driverFile path where the driver will be written
 * @param driver the driver to write
 *
 * @throw ifstream::failure Impossible to write the driver in driverFile
 */
void writeDriver(const char *driverFile, const Driver *driver)
{
    ofstream file(driverFile);
    if (!file.is_open())
        throw ifstream::failure("CRITICAL: Impossible to write the driver in " + string(driverFile));

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
Driver *readDriver(const char *driverFile)
{
    FILE *file = fopen(driverFile, "r");
    if (!file)
        throw ifstream::failure("CRITICAL: Impossible to open the driver at " + string(driverFile));

    Driver *driver = new Driver();
    fscanf(file, " device=%s", driver->device);
    fscanf(file, " unit=%hhd", &(driver->unit));
    fscanf(file, " selector=%hhd", &(driver->selector));
    fscanf(file, " size=%hd", &(driver->size));
    driver->control = (uint8_t *)malloc(driver->size * sizeof(uint8_t));
    for (unsigned i = 0; i < driver->size; ++i)
    {
        const char *key = (" control" + to_string(i) + "=%d").c_str();
        fscanf(file, key, &(driver->control[i]));
    }

    fclose(file);
    return driver;
}
