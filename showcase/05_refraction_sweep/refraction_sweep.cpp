// 05 - Refraction index sweep.
//
// Five identical glass spheres in front of a checkered wall, with refractive
// index 1.00 (air: the sphere disappears), 1.33 (water), 1.50 (glass),
// 1.90 (dense flint glass) and 2.42 (diamond). Higher indices bend light more,
// flip and shrink the wall pattern seen through the sphere, and give stronger
// edge reflections from total internal reflection and Schlick's approximation.
#include "showcase.h"

namespace {
const std::vector<std::pair<double, std::string>> kIndices = {
    {1.00, "1.00"}, {1.33, "1.33"}, {1.50, "1.50"}, {1.90, "1.90"}, {2.42, "2.42"},
};
constexpr double kRadius = 0.8;
double x_of(std::size_t i) { return -4.0 + 2.0 * static_cast<double>(i); }
}  // namespace

int main(int argc, char** argv) {
    try {
        const sc::Args a(argc, argv);
        sc::Scene s;
        Hittable_List w;
        sc::add_floor(w, sc::checker(1.0, Color(0.20, 0.20, 0.22), Color(0.80, 0.80, 0.78)));
        // Back wall at z = -3.25, half-way through a 0.5-unit checker cell.
        sc::add_wall(w, Point3(0, 0, -3.25), Vec3(0, 0, 1),
                     sc::checker(0.5, Color(0.10, 0.12, 0.30), Color(0.92, 0.86, 0.70)));
        for (std::size_t i = 0; i < kIndices.size(); i++) {
            sc::add_on_floor(w, x_of(i), 0.0, kRadius, sc::glass(kIndices[i].first));
        }
        s.world = sc::with_bvh(w);
        sc::set_camera(s.cam, Point3(0, 0.9, 11), Point3(0, 0.3, 0), 34);
        s.cam.background_color = sc::DAYLIGHT;

        sc::apply_render_settings(s.cam, a);
        const std::string out = a.str("out", "refraction_sweep.ppm");
        sc::write_ppm(s.cam, sc::render(s.cam, s.world), out);

        std::vector<sc::Label> labels;
        for (std::size_t i = 0; i < kIndices.size(); i++) {
            labels.push_back({Point3(x_of(i), sc::FLOOR_Y, 0.0), "n = " + kIndices[i].second});
        }
        sc::write_labels(s.cam, labels, out.substr(0, out.rfind('.')) + "_labels.txt");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
