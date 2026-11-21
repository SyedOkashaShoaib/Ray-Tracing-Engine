#include <iostream>
#include "scenes/benchmark_material_garden.h"
#include "scenes/benchmark_sphereflake.h"
int main() {

#if defined(BENCHMARK_A)

    benchmark_material_garden(1280, 128);

#elif defined(BENCHMARK_B)

    benchmark_sphereflake(4, 1280, 128);

#else

    std::cerr << "No benchmark selected.\n";
    return 1;

#endif

    return 0;
}
