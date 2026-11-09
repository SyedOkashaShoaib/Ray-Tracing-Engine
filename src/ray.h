#ifndef RAY_H
#define RAY_H

#include "vec3.h"


/*
 P = A + tB
 A -> Origin point of the ray. 
 B -> Length of the ray. 
 t -> Parameter controlling the position on the ray. 
    t = 0 -> P = A 
    t < 0  P is some point behind A. 
 */
class Ray {
public:
    Ray() {}
    Ray(const Vec3& direction, const Point3& origin) : orig(origin), dir(direction) {}

    const Point3& origin() { return orig; }
    const Vec3& direction() { return dir; }

    Point3 at(double t) {
        return orig + t * dir;
    }
private: 
    Point3 orig; //corresponds to A in the eq. 
    Vec3 dir; //B    
};
#endif