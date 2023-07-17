#ifndef CAMERAINSTRUCTION_HPP
#define CAMERAINSTRUCTION_HPP

#include <cstdint>
#include <string>
#include <vector>
using namespace std;

class Camera;

class CameraInstruction
{
protected:
    uint8_t unit;
    uint8_t selector;
    vector<uint8_t> curCtrl;
    vector<uint8_t> maxCtrl;
    vector<uint8_t> minCtrl;
    vector<uint8_t> resCtrl;

    static void logDebugCtrl(string prefixMsg, const vector<uint8_t> &control) noexcept;

    static void computeResCtrl(const vector<uint8_t> &first, const vector<uint8_t> &second, vector<uint8_t> &res) noexcept;

    static bool isReachable(const vector<uint8_t> &base, const vector<uint8_t> &res, const vector<uint8_t> &toReach) noexcept;

public:
    CameraInstruction(Camera &camera, uint8_t unit, uint8_t selector);

    CameraInstruction(uint8_t unit, uint8_t selector, const vector<uint8_t> &control);

    ~CameraInstruction();

    CameraInstruction &operator=(const CameraInstruction &);
    CameraInstruction(const CameraInstruction &);

    bool next();

    bool hasNext() const noexcept;

    const vector<uint8_t> &getCurrent() const noexcept;

    uint8_t getUnit() const noexcept;

    uint8_t getSelector() const noexcept;

    bool trySetMinAsCur() noexcept;
};

class CameraInstructionException : public exception
{
private:
    string message;

public:
    explicit CameraInstructionException(string device, uint8_t unit, uint8_t selector);

    virtual const char *what() const noexcept override;
};

#endif