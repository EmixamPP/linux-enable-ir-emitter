#ifndef DRIVER
#define DRIVER

#include <stdio.h>
#include <linux/uvcvideo.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

struct Driver
{
    char device[16];
    __u8 unit;
    __u8 selector;
    __u16 size;
    __u8 *control;
};

/**
 * @brief Write the driver in a file
 * 
 * @param driverFile path where the driver will be written
 * @param device path to the infrared camera, /dev/videoX
 * @param unit extension unit ID 
 * @param selector control selector
 * @param size size of the uvc control
 * @param control control value
 * @return non zero if error 
 */
inline int write_driver(const char *driverFile, const char *device, __u8 unit, __u8 selector, __u16 size, const __u8 *control)
{
    ofstream file(driverFile);
    if (!file.is_open())
    {
        cerr << "ERROR: Impossible to write the driver in " << driverFile << endl;
        return 1;
    }

    file << "device=" << device << endl;
    file << "unit=" << (int)unit << endl;
    file << "selector=" << (int)selector << endl;
    file << "size=" << (int)size << endl;
    for (unsigned i = 0; i < size; ++i)
        file << "control" << i << "=" << (int)control[i] << endl;

    file.close();
    return 0;
}

/**
 * @brief Read the driver and return its values
 * 
 * @param driverFile path where the driver is store
 * @return the driver values
 */
#pragma GCC diagnostic ignored "-Wformat" // ignore warning on fscanf type
inline struct Driver *read_driver(const char *driverFile)
{
    FILE *file = fopen(driverFile, "r");
    if (!file)
    {
        cerr << "ERROR: Impossible to read the driver in " << driverFile << endl;
        return nullptr;
    }

    Driver *driver = new Driver();
    fscanf(file, " device=%s", driver->device);
    fscanf(file, " unit=%d", &(driver->unit));
    fscanf(file, " selector=%d", &(driver->selector));
    fscanf(file, " size=%d", &(driver->size));
    driver->control = (__u8 *)malloc(driver->size * sizeof(__u8));
    for (unsigned i = 0; i < driver->size; ++i)
    {
        const char *key = (" control" + to_string(i) + "=%d").c_str();
        fscanf(file, key, &(driver->control[i]));
    }

    fclose(file);
    return driver;
}

#endif