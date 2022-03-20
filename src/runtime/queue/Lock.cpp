#include "Lock.h"
#include "ExecutionQueue.h"

using namespace swarmc::Runtime;

bool Lock::tryToAcquire() {
    return ExecutionQueue::getRedis()->setnx("lock:" + _name, _uuid);
}

void Lock::release() {
    if ( !held() ) return;

    _holders -= 1;

    if ( !held() ) {
        auto redis = ExecutionQueue::getRedis();
        auto result = redis->get("lock:" + _name);
        if ( result && (*result) == _uuid ) {
            redis->del({"lock:" + _name});
            _manager->_locks->erase(_name);
        }
    }
}
