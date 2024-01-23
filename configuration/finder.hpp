#ifndef FINDER_HPP
#define FINDER_HPP

#include <vector>
using namespace std;

#include "camera/camera.hpp"

class Finder
{
private:
    Camera &camera_;
    const unsigned emitters_;
    const unsigned neg_answer_limit_;

public:
    Finder() = delete;

    explicit Finder(Camera &camera, unsigned emitters, unsigned neg_answer_limit_);

    ~Finder() = default;

    Finder &operator=(const Finder &) = delete;

    Finder(const Finder &) = delete;

    Finder &operator=(Finder &&other) = delete;

    Finder(Finder &&other) = delete;

    bool find(vector<CameraInstruction> &intructions);
};

#endif