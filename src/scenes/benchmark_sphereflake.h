#pragma once
#include <chrono>
#include <iomanip>
#include <iostream>
#include "../bvh.h"
#include "../material.h"
#include "../sphere.h"
#include "../camera.h"
inline int add_sphereflake(
    Hittable_List &world,
    const Point3 &center,
    double radius,
    const Vec3 &outward,
    int remaining_levels,
    const std::shared_ptr<Material> &material)
{
    // Add the current sphere.
    world.add(std::make_shared<Sphere>(
        center, radius, material));

    int count = 1;

    // Base case: no more generations.
    if (remaining_levels == 0)
        return count;

    // Local coordinate frame around the outward direction.
    Vec3 n = unit(outward);

    Vec3 reference = (std::fabs(n.y()) > 0.9)
                         ? Vec3(1, 0, 0)
                         : Vec3(0, 1, 0);

    Vec3 tangent = unit(cross(reference, n));
    Vec3 bitangent = cross(n, tangent);

    double child_radius = radius / 3.0;
    double distance = radius + child_radius;

    constexpr double pi = 3.14159265358979323846;

    // Each sphere creates nine children.
    for (int i = 0; i < 9; i++)
    {
        Vec3 direction = n;

        if (i > 0)
        {
            double angle = 2.0 * pi * (i - 1) / 8.0;

            direction =
                0.5 * n + (std::sqrt(3.0) / 2.0) *
                              (std::cos(angle) * tangent + std::sin(angle) * bitangent);
        }

        Point3 child_center =
            center + distance * direction;

        count += add_sphereflake(
            world,
            child_center,
            child_radius,
            direction,
            remaining_levels - 1,
            material);
    }

    return count;
}

inline void benchmark_sphereflake(
    int levels = 2,
    int image_width = 400,
    int spp = 32)
{
    const auto setup_start = std::chrono::steady_clock::now();
    Hittable_List world;

    // --------------------------------------------------
    // Sphereflake material
    // --------------------------------------------------

    auto flake_material = std::make_shared<Metal>(
        Color(0.85, 0.78, 0.67),
        0.08);

    // --------------------------------------------------
    // Generate the sphereflake
    // --------------------------------------------------

    int sphere_count = add_sphereflake(
        world,
        Point3(0, 4, 0),
        2.0,
        Vec3(0, 1, 0),
        levels,
        flake_material);

    std::cerr << "Sphereflake spheres: "
              << sphere_count << '\n';

    // --------------------------------------------------
    // Ground
    // --------------------------------------------------

    auto ground_material = std::make_shared<Lambertian>(
        Color(0.12, 0.13, 0.19));

    world.add(std::make_shared<Sphere>(
        Point3(0, -1000, 0),
        1000,
        ground_material));

    // --------------------------------------------------
    // Warm spherical lighting
    // --------------------------------------------------

    auto warm_light = std::make_shared<Diffuse_Light>(
        Color(3.2, 0.85, 0.25));

    auto secondary_light = std::make_shared<Diffuse_Light>(
        Color(2.4, 0.75, 0.30));

    // Upper-left light: outside camera frame
    world.add(std::make_shared<Sphere>(
        Point3(-5.0, 12.0, 0.0),
        1.45,
        warm_light));

    // Upper-right light: outside camera frame
    world.add(std::make_shared<Sphere>(
        Point3(5.0, 12.0, -2.0),
        1.15,
        secondary_light));

    // --------------------------------------------------
    // BVH
    // --------------------------------------------------

    world = Hittable_List(
        std::make_shared<BVH_Node>(world));

    // --------------------------------------------------
    // Camera
    // --------------------------------------------------

    Camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = image_width;
    cam.samples_per_pixel = spp;
    cam.max_depth = 30;

    cam.background_color = Color(0.12, 0.10, 0.09);

    cam.look_from = Point3(8, 15, 13);
    cam.look_at = Point3(0, 5.6, 0);
    cam.vertical_fov = 27;
    cam.v_up = Vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.defocus_angle = 0;

    // End of scene setup, including BVH and camera configuration.
    const double setup_seconds =
        std::chrono::duration<double>(
            std::chrono::steady_clock::now() - setup_start)
            .count();

    // Render the scene.
    cam.renderImage(world);

// Report scene setup timing.
#if defined(RT_USE_MPI)

    double max_setup_seconds = 0.0;

    MPI_Reduce(
        &setup_seconds,
        &max_setup_seconds,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        0,
        MPI_COMM_WORLD);

    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        std::clog << std::fixed << std::setprecision(9)
                  << "RT_TIMING component=scene"
                  << " scene=B"
                  << " setup_s=" << max_setup_seconds
                  << " aggregate=rank_max"
                  << '\n';
    }

#else

    std::clog << std::fixed << std::setprecision(9)
              << "RT_TIMING component=scene"
              << " scene=B"
              << " setup_s=" << setup_seconds
              << " aggregate=single_process"
              << '\n';

#endif
}