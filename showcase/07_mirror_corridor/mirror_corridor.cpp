// 07 - Mirror corridor.
//
// Two parallel mirror walls (spheres of radius 1000, so flat over the visible
// area) at x = -3 and x = +3, with a few objects between them. Each
// reflection is one more bounce, so the corridor of copies goes as deep as
// max_depth allows; the mirrors reflect 92% of the light, so each copy is a
// little darker. Render with --depth 2, 4, 30 to see the corridor grow.
#include "showcase.h"

int main(int argc, char** argv) {
    return sc::render_main(argc, argv, [](const sc::Args&) {
        sc::Scene s;
        Hittable_List w;
        auto mirror = sc::metal(Color(0.92, 0.92, 0.95), 0.0);
        sc::add_floor(w, sc::checker(1.0, Color(0.12, 0.12, 0.14), Color(0.82, 0.80, 0.76)));
        sc::add_wall(w, Point3(-3, 0, 0), Vec3(1, 0, 0), mirror);
        sc::add_wall(w, Point3(3, 0, 0), Vec3(-1, 0, 0), mirror);
        sc::add_on_floor(w, -1.0, 0.5, 0.60, sc::matte(Color(0.80, 0.12, 0.10)));
        sc::add_on_floor(w, 1.0, -1.2, 0.60, sc::glass(1.5));
        sc::add_on_floor(w, 0.2, 2.2, 0.45, sc::metal(Color(0.85, 0.65, 0.30), 0.05));
        sc::add_on_floor(w, 0.6, -3.5, 0.50, sc::matte(Color(0.10, 0.50, 0.60)));
        sc::add(w, Point3(-0.4, 2.3, -1.0), 0.25, sc::light(Color(6.0, 5.0, 3.5)));
        s.world = sc::with_bvh(w);
        sc::set_camera(s.cam, Point3(1.8, 1.5, 6), Point3(-3.0, 0.7, -0.5), 40);
        s.cam.background_color = sc::DAYLIGHT;
        return s;
    }, "mirror_corridor.ppm");
}
