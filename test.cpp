#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>

int main() {
    uint8_t arr1[2] = {1,10};
    uint16_t combinedValue = (static_cast<uint16_t>(arr1[1]) << 8) | arr1[0];
    std::cout << "The integer value is: " << static_cast<int>(combinedValue) << std::endl;

    uint8_t arr2[2] = {2, 100};
    combinedValue = (static_cast<uint16_t>(arr2[1]) << 8) | arr2[0];
    std::cout << "The integer value is: " << static_cast<int>(combinedValue) << std::endl;

    std::vector<uint8_t> result(2);

    uint8_t carry = 0;
    for (size_t i = 0; i < 2; ++i) {
        uint16_t sum = static_cast<uint16_t>(arr1[i]) + arr2[i] + carry;
        std::cout << sum << std::endl;
        result[i] = (static_cast<uint8_t>(sum));
        std::cout << (sum >> 8) << std::endl;
        carry = static_cast<uint8_t>(sum >> 8);
    }

    std::cout << (int) result[0] << " " << (int) result[1] << std::endl;
    return 0;
}
