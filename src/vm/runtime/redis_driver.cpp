#include <cassert>
#include <sstream>
#include <sw/redis++/redis++.h>
#include "../../errors/InvalidStoreLocationError.h"
#include "../ISA.h"
#include "../walk/BinaryISAWalk.h"
#include "../VirtualMachine.h"
#include "redis_driver.h"
#include <iostream>

namespace swarmc::Runtime::RedisDriver {

    sw::redis::Redis* getRedis() {
        static sw::redis::Redis* redis = nullptr;
        if ( redis == nullptr ) {
            sw::redis::ConnectionOptions opts;
            opts.host = Configuration::REDIS_HOST;
            opts.port = Configuration::REDIS_PORT;
            opts.socket_timeout = std::chrono::milliseconds(0);
            redis = new sw::redis::Redis(opts);
            Framework::onShutdown([]() {
                delete redis;
            });
        }

        return redis;
    }

    bool redisSet(const std::string& key, ISA::Reference* ref, VirtualMachine* vm) {
        auto bin = Wire::references()->reduce(ref, vm);
        std::string s((char*)binn_ptr(bin), binn_size(bin));
        return getRedis()->set(key, s);
    }

    bool redisSet(const std::string& key, Type::Type* ref, VirtualMachine* vm) {
        auto bin = Wire::types()->reduce(ref, vm);
        std::string s((char*)binn_ptr(bin), binn_size(bin));
        return getRedis()->set(key, s);
    }

    binn* redisRead(sw::redis::OptionalString val) {
        if ( !val ) return nullptr;
        std::stringstream ss;

        ss << val.value();
        auto length = val.value().size();

        void* buf = malloc(sizeof(char) * length);
        ss.read(static_cast<char*>(buf), static_cast<std::streamsize>(length));

        return binn_open((char*)buf);
    }

    std::optional<std::string> GlobalServices::getKeyValue(const std::string &key) {
        return getRedis()->get(key);
    }

    void GlobalServices::putKeyValue(const std::string& key, const std::string& value) {
        getRedis()->set(key, value);
    }

    void GlobalServices::dropKeyValue(const std::string& key) {
        getRedis()->del(key);
    }

    ISA::Reference* RedisStorageInterface::load(ISA::LocationReference* loc) {
        auto b = redisRead(_redis->get(Configuration::REDIS_PREFIX + loc->fqName()));
        if ( !b ) throw Errors::InvalidStoreLocationError(s(loc), s(this));
        auto ref = Wire::references()->produce(b, _vm);
        binn_free(b);
        return ref;
    }

    void RedisStorageInterface::store(ISA::LocationReference* loc, ISA::Reference* value) {
        auto typeBinn = redisRead(_redis->get(Configuration::REDIS_PREFIX + "type:" + loc->fqName()));
        Type::Type* type = nullptr;
        if ( !typeBinn ) {
            redisSet(Configuration::REDIS_PREFIX + "type:" + loc->fqName(), value->type(), _vm);
            type = value->type();
        } else {
            type = Wire::types()->produce(typeBinn, _vm);
            binn_free(typeBinn);
        }
        assert(value->type()->isAssignableTo(type));

        redisSet(Configuration::REDIS_PREFIX + loc->fqName(), value, _vm);
    }

    bool RedisStorageInterface::has(ISA::LocationReference* ref) {
        return _redis->exists(Configuration::REDIS_PREFIX + ref->fqName());
    }

    bool RedisStorageInterface::manages(ISA::LocationReference* ref) {
        return ref->affinity() == ISA::Affinity::SHARED;
    }

    void RedisStorageInterface::drop(ISA::LocationReference* ref) {
        _redis->del(Configuration::REDIS_PREFIX + "type:" + ref->fqName());
        _redis->del(Configuration::REDIS_PREFIX + ref->fqName());
    }

    const Type::Type* RedisStorageInterface::typeOf(ISA::LocationReference* ref) {
        auto type = redisRead(_redis->get(Configuration::REDIS_PREFIX + "type:" + ref->fqName()));
        if ( type ) {
            return Wire::types()->produce(type, _vm);
        }
        return nullptr;
    }

    void RedisStorageInterface::typify(ISA::LocationReference* loc, Type::Type* type) {
        redisSet(Configuration::REDIS_PREFIX + "type:" + loc->fqName(), type, _vm);
    }

