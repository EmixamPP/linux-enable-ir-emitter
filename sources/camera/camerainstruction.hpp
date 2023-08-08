#ifndef CAMERAINSTRUCTION_HPP
#define CAMERAINSTRUCTION_HPP

#include <cstdint>
#include <string>
#include <vector>
using namespace std;

class Camera;

class CameraInstruction
{
private:
    uint8_t unit;
    uint8_t selector;

protected:
    vector<uint8_t> curCtrl;
    vector<uint8_t> maxCtrl;
    vector<uint8_t> minCtrl;

    static void logDebugCtrl(const string &prefixMsg, const vector<uint8_t> &control) noexcept;

public:
    CameraInstruction() = delete;

    explicit CameraInstruction(Camera &camera, uint8_t unit, uint8_t selector);

    explicit CameraInstruction(uint8_t unit, uint8_t selector, const vector<uint8_t> &control);

    ~CameraInstruction() = default;

    CameraInstruction &operator=(const CameraInstruction &) = default;

    CameraInstruction(const CameraInstruction &) = default;

    CameraInstruction &operator=(CameraInstruction &&other) = default;

    CameraInstruction(CameraInstruction &&other) = default;

    bool next() noexcept;

    const vector<uint8_t> &getCurrent() const noexcept;

    uint8_t getUnit() const noexcept;

    uint8_t getSelector() const noexcept;

    bool setMinAsCur() noexcept;

    bool setMaxAsCur() noexcept;
};

class CameraInstructionException : public exception
{
private:
    string message;

public:
    explicit CameraInstructionException(const string &device, uint8_t unit, uint8_t selector);

    const char *what() const noexcept override;
};

#endif