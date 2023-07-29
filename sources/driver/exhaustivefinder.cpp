#include "exhaustivefinder.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
using namespace std;

#include "finder.hpp"
#include "../camera/camera.hpp"

unique_ptr<vector<uint8_t>> ExhaustiveFinder::getUnits() noexcept
{
    auto units = make_unique<vector<uint8_t>>();
    for (unsigned unit = 0; unit < 256; ++unit)
        units->push_back(static_cast<uint8_t>(unit));
    return units;
}

ExhaustiveFinder::ExhaustiveFinder(Camera &camera, unsigned emitters, unsigned negAnswerLimit, const string &excludedPath)
    : Finder(camera, emitters, negAnswerLimit, excludedPath) {}
