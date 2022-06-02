#include <fcntl.h>
#include <unistd.h>
#include <iostream>
using namespace std;

#include "setquery.h"
#include "driver.hpp"

constexpr unsigned FILE_DESCRIPTOR_ERROR = 126;

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
 *            126 Unable to open the camera device
 */
int main(int, const char **argv)
{
    const Driver *driver = read_driver(argv[1]);
    if (!driver)
    {
        cerr << "CRITICAL: No driver for" << argv[1] << "has been configured." << endl;
        return 1;
    }

    errno = 0;
    const int fd = open(driver->device, O_WRONLY);
    if (fd < 0 || errno)
    {
        fprintf(stderr, "ERROR: Cannot access to %s\n", driver->device);
        delete driver;
        exit(FILE_DESCRIPTOR_ERROR);
    }

    int result = set_uvc_query(fd, driver->unit, driver->selector, driver->size, driver->control);

    delete driver;
    close(fd);

    return result;
}