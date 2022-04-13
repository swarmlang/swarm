#include "Configuration.h"

#ifdef SWARM_DEBUG
    bool Configuration::DEBUG = true;
#else
    bool Configuration::DEBUG = false;
#endif

bool Configuration::FORCE_LOCAL = false;

int Configuration::REDIS_PORT = 6379;

int Configuration::QUEUE_SLEEP_uS = 1000;
int Configuration::DEBUG_QUEUE_SLEEP_uS = 1000000;
int Configuration::LOCK_SLEEP_uS = 1000;
int Configuration::WAITER_SLEEP_uS = 1000;

size_t Configuration::ENUMERATION_UNROLLING_LIMIT = 200;

bool Configuration::THREAD_EXIT = false;

std::map<std::string, std::string> Configuration::QUEUE_FILTERS;