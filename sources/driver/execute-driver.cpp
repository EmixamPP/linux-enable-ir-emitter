#include <iostream>
using namespace std;

#include "driver.hpp"
#include "camera.hpp"
#include "camerainstruction.hpp"

#define EXIT_FD_ERROR 126;

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
    const Driver *driver = readDriver(argv[1]);
    bool result;
    try
    {
        Camera camera(driver->device);
        CameraInstruction instruction = CameraInstruction(driver->unit, driver->selector, driver->control);
        result = camera.apply(instruction);
    }
    catch (CameraException &e)
    {
        cerr << e.what() << endl;
        return EXIT_FD_ERROR;
    }

    delete driver;
    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}