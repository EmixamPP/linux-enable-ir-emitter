#ifndef FINDER_HPP
#define FINDER_HPP

#include <cstdint>
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
    vector<pair<uint8_t, uint8_t>> *excluded = nullptr;
    void initialize() noexcept;

protected:
    vector<uint8_t> *units = nullptr;

    static string *shellExec(const string &cmd) noexcept;

    virtual vector<uint8_t> *getUnits(const Camera &camera) noexcept;

    Driver *createDriverFromInstruction(const CameraInstruction &instruction, uint8_t unit, uint8_t selector) const noexcept;

    vector<pair<uint8_t, uint8_t>> *getExcluded() noexcept;

    bool isExcluded(uint8_t unit, uint8_t selector) const noexcept;

    void addToExclusion(uint8_t unit, uint8_t selector) noexcept;

public:
    Finder() = delete;

    explicit Finder(Camera &camera, unsigned emitters, unsigned negAnswerLimit, const string &excludedPath);

    virtual ~Finder();

    Finder &operator=(const Finder &) = delete;

    Finder(const Finder &) = delete;

    Finder &operator=(Finder &&other) = delete;

    Finder(Finder && other) = delete;

    Driver **find();
};

#endif