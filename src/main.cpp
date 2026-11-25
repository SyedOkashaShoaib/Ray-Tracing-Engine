
#include <chrono>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>

#if defined(RT_USE_MPI)
#include <mpi.h>
#endif

#include "scenes/benchmark_material_garden.h"
#include "scenes/benchmark_sphereflake.h"

// Default benchmark configuration
#ifndef RT_BENCH_WIDTH
#define RT_BENCH_WIDTH 1280
#endif

#ifndef RT_BENCH_SPP
#define RT_BENCH_SPP 128
#endif

int main(int argc, char** argv)
{
    const auto total_start = std::chrono::steady_clock::now();

    int rank = 0;

#if defined(RT_USE_MPI)

    int provided = 0;

    MPI_Init_thread(
        &argc,
        &argv,
        MPI_THREAD_FUNNELED,
        &provided
    );

    if (provided < MPI_THREAD_FUNNELED) {
        std::cerr << "Insufficient MPI thread support\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#endif



    std::ofstream rank_timing_file;
    std::streambuf* original_clog_buffer = nullptr;

    const char* timing_prefix = std::getenv("RT_TIMING_PREFIX");

    if (timing_prefix != nullptr && *timing_prefix != '\0') {

        const std::string filename =
            std::string(timing_prefix)
            + "_rank"
            + std::to_string(rank)
            + ".log";

        rank_timing_file.open(filename);

        if (!rank_timing_file) {
            std::cerr << "Cannot open timing log: "
                      << filename << '\n';

#if defined(RT_USE_MPI)
            MPI_Abort(MPI_COMM_WORLD, 1);
#endif
            return 1;
        }

        original_clog_buffer =
            std::clog.rdbuf(rank_timing_file.rdbuf());
    }
    int status = 0;

#if defined(BENCHMARK_A)

    benchmark_material_garden(
        RT_BENCH_WIDTH,
        RT_BENCH_SPP
    );

#elif defined(BENCHMARK_B)

    benchmark_sphereflake(
        4,
        RT_BENCH_WIDTH,
        RT_BENCH_SPP
    );

#else

    std::cerr << "No benchmark selected.\n";
    status = 1;

#endif

#if defined(RT_USE_MPI)
    MPI_Finalize();
#endif
    const double total_seconds =
        std::chrono::duration<double>(
            std::chrono::steady_clock::now() - total_start
        ).count();

    std::clog << std::fixed << std::setprecision(9)
              << "RT_TIMING component=application"
              << " rank=" << rank
              << " total_s=" << total_seconds
              << '\n';

    std::clog.flush();

    if (original_clog_buffer != nullptr) {

        std::clog.rdbuf(original_clog_buffer);

        rank_timing_file.close();

        if (!rank_timing_file) {
            std::cerr << "Failed to finalize timing log\n";
            return 1;
        }
    }

    return status;
}
