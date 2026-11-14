#include <iostream>
#include "utilities.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"

int main() {
    Hittable_List world;
    Camera cam;
    world.add(std::make_shared<Sphere>(Point3(-0.6,0,-1), 0.5));
    world.add(std::make_shared<Sphere>(Point3(0.6,0,-1), 0.5));
    world.add(std::make_shared<Sphere>(Point3(0,-100.5,-1), 100));
    cam.aspect_ratio = 16.0/ 9.0;
    cam.image_width = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth = 50;
    cam.renderImage(world); 

}