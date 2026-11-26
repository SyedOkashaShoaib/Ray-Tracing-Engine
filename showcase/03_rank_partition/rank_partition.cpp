// 03 - Which MPI rank rendered which rows.
//
// Splits the image into row blocks with exactly the same formula as
// Camera::renderImageMPI, renders each block with the engine's own
// renderRowsInitialized(), tints every rank's pixels with its own colour and
// gathers the image on rank 0 with MPI_Gatherv. Each rank also times its own
// rendering, and rank 0 prints and saves those per-rank times: the
// measurement the formal benchmarks did not record.
//
//   mpirun -np 4 rank_partition --scene A --width 1280 --spp 64 --out ranks.ppm
//   options: --tint 0 to skip the colouring, --alpha 0.08 tint strength
#include "showcase.h"

namespace {

Color rank_colour(int rank) {
    static const std::array<Color, 8> palette = {
        Color(1.00, 0.20, 0.20), Color(0.20, 1.00, 0.25), Color(0.25, 0.45, 1.00), Color(1.00, 0.90, 0.10),
        Color(1.00, 0.20, 1.00), Color(0.10, 1.00, 1.00), Color(1.00, 0.55, 0.10), Color(1.00, 1.00, 1.00),
    };
    return palette[static_cast<std::size_t>(rank) % palette.size()];
}

}  // namespace

int main(int argc, char** argv) {
    int provided = 0;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    if (provided < MPI_THREAD_FUNNELED) {
        std::cerr << "Insufficient MPI thread support\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int rank = 0, size = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    try {
        const sc::Args a(argc, argv);
        sc::Scene s = sc::named_scene(a.str("scene", "A"), a.integer("levels", 4));
        sc::apply_render_settings(s.cam, a);
        Camera& cam = s.cam;
        cam.initialize();

        // Same block distribution as Camera::renderImageMPI.
        const int H = cam.image_height;
        const int W = cam.image_width;
        const int base = H / size;
        const int remainder = H % size;
        const int local_rows = base + (rank < remainder ? 1 : 0);
        const int start_row = rank * base + std::min(rank, remainder);
        const int end_row = start_row + local_rows;

        MPI_Barrier(MPI_COMM_WORLD);  // start every rank's timer together
        const double t0 = MPI_Wtime();
        std::vector<Color> local = cam.renderRowsInitialized(s.world, start_row, end_row);
        const double render_seconds = MPI_Wtime() - t0;

        if (a.integer("tint", 1) != 0) {
            const double alpha = a.real("alpha", 0.08);
            const Color tint = rank_colour(rank);
            for (Color& c : local) {
                c = (1.0 - alpha) * c + alpha * tint;
            }
            // A white line marks where this rank's block begins.
            if (rank > 0) {
                for (int r = 0; r < std::min(2, local_rows); r++) {
                    for (int col = 0; col < W; col++) {
                        local[static_cast<std::size_t>(r) * W + col] = Color(1, 1, 1);
                    }
                }
            }
        }

        std::vector<double> times(rank == 0 ? size : 0);
        MPI_Gather(&render_seconds, 1, MPI_DOUBLE, times.data(), 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        MPI_Datatype mpi_color;
        MPI_Type_contiguous(3, MPI_DOUBLE, &mpi_color);
        MPI_Type_commit(&mpi_color);
        std::vector<int> counts, displs;
        std::vector<Color> full;
        if (rank == 0) {
            counts.resize(size);
            displs.resize(size);
            full.resize(static_cast<std::size_t>(W) * H);
            for (int r = 0; r < size; r++) {
                counts[r] = (base + (r < remainder ? 1 : 0)) * W;
                displs[r] = (r * base + std::min(r, remainder)) * W;
            }
        }
        MPI_Gatherv(local.data(), static_cast<int>(local.size()), mpi_color,
                    rank == 0 ? full.data() : nullptr, rank == 0 ? counts.data() : nullptr,
                    rank == 0 ? displs.data() : nullptr, mpi_color, 0, MPI_COMM_WORLD);
        MPI_Type_free(&mpi_color);

        if (rank == 0) {
            const std::string out = a.str("out", "ranks.ppm");
            sc::write_ppm(cam, full, out);

            const std::string csv = out.substr(0, out.rfind('.')) + "_rank_times.csv";
            std::ofstream f(csv);
            f << "rank,first_row,last_row,rows,render_s\n";
            std::cout << "rank  rows        render (s)\n";
            const double slowest = *std::max_element(times.begin(), times.end());
            for (int r = 0; r < size; r++) {
                const int first = r * base + std::min(r, remainder);
                const int rows = base + (r < remainder ? 1 : 0);
                f << r << ',' << first << ',' << first + rows - 1 << ',' << rows << ','
                  << std::setprecision(6) << times[r] << '\n';
                std::cout << std::setw(4) << r << "  " << std::setw(4) << first << "-" << std::setw(4) << std::left
                          << first + rows - 1 << std::right << "  " << std::fixed << std::setprecision(2)
                          << std::setw(9) << times[r] << "  (" << std::setprecision(0)
                          << 100.0 * times[r] / slowest << "% of slowest)\n";
                std::cout.unsetf(std::ios::fixed);
            }
            std::cout << "Saved " << out << " and " << csv << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "Rank " << rank << " error: " << e.what() << '\n';
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Finalize();
    return 0;
}
