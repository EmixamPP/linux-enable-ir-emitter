#ifndef FINDER_HPP
#define FINDER_HPP

#include <vector>
#include <cstdint>
#include <string>
using namespace std;

#include "../camera/camera.hpp"
#include "driver.hpp"

class Finder
{
protected:
    Camera &camera;
    vector<uint8_t> *units = nullptr;
    const unsigned emitters;
    const unsigned negAnswerLimit;
    const string excludedPath;
    vector<pair<uint8_t, uint8_t>> *excluded = nullptr;

    static string *shellExec(string cmd) noexcept;

    virtual vector<uint8_t> *getUnits(const Camera &camera) noexcept;

    virtual Driver *createDriverFromInstruction(const CameraInstruction &instruction, uint8_t unit, uint8_t selector) const noexcept;

    virtual vector<pair<uint8_t, uint8_t>> *getExcluded() noexcept;

    virtual bool isExcluded(uint8_t unit, uint8_t selector) const noexcept;

    virtual void addToExclusion(uint8_t unit, uint8_t selector) noexcept;

public:
    Finder(Camera &camera, unsigned emitters, unsigned negAnswerLimit, string excludedPath);

    virtual ~Finder();

    Finder &operator=(const Finder &) = delete;

    Finder(const Finder &) = delete;

    virtual Driver **find();
};

#endif