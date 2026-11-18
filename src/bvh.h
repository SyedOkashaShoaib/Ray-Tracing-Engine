#ifndef BVH_H
#define BVH_H

#include <vector>
#include <algorithm>
#include "hittable.h"
#include "hittable_list.h"
#include "aabb.h"

class BVH_Node: public Hittable{
    /*a hierarchal tree of bboxes. */
public:
    BVH_Node(Hittable_List hittable_list) : BVH_Node(hittable_list.objects, 0, hittable_list.objects.size()) {}
    BVH_Node(std::vector<std::shared_ptr<Hittable>>& objects, size_t start, size_t end ) {
        auto axis = random_int(0,2); 
        auto comparator = (axis == 0) ? box_x_compare : (axis == 1) ? box_y_compare : box_z_compare;
        size_t object_span = end -start; 
        /*its the range of objs for which we are constructing the bvh */
        if (object_span == 1) {
            left = objects[start];
            right = objects[start];
            /*this is the base case. were duplicating the pointers because hit() expects both left and right to point to something
              we just dont want to deal with the handling logic for a missing child node*/
        }
        else if(object_span == 2) {
            left = objects[start];
            right = objects[start + 1];
            /*another bae case*/
        }
        else {
            std::sort(std::begin(objects) + start, std::begin(objects)+end, comparator); 
            /*sort the objects along the axis. For eg, if the chosen axis is x, then sort according to each objects x-coord*/
            auto mid = start + object_span / 2;
            left = std::make_shared<BVH_Node>(objects, start, mid); //constructing the left child, and its left children and so on
            right = std::make_shared<BVH_Node>(objects, mid, end);

            
        }
        bbox = Aabb(left->bounding_box(), right->bounding_box());
    }
    bool Hit(const Ray& r, Interval ray_t, Hit_Record& rec ) const override {
        if (!bbox.hit(r, ray_t)) {
            return false;
            /*check if the ray intersects the bbox. if it does not, then we can prune the calculation in which we check whether
            this bbox's child bboxes are intersected by ray r*/
        }
        bool hit_left = left->Hit(r, ray_t, rec);  
        bool hit_right = right->Hit(r, Interval(ray_t.min, (hit_left) ? rec.t : ray_t.max), rec);
        /*whats with the max arg of right->hit? 
        because we want the closest point of intersection.  if left object was hit at t= 5, and 
        right object was hit at t= 8, then right obj is not closest. Thererfore we want the right child 
        to search only upto t=5. anything above that is not useful. This is another aspect of optimization via bvh */
        return hit_left || hit_right;
    }

    Aabb bounding_box() const override { return bbox; }
private:
    std::shared_ptr<Hittable> left;
    std::shared_ptr<Hittable> right;
    Aabb bbox;
        static bool box_compare( const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b, int axis_index) {
        auto a_axis_interval = a->bounding_box().axis_interval(axis_index);
        auto b_axis_interval = b->bounding_box().axis_interval(axis_index);
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool box_x_compare (const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b) {
        return box_compare(a, b, 0);
    }

    static bool box_y_compare (const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b) {
        return box_compare(a, b, 1);
    }

    static bool box_z_compare (const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b) {
        return box_compare(a, b, 2);
    }

    
};

#endif