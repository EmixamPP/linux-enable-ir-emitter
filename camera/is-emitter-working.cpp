#include <memory>
using namespace std;

#include "autocamera.hpp"
#include "opencv.hpp"

constexpr unsigned EXIT_FD_ERROR = 126;

/**
 * Check if a ir emitter is working using the automatic detection algorithm
 *
 * usage: is-emitter-camera [device]
 *        device           path to the camera
 *
 * Exit code: 0 Success
 *            1 Error
 *            126 Unable to open the camera device
 */
int main(int, const char **argv)
{
    AutoCamera camera(argv[1]);
    try
    {
        return static_cast<int>(camera.isEmitterWorkingNoConfirm());
    }
    catch (CameraException &e)
    {
        cerr << e.what() << endl;
        return EXIT_FD_ERROR;
    }
}