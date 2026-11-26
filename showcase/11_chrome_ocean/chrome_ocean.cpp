// 11 - Chrome sphere ocean at sunset.
//
// A 100 x 100 grid of mirror spheres (10,000 in total) whose heights follow
// two overlapping waves at dusk. Warm, magenta and cyan lights sit behind the
// camera, so every sphere reflects them as highlights, and a small sun sets on
// the horizon. Its reflection streaks across the wave crests, and every sphere also reflects
// its neighbours. Also a good bounding volume hierarchy workout.
//   --grid N   spheres per side (default 100)
#include "showcase.h"

int main(int argc, char** argv) {
    return sc::render_main(argc, argv, [](const sc::Args& a) {
        sc::Scene s;
        Hittable_List w;
        const int n = a.integer("grid", 100);
        const double spacing = 0.5, radius = 0.24;
        auto chrome = sc::metal(Color(0.92, 0.92, 0.95), 0.01);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                const double x = (i - (n - 1) / 2.0) * spacing;
                const double z = -j * spacing + 18.0;
                const double d = std::sqrt((x - 6.0) * (x - 6.0) + (z + 4.0) * (z + 4.0));
                const double y = 0.8 * std::sin(0.42 * x + 0.25 * z) + 0.5 * std::sin(0.9 * d);
                sc::add(w, Point3(x, y, z), radius, chrome);
            }
        }
        // Lights behind and above the camera: the camera-facing side of every
        // chrome sphere reflects them, so each sphere carries three coloured
        // highlights.
        sc::add(w, Point3(0, 30, 50), 16.0, sc::light(Color(3.0, 2.3, 1.6)));        // warm key, overhead
        sc::add(w, Point3(-35, 10, 40), 11.0, sc::light(Color(2.6, 0.7, 1.9)));      // magenta, left
        sc::add(w, Point3(35, 10, 40), 11.0, sc::light(Color(0.7, 1.9, 2.8)));       // cyan, right
        sc::add(w, Point3(0, 0.8, -95), 6.0, sc::light(Color(1.0, 0.45, 0.15)));      // setting sun (kept below 1 so it stays orange)
        s.world = sc::with_bvh(w);
        sc::set_camera(s.cam, Point3(0, 3.2, 22), Point3(0, -0.4, -12), 34);
        s.cam.background_color = Color(0.07, 0.035, 0.09);                           // dusk sky (gamma brightens it on screen)
        return s;
    }, "chrome_ocean.ppm");
}
