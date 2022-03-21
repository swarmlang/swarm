#ifndef SWARM_CONFIGURATION_H
#define SWARM_CONFIGURATION_H

#include <string>

class Configuration {
public:
    static bool DEBUG;
    static bool FORCE_LOCAL;

    static std::string REDIS_HOST;
    static int REDIS_PORT;
    static std::string REDIS_PREFIX;

    static int QUEUE_SLEEP_uS;
    static int LOCK_SLEEP_uS;
    static int WAITER_SLEEP_uS;

    static size_t ENUMERATION_UNROLLING_LIMIT;

    static bool THREAD_EXIT;
};

#endif //SWARM_CONFIGURATION_H
