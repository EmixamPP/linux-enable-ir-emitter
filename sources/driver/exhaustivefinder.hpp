#ifndef EXHAUSTIVEFINDER_HPP
#define EXHAUSTIVEFINDER_HPP

#include <vector>
using namespace std;

#include "../camera/camera.hpp"
#include "finder.hpp"

class ExhaustiveFinder: public Finder
{
protected:
    virtual vector<uint8_t> *getUnits(const Camera &camera) noexcept override;

public:
    ExhaustiveFinder(Camera &camera, unsigned emitters, unsigned negAnswerLimit, string excludedPath);

    virtual ~ExhaustiveFinder();

    ExhaustiveFinder &operator=(const ExhaustiveFinder &) = delete;

    ExhaustiveFinder(const ExhaustiveFinder &) = delete;
};

#endif