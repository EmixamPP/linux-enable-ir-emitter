#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <cstdint>
#include <cstdlib>
#include <string>
#include <linux/uvcvideo.h>
using namespace std;

class CameraInstruction;
class Camera
{
private:
    int id;
    int fd = -1;

    static int deviceId(const char *device);

    void openFd();

    void closeFd() noexcept;

public:
    string device;

    Camera(string device);

    ~Camera();

    Camera &operator=(const Camera &) = delete;

    Camera(const Camera &) = delete;

    bool apply(CameraInstruction instruction) noexcept;

    bool isEmitterWorking();

    int executeUvcQuery(const uvc_xu_control_query *query) noexcept;

    int setUvcQuery(uint8_t unit, uint8_t selector, uint16_t controlSize, uint8_t *control) noexcept;

    int getUvcQuery(uint8_t query_type, uint8_t unit, uint8_t selector, uint16_t controlSize, uint8_t *control) noexcept;

    uint16_t lenUvcQuery(uint8_t unit, uint8_t selector) noexcept;
};

class CameraInstruction
{
private:
    uint8_t unit;
    uint8_t selector;
    uint16_t ctrlSize;
    uint8_t *curCtrl = nullptr;
    uint8_t *maxCtrl = nullptr;
    uint8_t *minCtrl = nullptr;
    uint8_t *resCtrl = nullptr;

    static void logDebugCtrl(string prefixMsg, const uint8_t *control, const uint16_t len) noexcept;

    bool isMinConsistent() noexcept;

public:
    CameraInstruction(Camera &camera, uint8_t unit, uint8_t selector);

    CameraInstruction(uint8_t unit, uint8_t selector, uint8_t *control, uint16_t size);

    ~CameraInstruction();

    CameraInstruction &operator=(const CameraInstruction &);
    CameraInstruction(const CameraInstruction &);

    bool next();

    bool hasNext() const noexcept;

    const uint8_t *getCurrent() const noexcept;

    uint16_t getSize() const noexcept;

    uint8_t getUnit() const noexcept;

    uint8_t getSelector() const noexcept;

    bool trySetMinAsCur() noexcept;
};

class CameraException : public exception
{
private:
    string message;

public:
    CameraException(string device);

    const char *what();
};

class CameraInstructionException : public exception
{
private:
    string message;

public:
    CameraInstructionException(string device, uint8_t unit, uint8_t selector);

    const char *what();
};

#endif