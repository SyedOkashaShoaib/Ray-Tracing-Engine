// Shared helpers for the showcase renders.
//
// The showcase programs reuse the engine exactly as it is in src/. They need
// two things that Camera keeps private: initialize() and
// renderRowsInitialized() (plus image_height). Instead of editing the engine,
// this header reads the engine headers with `private` temporarily defined as
// `public`. Every standard and system header the engine uses is included
// first, so the macro only ever touches the engine's own classes.
#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <numbers>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__SSE2__)
#include <emmintrin.h>
#endif
#if defined(RT_USE_MPI)
#include <mpi.h>
#endif
#if defined(_OPENMP)
#include <omp.h>
#endif

#define private public
#include "camera.h"
#include "sphere.h"
#include "bvh.h"
#include "material.h"
#include "texture.h"
#include "hittable_list.h"
#undef private

// ---------------------------------------------------------------------------
// Capturing the two benchmark scenes
// ---------------------------------------------------------------------------
// benchmark_material_garden() and benchmark_sphereflake() build their world
// and camera and then immediately call cam.renderImage(world). To reuse the
// exact same scenes, the scene headers are read with `Camera` renamed to
// CaptureCamera, whose renderImage() stores a copy of the camera and world
// instead of rendering.
namespace sc {
inline Camera captured_camera;
inline Hittable_List captured_world;
inline bool captured = false;
}  // namespace sc

class CaptureCamera : public Camera {
public:
    void renderImage(const Hittable_List& world) {
        sc::captured_camera = static_cast<const Camera&>(*this);
        sc::captured_world = world;
        sc::captured = true;
    }
};

#define Camera CaptureCamera
#include "scenes/benchmark_material_garden.h"
#include "scenes/benchmark_sphereflake.h"
#undef Camera

namespace sc {

struct Scene {
    Camera cam;
    Hittable_List world;
};

// Scene A ("A"), Scene B ("B"), exactly as benchmarked. Width and samples per
// pixel are set later, so the values passed here do not matter.
inline Scene benchmark_scene(const std::string& which, int sphereflake_levels = 4) {
    captured = false;
    if (which == "A") {
        benchmark_material_garden(64, 1);
    } else if (which == "B") {
        benchmark_sphereflake(sphereflake_levels, 64, 1);
    } else {
        throw std::runtime_error("Unknown benchmark scene: " + which);
    }
    if (!captured) {
        throw std::runtime_error("Scene did not reach renderImage()");
    }
    return {captured_camera, captured_world};
}

// ---------------------------------------------------------------------------
// Command-line arguments: --key value
// ---------------------------------------------------------------------------
class Args {
public:
    Args(int argc, char** argv) {
        for (int i = 1; i < argc; i++) {
            std::string key = argv[i];
            if (key.rfind("--", 0) != 0 || i + 1 >= argc) {
                throw std::runtime_error("Expected --key value, got: " + key);
            }
            values[key.substr(2)] = argv[++i];
        }
    }
    bool has(const std::string& k) const { return values.count(k) != 0; }
    std::string str(const std::string& k, const std::string& d) const {
        auto it = values.find(k);
        return it == values.end() ? d : it->second;
    }
    int integer(const std::string& k, int d) const { return has(k) ? std::stoi(values.at(k)) : d; }
    double real(const std::string& k, double d) const { return has(k) ? std::stod(values.at(k)) : d; }

private:
    std::map<std::string, std::string> values;
};

// Applies --width, --spp and --depth (when given) on top of the scene's camera.
inline void apply_render_settings(Camera& cam, const Args& a) {
    cam.image_width = a.integer("width", cam.image_width);
    cam.samples_per_pixel = a.integer("spp", cam.samples_per_pixel);
    cam.max_depth = a.integer("depth", cam.max_depth);
}

// ---------------------------------------------------------------------------
// Rendering and output
// ---------------------------------------------------------------------------
inline double seconds_since(std::chrono::steady_clock::time_point t0) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
}

// Renders the whole image with the engine's own row loop (OpenMP when built
// with -DRT_USE_OPENMP), so pixels are identical to the main program's output.
inline std::vector<Color> render(Camera& cam, const Hittable_List& world) {
    cam.initialize();
    const auto t0 = std::chrono::steady_clock::now();
    std::vector<Color> fb = cam.renderRowsInitialized(world, 0, cam.image_height);
    std::clog << "Rendered " << cam.image_width << "x" << cam.image_height << " at "
              << cam.samples_per_pixel << " spp, depth " << cam.max_depth << " in "
              << std::fixed << std::setprecision(2) << seconds_since(t0) << " s\n";
    return fb;
}

inline void write_ppm(const Camera& cam, const std::vector<Color>& fb, const std::string& path) {
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("Cannot open " + path);
    }
    cam.writeImage(out, fb);
    if (!out) {
        throw std::runtime_error("Failed to write " + path);
    }
}

// ---------------------------------------------------------------------------
// Scene-building helpers (spheres only)
// ---------------------------------------------------------------------------
// The floor is a sphere of radius 1000. Its top sits at y = -0.5, half-way
// through a checker cell, so the 3D checker pattern does not flicker between
// two rows of cells right at the top of the sphere.
inline constexpr double FLOOR_Y = -0.5;
inline const Color DAYLIGHT(0.70, 0.80, 1.00);

