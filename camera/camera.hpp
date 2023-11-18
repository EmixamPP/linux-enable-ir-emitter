#pragma once

#include <cstdint>
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
    int id;
    int fd = -1;
    const shared_ptr<cv::VideoCapture> cap = make_shared<cv::VideoCapture>();
    bool noGui = false;

protected:
    int getFd() const noexcept;

    shared_ptr<cv::VideoCapture> getCap() const noexcept;

    void openFd();

    void closeFd() noexcept;

    void openCap();

    void closeCap() noexcept;

    static int deviceId(const string &device);

    int executeUvcQuery(const uvc_xu_control_query &query);

    bool isEmitterWorkingAsk();

    bool isEmitterWorkingAskNoGui();

public:
    const string device;

    Camera() = delete;

    explicit Camera(const string &device);

    virtual ~Camera();

    Camera &operator=(const Camera &) = delete;

    Camera(const Camera &) = delete;

    Camera &operator=(Camera &&other) = delete;

    Camera(Camera &&other) = delete;

    void play();

    bool apply(const CameraInstruction &instruction);

    virtual bool isEmitterWorking();

    cv::Mat read1();

    int setUvcQuery(uint8_t unit, uint8_t selector, vector<uint8_t> &control);

    int getUvcQuery(uint8_t query_type, uint8_t unit, uint8_t selector, vector<uint8_t> &control);

    uint16_t lenUvcQuery(uint8_t unit, uint8_t selector);

    bool isGrayscale();

    static shared_ptr<Camera> findGrayscaleCamera();

    void disableGui();
};

class CameraException : public exception
{
private:
    string message;

public:
    CameraException(const string &device);

    const char *what() const noexcept override;
};
