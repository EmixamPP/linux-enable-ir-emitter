#include <cstring>
#include <iostream>
#include <memory>
using namespace std;

#include "../camera/camera.hpp"
#include "../camera/autocamera.hpp"
#include "finder.hpp"
#include "exhaustivefinder.hpp"
#include "driver.hpp"
#include "../utils/logger.hpp"

constexpr unsigned EXIT_FD_ERROR  = 126;

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
    shared_ptr<Camera> camera;
    if (atoi(argv[6]) == 1)
        camera = make_shared<Camera>(device);
    else
        camera = make_shared<AutoCamera>(device);
    shared_ptr<Finder> finder;
    if (atoi(argv[7]) == 1)
        finder = make_unique<ExhaustiveFinder>(*camera, emitters, negAnswerLimit, excludedPath);
    else
        finder = make_unique<Finder>(*camera, emitters, negAnswerLimit, excludedPath);

    try
    {
        if (camera->isEmitterWorking())
        {
            Logger::error("Your emiter is already working, skipping the configuration.");
            return EXIT_FAILURE;
        }

        auto drivers = finder->find();
        if (drivers->empty())
            return EXIT_FAILURE;

        for (unsigned i = 0; i < drivers->size(); ++i)
        {
            string driverPath = workspace + deviceName + "_emitter" + to_string(i) + ".driver";
            auto &driver = drivers->at(i);
            Driver::writeDriver(driverPath, driver);
        }
    }
    catch (CameraException &e)
    {
        cerr << e.what() << endl;
        return EXIT_FD_ERROR;
    }

    return EXIT_SUCCESS;
}
