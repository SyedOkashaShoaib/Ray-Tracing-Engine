#ifndef SPHERE_H
#define SPHERE_H
#include "hittable.h"

class Sphere: Hittable {
public:
    Sphere(const Point3& sphere_center, const double radius ): sphere_center(sphere_center), radius(std::fmax(0, radius)){};
    bool Hit(const Ray& r, double r_tmin, double r_tmax, Hit_Record& rec ) const override {
   /*
    if the spheres center is not at the origin, then its eq becomes: 
    (Cx−x)^2+(Cy−y)^2+(Cz−z)^2=r^2 (Cx, ..) is the center of the sphere.
    here (x,y,z) belongs to ray r. therefore
    (C - P). (C - P) = r^2 
    if r hits the sphere at any point, then there is a value of t that satisfies the above equation.
    => (C - (Q + t.d)).(C - (Q + t.d)) = r^2
    solving the equation gives us: 
    t^2d⋅d−2td⋅(C−Q)+(C−Q)⋅(C−Q)−r^2=0
    This is a quadratic equation. we can find t using the quadratic formula. 
    if b-4ac is negative -> the ray is not hitting the sphere. 
    if b-4ac is zero, the ray is a tangent
    else the ray intersects the sphere at 2 ponts
    */
        Vec3 oc = sphere_center - r.origin(); //(C - P)
        auto a = dot(r.direction(), r.direction());
        auto h = dot( r.direction(), oc);
    /*
    whats h?? its its an optimization. assume b = -2h. -2h = -2d.(C - Q) */
        auto c = oc.length_squared() - radius*radius;
        auto discriminant = h*h - (a*c);
        if(discriminant < 0.0) {
            return false;
        }
        auto sqrtd = std::sqrt(discriminant);
        auto root = (h - sqrtd) / a;
        if (root <= r_tmin || root >= r_tmax) {
            root = (h + sqrtd) / a;
            if (root <= r_tmin || root >= r_tmax) {
                return false;
            }
        }

        rec.t = root; 
        rec.p = r.at(rec.t);
        Vec3 outward_normal= (rec.p - sphere_center) / radius;
        rec.set_face_normal(r, outward_normal);

        return true;
    }
private:
    Point3 sphere_center;
    double radius;
};
#endif