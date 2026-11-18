#ifndef AABB_H
#define AABB_H
#include "utilities.h"

class Aabb {
public: 
    Interval x,y,z;
    Aabb() {}
    Aabb(const Interval& x, const Interval& y, const Interval& z ): x(x), y(y), z(z) {}
    Aabb(const Point3& a, const Point3& b) {
        x = (a.x() <= b.x()) ? Interval(a.x(), b.x()) : Interval(b.x(), a.x());
        y = (a.y() <= b.y()) ? Interval(a.y(), b.y()) : Interval(b.y(), a.y());
        z = (a.z() <= b.z()) ? Interval(a.z(), b.z()) : Interval(b.z(), a.z());
    }

    Aabb(const Aabb& box0, const Aabb& box1) {
        /*we need to be able to enclose boundary boxes inside larger bounder boxes to enforce hierarchy in bvh*/
        x = Interval(box0.x, box1.x);
        y = Interval(box0.y, box1.y);
        z = Interval(box0.z, box1.z);
    }
    const Interval& axis_interval (int n) const {
        if (n == 0 ) return x;
        if (n == 1) return y;
        return z;
    }
    bool hit(const Ray& r, Interval ray_t) const {
        const Point3& ray_origin = r.origin();
        const Vec3& ray_dir = r.direction();

        for (int axis=0; axis < 3; axis++) {
            const Interval& ax = axis_interval(axis);
            const auto ax_dir_inv = 1 / ray_dir[axis];
            auto t0 = (ax.min - ray_origin[axis]) * ax_dir_inv;
            auto t1 = (ax.max - ray_origin[axis]) * ax_dir_inv;

            if (t0 < t1) {
               if (t0 > ray_t.min) ray_t.min = t0;
                if (t1 < ray_t.max) ray_t.max = t1;
            } 
            else {
                if (t1 > ray_t.min) ray_t.min = t1;
                if (t0 < ray_t.max) ray_t.max = t0;
            }

            if (ray_t.max <= ray_t.min) return false;
        }
        return true; 
    }
    

};
#endif