#pragma once

#include <linux/uvcvideo.h>
#include <memory>
#include <string>
#include <vector>
using namespace std;

#include "opencv.hpp"

class CameraInstruction;

class Camera
{
private:
    bool noGui = false;
    int fd = -1;
    const shared_ptr<cv::VideoCapture> cap = make_shared<cv::VideoCapture>();
    const vector<int> capParams;
    int id;

    void openFd();

    void closeFd() noexcept;

    void openCap();

    void closeCap() noexcept;

    int executeUvcQuery(const uvc_xu_control_query &query);

    bool isEmitterWorkingAsk();

    bool isEmitterWorkingAskNoGui();

public:
    const string device;

    Camera() = delete;

    explicit Camera(const string &device, int width = -1, int height = -1);

    virtual ~Camera();

    Camera &operator=(const Camera &) = delete;

    Camera(const Camera &) = delete;

    Camera &operator=(Camera &&other) = delete;

    Camera(Camera &&other) = delete;

    void disableGui();

    function<void()> play();

    void playForever();

    cv::Mat read1();

    vector<cv::Mat> readDuring(unsigned captureTimeMs);

    virtual bool isEmitterWorking();

    bool isGrayscale();

    bool apply(const CameraInstruction &instruction);

    int setUvcQuery(uint8_t unit, uint8_t selector, vector<uint8_t> &control);

    int getUvcQuery(uint8_t query_type, uint8_t unit, uint8_t selector, vector<uint8_t> &control);

    uint16_t lenUvcQuery(uint8_t unit, uint8_t selector);

    static shared_ptr<Camera> findGrayscaleCamera(int width = -1, int height = -1);
};

class CameraException : public exception
{
private:
    string message;

public:
    CameraException(const string &device);

    const char *what() const noexcept override;
};
