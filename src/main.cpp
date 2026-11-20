#include <iostream>
#include "utilities.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#include "texture.h"
void checkered_spheres() {
    Hittable_List world;

    auto checker = std::make_shared<Checkered_Texture>(
        0.32,
        Color(0.2, 0.3, 0.1),
        Color(0.9, 0.9, 0.9)
    );

    world.add(std::make_shared<Sphere>(
        Point3(0, -10, 0),
        10,
        std::make_shared<Lambertian>(checker)
    ));

    world.add(std::make_shared<Sphere>(
        Point3(0, 10, 0),
        10,
        std::make_shared<Lambertian>(checker)
    ));
    world = Hittable_List(std::make_shared<BVH_Node>(world));
    Camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth = 50;

    cam.vertical_fov = 20;
    cam.look_from = Point3(13, 2, 3);
    cam.look_at = Point3(0, 0, 0);
    cam.v_up = Vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.renderImage(world);
}

void earth() {
    auto earth_texture =
        std::make_shared<Image_Texture>("images/input/globe.png");

    auto earth_surface =
        std::make_shared<Lambertian>(earth_texture);

    auto globe = std::make_shared<Sphere>(
        Point3(0, 0, 0),
        2.0,
        earth_surface
    );

    Camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth = 50;

    cam.vertical_fov = 20;
    cam.look_from = Point3(0, 0, 12);
    cam.look_at = Point3(0, 0, 0);
    cam.v_up = Vec3(0, 1, 0);
    cam.defocus_angle = 0;

    Hittable_List world;
    world.add(globe);

    cam.renderImage(world);
}

void perlin_spheres() {
    Hittable_List world;

    auto pertext = std::make_shared<Noise_Texture>(5);

    world.add(std::make_shared<Sphere>(
        Point3(0, -1000, 0),
        1000,
        std::make_shared<Lambertian>(pertext)
    ));

    world.add(std::make_shared<Sphere>(
        Point3(2, 2, 0),
        2,
        std::make_shared<Lambertian>(pertext)
    ));

    Camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth = 50;

    cam.vertical_fov = 35;

    cam.look_from = Point3(13, 4, 8);
    cam.look_at = Point3(2, 1.5, 0);

    cam.v_up = Vec3(0, 1, 0);
    cam.defocus_angle = 0;

    cam.renderImage(world);
}

int main() {

    // checkered_spheres();
    // earth();
    perlin_spheres();

}