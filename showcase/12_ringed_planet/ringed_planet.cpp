// 12 - Ringed planet.
//
// The Earth texture on a planet, a tilted ring of about 20,000 small icy
// spheres, a marble-textured moon and a single bright star lighting
// everything from the side, against black space with distant star points.
// Lit by one light in the dark, so it needs many samples per pixel.
//   --ring N   ring particles (default 20000)
#include "showcase.h"

int main(int argc, char** argv) {
    return sc::render_main(argc, argv, [](const sc::Args& a) {
        sc::Scene s;
        Hittable_List w;
        RNG g(7);
        auto earth = std::make_shared<Lambertian>(std::make_shared<Image_Texture>("src/images/input/globe.png"));
        sc::add(w, Point3(0, 0, 0), 3.0, earth);

        // Ring in a plane tilted 22 degrees about the x axis.
        const double tilt = degree_to_radian(22.0);
        const int ring = a.integer("ring", 20000);
        for (int i = 0; i < ring; i++) {
            double r = 4.0 + 3.2 * g.uniform();
            if (r > 5.6 && r < 5.85) r += 0.3;                                  // a gap in the ring
            const double phi = 2.0 * pi * g.uniform();
            const double lx = r * std::cos(phi), lz = r * std::sin(phi), ly = 0.025 * sc::gauss(g);
            const Point3 p(lx, ly * std::cos(tilt) - lz * std::sin(tilt), ly * std::sin(tilt) + lz * std::cos(tilt));
            const double shade = 0.55 + 0.4 * g.uniform();
            sc::add(w, p, 0.02 + 0.035 * g.uniform(), sc::matte(shade * Color(0.95, 0.88, 0.75)));
        }
        sc::add(w, Point3(7.5, 2.2, -5.0), 0.7, sc::marble(6.0));                       // moon
        sc::add(w, Point3(-45, 14, 30), 14.0, sc::light(Color(16, 15, 13)));            // the star
        for (int i = 0; i < 400; i++) {                                                 // background stars
            const Vec3 dir = unit(Vec3(sc::gauss(g), sc::gauss(g), sc::gauss(g)));
            sc::add(w, Point3(0, 0, 0) + 250.0 * dir, 0.25 + 0.25 * g.uniform(),
                    sc::light((1.0 + 2.0 * g.uniform()) * Color(0.9, 0.93, 1.0)));
        }
        s.world = sc::with_bvh(w);
        sc::set_camera(s.cam, Point3(4.5, 3.5, 17), Point3(0.8, 0.0, 0), 34);
        s.cam.background_color = Color(0, 0, 0);
        return s;
    }, "ringed_planet.ppm");
}
