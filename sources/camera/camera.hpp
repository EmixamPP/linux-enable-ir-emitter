#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <cstdint>
#include <linux/uvcvideo.h>
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
    cv::VideoCapture *cap = new cv::VideoCapture();

protected:
    int getFd() const noexcept;

    cv::VideoCapture *getCap() const noexcept;

    void openFd();

    void closeFd() noexcept;

    void openCap();

    void closeCap() noexcept;

    static int deviceId(const string &device);

    int executeUvcQuery(const uvc_xu_control_query &query);

public:
    const string device;

    Camera() = delete;

    explicit Camera(const string &device);

    virtual ~Camera();

    Camera &operator=(const Camera &) = delete;

    Camera(const Camera &) = delete;

    Camera &operator=(Camera &&other) = delete;

    Camera(Camera &&other) = delete;

    bool apply(const CameraInstruction &instruction);

    virtual bool isEmitterWorking();

    unique_ptr<cv::Mat> read1();

    int setUvcQuery(uint8_t unit, uint8_t selector, vector<uint8_t> &control);

    int getUvcQuery(uint8_t query_type, uint8_t unit, uint8_t selector, vector<uint8_t> &control);

    uint16_t lenUvcQuery(uint8_t unit, uint8_t selector);
};

class CameraException : public exception
{
private:
    string message;

public:
    CameraException(const string &device);

    const char *what() const noexcept override;
};

#endif