// 17 - Apollonian sphere packing.
//
// A ball filled with spheres that all touch their neighbours: four equal
// spheres fill a bounding ball, then every gap between any four touching
// spheres is filled with the largest sphere that fits, and so on, down to a
// minimum size. Each new sphere comes from the 3D Descartes relation: in a
// set of five mutually touching spheres, replacing sphere i gives
//     k' = (sum of the other four curvatures) - k_i
//     k'c' = (sum of the other four k*c) - k_i*c_i       (c = centre)
// A wedge is cut away so you can see inside. Rendered in polished metal,
// coloured by size, on a dark mirror floor.
//   --min_radius R   smallest sphere, as a fraction of the ball (default 0.012)
#include "showcase.h"

#include <unordered_set>

namespace {
struct S {
    double k;  // curvature (1 / radius); negative for the outer bounding sphere
    Vec3 c;    // centre
};
using Config = std::array<S, 5>;

S replace(const Config& conf, int i) {
    double k = -conf[i].k;
    Vec3 kc = -conf[i].k * conf[i].c;
    for (int j = 0; j < 5; j++) {
        if (j != i) {
            k += conf[j].k;
            kc += conf[j].k * conf[j].c;
        }
    }
    return {k, kc / k};
}
}  // namespace

int main(int argc, char** argv) {
    return sc::render_main(argc, argv, [](const sc::Args& a) {
        const double rmin = a.real("min_radius", 0.012);

        // Starting configuration: unit bounding ball and four equal spheres at
        // the corners of a regular tetrahedron.
        const double r0 = 1.0 / (1.0 + std::sqrt(1.5));
        const double d = (1.0 - r0) / std::sqrt(3.0);
        Config start = {S{-1.0, Vec3(0, 0, 0)}, S{1 / r0, d * Vec3(1, 1, 1)}, S{1 / r0, d * Vec3(1, -1, -1)},
                        S{1 / r0, d * Vec3(-1, 1, -1)}, S{1 / r0, d * Vec3(-1, -1, 1)}};

        std::vector<std::pair<Vec3, double>> spheres;
        std::unordered_set<std::string> seen;
        auto keep = [&](const Vec3& c, double r) {
            char key[96];
            std::snprintf(key, sizeof key, "%.6f,%.6f,%.6f", c.x(), c.y(), c.z());
            if (seen.insert(key).second) spheres.push_back({c, r});
        };
        for (int i = 1; i < 5; i++) keep(start[i].c, 1 / start[i].k);

        std::vector<Config> stack = {start};
        while (!stack.empty()) {
            const Config conf = stack.back();
            stack.pop_back();
            for (int i = 0; i < 5; i++) {
                const S n = replace(conf, i);
                // Only descend into smaller gaps; this never revisits a parent.
                if (n.k <= conf[i].k || 1.0 / n.k < rmin) continue;
                keep(n.c, 1.0 / n.k);
                Config next = conf;
                next[i] = n;
                stack.push_back(next);
            }
        }
        std::clog << "Apollonian packing: " << spheres.size() << " spheres\n";

        sc::Scene s;
        Hittable_List w;
        sc::add_floor(w, sc::metal(Color(0.18, 0.18, 0.20), 0.05));
        const double R = 2.2;
        const Point3 centre(0, sc::FLOOR_Y + R, 0);
        const Color small(0.85, 0.88, 0.95), large(0.95, 0.70, 0.35);
        for (const auto& [c, r] : spheres) {
            if (c.x() > 0.0 && c.z() > 0.0) continue;           // cut away a quarter wedge
            const double t = std::clamp(std::log(r / rmin) / std::log(r0 / rmin), 0.0, 1.0);
            sc::add(w, centre + R * c, R * r, sc::metal(sc::lerp(small, large, t), 0.03));
        }
        sc::add(w, Point3(-5, 8, 6), 2.5, sc::light(Color(5.0, 4.6, 4.0)));
        s.world = sc::with_bvh(w);
        const Point3 from(6.0, 3.6, 7.5), at(0, sc::FLOOR_Y + R * 0.9, 0);
        sc::set_camera(s.cam, from, at, 30);
        s.cam.background_color = Color(0.45, 0.52, 0.68);
        return s;
    }, "apollonian_packing.ppm");
}
