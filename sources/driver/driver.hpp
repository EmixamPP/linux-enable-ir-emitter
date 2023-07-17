#ifndef DRIVER_HPP
#define DRIVER_HPP

#include <cstdint>
#include <string>
#include <vector>
using namespace std;

class Driver
{
public:
    string device;
    uint8_t unit;
    uint8_t selector;
    vector<uint8_t> control;

    Driver(string device, uint8_t unit, uint8_t selector, const vector<uint8_t> &control);

    ~Driver();

    Driver &operator=(const Driver &) = delete;
    Driver(const Driver &) = delete;
};

void writeDriver(string driverFile, const Driver *driver);

Driver *readDriver(string driverFile);

#endif
