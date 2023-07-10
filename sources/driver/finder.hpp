#ifndef FINDER_HPP
#define FINDER_HPP

#include <vector>
#include <cstdint>
#include <string>
using namespace std;

#include "camera.hpp"
#include "driver.hpp"

class Finder
{
protected:
    Camera &camera;
    const vector<uint8_t> *units;
    const unsigned emitters;
    const unsigned negAnswerLimit;
    const string excludedPath;
    const vector<pair<uint8_t, uint8_t>> *excluded;

    static string *shellExec(string cmd) noexcept;

    static vector<uint8_t> *getUnits(const Camera &camera) noexcept;

    virtual Driver *createDriverFromInstruction(const CameraInstruction &instruction, uint8_t unit, uint8_t selector) const noexcept;

    virtual vector<pair<uint8_t, uint8_t>> *getExcluded() noexcept;

    virtual bool isExcluded(uint8_t unit, uint8_t selector) const noexcept;

    virtual void addToExclusion(uint8_t unit, uint8_t selector) noexcept;

public:
    Finder(Camera &camera, unsigned emitters, unsigned negAnswerLimit, string excludedPath) noexcept;

    virtual ~Finder();

    Finder &operator=(const Finder &) = delete;

    Finder(const Finder &) = delete;

    virtual Driver **find();
};

#endif