#include "autocamera.hpp"

#include <vector>
#include <string>
#include <chrono>
using namespace std;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include <opencv2/videoio.hpp>
#pragma GCC diagnostic pop

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
    bool isWorking = false;

    for (auto frame : frames)
        delete frame;

    return isWorking;
}

AutoCamera::AutoCamera(string device, unsigned captureTime) : Camera(device), captureTime(captureTime) {}

AutoCamera::~AutoCamera() {}