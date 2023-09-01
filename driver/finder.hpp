#ifndef FINDER_HPP
#define FINDER_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
using namespace std;

#include "../camera/camera.hpp"
#include "driver.hpp"

class Finder
{
private:
    Camera &camera;
    const unsigned emitters;
    const unsigned negAnswerLimit;
    const string excludedPath;
    unique_ptr<vector<pair<uint8_t, uint8_t>>> excluded = nullptr;
    vector<uint8_t> units;

protected:
    unique_ptr<Driver> createDriverFromInstruction(const CameraInstruction &instruction, uint8_t unit, uint8_t selector) const noexcept;

    unique_ptr<vector<pair<uint8_t, uint8_t>>> getExcluded() noexcept;

    bool isExcluded(uint8_t unit, uint8_t selector) const noexcept;

    void addToExclusion(uint8_t unit, uint8_t selector) noexcept;

public:
    Finder() = delete;

    explicit Finder(Camera &camera, unsigned emitters, unsigned negAnswerLimit, const string &excludedPath);

    ~Finder() = default;

    Finder &operator=(const Finder &) = delete;

    Finder(const Finder &) = delete;

    Finder &operator=(Finder &&other) = delete;

    Finder(Finder &&other) = delete;

    unique_ptr<vector<unique_ptr<Driver>>> find();
};

#endif