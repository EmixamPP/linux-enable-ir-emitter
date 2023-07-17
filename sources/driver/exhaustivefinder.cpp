#include "exhaustivefinder.hpp"

#include <vector>
#include <string>
#include <cstdint>
using namespace std;

#include "finder.hpp"
#include "../camera/camera.hpp"

vector<uint8_t> *ExhaustiveFinder::getUnits(const Camera &) noexcept
{
    auto *units = new vector<uint8_t>();
    for (unsigned unit = 0; unit < 256; ++unit)
        units->push_back(static_cast<uint8_t>(unit));
    return units;
}

ExhaustiveFinder::ExhaustiveFinder(Camera &camera, unsigned emitters, unsigned negAnswerLimit, string excludedPath)
    : Finder(camera, emitters, negAnswerLimit, excludedPath) {}

ExhaustiveFinder::~ExhaustiveFinder() {}