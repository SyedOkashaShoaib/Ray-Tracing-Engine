#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include <vector>
#include "hittable.h"
#include "utilities.h"

class Hittable_List: public Hittable {
public:
    std::vector<std::shared_ptr<Hittable>> objects;
    /*look up shared ptrs. essentially every time you assign a shared ptrs value to another ptr, 
    the reference count goes up, and vice versa. Frees you up from manual memory management to an extent*/
    Hittable_List() {}
    Hittable_List(std::shared_ptr<Hittable> object) { add(object);}
    void add(std::shared_ptr<Hittable> object) {
        objects.push_back(object);
        bbox = Aabb(bbox, object->bounding_box());
    }
    void clear() {
        objects.clear();
    }
    bool Hit(const Ray& r, Interval ray_t, Hit_Record& rec) const override {
        bool is_hitting_something_something = false; 
        double closest_hit_so_far = ray_t.max;
        Hit_Record temp_rec; 
        for (const auto& obj : objects) {
            if (obj->Hit(r, Interval(ray_t.min, closest_hit_so_far), temp_rec)) {
                is_hitting_something_something = true;
                closest_hit_so_far = temp_rec.t;
                /*
                By the end of the loop, rec contains info about the intersection of the closest object to the camera. 
                initialy closest_hit_so_far was tmax, then it shrunk to < tmax, and it got lesser and lesser. */
                rec = temp_rec;

            }
        }
        return is_hitting_something_something;
    }

    Aabb bounding_box() const override {
        return bbox;
    }
private:
    Aabb bbox;

};
#endif