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
    cam.background_color = Color(0.7,0.8, 1);
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
    cam.background_color = Color(0.7,0.8, 1);
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
    cam.background_color = Color(0.7,0.8, 1);
    cam.vertical_fov = 35;

    cam.look_from = Point3(13, 4, 8);
    cam.look_at = Point3(2, 1.5, 0);

    cam.v_up = Vec3(0, 1, 0);
    cam.defocus_angle = 0;

    cam.renderImage(world);
}

void simple_light() {
    Hittable_List world;

    // Procedural texture shared by the ground and sphere
    auto pertext = std::make_shared<Noise_Texture>(5);

    // Ground sphere
    world.add(std::make_shared<Sphere>(
        Point3(0, -1000, 0),
        1000,
        std::make_shared<Lambertian>(pertext)
    ));

    // Smaller sphere
    world.add(std::make_shared<Sphere>(
        Point3(0, 2, 0),
        2,
        std::make_shared<Lambertian>(pertext)
    ));

    // Light-emitting material
    auto difflight = std::make_shared<Diffuse_Light>(
        Color(4, 4, 4)
    );

    // Spherical light source
    world.add(std::make_shared<Sphere>(
        Point3(0, 7, 0),
        2,
        difflight
    ));

    Camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 400;
    cam.samples_per_pixel = 1000;
    cam.max_depth = 50;

    cam.background_color = Color(0, 0, 0);

    cam.vertical_fov = 45;
    cam.look_from = Point3(13, 4, 8);
    cam.look_at = Point3(0, 3.5, 0);
    cam.v_up = Vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.renderImage(world);
}

void simple_light_2() {
    Hittable_List world;

    // Procedural texture shared by the ground and sphere
    auto pertext = std::make_shared<Noise_Texture>(5);

    // Ground sphere
    world.add(std::make_shared<Sphere>(
        Point3(0, -1000, 0),
        1000,
        std::make_shared<Lambertian>(pertext)
    ));

    // Smaller sphere
    world.add(std::make_shared<Sphere>(
        Point3(0, 2, 0),
        2,
        std::make_shared<Lambertian>(pertext)
    ));

    // Light-emitting material
    auto difflight = std::make_shared<Diffuse_Light>(
        Color(4, 4, 4)
    );

    // Spherical light source
    world.add(std::make_shared<Sphere>(
        Point3(5, 3, 0),
        1,
        difflight
    ));

    Camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 400;
    cam.samples_per_pixel = 1000;
    cam.max_depth = 50;

    cam.background_color = Color(0, 0, 0);

    cam.vertical_fov = 45;
    cam.look_from = Point3(13, 4, 8);
    cam.look_at = Point3(0, 3.5, 0);
    cam.v_up = Vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.renderImage(world);
}
void orbital_showcase() {
    Hittable_List world;

    // --- Ground: blood-red ---
    auto blood_red = std::make_shared<Lambertian>(
        Color(0.45, 0.03, 0.04)
    );

    world.add(std::make_shared<Sphere>(
        Point3(0, -1000, 0),
        1000,
        blood_red
    ));

    // --- Center light sphere ---
    auto center_light = std::make_shared<Diffuse_Light>(
        Color(5, 5, 5)
    );

    world.add(std::make_shared<Sphere>(
        Point3(0, 3, 0),
        1.2,
        center_light
    ));

    // --- Shared settings for orbiting spheres ---
    double orbit_y = 3.0;
    double orbit_radius = 4.5;
    double sphere_radius = 1.0;

    // --- Materials for orbiting spheres ---
    auto glass_mat = std::make_shared<Dielectric>(1.5);

    auto lambertian_mat = std::make_shared<Lambertian>(
        Color(0.2, 0.7, 0.9)
    );

    auto metal_mat = std::make_shared<Metal>(
        Color(0.8, 0.8, 0.85), 0.05
    );

    auto marble_tex = std::make_shared<Noise_Texture>(5);
    auto marble_mat = std::make_shared<Lambertian>(marble_tex);

    // --- Orbiting spheres ---
    // Right of center: Glass
    world.add(std::make_shared<Sphere>(
        Point3( orbit_radius, orbit_y, 0),
        sphere_radius,
        glass_mat
    ));

    // Front: Lambertian
    world.add(std::make_shared<Sphere>(
        Point3(0, orbit_y, orbit_radius),
        sphere_radius,
        lambertian_mat
    ));

    // Left: Metal
    world.add(std::make_shared<Sphere>(
        Point3(-orbit_radius, orbit_y, 0),
        sphere_radius,
        metal_mat
    ));

    // Back: Marble
    world.add(std::make_shared<Sphere>(
        Point3(0, orbit_y, -orbit_radius),
        sphere_radius,
        marble_mat
    ));

    // Optional BVH
    world = Hittable_List(std::make_shared<BVH_Node>(world));

    Camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 600;
    cam.samples_per_pixel = 1000;
    cam.max_depth = 50;

    // Dark background so the light stands out
    cam.background_color = Color(0, 0, 0);

    // Slightly elevated viewing angle
    cam.vertical_fov = 30;
    cam.look_from = Point3(10, 7, 12);
    cam.look_at   = Point3(0, 2.8, 0);
    cam.v_up      = Vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.renderImage(world);
}
int main() {

    // checkered_spheres();
    // earth();
    // perlin_spheres();
    // simple_light_2();
    orbital_showcase();

}