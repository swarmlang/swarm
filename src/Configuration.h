#ifndef SWARM_CONFIGURATION_H
#define SWARM_CONFIGURATION_H

#include <string>
#include <map>

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
    static std::map<std::string, std::string> QUEUE_FILTERS;
};

#endif //SWARM_CONFIGURATION_H
