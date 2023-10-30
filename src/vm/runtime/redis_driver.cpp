#include <cassert>
#include <sstream>
#include <sw/redis++/redis++.h>
#include "../../errors/InvalidStoreLocationError.h"
#include "../ISA.h"
#include "../walk/BinaryISAWalk.h"
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

    bool redisSet(std::string key, ISA::Reference* ref, VirtualMachine* vm) {
        auto bin = Wire::references()->reduce(ref, vm);
        std::string s((char*)binn_ptr(bin), binn_size(bin));
        return getRedis()->set(key, s);
    }

    bool redisSet(std::string key, Type::Type* ref, VirtualMachine* vm) {
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
        ss.read(static_cast<char*>(buf), length);

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
        for ( auto key : keys ) {
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

    std::string RedisStorageLock::toString() const {
        return "RedisDriver::RedisStorageLock<loc: " + _loc->toString() + ">";
    }

    Stream::~Stream() noexcept {
        close();
        while ( !_redis->exists(Configuration::REDIS_PREFIX + _id) ) {
            _redis->rpop(Configuration::REDIS_PREFIX + _id);
        }
    }

    void Stream::open() {
        // I imagine if the stream is already open, it doesn't matter. That just means
        // someone else opened it
        auto bin = Wire::types()->reduce(_innerType, _vm);
        std::string s((char*)binn_ptr(bin), binn_size(bin));

        if ( _redis->setnx(Configuration::REDIS_PREFIX + _id + "_type", s) ) {
            _redis->setnx(Configuration::REDIS_PREFIX + _id + "_open", "true");
        }
    }

    void Stream::close() {
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
