#ifndef SWARM_RAND_H
#define SWARM_RAND_H

#include <limits>
#include <random>

double randomDouble() {
    static std::random_device rd;
    static std::default_random_engine eng(rd());
    static std::uniform_real_distribution<double> distribution(
        1,
        0
    );

    return distribution(eng);
}

#endif //SWARM_RAND_H
