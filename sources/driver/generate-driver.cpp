/*** DOCUMENTATION
- https://www.kernel.org/doc/html/latest/userspace-api/media/drivers/uvcvideo.html
    info 1: uvc queries are explained
    info 2: units can be found by parsing the uvc descriptor
- https://www.mail-archive.com/search?l=linux-uvc-devel@lists.berlios.de&q=subject:%22Re%5C%3A+%5C%5BLinux%5C-uvc%5C-devel%5C%5D+UVC%22&o=newest&f=1
    info 1: selector is on 8 bits and since the manufacturer does not provide a driver, it is impossible to know which value it is.
***/

#include <iostream>
#include <cstring>
using namespace std;

#include "camera.hpp"
#include "autocamera.hpp"
#include "finder.hpp"
#include "driver.hpp"
#include "../utils/logger.hpp"

#define EXIT_FD_ERROR 126

/**
 * Generate a driver for the infrared emitter
 *
 * usage: generate-driver [device] [emitters] [negAnswerLimit] [workspace] [debug] [auto]
 *        device           path to the infrared camera
 *        emitters         number of emitters on the device
 *        negAnswerLimit   the number of negative answer before the pattern is skiped. Use 256 for unlimited
 *        workspace        directory where store the driver
 *        debug            1 for print debug information, otherwise 0
 *        auto             1 for fully automatic generation, otherwise 0
 *
 * Exit code: 0 Success
 *            1 Error
 *            126 Unable to open the camera device
 */
int main(int, const char *argv[])
{   
    const string device = argv[1];
    const string deviceName = device.substr(device.find_last_of("/") + 1);
    const unsigned emitters = (unsigned) atoi(argv[2]);
    const unsigned negAnswerLimit = (unsigned) atoi(argv[3]);
    const string workspace = string(argv[4]) + "/";
    const string excludedPath = workspace + deviceName + ".excluded";
    if (atoi(argv[5]) == 1)
        Logger::enableDebug();
    Camera *camera;
    if (atoi(argv[5]) == 1)
        camera = new Camera(device);
    else
        camera = new AutoCamera(device);

    try
    {   
        if (camera->isEmitterWorking())
        {
            Logger::error("Your emiter is already working, skipping the configuration.");
            delete camera;
            return EXIT_FAILURE;
        }
        
        Finder finder(*camera, emitters, negAnswerLimit, excludedPath);
        Driver **driver = finder.find();
        if (driver == nullptr) {
            delete camera;
            return EXIT_FAILURE;
        }

        for (unsigned i = 0; i < emitters; ++i)
        {
            string driverPath = workspace + deviceName + "_emitter" + to_string(i) + ".driver"; ;
            writeDriver(driverPath, driver[i]);
            delete driver[i];
        }
        delete[] driver;
    }
    catch (CameraException &e)
    {   
        cerr << e.what() << endl;
        delete camera;
        return EXIT_FD_ERROR;
    }

    delete camera;
    return EXIT_SUCCESS;
}
