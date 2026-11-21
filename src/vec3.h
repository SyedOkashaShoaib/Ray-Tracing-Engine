#ifndef VEC3_H
#define VEC3_H

#include "core_utilities.h"
#include "rng.h"
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
    double operator[](int i) const {
        return comp[i];
    }

    double& operator[](int i) {
        return comp[i];
    }
    double length() const {
        return std::sqrt(length_squared());
    }
    double length_squared() const {
        return comp[0] * comp[0] + comp[1] * comp[1] + comp[2] * comp[2];
    }
    bool near_zero() const {
        float small_val = 1e-8;
        return (std::fabs(comp[0]) < small_val ) && (std::fabs(comp[1]) < small_val ) && (std::fabs(comp[2]) < small_val );
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
inline Vec3 reflect(Vec3 v, Vec3 n) {
    return v - 2*dot(v,n)*n; //look metal class for explanatoin
}
inline Vec3 refract(const Vec3& uv, const Vec3& n, double ref_ind1_over_ref_ind2) {
    /*for dielectric surfaces which refract (light bends when it enters a dielectric medium)
    using snells law: 
        refractive index of medium 1 x sin(angle with incident ray and normal) = ref_index_2 x sin(angle of refracted ray and normal)
        we break the refracted ray R into R_perp, and R_par so we can represent snells law in Vec3 form. idk how the equations are
        dervied.*/
    auto cos_theta = std::fmin(dot(-uv, n), 1.0);
    Vec3 r_out_perp =  ref_ind1_over_ref_ind2 * (uv + cos_theta*n);
    Vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

inline Vec3 random_on_unit_disk() {
    while( true) {
        auto p = Vec3(random_double(-1,1), random_double(-1,1), 0);
        if (p.length_squared() < 1) {
            return p;
        }
    }
}
inline Vec3 random_vector( RNG& gen, double min, double max) {
    return Vec3( gen.uniform(min, max), gen.uniform(min, max), gen.uniform(min, max));
    
}

inline Vec3 random_unit_vector(RNG& gen) {
    while (true) {
        auto point = random_vector(gen, -1, 1);
        auto lensq = point.length_squared();

        if (lensq <= 1 && lensq > 1e-160) {
            return point / std::sqrt(lensq);
        }
    }
}

inline Vec3 random_on_unit_disk(RNG& gen) {
    while (true) {
        auto p = Vec3(
            gen.uniform(-1, 1),
            gen.uniform(-1, 1),
            0
        );

        if (p.length_squared() < 1) {
            return p;
        }
    }
}


#endif
