#ifndef AUTOCAMERA_HPP
#define AUTOCAMERA_HPP

#include <string>
using namespace std;

#include "camera.hpp"

class AutoCamera : public Camera
{
public:
    AutoCamera(string device);

    virtual ~AutoCamera();

    virtual bool isEmitterWorking() override;

    AutoCamera &operator=(const AutoCamera &) = delete;

    AutoCamera(const AutoCamera &) = delete;
};

#endif