#include <iostream>
#include "utilities.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
int main() {
    // Hittable_List world;
    // Camera cam;
    // auto material_ground = std::make_shared<Metal>(Color(0.8, 0.8, 0.8), 0.2);
    // auto material_center = std::make_shared<Lambertian>(Color(0.1, 0.2, 0.5));
    // auto material_left = std::make_shared<Dielectric>(1.5);
    // auto material_right  = std::make_shared<Metal>(Color(0.8, 0.6, 0.2), 0.1);
    // world.add(std::make_shared<Sphere>(Point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    // world.add(std::make_shared<Sphere>(Point3( 0.0, 0.0, -1.2),  0.5, material_center));
    // world.add(std::make_shared<Sphere>(Point3(-1.0, 0.0, -1.0),  0.5, material_left));
    // world.add(std::make_shared<Sphere>(Point3( 1.0, 0.0, -1.0),  0.5, material_right));
    // cam.aspect_ratio = 16.0/ 9.0;
    // cam.image_width = 400;
    // cam.samples_per_pixel = 100;
    // cam.max_depth = 50;
    // cam.vertical_fov = 60;
    // cam.look_from = Point3(-2,0,0.5);
    // cam.look_at = Point3(0.0, 0.0, -1.2);
    // cam.v_up = Vec3(0,1,0);
    // cam.defocus_angle = 5.0;
    // cam.focus_dist = 2.625;
    // cam.renderImage(world); 

        Hittable_List world;

    auto ground_material = std::make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
    world.add(std::make_shared<Sphere>(Point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            Point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - Point3(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<Material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = random_vector() * random_vector();
                    sphere_material = std::make_shared<Lambertian>(albedo);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = random_vector(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = std::make_shared<Metal>(albedo, fuzz);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = std::make_shared<Dielectric>(1.5);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = std::make_shared<Dielectric>(1.5);
    world.add(std::make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
    world.add(std::make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
    world.add(std::make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

    Camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1200;
    cam.samples_per_pixel = 500;
    cam.max_depth         = 50;

    cam.vertical_fov     = 20;
    cam.look_from = Point3(13,2,3);
    cam.look_at   = Point3(0,0,0);
    cam.v_up      = Vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    cam.renderImage(world);

}