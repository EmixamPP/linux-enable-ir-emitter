#include "commands.hpp"

#include <memory>
using namespace std;

#include "globals.hpp"
#include "../driver/driver.hpp"
#include "../camera/camera.hpp"
#include "../camera/camerainstruction.hpp"
#include "../utils/logger.hpp"

/**
 * @brief Execute a driver.
 *
 * @param device path to the infrared camera, empty string to execute all driver
 *
 * @return exit code
 */
ExitCode run(const char* device)
{   
    
    auto paths = get_drivers_path(device);

    if (paths->empty())
        Logger::critical(ExitCode::FAILURE, "No driver for", device, "has been configured.");

    ExitCode code = ExitCode::SUCCESS;
    for (auto &driverFile : *paths)
    {
        const unique_ptr<Driver> driver = Driver::readDriver(driverFile);
        try
        {
            Camera camera(driver->device);
            CameraInstruction instruction = CameraInstruction(driver->unit, driver->selector, driver->control);
            if (!camera.apply(instruction))
            {
                Logger::error("Failed to apply the driver of", driver->device);
                code = ExitCode::FAILURE;
            }
        }
        catch (CameraException &e)
        {
            Logger::critical(ExitCode::FILE_DESCRIPTOR_ERROR, e.what());
        }
    }

    return code;
}