    void RedisStorageInterface::clear() {
        std::set<std::string> keys;
        auto cursor = 0LL;
        while (true) {
            cursor = _redis->scan(
                cursor,
                Configuration::REDIS_PREFIX + "*",
                10,
                std::inserter(keys, keys.end())
            );
            if ( cursor == 0 ) break;
        }
        for ( const auto& key : keys ) {
            _redis->del(key);
        }
    }

    IStorageInterface* RedisStorageInterface::copy() {
        return new RedisStorageInterface(_vm);
    }

    IStorageLock* RedisStorageInterface::acquire(ISA::LocationReference* ref) {
        if ( _redis->setnx(Configuration::REDIS_PREFIX + "lock:" + ref->fqName(), "true") ) {
            _locks.insert({ ref->fqName(), new RedisStorageLock(this, ref) });
            return _locks[ref->fqName()];
        }
        return nullptr;
    }

    void RedisStorageLock::release() {
        _store->_locks.erase(_loc->fqName());
        getRedis()->del(Configuration::REDIS_PREFIX + "lock:" + _loc->fqName());
    }

    void RedisQueue::setContext(QueueContextID context) {
        // lock on the queue shouldn't be need like it was in multithreaded bc we arent sharing the same
        // storage object with multiple threads. Same goes for getContext
        if ( _context != context ) {
            _context = context;
            _redis->hsetnx(Configuration::REDIS_PREFIX + "contextProgress", _context, "0");
        }
    }

    QueueContextID RedisQueue::getContext() {
        return _context;
    }

    IQueueJob* RedisQueue::build(VirtualMachine* vm, IFunctionCall* call) {
        auto id = _redis->incr(Configuration::REDIS_PREFIX + "nextJobID");
        auto dummyLocal = ISA::LocationReference(ISA::Affinity::LOCAL, "dummy");
        return new RedisQueueJob(
            id,
            Wire::calls()->reduce(call, vm),
            Wire::states()->reduce(vm->getState(), vm),
            Wire::scopes()->reduce(vm->getScopeFrame(), vm),
            Wire::stores()->reduce(vm->getStore(&dummyLocal), vm)
        );
    }

    void RedisQueue::push(VirtualMachine* vm, IQueueJob* job) {
        // We should only accept RedisQueueJobs anyway, cast so I can get its important members
        auto rjob = dynamic_cast<RedisQueueJob*>(job);
        // convert binn to strings to be pushed to redis
        std::string callstr((char*)binn_ptr(rjob->getCallBinn()), binn_size(rjob->getCallBinn()));
        std::string statestr((char*)binn_ptr(rjob->getStateBinn()), binn_size(rjob->getStateBinn()));
        std::string scopestr((char*)binn_ptr(rjob->getScopeBinn()), binn_size(rjob->getScopeBinn()));
        std::string storestr((char*)binn_ptr(rjob->getLocalStoreBinn()), binn_size(rjob->getLocalStoreBinn()));

        std::unordered_map<std::string, std::string> m = {
            {"ID", s(job->id())},
            {"Call", callstr},
            {"VMState", statestr},
            {"VMScope", scopestr},
            {"LocalStore", storestr}
        };
        _redis->hset(Configuration::REDIS_PREFIX + "job_" + m["ID"], m.begin(), m.end());
        _redis->set(Configuration::REDIS_PREFIX + "status_" + m["ID"], s(JobState::PENDING), Configuration::REDIS_DEFAULT_TLL);
        _redis->lpush(Configuration::REDIS_PREFIX + "queue_" + _context, Configuration::REDIS_PREFIX + "job_" + m["ID"]);
    }

    IQueueJob* RedisQueue::pop() {
        return popFromContext(_context);
    }

    bool RedisQueue::isEmpty(QueueContextID id) {
        return !_redis->exists(Configuration::REDIS_PREFIX + "queue_" + id) && finished(id);
    }

    void RedisQueue::tick() {
        tryToProcessJob();
        Framework::tick();
    }

