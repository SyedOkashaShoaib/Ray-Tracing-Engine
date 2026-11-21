#ifndef RNG_H
#define RNG_H

#include <cstdint>
/*our very own custom random number generator!
Why does the paralel version need a custom rng when we were rand in the serial version?
rand returns pseudorandom nums. it keeps an internal state, and when we call rand, that internal
state is advanced, and we get the next pseudo random number. meaning that with the same seed, we get the same 
sequence of pseudorand nums. this is not an issue in serial code. but in parallel code, the same thread may not
get the same random nums every time. this means that we cant 100% reproduce an image. which means that our benchmakrs 
wont be accirate. furthermore, multiple threads waiting for the same rng can lead to waiting. with this class each thread gets
an rng obj. no waiting! */

class RNG {
public:
    explicit RNG(std::uint64_t seed)
        : state(seed) {}

    std::uint64_t next() { //get the next state
        std::uint64_t z =
            (state += 0x9E3779B97F4A7C15ULL);

        z = (z ^ (z >> 30)) *
            0xBF58476D1CE4E5B9ULL;

        z = (z ^ (z >> 27)) *
            0x94D049BB133111EBULL;

        return z ^ (z >> 31);
    }

    double uniform() { //clamp it between 0-1
        return (next() >> 11) *
               (1.0 / 9007199254740992.0);
    }

    double uniform(double min, double max) {
        return min + (max - min) * uniform();
    }

private:
    std::uint64_t state;
};

#endif