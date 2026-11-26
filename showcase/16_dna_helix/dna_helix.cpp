// 16 - DNA double helix.
//
// Two spiral strands of gold beads joined by rungs of small glowing spheres
// (teal for A-T pairs, magenta for G-C pairs), rising diagonally through the
// frame on a dark background. The gold strands pick up the rungs' colours,
// with a soft key light from the side. Depth of field blurs the far end.
#include "showcase.h"

namespace {
// Rotate about the z axis (tilts the helix across the frame).
Point3 tilt(const Point3& p, double a) {
    return Point3(p.x() * std::cos(a) - p.y() * std::sin(a), p.x() * std::sin(a) + p.y() * std::cos(a), p.z());
}
}  // namespace

int main(int argc, char** argv) {
    return sc::render_main(argc, argv, [](const sc::Args&) {
        sc::Scene s;
        Hittable_List w;
        RNG g(3);
        const double radius = 1.4, rise = 0.34, twist = degree_to_radian(36.0), angle = degree_to_radian(-35.0);
        const int pairs = 44;
        auto gold = sc::metal(Color(0.95, 0.75, 0.35), 0.35);   // brushed gold catches the key light
        for (int k = 0; k < pairs; k++) {
            const double y = (k - pairs / 2.0) * rise;
            const double th = k * twist;
            const Point3 a(radius * std::cos(th), y, radius * std::sin(th));
            const Point3 b(radius * std::cos(th + pi), y, radius * std::sin(th + pi));
            // Backbone beads, with two linking beads towards the next pair.
            for (int strand = 0; strand < 2; strand++) {
                for (int sub = 0; sub < 3; sub++) {
                    const double f = sub / 3.0;
                    const double t2 = th + strand * pi + f * twist;
                    const Point3 p(radius * std::cos(t2), y + f * rise, radius * std::sin(t2));
                    sc::add(w, tilt(p, angle), sub == 0 ? 0.26 : 0.17, gold);
                }
            }
            // Rung: glowing beads across the middle.
            const bool at = g.uniform() < 0.5;
            const Color c = at ? Color(0.2, 1.0, 0.85) : Color(1.0, 0.25, 0.75);
            for (int j = 1; j <= 4; j++) {
                const Point3 p = a + (j / 5.0) * (b - a);
                sc::add(w, tilt(p, angle), 0.07, sc::light(2.2 * c));
            }
        }
        sc::add(w, Point3(8, 8, 12), 6.0, sc::light(Color(5.0, 4.5, 3.8)));    // soft key light on the gold
        s.world = sc::with_bvh(w);
        const Point3 from(0, 0.5, 13.0), at(0, 0, 0);
        sc::set_camera(s.cam, from, at, 34);
        s.cam.background_color = Color(0.06, 0.07, 0.12);
        s.cam.defocus_angle = 0.6;
        s.cam.focus_dist = (from - Point3(0, 0, 1.0)).length();
        return s;
    }, "dna_helix.ppm");
}
