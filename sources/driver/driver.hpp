#ifndef DRIVER_HPP
#define DRIVER_HPP

#include <cstdint>
using namespace std;

struct Driver
{
    char device[128];
    uint8_t unit;
    uint8_t selector;
    uint16_t size;
    uint8_t *control;
};

void writeDriver(const char *driverFile, const Driver *driver);

Driver *readDriver(const char *driverFile);

#endif
