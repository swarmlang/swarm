#ifndef SWARMVM_REDIS_DRIVER_H
#define SWARMVM_REDIS_DRIVER_H

#include <sw/redis++/redis++.h>
#include "interfaces.h"
#include "single_threaded.h"
#include "../../Configuration.h"

namespace swarmc::Runtime {
    class VirtualMachine;
}

namespace swarmc::Runtime::RedisDriver {
    class RedisStorageLock;

    sw::redis::Redis* getRedis();
    bool redisSet(std::string, ISA::Reference*, VirtualMachine*);
    bool redisSet(std::string, Type::Type*, VirtualMachine*);
    binn* redisRead(sw::redis::OptionalString);

    class GlobalServices : public SingleThreaded::GlobalServices {
    public:
        std::string getNodeId() override {
            return "??";
        }

        [[nodiscard]] std::string toString() const override {
            return "RedisDriver::GlobalServices<>";
        }
    };

    class RedisStorageInterface : public IStorageInterface {
    public:
        explicit RedisStorageInterface(VirtualMachine* vm) : _redis(getRedis()), _vm(vm) {}
        /** Load the actual value of a variable. */
        virtual ISA::Reference* load(ISA::LocationReference*) override;

        /** Store a value into a variable. */
        virtual void store(ISA::LocationReference*, ISA::Reference*) override;

        /** Returns true if this backend has a value for the given variable. */
        virtual bool has(ISA::LocationReference*) override;

        /** Returns true if this backend should be used to lock/store the given variable. */
        virtual bool manages(ISA::LocationReference*) override;

        /** Make the backend forget the value of the given variable, if it exists. */
        virtual void drop(ISA::LocationReference*) override;

        /** Get the type of the given location. */
        virtual const Type::Type* typeOf(ISA::LocationReference*) override;

        /** Constrain the given location to a specific type. */
        virtual void typify(ISA::LocationReference*, Type::Type*) override;

        virtual IStorageLock* acquire(ISA::LocationReference*) override;

        /** Forget all stored variables. */
        virtual void clear() override;

        virtual IStorageInterface* copy() override;

        [[nodiscard]] std::string toString() const override {
            return "RedisDriver::RedisStorageInterface<>";
        }
    protected:
        sw::redis::Redis* _redis;
        VirtualMachine* _vm;
        std::unordered_map<std::string, RedisStorageLock*> _locks;

        friend class RedisStorageLock;
    };

    class RedisStorageLock : public IStorageLock {
    public:
        RedisStorageLock(RedisStorageInterface* store, ISA::LocationReference* loc) : _store(store), _loc(loc) {}

        [[nodiscard]] ISA::LocationReference* location() const override { return _loc; }

        void release() override;

        [[nodiscard]] std::string toString() const override;
    protected:
        RedisStorageInterface* _store = nullptr;
        ISA::LocationReference* _loc = nullptr;
    };

    class Stream : public IStream {
    public:
        Stream(std::string id, Type::Type* innerType, VirtualMachine* vm) : _id(std::move(id)), _innerType(std::move(innerType)), _vm(vm) {
            open();
        }

        ~Stream() noexcept override;

        void open() override;

        void close() override;

        bool isOpen() override { return _redis->exists(Configuration::REDIS_PREFIX + _id + "_open"); }

        Type::Type* innerType() override { return _innerType; }

        void push(ISA::Reference* value) override;

        ISA::Reference* pop() override;

        bool isEmpty() override { return !_redis->exists(Configuration::REDIS_PREFIX + _id); }

        [[nodiscard]] std::string id() const override { return Configuration::REDIS_PREFIX + _id; }

        [[nodiscard]] std::string toString() const override;
    protected:
        std::string _id;
        Type::Type* _innerType;
        sw::redis::Redis* _redis = getRedis();
        VirtualMachine* _vm;
    };

    class RedisStreamDriver : public IStreamDriver {
    public:
        explicit RedisStreamDriver(VirtualMachine* vm) : _vm(vm) {}

        IStream* open(const std::string& id, Type::Type* innerType) override;

        [[nodiscard]] std::string toString() const override {
            return "RedisDriver::StreamDriver<>";
        }
    protected:
        VirtualMachine* _vm;
    };

}

#endif
