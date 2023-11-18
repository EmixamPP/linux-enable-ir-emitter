#ifndef FINDER_HPP
#define FINDER_HPP

#include <cstdint>
#include <string>
#include <vector>
using namespace std;

#include "camera/camera.hpp"

class Finder
{
private:
    Camera &camera;
    const unsigned emitters;
    const unsigned negAnswerLimit;
    vector<CameraInstruction> &intructions;

public:
    Finder() = delete;

    explicit Finder(Camera &camera, unsigned emitters, unsigned negAnswerLimit, vector<CameraInstruction> &intructions);

    ~Finder() = default;

    Finder &operator=(const Finder &) = delete;

    Finder(const Finder &) = delete;

    Finder &operator=(Finder &&other) = delete;

    Finder(Finder &&other) = delete;

    vector<CameraInstruction> find();
};

#endif