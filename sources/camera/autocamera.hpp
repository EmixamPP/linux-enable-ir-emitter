#ifndef AUTOCAMERA_HPP
#define AUTOCAMERA_HPP

#include <string>
using namespace std;

#include "camera.hpp"

class AutoCamera : public Camera
{
protected:
    unsigned captureTime;
    
public:
    AutoCamera(string device, unsigned captureTime = 2);

    virtual ~AutoCamera();

    virtual bool isEmitterWorking() override;

    AutoCamera &operator=(const AutoCamera &) = delete;

    AutoCamera(const AutoCamera &) = delete;
};

#endif