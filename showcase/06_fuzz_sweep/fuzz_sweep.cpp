// 06 - Metal fuzz sweep.
//
// Six identical silver spheres with fuzz 0, 0.1, 0.2, 0.4, 0.7 and 1.0 (the
// engine's maximum). Fuzz randomly jitters each reflected ray, so the
// reflection goes from a sharp mirror to brushed and then rough metal.
// Large coloured spheres behind the camera give the mirrors something to
// reflect.
#include "showcase.h"

namespace {
const std::vector<std::pair<double, std::string>> kFuzz = {
    {0.0, "0"}, {0.1, "0.1"}, {0.2, "0.2"}, {0.4, "0.4"}, {0.7, "0.7"}, {1.0, "1.0"},
};
constexpr double kRadius = 0.7;
double x_of(std::size_t i) { return -4.5 + 1.8 * static_cast<double>(i); }
}  // namespace

int main(int argc, char** argv) {
    try {
        const sc::Args a(argc, argv);
        sc::Scene s;
        Hittable_List w;
        sc::add_floor(w, sc::checker(1.0, Color(0.15, 0.16, 0.20), Color(0.85, 0.85, 0.82)));
        for (std::size_t i = 0; i < kFuzz.size(); i++) {
            sc::add_on_floor(w, x_of(i), 0.0, kRadius, sc::metal(Color(0.85, 0.85, 0.88), kFuzz[i].first));
        }
        // Behind the camera: only visible in the reflections.
        sc::add(w, Point3(-9, 4.0, 19), 5.5, sc::matte(Color(0.80, 0.15, 0.10)));
        sc::add(w, Point3(0, 4.0, 21), 5.5, sc::matte(Color(0.95, 0.80, 0.15)));
        sc::add(w, Point3(9, 4.0, 19), 5.5, sc::matte(Color(0.15, 0.35, 0.85)));
        s.world = sc::with_bvh(w);
        sc::set_camera(s.cam, Point3(0, 1.0, 11), Point3(0, 0.2, 0), 34);
        s.cam.background_color = sc::DAYLIGHT;

        sc::apply_render_settings(s.cam, a);
        const std::string out = a.str("out", "fuzz_sweep.ppm");
        sc::write_ppm(s.cam, sc::render(s.cam, s.world), out);

        std::vector<sc::Label> labels;
        for (std::size_t i = 0; i < kFuzz.size(); i++) {
            labels.push_back({Point3(x_of(i), sc::FLOOR_Y, 0.0), "fuzz " + kFuzz[i].second});
        }
        sc::write_labels(s.cam, labels, out.substr(0, out.rfind('.')) + "_labels.txt");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
