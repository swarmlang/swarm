#ifndef SWARM_RAND_H
#define SWARM_RAND_H

#include <limits>
#include <random>

double randomDouble() {
    static std::random_device rd;
    static std::default_random_engine eng(rd());
    static std::uniform_real_distribution<double> distribution(
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::min()
    );

    return distribution(eng);
}

#endif //SWARM_RAND_H
