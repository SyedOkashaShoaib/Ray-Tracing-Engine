// 15 - Snow globe.
//
// A large water-filled globe (refractive index 1.33) packed with about 600
// small coloured spheres and some tiny white "snow" spheres, placed randomly
// so none overlap. Everything inside is bent and magnified by the water,
// like a real snow globe. It sits on a wooden pedestal.
//   --fill N   spheres inside (default 600)
#include "showcase.h"

int main(int argc, char** argv) {
    return sc::render_main(argc, argv, [](const sc::Args& a) {
        sc::Scene s;
        Hittable_List w;
        RNG g(5);
        sc::add_floor(w, sc::checker(1.0, Color(0.18, 0.16, 0.15), Color(0.82, 0.78, 0.72)));
        const double R = 2.4;
        const Point3 C(0, sc::FLOOR_Y + 0.6 + R, 0);
        sc::add(w, Point3(0, sc::FLOOR_Y + 0.25, 0), 1.0, sc::matte(Color(0.30, 0.17, 0.10)));   // wooden pedestal
        sc::add(w, C, R, sc::glass(1.33));                                            // water globe

        const std::array<Color, 6> palette = {
            Color(0.90, 0.25, 0.25), Color(0.95, 0.70, 0.20), Color(0.25, 0.70, 0.35),
            Color(0.20, 0.45, 0.90), Color(0.65, 0.30, 0.80), Color(0.95, 0.95, 0.92),
        };
        struct Ball { Point3 p; double r; };
        std::vector<Ball> balls;
        const int fill = a.integer("fill", 600);
        int attempts = 0;
        while (static_cast<int>(balls.size()) < fill && attempts++ < 400000) {
            const bool snow = balls.size() % 3 == 2;
            const double r = snow ? 0.035 : 0.07 + 0.09 * g.uniform();
            const Vec3 off(2 * g.uniform() - 1, 2 * g.uniform() - 1, 2 * g.uniform() - 1);
            if (off.length() > 1.0) continue;
            const Point3 p = C + (R - r - 0.05) * off;
            bool fits = (p - C).length() + r < R - 0.04;
            for (std::size_t k = 0; fits && k < balls.size(); k++) {
                if ((p - balls[k].p).length() < r + balls[k].r + 0.01) fits = false;
            }
            if (fits) balls.push_back({p, r});
        }
        for (std::size_t i = 0; i < balls.size(); i++) {
            const bool snow = i % 3 == 2;
            sc::add(w, balls[i].p, balls[i].r,
                    snow ? sc::matte(Color(0.95, 0.95, 0.95)) : sc::matte(palette[i % palette.size()]));
        }
        sc::add(w, Point3(5, 9, 6), 2.0, sc::light(Color(4.5, 4.2, 3.8)));          // soft key light
        s.world = sc::with_bvh(w);
        sc::set_camera(s.cam, Point3(0, 3.6, 10.5), Point3(0, 2.2, 0), 32);
        s.cam.background_color = sc::DAYLIGHT;
        return s;
    }, "snow_globe.ppm");
}
