#ifndef CORE_UTILITIES_H
#define CORE_UTILITIES_H

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <limits>
#include <numbers>

inline constexpr auto infinity = std::numeric_limits<double>::infinity();

inline constexpr double pi = std::numbers::pi_v<double>;

inline double degree_to_radian(double degree) {
    return degree * pi / 180.0;
}

inline double random_double() {
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

inline int random_int(int min, int max) {
    return int(random_double(min, max + 1));
}

#endif
