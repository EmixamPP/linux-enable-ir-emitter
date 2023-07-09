#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <linux/uvcvideo.h>
using namespace std;

class CameraInstruction;

class Camera
{
private:
    int id;
    int fd = -1;

    void openFd();

    void closeFd() noexcept;

protected:
    static int deviceId(string device);

    int executeUvcQuery(const struct uvc_xu_control_query &query) noexcept;

public:
    string device;

    Camera(string device);

    ~Camera();

    Camera &operator=(const Camera &) = delete;

    Camera(const Camera &) = delete;

    bool apply(const CameraInstruction &instruction) noexcept;

    bool isEmitterWorking();

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

    const char *what() const noexcept override;
};

#endif