#include <iostream>
#include <array>

namespace helper {
    template<size_t len>
    void std_print_sensor_values(const std::array<uint16_t, len>& values) {
        for (size_t i = 0; i < len; ++i) {
            std::cout << values[i] << ",\t";
        }
        std::cout << std::endl;
    }
}