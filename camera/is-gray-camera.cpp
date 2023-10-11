#include <memory>
using namespace std;

#include "camera.hpp"
#include "opencv.hpp"

/**
 * Check if a camera is in gray scale
 *
 * usage: is-gray-camera [device]
 *        device           path to the camera
 *
 * Exit code: 0 Success
 *            1 Error
 *            126 Unable to open the camera device
 */
int main(int, const char **argv)
{
    Camera camera(argv[1]);
    unique_ptr<cv::Mat> frame = camera.read1();

    if (frame->channels() != 3)
        return 1;

    for (int r = 0; r < frame->rows; ++r)
        for (int c = 0; c < frame->cols; ++c)
        {
            const cv::Vec3b &pixel = frame->at<cv::Vec3b>(r, c);
            if (pixel[0] != pixel[1] || pixel[0] != pixel[2])
                return 1;
        }

    return 0;
}