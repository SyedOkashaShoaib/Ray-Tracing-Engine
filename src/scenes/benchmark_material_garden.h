#pragma once

#include "../camera.h"
#include "../sphere.h"
#include "../bvh.h"
#include "../material.h"
#include "../texture.h"

inline void benchmark_material_garden(int image_width = 400, int spp = 32)
{
    Hittable_List world;

    // ---------------------------------------------------------
    // Helpers
    // ---------------------------------------------------------

    auto add = [&](const Point3 &center,
                   double radius,
                   std::shared_ptr<Material> material)
    {
        world.add(std::make_shared<Sphere>(
            center, radius, material));
    };

    // The ground is a sphere centered at (0, -1000, 0).
    // This computes its actual surface height at (x, z).
    auto add_grounded = [&](double x,
                            double z,
                            double radius,
                            std::shared_ptr<Material> material)
    {
        double ground_y =
            -1000.0 + std::sqrt(1000000.0 - x * x - z * z);

        add(Point3(x, ground_y + radius, z),
            radius, material);
    };

    // ---------------------------------------------------------
    // Materials and textures
    // ---------------------------------------------------------

    auto ground = std::make_shared<Lambertian>(
        Color(0.35, 0.025, 0.035));
    // auto ground = std::make_shared<Lambertian>(
    //     Color(1.00, 0.898, 0.706));

    auto glass = std::make_shared<Dielectric>(1.5);

    auto silver = std::make_shared<Metal>(
        Color(0.87, 0.88, 0.93), 0.03);

    auto gold = std::make_shared<Metal>(
        Color(0.85, 0.60, 0.25), 0.07);

    auto dark_metal = std::make_shared<Metal>(
        Color(0.47, 0.52, 0.58), 0.15);

    auto teal = std::make_shared<Lambertian>(
        Color(0.06, 0.55, 0.58));

    auto ivory = std::make_shared<Lambertian>(
        Color(0.78, 0.67, 0.50));

    auto olive = std::make_shared<Lambertian>(
        Color(0.16, 0.31, 0.25));

    auto violet = std::make_shared<Lambertian>(
        Color(0.43, 0.29, 0.61));

    auto blue = std::make_shared<Lambertian>(
        Color(0.10, 0.26, 0.50));

    // Uses your existing marble-style Noise_Texture.
    auto marble_texture = std::make_shared<Noise_Texture>(5);
    auto marble = std::make_shared<Lambertian>(marble_texture);

    // Existing Earth image.
    auto earth_texture = std::make_shared<Image_Texture>(
        "src/images/input/globe.png");
    auto earth = std::make_shared<Lambertian>(earth_texture);

    auto checker_texture = std::make_shared<Checkered_Texture>(
        0.35,
        Color(0.14, 0.16, 0.20),
        Color(0.70, 0.66, 0.58));
    auto checker = std::make_shared<Lambertian>(checker_texture);

    // ---------------------------------------------------------
    // Ground: 1 sphere
    // ---------------------------------------------------------

    add(Point3(0, -1000, 0), 1000, ground);

    // ---------------------------------------------------------
    // Primary lights: 2 spheres
    // ---------------------------------------------------------

    auto warm_light = std::make_shared<Diffuse_Light>(
        Color(3.2, 2.8, 2.4));

    auto cool_light = std::make_shared<Diffuse_Light>(
        Color(1.6, 2.0, 2.7));

    add(Point3(-1.0, 6.6, -1.2), 1.35, warm_light); // L1
    add(Point3(5.5, 5.6, -3.5), 1.10, cool_light);  // L2

    // ---------------------------------------------------------
    // Accent lights: 4 spheres
    // ---------------------------------------------------------

    auto amber = std::make_shared<Diffuse_Light>(
        Color(1.3, 0.75, 0.5));

    auto ice = std::make_shared<Diffuse_Light>(
        Color(0.55, 1.1, 1.5));

    auto purple_light = std::make_shared<Diffuse_Light>(
        Color(1.3, 0.85, 1.5));

    auto golden_light = std::make_shared<Diffuse_Light>(
        Color(1.45, 1.35, 0.7));

    add(Point3(-7.3, 3.4, -4.5), 0.45, amber);       // L3
    add(Point3(7.6, 3.0, 3.5), 0.40, ice);           // L4
    add(Point3(-5.4, 5.3, 4.2), 0.35, purple_light); // L5
    add(Point3(2.4, 4.6, 6.6), 0.30, golden_light);  // L6

    // ---------------------------------------------------------
    // Hero spheres: 5
    // ---------------------------------------------------------

    add_grounded(-4.0, 2.0, 1.25, glass); // H1 Glass
    add_grounded(4.0, 2.2, 1.25, silver); // H2 Silver

    add(Point3(-0.5, 1.35, 3.3), 1.12, teal);     // H3
    add(Point3(-2.25, 2.45, -2.5), 1.10, marble); // H4
    add(Point3(2.65, 2.15, -3.3), 1.10, earth);   // H5

    // ---------------------------------------------------------
    // Grounded secondary spheres: 12
    // ---------------------------------------------------------

    add_grounded(-7.4, 3.0, 0.75, teal);    // G1
    add_grounded(-8.1, -0.3, 0.72, silver); // G2
    add_grounded(-7.0, -3.7, 0.65, glass);  // G3
    add_grounded(-5.6, -6.2, 0.80, marble); // G4

    add_grounded(-2.7, -7.0, 0.74, ivory);     // G5
    add_grounded(0.1, -7.3, 0.62, gold);       // G6
    add_grounded(3.1, -7.2, 0.72, olive);      // G7
    add_grounded(6.2, -6.4, 0.76, dark_metal); // G8

    add_grounded(8.1, -3.5, 0.66, glass);  // G9
    add_grounded(8.3, 0.3, 0.78, checker); // G10
    add_grounded(7.4, 6.5, 0.65, blue);    // G11
    add_grounded(-7.4, 6.8, 0.60, marble); // G12

    // ---------------------------------------------------------
    // Floating secondary spheres: 8
    // ---------------------------------------------------------

    add(Point3(-6.0, 3.1, -1.1), 0.67, glass); // F1
    add(Point3(-5.3, 3.2, 5.0), 0.67, silver); // F2

    add(Point3(-0.8, 3.0, -6.3), 0.60, ivory); // F3
    add(Point3(1.2, 3.4, -5.4), 0.68, marble); // F4

    add(Point3(6.1, 3.2, -0.9), 0.65, glass); // F5
    add(Point3(5.7, 3.5, 5.8), 0.65, gold);   // F6

    add(Point3(0.7, 4.5, 5.8), 0.62, violet); // F7
    add(Point3(-3.1, 3.6, 0.5), 0.62, earth); // F8

    // ---------------------------------------------------------
    // Small detail spheres: 4
    // ---------------------------------------------------------

    add_grounded(-1.8, 6.3, 0.38, checker); // S1
    add_grounded(2.5, 6.6, 0.36, marble);   // S2

    add(Point3(-6.9, 1.8, 1.4), 0.35, dark_metal); // S3
    add(Point3(6.8, 2.0, -5.1), 0.35, olive);      // S4

    // ---------------------------------------------------------
    // Background sphere garden: 63 spheres
    // ---------------------------------------------------------

    for (int row = 0; row < 7; row++)
    {
        double z = -10.5 - row * 2.6;

        for (int col = 0; col < 9; col++)
        {

            // Alternating row offsets create staggered spacing.
            double x = (col - 4) * 2.7 + (row % 2 == 0 ? 0.0 : 0.45);

            // Deterministic variation in sphere sizes.
            double radius =
                0.40 + 0.11 * ((3 * col + 2 * row) % 5);

            // Select from our existing materials.
            std::shared_ptr<Material> material;

            switch ((col + 2 * row) % 8)
            {
            case 0:
                material = glass;
                break;
            case 1:
                material = silver;
                break;
            case 2:
                material = teal;
                break;
            case 3:
                material = marble;
                break;
            case 4:
                material = gold;
                break;
            case 5:
                material = checker;
                break;
            case 6:
                material = ivory;
                break;
            default:
                material = dark_metal;
                break;
            }

            // Some spheres float above the ground.
            if ((col + 3 * row) % 5 == 0)
            {
                double y = 2.0 + 0.35 * ((col + row) % 4);

                add(Point3(x, y, z), radius, material);
            }
            else
            {
                add_grounded(x, z, radius, material);
            }
        }
    }

    // ---------------------------------------------------------
    // Background lighting: 2 additional spheres
    // ---------------------------------------------------------

    auto rear_warm_light = std::make_shared<Diffuse_Light>(
        Color(1.1, 0.65, 0.35));

    auto rear_cool_light = std::make_shared<Diffuse_Light>(
        Color(0.45, 0.85, 1.25));

    add(
        Point3(-7.2, 5.7, -15.0),
        0.85,
        rear_warm_light);

    add(
        Point3(7.4, 5.3, -18.5),
        0.75,
        rear_cool_light);
    // ---------------------------------------------------------
    // v1.2 balancing patch: left-side fill
    // ---------------------------------------------------------

    // Only add this material if you have not already declared it.
    auto soft_left_light = std::make_shared<Diffuse_Light>(
        Color(0.75, 0.55, 0.95));

    // Left-side balancing spheres
    add(Point3(-8.6, 2.8, -1.5), 0.72, glass); // LP1
    add_grounded(-9.2, 1.8, 0.78, marble);     // LP2
    add_grounded(-8.0, -2.0, 0.70, silver);    // LP3

    // Optional subtle accent light
    add(Point3(-9.5, 4.2, 0.8), 0.42, soft_left_light); // LP4
    // ---------------------------------------------------------
    // Acceleration structure
    // ---------------------------------------------------------

    world = Hittable_List(
        std::make_shared<BVH_Node>(world));

    // ---------------------------------------------------------
    // Camera
    // ---------------------------------------------------------

    Camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = image_width;
    cam.samples_per_pixel = spp;
    cam.max_depth = 30;

    cam.background_color = Color(0, 0, 0);

    cam.vertical_fov = 35;
    cam.look_from = Point3(13, 9, 22);
    cam.look_at = Point3(0, 2.3, 0);
    cam.v_up = Vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.renderImage(world);
}