#include "autocamera.hpp"

#include <string>
using namespace std;

bool AutoCamera::isEmitterWorking() {
    return false;
}

AutoCamera::AutoCamera(string device) : Camera(device) {}

AutoCamera::~AutoCamera() {}