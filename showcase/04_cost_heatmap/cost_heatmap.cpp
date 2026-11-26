// 04 - Cost heatmap: how much work each pixel takes.
//
// Renders the scene with the engine's own camera code (get_ray, ray_color and
// the same per-pixel seeds as renderRowsInitialized, so the image is
// identical), while counting how many rays each pixel traces and timing each
// row. Every call to ray_color() that reaches the scene makes exactly one
// world.Hit() call, so wrapping the world in a counter counts rays traced.
//
// Writes:  <out>.ppm               the rendered image
//          <out>_rays.u32          rays per pixel (uint32, row-major)
//          <out>_meta.txt          "width height spp"
//          <out>_rows.csv          per-row render time and ray count
//
//   cost_heatmap --scene A --width 1280 --spp 32 --out output/A
#include "showcase.h"

namespace {

class CountingHittable : public Hittable {
public:
    explicit CountingHittable(const Hittable& inner) : inner(inner) {}
    bool Hit(const Ray& r, Interval ray_t, Hit_Record& rec) const override {
        ++counter();
        return inner.Hit(r, ray_t, rec);
    }
    Aabb bounding_box() const override { return inner.bounding_box(); }
    static std::uint64_t& counter() {
        thread_local std::uint64_t count = 0;
        return count;
    }

private:
    const Hittable& inner;
};

}  // namespace

int main(int argc, char** argv) {
    try {
        const sc::Args a(argc, argv);
        sc::Scene s = sc::named_scene(a.str("scene", "A"), a.integer("levels", 4));
        sc::apply_render_settings(s.cam, a);
        Camera& cam = s.cam;
        cam.initialize();

        const int W = cam.image_width;
        const int H = cam.image_height;
        const CountingHittable counted(s.world);
        std::vector<Color> image(static_cast<std::size_t>(W) * H);
        std::vector<std::uint32_t> rays(static_cast<std::size_t>(W) * H);
        std::vector<double> row_seconds(H);
        std::vector<std::uint64_t> row_rays(H);

        const auto t_all = std::chrono::steady_clock::now();
#pragma omp parallel for schedule(dynamic, 1)
        for (int row = 0; row < H; row++) {
            const auto t0 = std::chrono::steady_clock::now();
            std::uint64_t in_row = 0;
            for (int col = 0; col < W; col++) {
                const std::uint64_t pixel_id =
                    static_cast<std::uint64_t>(row) * static_cast<std::uint64_t>(W) + static_cast<std::uint64_t>(col);
                RNG gen(cam.rng_seed + pixel_id);  // same seed as the engine's render loop
                const std::uint64_t before = CountingHittable::counter();
                Color c(0, 0, 0);
                for (int sample = 0; sample < cam.samples_per_pixel; sample++) {
                    Ray r = cam.get_ray(col, row, gen);
                    c += cam.ray_color(r, counted, cam.max_depth, gen);
                }
                const std::size_t i = static_cast<std::size_t>(row) * W + col;
                image[i] = c * cam.pixel_samples_scale;
                const std::uint64_t n = CountingHittable::counter() - before;
                rays[i] = static_cast<std::uint32_t>(std::min<std::uint64_t>(n, UINT32_MAX));
                in_row += n;
            }
            row_seconds[row] = sc::seconds_since(t0);
            row_rays[row] = in_row;
        }
        const double total = sc::seconds_since(t_all);

        const std::string out = a.str("out", "heatmap");
        sc::write_ppm(cam, image, out + ".ppm");
        {
            std::ofstream f(out + "_rays.u32", std::ios::binary);
            f.write(reinterpret_cast<const char*>(rays.data()),
                    static_cast<std::streamsize>(rays.size() * sizeof(std::uint32_t)));
        }
        {
            std::ofstream f(out + "_meta.txt");
            f << W << ' ' << H << ' ' << cam.samples_per_pixel << '\n';
        }
        {
            std::ofstream f(out + "_rows.csv");
            f << "row,render_s,rays\n";
            for (int row = 0; row < H; row++) {
                f << row << ',' << std::setprecision(9) << row_seconds[row] << ',' << row_rays[row] << '\n';
            }
        }
        std::uint64_t all_rays = 0;
        for (auto n : row_rays) all_rays += n;
        std::clog << "Rendered " << W << "x" << H << " at " << cam.samples_per_pixel << " spp in " << std::fixed
                  << std::setprecision(2) << total << " s; " << all_rays << " rays traced\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
