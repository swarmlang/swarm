#include "Configuration.h"

#ifdef SWARM_DEBUG
    bool Configuration::DEBUG = true;
#else
    bool Configuration::DEBUG = false;
#endif

bool Configuration::VERBOSE = false;
bool Configuration::FORCE_LOCAL = false;

bool Configuration::WITH_PROLOGUE = true;

int Configuration::REDIS_PORT = 6379;

int Configuration::QUEUE_SLEEP_uS = 1000;
int Configuration::DEBUG_QUEUE_SLEEP_uS = 1000000;
int Configuration::LOCK_SLEEP_uS = 1000;
int Configuration::LOCK_MAX_RETRIES = 1000000;
int Configuration::WAITER_SLEEP_uS = 1000;

std::size_t Configuration::ENUMERATION_UNROLLING_LIMIT = 200;

bool Configuration::THREAD_EXIT = false;

std::map<std::string, std::string> Configuration::QUEUE_FILTERS;

std::string Configuration::DEBUG_SERVER_DATA_PATH = "/tmp/swarm-debug.sock";
std::string Configuration::DEBUG_SERVER_CMD_PATH = "/tmp/swarm-command.sock";

size_t Configuration::MAX_THREADS = 4;