#ifndef EXHAUSTIVEFINDER_HPP
#define EXHAUSTIVEFINDER_HPP

#include <memory>
#include <vector>
using namespace std;

#include "../camera/camera.hpp"
#include "finder.hpp"

class ExhaustiveFinder: public Finder
{
protected:
    unique_ptr<vector<uint8_t>> getUnits() noexcept override;

public:
    ExhaustiveFinder() = delete;

    explicit ExhaustiveFinder(Camera &camera, unsigned emitters, unsigned negAnswerLimit, const string &excludedPath);

    ~ExhaustiveFinder() override = default;

    ExhaustiveFinder &operator=(const ExhaustiveFinder &) = delete;

    ExhaustiveFinder(const ExhaustiveFinder &) = delete;

    ExhaustiveFinder &operator=(ExhaustiveFinder &&other) = delete;

    ExhaustiveFinder(ExhaustiveFinder && other) = delete;
};

#endif