#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <linux/uvcvideo.h>
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

    static int deviceId(string device);

    int executeUvcQuery(const uvc_xu_control_query &query) noexcept;

public:
    string device;

    Camera(string device);

    virtual ~Camera();

    Camera &operator=(const Camera &) = delete;

    Camera(const Camera &) = delete;

    bool apply(const CameraInstruction &instruction) noexcept;

    virtual bool isEmitterWorking();

    int setUvcQuery(uint8_t unit, uint8_t selector, vector<uint8_t> &control) noexcept;

    int getUvcQuery(uint8_t query_type, uint8_t unit, uint8_t selector, vector<uint8_t> &control) noexcept;

    uint16_t lenUvcQuery(uint8_t unit, uint8_t selector) noexcept;
};

class CameraException : public exception
{
protected:
    string message;

public:
    explicit CameraException(string device);

    virtual const char *what() const noexcept override;
};

#endif