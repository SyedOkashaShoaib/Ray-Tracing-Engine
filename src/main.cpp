#include <iostream>

#if defined(RT_USE_MPI)
#include <mpi.h>
#endif

#include "scenes/benchmark_material_garden.h"
#include "scenes/benchmark_sphereflake.h"

int main(int argc, char** argv) {

#if defined(RT_USE_MPI)

    int provided = 0;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);

    if (provided < MPI_THREAD_FUNNELED) {
        std::cerr << "Insufficient MPI thread support\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }

#endif

    int status = 0;

#if defined(BENCHMARK_A)

    benchmark_material_garden(1280, 128);

#elif defined(BENCHMARK_B)

    benchmark_sphereflake(4, 1280, 128);

#else

    std::cerr << "No benchmark selected.\n";
    status = 1;

#endif

#if defined(RT_USE_MPI)
    MPI_Finalize();
#endif

    return status;
}