    void RedisQueue::tryToProcessJob() {
        auto job = tryGetJob();
        // we have to restore the vm, so we need a Jobject that contains State and ScopeFrame info
        auto rjob = dynamic_cast<RedisQueueJob*>(job.first);

        if ( job.first != nullptr ) {
            _vm->enterQueueContext(job.second);
            try {
                Console::get()->debug("Running job: " + s(rjob));

                ISA::Reference* ret = nullptr;
                _vm->copy([rjob, &ret](VirtualMachine* vm) -> void {
                    vm->restore(nullptr, Wire::states()->produce(rjob->getStateBinn(), vm));
                    vm->restore(Wire::scopes()->produce(rjob->getScopeBinn(), vm));
                    vm->addStore(Wire::stores()->produce(rjob->getLocalStoreBinn(), vm));
                    auto call = useref(Wire::calls()->produce(rjob->getCallBinn(), vm));
                    vm->executeCall(call);
                    if ( call->returnType()->intrinsic() != Type::Intrinsic::VOID ) {
                        ret = call->getReturn();
                    }
                });

                setJobReturn(job.second, rjob->id(), ret);
                rjob->setState(JobState::COMPLETE);
            } catch (Errors::SwarmError& e) {
                Console::get()->error(e.what());
                rjob->setState(JobState::ERROR);
            }
            _vm->exitQueueContext();
            _redis->hincrby(Configuration::REDIS_PREFIX + "contextProgress", job.second, -1);
            delete rjob;
        }
    }

    std::pair<IQueueJob*, QueueContextID> RedisQueue::tryGetJob() {
        // attempt popping from context
        auto incontext = popFromContext(_context);
        std::string contextProgress = Configuration::REDIS_PREFIX + "contextProgress";
        if ( !incontext ) {
            // scan for other contexts
            auto cursor = 0LL;
            while ( true ) {
                std::vector<std::string> contexts;
                cursor = _redis->hscan(
                    contextProgress,
                    cursor,
                    "*",
                    10,
                    std::inserter(contexts, contexts.begin())
                );
                for ( auto context : contexts ) {
                    if ( auto job = popFromContext(context) ) {
                        _redis->hincrby(contextProgress, context, 1);
                        return { job, context };
                    }
                }
                if ( cursor == 0 ) break;
            }
            return { nullptr, _context };
        }
        _redis->hincrby(contextProgress, _context, 1);
        return { incontext, _context };
    }

    IQueueJob* RedisQueue::popFromContext(const QueueContextID& context) {
        auto job = _redis->rpop(Configuration::REDIS_PREFIX + "queue_" + context);
        if ( !job ) return nullptr;
        std::map<std::string, std::string> jobValues;
        _redis->hgetall(job.value(), std::inserter(jobValues, jobValues.begin()));
        // for ( const auto& p : jobValues ) {
        //     _redis->hdel(job.value(), p.first);
        // }

        auto qjob = new RedisQueueJob(
            static_cast<JobID>(std::atoi(jobValues["ID"].c_str())),
            redisRead(jobValues["Call"]),
            redisRead(jobValues["VMState"]),
            redisRead(jobValues["VMScope"]),
            redisRead(jobValues["LocalStore"])
        );

        return qjob;
    }

    bool RedisQueue::finished(const QueueContextID& context) {
        // return true if inProgress is 0 or if context doesnt exist
        auto val = _redis->hget(Configuration::REDIS_PREFIX + "contextProgress", context);
        return val.value_or("0") == "0";
    }

    Stream::~Stream() noexcept {
        clearKeys();
        while ( !_redis->exists(Configuration::REDIS_PREFIX + _id) ) {
            _redis->rpop(Configuration::REDIS_PREFIX + _id);
        }
    }

    void Stream::open() {
        setKeys();
    }

    void Stream::close() {
        clearKeys();
    }

    void Stream::setKeys() {
        auto bin = Wire::types()->reduce(_innerType, _vm);
        std::string s((char*)binn_ptr(bin), binn_size(bin));

        if ( _redis->setnx(Configuration::REDIS_PREFIX + _id + "_type", s) ) {
            _redis->setnx(Configuration::REDIS_PREFIX + _id + "_open", "true");
        }
    }

    void Stream::clearKeys() {
        _redis->del(Configuration::REDIS_PREFIX + _id + "_type");
        _redis->del(Configuration::REDIS_PREFIX + _id + "_open");
    }

    void Stream::push(ISA::Reference* val) {
        auto bin = Wire::references()->reduce(val, _vm);
        std::string s((char*)binn_ptr(bin), binn_size(bin));
        _redis->lpush(Configuration::REDIS_PREFIX + _id, s);
    }

    ISA::Reference* Stream::pop() {
        assert(!isEmpty());
        auto ref = redisRead(_redis->rpop(Configuration::REDIS_PREFIX + _id));
        return Wire::references()->produce(ref, _vm);
    }

    std::string Stream::toString() const {
        return "RedisDriver::Stream<of: " + _innerType->toString() + ", id: " + _id + ">";
    }

    IStream* RedisStreamDriver::open(const std::string &id, Type::Type* innerType) {
        return new Stream(id, innerType, _vm);
    }
}
