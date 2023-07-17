#ifndef MATH_HPP
#define MATH_HPP

#include <cstdint>
#include <numeric>
#include <vector>
using namespace std;

unsigned array_gcd(const vector<uint8_t> &arr) noexcept
{
    unsigned result = arr[0];
    for (unsigned i = 1; i < arr.size(); ++i)
    {
        result = gcd(arr[i], result);
        if (result == 1)
            return 1;
    }
    return result;
};

#endif