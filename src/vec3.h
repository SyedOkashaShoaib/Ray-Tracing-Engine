#ifndef VEC3_H
#define VEC3_H

#include "utilities.h"

class Vec3
{
public:
    double comp[3];
    Vec3() : comp{0, 0, 0} {};
    Vec3(double x, double y, double z) : comp{x, y, z} {};

    double x() const { return comp[0]; }
    double y() const { return comp[1]; }
    double z() const { return comp[2]; }

    Vec3 operator-() const {
        return Vec3(-comp[0], -comp[1], -comp[2]);
    }

    Vec3 &operator+=(const Vec3 &v) {
        comp[0] += v.comp[0];
        comp[1] += v.comp[1];
        comp[2] += v.comp[2];

        return *this;
    }

    Vec3 &operator*=(double scalar) {
        comp[0] *= scalar;
        comp[1] *= scalar;
        comp[2] *= scalar;

        return *this;
    }

    Vec3& operator/=(double scalar) {
        return *this*=1/scalar; //cheeky.
    }

    double length() const {
        return std::sqrt(length_squared());
    }
    double length_squared() const {
        return comp[0] * comp[0] + comp[1] * comp[1] + comp[2] * comp[2];
    }


};
using Point3 = Vec3; //type alias. We would be using vec3 class for points as well

inline std::ostream& operator<<(std::ostream& out, const Vec3 v) {
    return out << v.comp[0] << " " << v.comp[1] << " " << v.comp[2];
}
/* 
so we can write std::cout << vector
out is equivalent to cout; An obj used to write on ostream. we have not added const cause writing on stream changes its
state. Why are we returing ostream and not void? For chaining (std::cout << V) << '\n';
*/
inline Vec3 operator+(const Vec3& u, const Vec3& v) {
    return Vec3(u.comp[0] + v.comp[0], u.comp[1] + v.comp[1], u.comp[2] + v.comp[2]);
}
inline Vec3 operator-(const Vec3& u, const Vec3& v) {
    return Vec3(u.comp[0] - v.comp[0], u.comp[1] - v.comp[1], u.comp[2] - v.comp[2]);
}

inline Vec3 operator*(const Vec3& u, const Vec3& v) {
    return Vec3(u.comp[0] * v.comp[0], u.comp[1] * v.comp[1], u.comp[2] * v.comp[2]);
}

inline Vec3 operator*(double t, const Vec3& v) {
    return Vec3(t*v.comp[0], t*v.comp[1], t*v.comp[2]);
}

inline Vec3 operator*(const Vec3& v, double t) {
    return t * v;
}

inline Vec3 operator/(const Vec3& v, double t) {
    return (1/t) * v;
}

inline double dot(const Vec3 &v, const Vec3 &u) {
    return v.comp[0] * u.comp[0] + v.comp[1] * u.comp[1] + v.comp[2] * u.comp[2];  
}

inline Vec3 cross(const Vec3& u, const Vec3& v) {
    return Vec3(u.comp[1] * v.comp[2] - u.comp[2] * v.comp[1],
                u.comp[2] * v.comp[0] - u.comp[0] * v.comp[2],
                u.comp[0] * v.comp[1] - u.comp[1] * v.comp[0]);
}

inline Vec3 unit(const Vec3 &v) {
    return v/v.length();
}
inline Vec3 random_vector() {
        return Vec3(random_double(), random_double(), random_double());
    }
inline Vec3 random_vector(double min, double max){
        return Vec3(random_double(min, max), random_double(min, max), random_double(min, max));
}
inline Vec3 random_unit_vector() {
    while(true) {
        auto point = random_vector(-1,1);
        auto lensq = point.length_squared();
        if (lensq <= 1 && lensq > 1e-160) {
            return point/sqrt(lensq);
        }
    }
}
inline Vec3 random_vector_on_hemisphere(const Vec3& normal ) {
    auto on_unit_sphere = random_unit_vector();
    if (dot(normal, on_unit_sphere) > 0.0) {
        return on_unit_sphere;
    }
    return -on_unit_sphere;
}
#endif
