#ifndef SWARM_CONFIGURATION_H
#define SWARM_CONFIGURATION_H

#include <string>
#include <map>

class Configuration {
public:
    static bool DEBUG;
    static bool VERBOSE;
    static bool FORCE_LOCAL;

    static bool WITH_PROLOGUE;

    inline static const std::string REDIS_HOST = "localhost";
    static int REDIS_PORT;
    inline static const std::string REDIS_PREFIX = "swarm_";

    static int QUEUE_SLEEP_uS;
    static int DEBUG_QUEUE_SLEEP_uS;
    static int LOCK_SLEEP_uS;
    static int LOCK_MAX_RETRIES;
    static int WAITER_SLEEP_uS;

    static size_t ENUMERATION_UNROLLING_LIMIT;

    static bool THREAD_EXIT;
    static std::map<std::string, std::string> QUEUE_FILTERS;
};

#endif //SWARM_CONFIGURATION_H
