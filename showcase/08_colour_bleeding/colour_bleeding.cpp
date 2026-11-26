// 08 - Colour bleeding (global illumination).
//
// A room built from spheres of radius 1000: white floor, back wall and
// ceiling, a red left wall and a green right wall, lit only by a glowing
// half-dome in the ceiling. Light that bounces off the coloured walls tints
// the white spheres red on one side and green on the other. Nothing in the
// code adds that tint; it comes from following rays through several bounces.
// The room is lit by one lamp, so it needs many samples per pixel. The
// bounce limit defaults to 8 here (override with --depth).
#include "showcase.h"

int main(int argc, char** argv) {
    return sc::render_main(argc, argv, [](const sc::Args&) {
        sc::Scene s;
        Hittable_List w;
        auto white = sc::matte(Color(0.73, 0.73, 0.73));
        const double ceiling = 4.5;
        sc::add_floor(w, white);
        sc::add_wall(w, Point3(0, ceiling, 0), Vec3(0, -1, 0), white);                    // ceiling
        sc::add_wall(w, Point3(0, 0, -4), Vec3(0, 0, 1), white);                          // back wall
        sc::add_wall(w, Point3(-3, 0, 0), Vec3(1, 0, 0), sc::matte(Color(0.65, 0.05, 0.05)));   // red
        sc::add_wall(w, Point3(3, 0, 0), Vec3(-1, 0, 0), sc::matte(Color(0.12, 0.45, 0.15)));   // green
        sc::add(w, Point3(0, ceiling, -1.5), 1.4, sc::light(Color(5, 5, 5)));            // dome lamp
        sc::add_on_floor(w, -1.6, -2.0, 1.0, white);
        sc::add_on_floor(w, 1.6, -1.0, 1.0, white);
        sc::add_on_floor(w, 0.1, 0.8, 0.55, sc::glass(1.5));
        s.world = sc::with_bvh(w);
        sc::set_camera(s.cam, Point3(0, 2.0, 9.5), Point3(0, 1.8, -1), 34);
        s.cam.background_color = Color(0, 0, 0);
        // In a closed room almost no ray escapes, so every path would run the
        // full 30 bounces. After 8 bounces off 73% white walls less than 10% of
        // the light is left, so 8 keeps the look at a fraction of the cost.
        s.cam.max_depth = 8;
        return s;
    }, "colour_bleeding.ppm");
}
