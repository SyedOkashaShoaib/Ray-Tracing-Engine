// 14 - Glowing marbles.
//
// Glass marbles, each holding a small glowing core, scattered on a dark
// mirror floor in near darkness. The cores are seen through the glass, so
// they are bent and magnified; depth of field keeps the front marble sharp.
#include "showcase.h"

int main(int argc, char** argv) {
    return sc::render_main(argc, argv, [](const sc::Args&) {
        sc::Scene s;
        Hittable_List w;
        RNG g(11);
        sc::add_floor(w, sc::metal(Color(0.12, 0.12, 0.14), 0.03));
        const std::array<Color, 6> palette = {
            Color(1.00, 0.55, 0.15), Color(0.15, 0.85, 0.80), Color(0.95, 0.25, 0.65),
            Color(0.55, 0.95, 0.25), Color(0.30, 0.45, 1.00), Color(1.00, 0.85, 0.30),
        };
        struct Marble { double x, z, r; };
        std::vector<Marble> placed;
        int attempts = 0;
        while (placed.size() < 16 && attempts++ < 5000) {
            const Marble m{-2.6 + 5.2 * g.uniform(), -3.0 + 4.6 * g.uniform(), 0.32 + 0.3 * g.uniform()};
            bool fits = true;
            for (const Marble& o : placed) {
                const double d = std::hypot(m.x - o.x, m.z - o.z);
                if (d < m.r + o.r + 0.12) { fits = false; break; }
            }
            if (fits) placed.push_back(m);
        }
        for (std::size_t i = 0; i < placed.size(); i++) {
            const Marble& m = placed[i];
            const Point3 centre(m.x, sc::FLOOR_Y + m.r, m.z);
            sc::add(w, centre, m.r, sc::glass(1.5));
            sc::add(w, centre, 0.2 * m.r, sc::light(3.0 * palette[i % palette.size()]));
        }
        s.world = sc::with_bvh(w);
        const Point3 from(0.6, 1.5, 6.5), at(0.0, -0.1, 0.0);
        sc::set_camera(s.cam, from, at, 36);
        s.cam.background_color = Color(0.010, 0.012, 0.020);
        s.cam.defocus_angle = 1.6;
        s.cam.focus_dist = (from - Point3(0, 0, 1.5)).length();
        return s;
    }, "glowing_marbles.ppm");
}
