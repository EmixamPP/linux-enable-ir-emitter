#ifndef FINDER_HPP
#define FINDER_HPP

#include <vector>
using namespace std;

#include "camera/camera.hpp"
#include "camera/camerainstruction.hpp"

class Finder
{
private:
    shared_ptr<Camera> camera_;
    const unsigned emitters_;
    const unsigned neg_answer_limit_;

public:
    Finder() = delete;

    explicit Finder(shared_ptr<Camera> camera, unsigned emitters, unsigned neg_answer_limit_);

    ~Finder() = default;

    Finder &operator=(const Finder &) = delete;

    Finder(const Finder &) = delete;

    Finder &operator=(Finder &&other) = delete;

    Finder(Finder &&other) = delete;

    bool find(CameraInstructions &instructions);
};

#endif