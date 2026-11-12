#ifndef HITTABLE_H
#define HITTABLE_H
#include "ray.h"
class Hit_Record {
public:
    double t; 
    Vec3 normal;
    Point3 p;
    bool front_face;
    void set_face_normal(const Ray& r, const Vec3& outward_normal) {
        /*
        in this ray tracer engine implementation, we have decided that the normla vector will always point agaisnt
        the ray. if the ray hits from outside the sphere, the normal points
        outwards, and inwards if the ray hits the isnide of the sphere.
        if dot product of ray and nrrmal is positive, the normal points inwrads.  */
        front_face = dot(r.direction(), outward_normal) < 0.0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};
class Hittable {
public:
    virtual ~Hittable() = default; //destructor. =default means compiler will implement the destruction code.
    virtual bool Hit(const Ray& r, double r_tmin, double r_tmax, Hit_Record& rec ) const = 0;
    /*
    a hit only counts if  t_min < t < t_max. 
    every object implements its own version of this.  */

};
#endif