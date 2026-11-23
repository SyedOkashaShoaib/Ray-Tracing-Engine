
//   sphereflake --levels 3 --width 800 --spp 64 --style rainbow > out.ppm
//
// Options: --levels N  --scale S  --style rainbow|brass|gold|chrome|glass
//          --width W   --spp N    --depth N  --plain-floor  --seed N
#include <cstdlib>
#include <iostream>
#include <string>

#include "scenes/sphereflake.h"

int main(int argc, char* argv[]) {
    Sphereflake_Settings s;
    unsigned seed = 1;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        auto next = [&]() -> std::string {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << arg << '\n';
                std::exit(1);
            }
            return argv[++i];
        };

        if      (arg == "--levels")      s.levels = std::stoi(next());
        else if (arg == "--scale")       s.child_scale = std::stod(next());
        else if (arg == "--style")       s.style = next();
        else if (arg == "--width")       s.image_width = std::stoi(next());
        else if (arg == "--spp")         s.samples_per_pixel = std::stoi(next());
        else if (arg == "--depth")       s.max_depth = std::stoi(next());
        else if (arg == "--seed")        seed = unsigned(std::stoul(next()));
        else if (arg == "--plain-floor") s.checker_floor = false;
        else {
            std::cerr << "Unknown option: " << arg << '\n';
            return 1;
        }
    }

    if (s.levels < 0 || s.levels > 6) {
        std::cerr << "--levels must be 0..6 (level 6 is already ~600k spheres)\n";
        return 1;
    }
    if (s.child_scale <= 0.0 || s.child_scale > 0.5) {
        std::cerr << "--scale must be in (0, 0.5]\n";
        return 1;
    }

    std::srand(seed); 
    render_sphereflake(s);
    return 0;
}
