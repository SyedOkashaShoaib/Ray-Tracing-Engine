// 09 - Coloured light mixing.
//
// White spheres on a white floor in the dark, lit by a red, a green and a blue
// glowing sphere. Where all three lights reach, the floor is white. Each
// object blocks a different light in each direction, so its shadows come out
// in the mixed colours: blocking red leaves cyan (green + blue), blocking
// green leaves magenta, and blocking blue leaves yellow.
#include "showcase.h"

int main(int argc, char** argv) {
    return sc::render_main(argc, argv, [](const sc::Args&) {
        sc::Scene s;
        Hittable_List w;
        auto white = sc::matte(Color(0.80, 0.80, 0.80));
        sc::add_floor(w, white);
        sc::add_on_floor(w, 0.0, 0.0, 1.0, white);
        sc::add_on_floor(w, -2.0, 1.4, 0.5, white);
        sc::add_on_floor(w, 2.0, 1.4, 0.5, white);
        sc::add_on_floor(w, 0.0, -2.2, 0.45, white);
        sc::add(w, Point3(-2.8, 3.0, 1.6), 0.45, sc::light(Color(6, 0, 0)));   // red
        sc::add(w, Point3(0.0, 3.4, -2.6), 0.45, sc::light(Color(0, 6, 0)));   // green
        sc::add(w, Point3(2.8, 3.0, 1.6), 0.45, sc::light(Color(0, 0, 6)));    // blue
        s.world = sc::with_bvh(w);
        sc::set_camera(s.cam, Point3(0, 4.2, 8), Point3(0, 0, 0), 36);
        s.cam.background_color = Color(0, 0, 0);
        return s;
    }, "rgb_light_mixing.ppm");
}
