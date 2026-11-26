// 18 - Pastel room.
//
// A soft, hazy room with a sage-green left wall, a peach right wall and warm
// off-white floor, ceiling and back wall, filled with floating spheres:
// polished chrome, matte porcelain and softly glowing pastel lights (peach,
// lilac, mint, sky blue, butter). The glowing spheres tint the room, and a
// pale haze coming in through the open front keeps the shadows soft and the
// contrast low, with a large soft light hidden behind the camera. The chrome
// spheres reflect the room and each other.
//   --count N   floating spheres (default 26)
#include "showcase.h"

int main(int argc, char** argv) {
    return sc::render_main(argc, argv, [](const sc::Args& a) {
        sc::Scene s;
        Hittable_List w;
        RNG g(18);

        // The room: walls are spheres of radius 1000, flat at this scale.
        const double left = -3.4, right = 3.4, ceiling = 4.2, back = -4.5;
        auto warm_white = sc::matte(Color(0.88, 0.87, 0.83));
        sc::add_floor(w, warm_white);
        sc::add_wall(w, Point3(0, ceiling, 0), Vec3(0, -1, 0), warm_white);
        sc::add_wall(w, Point3(0, 0, back), Vec3(0, 0, 1), sc::matte(Color(0.86, 0.88, 0.85)));
        sc::add_wall(w, Point3(left, 0, 0), Vec3(1, 0, 0), sc::matte(Color(0.45, 0.74, 0.58)));    // sage green
        sc::add_wall(w, Point3(right, 0, 0), Vec3(-1, 0, 0), sc::matte(Color(0.88, 0.64, 0.58)));  // peach

        const std::array<Color, 5> glow = {
            Color(1.00, 0.80, 0.66),   // peach
            Color(0.86, 0.74, 1.00),   // lilac
            Color(0.72, 1.00, 0.82),   // mint
            Color(0.70, 0.88, 1.00),   // sky blue
            Color(1.00, 0.96, 0.72),   // butter
        };
        auto chrome = sc::metal(Color(0.90, 0.91, 0.92), 0.0);
        auto porcelain = sc::matte(Color(0.82, 0.83, 0.82));

        // Floating spheres, placed at random without touching each other or
        // the walls. Every third is chrome, every third glows, the rest are matte.
        struct Ball { Point3 p; double r; };
        std::vector<Ball> balls;
        const int count = a.integer("count", 26);
        int attempts = 0;
        while (static_cast<int>(balls.size()) < count && attempts++ < 20000) {
            const double r = 0.25 + 0.45 * g.uniform();
            const Point3 p(left + 0.4 + r + (right - left - 0.8 - 2 * r) * g.uniform(),
                           sc::FLOOR_Y + 0.3 + r + (ceiling - sc::FLOOR_Y - 0.6 - 2 * r) * g.uniform(),
                           back + 0.8 + r + 4.2 * g.uniform());
            bool fits = true;
            for (const Ball& b : balls) {
                if ((p - b.p).length() < r + b.r + 0.12) { fits = false; break; }
            }
            if (fits) balls.push_back({p, r});
        }
        int lights = 0;
        for (std::size_t i = 0; i < balls.size(); i++) {
            std::shared_ptr<Material> m;
            switch (i % 3) {
                case 0: m = chrome; break;
                case 1: m = sc::light(0.98 * glow[lights++ % glow.size()]); break;   // below 1 keeps the pastel tint
                default: m = porcelain; break;
            }
            sc::add(w, balls[i].p, balls[i].r, m);
        }

        // A large soft light behind and above the camera, out of frame, fills
        // the room evenly the way a photographer's softbox would.
        sc::add(w, Point3(0, 3.6, 12.0), 3.2, sc::light(Color(2.8, 2.85, 2.65)));
        s.world = sc::with_bvh(w);
        sc::set_camera(s.cam, Point3(0, 1.8, 6.8), Point3(0, 1.75, -2), 46);
        // Pale haze entering through the open front: soft fill light.
        s.cam.background_color = Color(0.78, 0.86, 0.80);
        // Every ray in a closed room keeps bouncing; 10 bounces keeps the look
        // at a fraction of the cost (see 08_colour_bleeding).
        s.cam.max_depth = 10;
        return s;
    }, "pastel_room.ppm");
}
