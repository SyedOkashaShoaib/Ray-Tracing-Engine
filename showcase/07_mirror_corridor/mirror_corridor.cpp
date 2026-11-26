// 07 - Mirror corridor.
//
// Two parallel mirror walls (spheres of radius 1000, so flat over the visible
// area) at x = -3 and x = +3, with a few objects between them. The camera
// looks almost straight into the left mirror, so the reflections of the
// objects line up and shrink into the distance. Each copy is one more bounce;
// the mirrors are slightly green-tinted, like real silvered glass, so each
// copy is a little darker and greener. The black spot at the end is where the
// bounce limit runs out. Render with --depth 2, 4, 8, 30 to see the corridor grow.
#include "showcase.h"

int main(int argc, char** argv) {
    return sc::render_main(argc, argv, [](const sc::Args&) {
        sc::Scene s;
        Hittable_List w;
        // Slightly green-tinted, like real silvered glass, so each copy is visibly darker.
        auto mirror = sc::metal(Color(0.86, 0.92, 0.88), 0.0);
        sc::add_floor(w, sc::checker(1.0, Color(0.12, 0.12, 0.14), Color(0.82, 0.80, 0.76)));
        sc::add_wall(w, Point3(-3, 0, 0), Vec3(1, 0, 0), mirror);
        sc::add_wall(w, Point3(3, 0, 0), Vec3(-1, 0, 0), mirror);
        sc::add_on_floor(w, -0.9, -0.6, 0.55, sc::matte(Color(0.80, 0.12, 0.10)));
        sc::add_on_floor(w, 0.3, 0.7, 0.45, sc::metal(Color(0.85, 0.65, 0.30), 0.05));
        sc::add_on_floor(w, 1.2, -1.4, 0.50, sc::glass(1.5));
        sc::add(w, Point3(-0.2, 2.4, -0.4), 0.25, sc::light(Color(6.0, 5.0, 3.5)));
        s.world = sc::with_bvh(w);
        // Looking almost straight into the left mirror, so the copies line up.
        sc::set_camera(s.cam, Point3(2.6, 1.3, 2.2), Point3(-3.0, 0.8, -0.6), 45);
        s.cam.background_color = sc::DAYLIGHT;
        return s;
    }, "mirror_corridor.ppm");
}
