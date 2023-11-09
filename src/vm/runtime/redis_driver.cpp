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

    ISA::Reference* RedisStorageInterface::load(ISA::LocationReference* loc) {
        auto b = redisRead(_redis->get(Configuration::REDIS_PREFIX + loc->fqName()));
        if ( !b ) throw Errors::InvalidStoreLocationError(s(loc), s(this));
        auto ref = Wire::references()->produce(b, _vm);
        binn_free(b);
        return ref;
    }

    void RedisStorageInterface::store(ISA::LocationReference* loc, ISA::Reference* value) {
        auto type = redisRead(_redis->get(Configuration::REDIS_PREFIX + "type:" + loc->fqName()));
        if ( !type ) {
            redisSet(Configuration::REDIS_PREFIX + "type:" + loc->fqName(), value->type(), _vm);
        }
        auto b = redisRead(_redis->get(Configuration::REDIS_PREFIX + "type:" + loc->fqName()));
        assert(value->typei()->isAssignableTo(Wire::types()->produce(b, _vm)));
        binn_free(b);

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
        _context = context;
        _redis->hsetnx(Configuration::REDIS_PREFIX + "contexts", _context, "true");
    }

    QueueContextID RedisQueue::getContext() {
        return _context;
    }

    IQueueJob* RedisQueue::build(VirtualMachine* vm, IFunctionCall* call) {
        auto id = _redis->incr(Configuration::REDIS_PREFIX + "nextJobID");
        auto dummyLocal = ISA::LocationReference(ISA::Affinity::LOCAL, "dummy");
        return new RedisQueueJob(
            id,
            JobState::PENDING,
            Wire::calls()->reduce(call, vm),
            vm->getState()->copy(),
            vm->getScopeFrame()->copy(),
            vm->getStore(&dummyLocal)
        );
    }

    void RedisQueue::push(VirtualMachine* vm, IQueueJob* job) {
        // We should only accept RedisQueueJobs anyway, cast so I can get its important members
        auto rjob = dynamic_cast<RedisQueueJob*>(job);
        std::string callstr((char*)binn_ptr(rjob->getCallBin()), binn_size(rjob->getCallBin()));
        auto statebin = Wire::states()->reduce(rjob->getVMState(), vm);
        std::string statestr((char*)binn_ptr(statebin), binn_size(statebin));
        auto scopebin = Wire::scopes()->reduce(rjob->getVMScope(), vm);
        std::string scopestr((char*)binn_ptr(scopebin), binn_size(scopebin));
        auto storebin = Wire::stores()->reduce(rjob->getLocalStore(), vm);
        std::string storestr((char*)binn_ptr(storebin), binn_size(storebin));
        binn_free(statebin);
        binn_free(scopebin);
        binn_free(storebin);

        std::unordered_map<std::string, std::string> m = {
            {"ID", std::to_string(job->id())},
            {"JobState", std::to_string(static_cast<std::size_t>(job->state()))},
            {"Call", callstr},
            {"VMState", statestr},
            {"VMScope", scopestr},
            {"LocalStore", storestr}
        };
        _redis->hmset(Configuration::REDIS_PREFIX + "job_" + m["ID"], m.begin(), m.end());
        _redis->lpush(Configuration::REDIS_PREFIX + "queue_" + _context, Configuration::REDIS_PREFIX + "job_" + m["ID"]);
    }

    IQueueJob* RedisQueue::pop() {
        return popFromContext(_context);
    }

    bool RedisQueue::isEmpty(QueueContextID id) {
        return !_redis->exists(Configuration::REDIS_PREFIX + "queue_" + _context)
            && _redis->hget(Configuration::REDIS_PREFIX + "contextProgress", _context).value_or("0") == "0";
    }

    void RedisQueue::tick() {
        tryToProcessJob();
        Framework::tick();
    }

    void RedisQueue::tryToProcessJob() {
        auto job = tryGetJob();
        // we have to restore the vm, so we need a Jobject that contains State and ScopeFrame info
        auto redisjob = dynamic_cast<RedisQueueJob*>(job.first);
        if ( job.first != nullptr ) {
            // FIXME: setting the job state doesn't really do anything here,
            // because the jobjects are created here by deserializing redis.
            // Compare to how the multithreaded driver uses the same jobject
            // both before pushing and after popping because it doesn't
            // have to serialize anythin into a database
            try {
                Console::get()->debug("Running job: " + s(redisjob));
                _vm->copy([redisjob](VirtualMachine* vm) -> void {
                    vm->restore(redisjob->getVMScope(), redisjob->getVMState());
                    auto call = useref(Wire::calls()->produce(redisjob->getCallBin(), vm));
                    vm->executeCall(call);
                });
                redisjob->setState(JobState::COMPLETE);
            } catch (Errors::SwarmError& e) {
                Console::get()->error(e.what());
                redisjob->setState(JobState::ERROR);
            } catch (...) {
                Console::get()->error("Unknown error!");
                redisjob->setState(JobState::ERROR);
            }
            _redis->hincrby(Configuration::REDIS_PREFIX + "contextProgress", job.second, -1);
            delete redisjob;
        }
    }

    std::pair<IQueueJob*, QueueContextID> RedisQueue::tryGetJob() {
        // attempt popping from context
        auto incontext = popFromContext(_context);
        if ( !incontext ) {
            // scan for other contexts
            auto cursor = 0LL;
            while ( true ) {
                std::vector<std::string> contexts;
                cursor = _redis->hscan(
                    Configuration::REDIS_PREFIX + "contexts",
                    cursor,
                    "*",
                    10,
                    std::inserter(contexts, contexts.begin())
                );
                for ( auto context : contexts ) {
                    if ( auto job = popFromContext(context) ) {
                        _redis->hincrby(Configuration::REDIS_PREFIX + "contextProgress", context, 1);
                        return { job, context };
                    }
                }
                if ( cursor == 0 ) break;
            }
            return { nullptr, _context };
        }
        _redis->hincrby(Configuration::REDIS_PREFIX + "contextProgress", _context, 1);
        return { incontext, _context };
    }

    IQueueJob* RedisQueue::popFromContext(const QueueContextID& context) {
        auto job = _redis->rpop(Configuration::REDIS_PREFIX + "queue_" + context);
        if ( !job ) return nullptr;
        std::map<std::string, std::string> jobValues;
        _redis->hgetall(job.value(), std::inserter(jobValues, jobValues.begin()));
        for ( auto p : jobValues ) {
            _redis->hdel(job.value(), p.first);
        }

        auto statebin = redisRead(jobValues["VMState"]);
        auto scopebin = redisRead(jobValues["VMScope"]);
        auto storebin = redisRead(jobValues["LocalStore"]);
        auto qjob = new RedisQueueJob(
            static_cast<JobID>(std::atoi(jobValues["ID"].c_str())),
            static_cast<JobState>(std::atoi(jobValues["JobState"].c_str())),
            redisRead(jobValues["Call"]),
            Wire::states()->produce(statebin, _vm),
            Wire::scopes()->produce(scopebin, _vm),
            Wire::stores()->produce(storebin, _vm)
        );
        binn_free(statebin);
        binn_free(scopebin);
        binn_free(storebin);

        return qjob;
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
