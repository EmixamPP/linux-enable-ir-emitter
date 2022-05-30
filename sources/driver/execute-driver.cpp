#include <iostream>
using namespace std;

#include "setquery.h"
#include "driver.hpp"

/**
 * Execute a driver created by driver-generator
 *
 * usage: execute-driver [driverFile]
 *        driverFile     path where the driver have been written
 *
 * See std output for debug information and stderr for error information
 *
 * Exit code: 0 Success
 *            1 Error
 *            265 Unable to open the camera device
 */
int main(int, char **argv)
{
    Driver *driver = read_driver(argv[1]);
    if (!driver) {
        cerr << "CRITICAL: No driver for" << argv[1] << "has been configured." << endl;
        return 1;
    }
    int res = set_uvc_query(driver->device, driver->unit, driver->selector, driver->size, driver->control);
    delete driver;
    return res;
}