// 10 - Spiral galaxy.
//
// About 30,000 tiny glowing spheres: a warm, dense core and two blue-white
// spiral arms, on black. Every ray either hits a star and stops or flies off
// into the black background, so the image is almost noise-free even at low
// samples per pixel, and it is the cheapest of the showcase scenes.
//   --stars N   number of stars in the arms (default 28000)
#include "showcase.h"

int main(int argc, char** argv) {
    return sc::render_main(argc, argv, [](const sc::Args& a) {
        sc::Scene s;
        Hittable_List w;
        RNG g(2026);
        const int arm_stars = a.integer("stars", 28000);
        const double R = 10.0;                       // galaxy radius
        const Color core_colour(1.00, 0.82, 0.55), arm_colour(0.55, 0.72, 1.00);

        // Two logarithmic-looking spiral arms.
        for (int i = 0; i < arm_stars; i++) {
            const int arm = i % 2;
            const double t = std::pow(g.uniform(), 0.7);         // more stars near the centre
            const double r = 0.8 + t * R;
            const double theta = arm * pi + 2.6 * std::log(r) + 0.35 * sc::gauss(g) / (0.4 + t);
            const double spread = 0.25 + 0.35 * t;
            const double x = r * std::cos(theta) + spread * sc::gauss(g);
            const double z = r * std::sin(theta) + spread * sc::gauss(g);
            const double y = 0.18 * (1.0 - 0.7 * t) * sc::gauss(g);
            const double brightness = 0.5 + 1.2 * g.uniform() * (1.0 - 0.4 * t);
            const double size = 0.018 + 0.03 * g.uniform() * g.uniform();
            const Color c = sc::lerp(core_colour, arm_colour, std::min(1.0, t * 1.3));
            sc::add(w, Point3(x, y, z), size, sc::light(brightness * c));
        }
        // Bright central bulge.
        for (int i = 0; i < 6000; i++) {
            const double x = 0.9 * sc::gauss(g), y = 0.45 * sc::gauss(g), z = 0.9 * sc::gauss(g);
            const double brightness = 1.0 + 1.6 * g.uniform();
            sc::add(w, Point3(x, y, z), 0.02 + 0.02 * g.uniform(), sc::light(brightness * core_colour));
        }
        // A few large, distant background stars.
        for (int i = 0; i < 250; i++) {
            const Vec3 dir = unit(Vec3(sc::gauss(g), sc::gauss(g), sc::gauss(g)));
            sc::add(w, Point3(0, 0, 0) + 120.0 * dir, 0.12 + 0.1 * g.uniform(),
                    sc::light((1.5 + 2.0 * g.uniform()) * Color(0.9, 0.92, 1.0)));
        }
        s.world = sc::with_bvh(w);
        sc::set_camera(s.cam, Point3(0, 10.5, 21.5), Point3(0, -0.6, 0), 42);
        s.cam.background_color = Color(0, 0, 0);
        s.cam.max_depth = 4;   // nothing here scatters light
        return s;
    }, "galaxy.ppm");
}
