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

        binn* b = binn_open((char*)buf + 4);
        free(buf);
        return b;
    }

    ISA::Reference* RedisStorageInterface::load(ISA::LocationReference* loc) {
        auto b = redisRead(_redis->get(loc->fqName()));
        std::cout << "getting value of " << loc->fqName() << std::endl;
        if ( !b ) throw Errors::InvalidStoreLocationError(s(loc), s(this));
        // Please dear god let it be this easy
        auto ref = Wire::references()->produce(b, _vm);
        binn_free(b);
        return ref;
    }

    void RedisStorageInterface::store(ISA::LocationReference* loc, ISA::Reference* value) {
        auto type = redisRead(_redis->get("type:" + loc->fqName()));
        if ( !type ) {
            redisSet("type:" + loc->fqName(), value->type(), _vm);
        }
        auto b = redisRead(_redis->get("type:" + loc->fqName()));
        auto zoinks = Wire::types()->produce(b, _vm);
        std::cout << zoinks->toString() << std::endl;
        assert(value->typei()->isAssignableTo(zoinks));
        binn_free(b);

        redisSet(loc->fqName(), value, _vm);
        std::cout << "stored " << loc->fqName() << std::endl;
    }

    bool RedisStorageInterface::has(ISA::LocationReference* ref) {
        return _redis->exists(ref->fqName());
    }

    bool RedisStorageInterface::manages(ISA::LocationReference* ref) {
        return ref->affinity() == ISA::Affinity::SHARED;
    }

    void RedisStorageInterface::drop(ISA::LocationReference* ref) {
        _redis->del("type:" + ref->fqName());
        _redis->del(ref->fqName());
        std::cout << "dropped " << ref->fqName() << std::endl;
    }

    const Type::Type* RedisStorageInterface::typeOf(ISA::LocationReference* ref) {
        auto type = redisRead(_redis->get("type:" + ref->fqName()));
        if ( type ) {
            return Wire::types()->produce(type, _vm);
        }
        return nullptr;
    }

    void RedisStorageInterface::typify(ISA::LocationReference* loc, Type::Type* type) {
        redisSet("type:" + loc->fqName(), type, _vm);
    }

    void RedisStorageInterface::clear() {
        // This interface could only possibly keep track of variables that it has interacted with before
        // So I'm not sure how to clear out the entire store atm unless I can search for partial keys in redis
    }

    IStorageInterface* RedisStorageInterface::copy() {
        // maybe `return this` instead? Only really matters if we need to copy locks since thats
        // all thats in here atm
        return new RedisStorageInterface(_vm);
    }

    IStorageLock* RedisStorageInterface::acquire(ISA::LocationReference* ref) {
        if ( _redis->setnx("lock:" + ref->fqName(), "true") ) {
            std::cout << "acquiring lock for " << ref->fqName() << std::endl;
            _locks.insert({ ref->fqName(), new RedisStorageLock(this, ref) });
            return _locks[ref->fqName()];
        }
        return nullptr;
    }

    void RedisStorageLock::release() {
        _store->_locks.erase(_loc->fqName());
        getRedis()->del("lock:" + _loc->fqName());
        std::cout << "releasing lock for " << _loc->fqName() << std::endl;
    }

    std::string RedisStorageLock::toString() const {
        return "RedisDriver::RedisStorageLock<loc: " + _loc->toString() + ">";
    }

    Stream::~Stream() noexcept {
        close();
        while ( !_redis->exists(_id) ) {
            _redis->rpop(_id);
        }
    }

    void Stream::open() {
        // I imagine if the stream is already open, it doesn't matter. That just means
        // someone else opened it
        std::cout << "opening stream " << _id << std::endl;

        auto bin = Wire::types()->reduce(_innerType, _vm);
        std::string s((char*)binn_ptr(bin), binn_size(bin));

        if ( _redis->setnx(_id + "_type", s) ) {
            _redis->setnx(_id + "_open", "true");
        }
    }

    void Stream::close() {
        std::cout << "closing stream " << _id << std::endl;
        _redis->del(_id + "_type");
        _redis->del(_id + "_open");
    }

    void Stream::push(ISA::Reference* val) {
        auto bin = Wire::references()->reduce(val, _vm);
        std::string s((char*)binn_ptr(bin), binn_size(bin));
        std::cout << "pushing " << val->toString() << " to stream " << _id << std::endl;
        _redis->lpush(_id, s);
    }

    ISA::Reference* Stream::pop() {
        std::cout << "popping from stream " << _id << std::endl;
        assert(!isEmpty());
        auto ref = redisRead(_redis->rpop(_id));
        return Wire::references()->produce(ref, _vm);
    }

    std::string Stream::toString() const {
        return "RedisDriver::Stream<of: " + _innerType->toString() + ">";
    }

    IStream* RedisStreamDriver::open(const std::string &id, Type::Type* innerType) {
        return new Stream(id, innerType, _vm);
    }
}