inline std::shared_ptr<Material> matte(const Color& c) { return std::make_shared<Lambertian>(c); }
inline std::shared_ptr<Material> metal(const Color& c, double fuzz) { return std::make_shared<Metal>(c, fuzz); }
inline std::shared_ptr<Material> glass(double ior = 1.5) { return std::make_shared<Dielectric>(ior); }
inline std::shared_ptr<Material> light(const Color& c) { return std::make_shared<Diffuse_Light>(c); }
inline std::shared_ptr<Material> checker(double scale, const Color& a, const Color& b) {
    return std::make_shared<Lambertian>(std::make_shared<Checkered_Texture>(scale, a, b));
}
inline std::shared_ptr<Material> marble(double scale = 4.0) {
    return std::make_shared<Lambertian>(std::make_shared<Noise_Texture>(scale));
}

inline void add(Hittable_List& w, const Point3& c, double r, std::shared_ptr<Material> m) {
    w.add(std::make_shared<Sphere>(c, r, m));
}
inline void add_on_floor(Hittable_List& w, double x, double z, double r, std::shared_ptr<Material> m) {
    add(w, Point3(x, FLOOR_Y + r, z), r, m);
}
inline void add_floor(Hittable_List& w, std::shared_ptr<Material> m) {
    add(w, Point3(0, FLOOR_Y - 1000.0, 0), 1000.0, m);
}
// A "wall": a sphere of radius 1000 whose surface passes through `surface`,
// facing back along `normal` (unit axis vector pointing into the room).
inline void add_wall(Hittable_List& w, const Point3& surface, const Vec3& normal, std::shared_ptr<Material> m) {
    add(w, surface - 1000.0 * normal, 1000.0, m);
}

inline Hittable_List with_bvh(const Hittable_List& objects) {
    return Hittable_List(std::make_shared<BVH_Node>(objects));
}

inline void set_camera(Camera& cam, const Point3& from, const Point3& at, double vfov) {
    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 1280;
    cam.samples_per_pixel = 128;
    cam.max_depth = 30;
    cam.look_from = from;
    cam.look_at = at;
    cam.v_up = Vec3(0, 1, 0);
    cam.vertical_fov = vfov;
    cam.defocus_angle = 0;
    cam.focus_dist = (from - at).length();
}

// A small daylight scene with a mirror, glass, matte, textured, gold and
// glowing sphere. Used by 01_bounce_depth and 02_spp_convergence.
inline Scene demo_scene() {
    Scene s;
    Hittable_List w;
    add_floor(w, checker(1.0, Color(0.15, 0.18, 0.22), Color(0.85, 0.85, 0.82)));
    add_on_floor(w, -2.8, -0.8, 1.3, metal(Color(0.90, 0.90, 0.92), 0.0));   // mirror
    add_on_floor(w, 0.0, 0.4, 1.0, glass(1.5));                              // glass
    add_on_floor(w, 2.6, -0.4, 1.0, matte(Color(0.75, 0.35, 0.20)));         // terracotta
    add_on_floor(w, 1.3, 2.0, 0.55, metal(Color(0.85, 0.65, 0.30), 0.15));   // gold
    add_on_floor(w, -1.2, 2.3, 0.45, matte(Color(0.10, 0.55, 0.58)));        // teal
    add_on_floor(w, 3.8, 1.6, 0.60, marble(4.0));                            // marble
    add_on_floor(w, -0.4, 3.0, 0.30, light(Color(6.0, 4.5, 3.0)));           // small lamp
    s.world = with_bvh(w);
    set_camera(s.cam, Point3(0, 2.2, 10), Point3(0, 0.4, 0), 32);
    s.cam.background_color = DAYLIGHT;
    return s;
}

// "demo", "A" or "B".
inline Scene named_scene(const std::string& name, int sphereflake_levels = 4) {
    if (name == "demo") {
        return demo_scene();
    }
    return benchmark_scene(name, sphereflake_levels);
}

// Pixel position of a 3D point in the final image (camera must be initialized).
inline std::pair<double, double> project(const Camera& cam, const Point3& p) {
    const Vec3 d = p - cam.camera_center;
    const Vec3 forward = -cam.cam_backward;
    const double t = cam.focus_dist / dot(d, forward);
    const Vec3 on_plane = cam.camera_center + t * d - cam.pixel00_loc;
    const double x = dot(on_plane, cam.pixel_delta_u) / cam.pixel_delta_u.length_squared();
    const double y = dot(on_plane, cam.pixel_delta_v) / cam.pixel_delta_v.length_squared();
    return {x, y};
}

struct Label {
    Point3 anchor;  // 3D point the label sits under (e.g. the bottom of a sphere)
    std::string text;
};

// Writes "x y text" lines that common/env.sh's annotate() draws onto the PNG.
inline void write_labels(const Camera& cam, const std::vector<Label>& labels, const std::string& path) {
    std::ofstream f(path);
    for (const Label& l : labels) {
        const auto [x, y] = project(cam, l.anchor);
        f << static_cast<int>(std::lround(x)) << ' ' << static_cast<int>(std::lround(y)) << ' ' << l.text << '\n';
    }
}

// Standard main() for a single-scene program: builds the scene, applies
// --width/--spp/--depth, renders and writes --out.
template <typename Build>
int render_main(int argc, char** argv, Build build_scene, const std::string& default_out) {
    try {
        const Args a(argc, argv);
        Scene s = build_scene(a);
        apply_render_settings(s.cam, a);
        const std::vector<Color> fb = render(s.cam, s.world);
        write_ppm(s.cam, fb, a.str("out", default_out));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}

}  // namespace sc
