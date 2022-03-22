#include "Configuration.h"

#ifdef SWARM_DEBUG
    bool Configuration::DEBUG = true;
#else
    bool Configuration::DEBUG = false;
#endif

bool Configuration::FORCE_LOCAL = false;

std::string Configuration::REDIS_HOST = "127.0.0.1";
int Configuration::REDIS_PORT = 6379;
std::string Configuration::REDIS_PREFIX = "swarm_";

int Configuration::QUEUE_SLEEP_uS = 1000;
int Configuration::LOCK_SLEEP_uS = 1000;
int Configuration::WAITER_SLEEP_uS = 1000;

size_t Configuration::ENUMERATION_UNROLLING_LIMIT = 200;

bool Configuration::THREAD_EXIT = false;
