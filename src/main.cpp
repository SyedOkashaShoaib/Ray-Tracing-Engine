#include <iostream>
#include "utilities.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
int main() {
    Hittable_List world;
    Camera cam;
    auto material_ground = std::make_shared<Metal>(Color(0.8, 0.8, 0.8), 0.2);
    auto material_center = std::make_shared<Lambertian>(Color(0.1, 0.2, 0.5));
    auto material_left = std::make_shared<Dielectric>(1.5);
    auto material_right  = std::make_shared<Metal>(Color(0.8, 0.6, 0.2), 0.1);
    world.add(std::make_shared<Sphere>(Point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(std::make_shared<Sphere>(Point3( 0.0, 0.0, -1.2),  0.5, material_center));
    world.add(std::make_shared<Sphere>(Point3(-1.0, 0.0, -1.0),  0.5, material_left));
    world.add(std::make_shared<Sphere>(Point3( 1.0, 0.0, -1.0),  0.5, material_right));
    cam.aspect_ratio = 16.0/ 9.0;
    cam.image_width = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth = 50;
    cam.vertical_fov = 60;
    cam.look_from = Point3(-2,0,0.5);
    cam.look_at = Point3(0.0, 0.0, -1.2);
    cam.v_up = Vec3(0,1,0);
    cam.defocus_angle = 5.0;
    cam.focus_dist = 2.625;
    cam.renderImage(world); 

     world = Hittable_List(std::make_shared<BVH_Node>(world));



    cam.renderImage(world);

}