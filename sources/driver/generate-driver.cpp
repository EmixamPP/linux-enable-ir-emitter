#include <iostream>
#include <cstring>
using namespace std;

#include "../camera/camera.hpp"
#include "../camera/autocamera.hpp"
#include "finder.hpp"
#include "exhaustivefinder.hpp"
#include "driver.hpp"
#include "../utils/logger.hpp"

#define EXIT_FD_ERROR 126

/**
 * Generate a driver for the infrared emitter
 *
 * usage: generate-driver [device] [emitters] [negAnswerLimit] [workspace] [debug] [manual] [exhaustive]
 *        device           path to the infrared camera
 *        emitters         number of emitters on the device
 *        negAnswerLimit   the number of negative answer before the pattern is skiped. Use 256 for unlimited
 *        workspace        directory where store the driver
 *        debug            1 for print debug information, otherwise 0
 *        manual           1 for manual configuration, 0 for automatic
 *        exhaustive       1 for exhaustive configuration, otherwise 0
 *
 * Exit code: 0 Success
 *            1 Error
 *            126 Unable to open the camera device
 */
int main(int, const char *argv[])
{
    const string device = argv[1];
    const string deviceName = device.substr(device.find_last_of("/") + 1);
    const unsigned emitters = static_cast<unsigned>(atoi(argv[2]));
    const unsigned negAnswerLimit = static_cast<unsigned>(atoi(argv[3]));
    const string workspace = string(argv[4]) + "/";
    const string excludedPath = workspace + deviceName + ".excluded";
    if (atoi(argv[5]) == 1)
        Logger::enableDebug();
    Camera *camera;
    if (atoi(argv[5]) == 1)
        camera = new Camera(device);
    else
        camera = new AutoCamera(device);
    Finder *finder;
    if (atoi(argv[6]) == 1)
        finder = new ExhaustiveFinder(*camera, emitters, negAnswerLimit, excludedPath);
    else
        finder = new Finder(*camera, emitters, negAnswerLimit, excludedPath);

    try
    {
        if (camera->isEmitterWorking())
        {
            Logger::error("Your emiter is already working, skipping the configuration.");
            delete camera;
            delete finder;
            return EXIT_FAILURE;
        }

        Driver **driver = finder->find();
        if (driver == nullptr)
        {
            delete camera;
            delete finder;
            return EXIT_FAILURE;
        }

        for (unsigned i = 0; i < emitters; ++i)
        {
            string driverPath = workspace + deviceName + "_emitter" + to_string(i) + ".driver";
            writeDriver(driverPath, driver[i]);
            delete driver[i];
        }
        delete[] driver;
    }
    catch (CameraException &e)
    {
        cerr << e.what() << endl;
        delete finder;
        delete camera;
        return EXIT_FD_ERROR;
    }

    delete camera;
    delete finder;
    return EXIT_SUCCESS;
}
