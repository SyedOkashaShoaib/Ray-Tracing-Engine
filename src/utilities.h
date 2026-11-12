#ifndef UTILITIES_H
#define UTILITIES_H
#include <iostream>
#include <cmath>
#include <memory>
#include <limits>
#include <numbers>

inline constexpr auto infinity = std::numeric_limits<double>::infinity();
inline constexpr double pi = std::numbers::pi_v<double>;


inline double degree_to_radian(double degree) {
    return degree * pi / 180.0;
}

#include "color.h"
#include "ray.h"
#include "vec3.h"
#endif