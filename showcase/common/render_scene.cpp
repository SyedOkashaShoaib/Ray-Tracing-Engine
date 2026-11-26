// Renders one of the named scenes ("demo", "A" or "B") with optional overrides.
// Used by 01_bounce_depth and 02_spp_convergence.
//
//   render_scene --scene demo --width 960 --spp 256 --depth 5 --out image.ppm
#include "showcase.h"

int main(int argc, char** argv) {
    try {
        const sc::Args a(argc, argv);
        sc::Scene s = sc::named_scene(a.str("scene", "demo"), a.integer("levels", 4));
        sc::apply_render_settings(s.cam, a);
        const std::vector<Color> fb = sc::render(s.cam, s.world);
        sc::write_ppm(s.cam, fb, a.str("out", "render.ppm"));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
