// 13 - Golden-spiral sunflower.
//
// A sunflower head built from spheres. Seed i sits at angle i x 137.508 degrees
// (the golden angle) and distance sqrt(i) from the centre, which is exactly
// how real sunflower seeds pack. Seeds are coloured by their position along
// one family of 34 spirals, so the spiral arms show up as bands, and they grow
// larger towards the rim. A ring of petals, each three rows of yellow spheres
// tapering to a point, surrounds the head. Seen from above at an angle, on a dark floor.
//   --seeds N    number of seeds (default 900)
//   --petals N   number of petals (default 26)
#include "showcase.h"

int main(int argc, char** argv) {
    return sc::render_main(argc, argv, [](const sc::Args& a) {
        sc::Scene s;
        Hittable_List w;
        const int seeds = a.integer("seeds", 900);
        const int petals = a.integer("petals", 26);
        const double golden = degree_to_radian(137.50776);
        const double c = 0.135;                          // seed spacing
        const double head = c * std::sqrt(static_cast<double>(seeds));
        const double y0 = sc::FLOOR_Y + 0.9;              // the head floats a little

        sc::add_floor(w, sc::matte(Color(0.10, 0.12, 0.10)));

        // Seeds: dark bronze in the middle, through copper to warm gold at the
        // rim, with alternating light and dark bands along 34 spirals.
        const Color centre(0.25, 0.14, 0.07), middle(0.70, 0.38, 0.16), rim(0.95, 0.72, 0.30);
        for (int i = 1; i <= seeds; i++) {
            const double t = static_cast<double>(i) / seeds;
            const double r = c * std::sqrt(static_cast<double>(i));
            const double theta = i * golden;
            const double size = 0.095 + 0.030 * t;      // seeds nearly touch
            Color base = t < 0.5 ? sc::lerp(centre, middle, t / 0.5) : sc::lerp(middle, rim, (t - 0.5) / 0.5);
            if (i % 21 % 3 == 0) base = 0.45 * base;      // every third of the 21 spiral arms is darker
            const double dome = 0.25 * (1.0 - t * t);      // slightly domed head
            sc::add(w, Point3(r * std::cos(theta), y0 + dome, r * std::sin(theta)), size,
                    sc::metal(base, 0.18));
        }

        // Petals: each is a broad leaf made of three rows of yellow spheres that
        // fan out from the head and meet again at a pointed tip.
        for (int p = 0; p < petals; p++) {
            const double phi = 2.0 * pi * p / petals;
            const Vec3 out(std::cos(phi), 0, std::sin(phi)), side(-std::sin(phi), 0, std::cos(phi));
            for (int k = 0; k < 12; k++) {
                const double f = k / 11.0;
                const double rr = head + 0.05 + f * 1.7;
                const double width = 0.24 * std::sin(pi * (0.15 + 0.85 * f));   // widest in the middle
                const double size = 0.17 * (1.0 - 0.45 * f);
                const double droop = -0.20 * f * f;
                const Color yellow = sc::lerp(Color(0.98, 0.66, 0.06), Color(1.00, 0.85, 0.22), f);
                for (int row = -1; row <= 1; row++) {
                    const Point3 c0 = Point3(0, y0 + droop, 0) + rr * out + (row * width) * side;
                    sc::add(w, c0, size, sc::matte(yellow));
                }
            }
        }

        sc::add(w, Point3(-9, 24, 16), 4.0, sc::light(Color(6.0, 5.6, 4.9)));    // warm key light
        s.world = sc::with_bvh(w);
        const Point3 from(0, 15.0, 8.5), at(0, y0, 0.6);
        sc::set_camera(s.cam, from, at, 42);
        s.cam.background_color = Color(0.42, 0.48, 0.55);
        return s;
    }, "golden_spiral.ppm");
}
