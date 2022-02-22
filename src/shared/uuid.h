#ifndef SWARMC_UUID_H
#define SWARMC_UUID_H

#include <random>
#include <sstream>

namespace util {
    // static std::random_device device;
    static std::mt19937_64 generator;
    static std::uniform_int_distribution<> distribution(0, 15);
    static std::uniform_int_distribution<> distribution2(8, 11);

    static size_t DETERMINISTIC_UUID_LAST = 0;
    static bool USE_DETERMINISTIC_UUIDS = false;

    /** Adapted from https://stackoverflow.com/questions/24365331, modified to use mersenne x64 */
    inline std::string uuid4() {
        if ( USE_DETERMINISTIC_UUIDS ) {
            return "d-guid-" + std::to_string(DETERMINISTIC_UUID_LAST++);
        }

        std::stringstream s;
        int i;

        s << std::hex;

        for ( i = 0; i < 8; i++ ) {
            s << distribution(generator);
        }

        s << "-";

        for ( i = 0; i < 4; i++ ) {
            s << distribution(generator);
        }

        s << "-4";

        for ( i = 0; i < 3; i++ ) {
            s << distribution(generator);
        }

        s << "-" << distribution2(generator);

        for ( i = 0; i < 3; i++ ) {
            s << distribution(generator);
        }

        s << "-";

        for ( i = 0; i < 12; i++ ) {
            s << distribution(generator);
        }

        return s.str();
    }
}

#endif
