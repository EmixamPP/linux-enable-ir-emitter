#include "autocamera.hpp"

#include <chrono>
#include <string>
#include <vector>
using namespace std;

#include "opencv.hpp"

bool AutoCamera::isEmitterWorking()
{
    openCap();
    cv::VideoCapture *cap = getCap();
    vector<cv::Mat *> frames;

    const auto stopTime = chrono::steady_clock::now() + chrono::seconds(captureTime);
    while (chrono::steady_clock::now() < stopTime)
    {
        auto *frame = new cv::Mat();
        cap->read(*frame);
        if (!frame->empty())
            frames.push_back(frame);
    }

    // TODO compare frames to determine if emitter is working
    // cv::Mat doc: https://docs.opencv.org/4.8.0/d3/d63/classcv_1_1Mat.html
    const bool isWorking = false;

    for (auto *frame : frames)
        delete frame;

    return isWorking;
}

AutoCamera::AutoCamera(const string &device, unsigned captureTime) : Camera(device), captureTime(captureTime) {}
