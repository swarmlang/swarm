#ifndef SWARMVM_REDIS_DRIVER_H
#define SWARMVM_REDIS_DRIVER_H

#include <sw/redis++/redis++.h>
#include "interfaces.h"
#include "single_threaded.h"
#include "State.h"
#include "../../Configuration.h"

namespace swarmc::Runtime {
    class VirtualMachine;
}

namespace swarmc::Runtime::RedisDriver {
    class RedisStorageLock;

    sw::redis::Redis* getRedis();
    bool redisSet(const std::string&, ISA::Reference*, VirtualMachine*);
    bool redisSet(const std::string&, Type::Type*, VirtualMachine*);
    binn* redisRead(sw::redis::OptionalString);

    class GlobalServices : public SingleThreaded::GlobalServices {
    public:
        GlobalServices() :_nodeID(std::to_string(getRedis()->incr("nextNodeID"))) {}

        std::string getNodeId() override {
            return _nodeID;
        }

        [[nodiscard]] std::string toString() const override {
            return "RedisDriver::GlobalServices<>";
        }
    protected:
        std::string _nodeID;
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

        [[nodiscard]] virtual serial::tag_t getSerialKey() const override { return "swarm::RedisDriver::RedisStorageInterface"; }

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

        [[nodiscard]] std::string toString() const override {
            return "RedisDriver::RedisStorageLock<loc: " + _loc->toString() + ">";
        }
    protected:
        RedisStorageInterface* _store = nullptr;
        ISA::LocationReference* _loc = nullptr;
    };

    class RedisQueueJob : public IQueueJob {
    public:
        RedisQueueJob(JobID id, binn* call, State* vmState, binn* vmScope, IStorageInterface* localStore)
            : _id(id), _call(call), _vmState(useref(vmState)), _vmScope(vmScope), _localStore(useref(localStore)) {}

        ~RedisQueueJob() {
            binn_free(_call);
            freeref(_vmState);
            binn_free(_vmScope);
            freeref(_localStore);
        }

        /** Get the tracking ID for this job. */
        [[nodiscard]] virtual JobID id() const override { return _id; };

        /** Get the current status of this job. */
        [[nodiscard]] virtual JobState state() const override { 
            auto state = getRedis()->get(Configuration::REDIS_PREFIX + "status_" + s(_id));
            return (JobState)std::atoi(state.value_or(s(JobState::UNKNOWN)).c_str());
        }

        virtual void setState(JobState state) override {
            getRedis()->set(Configuration::REDIS_PREFIX + "status_" + s(_id), s(state), Configuration::REDIS_DEFAULT_TLL);
        }

        // FIXME: could store this separately preserialization, but becomes impossible postserialization without restoring vm
        [[nodiscard]] virtual IFunctionCall* getCall() const override { return nullptr; }

        [[nodiscard]] virtual State* getVMState() const { return _vmState; }

        [[nodiscard]] virtual binn* getScopeBinn() const { return _vmScope; }

        [[nodiscard]] virtual binn* getCallBin() const { return _call; }

        [[nodiscard]] virtual IStorageInterface* getLocalStore() const { return _localStore; }

        virtual void setFilters(SchedulingFilters filters) override { _filters = filters; }

        [[nodiscard]] virtual SchedulingFilters getFilters() const override { return _filters; }

        [[nodiscard]] std::string toString() const override {
            return "RedisDriver::RedisQueueJob<id: " + std::to_string(_id) + ">";
        }
    protected:
        JobID _id;
        binn* _call;
        State* _vmState;
        binn* _vmScope;
        IStorageInterface* _localStore;

        SchedulingFilters _filters;
    };

    class RedisQueue : public IQueue {
    public:
        RedisQueue(VirtualMachine* vm) : _redis(getRedis()), _vm(vm) {}
        /**
         * Focuses the queue on a particular context.
         * Contexts provide a way for the VM to isolate jobs in batches.
         * e.g. an `enumerate` instruction will produce a batch of jobs, one for each element.
         * The VM will push this batch into its own context so they can be awaited
         * independently.
         */
        virtual void setContext(QueueContextID) override;

        /** Get the ID of the current queue context. */
        virtual QueueContextID getContext() override;

        /** Determines whether this queue should execute the given function call. */
        virtual bool shouldHandle(IFunctionCall*) override { return true; }

        /** Given a function call and context, instantiate a new IQueueJob. */
        virtual IQueueJob* build(VirtualMachine*, IFunctionCall*) override;

        /** Push a call onto this queue. */
        virtual void push(VirtualMachine*, IQueueJob*) override;

        /** Remove the next pending job from the queue and return it. */
        virtual IQueueJob* pop() override;

        /** Returns true if there are no pending jobs. */
        virtual bool isEmpty(QueueContextID) override;

        virtual void tick() override;

        void initialize() {
            _redis->setnx(Configuration::REDIS_PREFIX + "nextJobID", "0");
        }

        [[nodiscard]] std::string toString() const override {
            return "RedisDriver::RedisQueue<ctx: " + _context + ">";
        }
    protected:
        sw::redis::Redis* _redis;
        VirtualMachine* _vm;
        QueueContextID _context;

        void tryToProcessJob();
        std::pair<IQueueJob*, QueueContextID> tryGetJob();
        IQueueJob* popFromContext(const QueueContextID&);
    };

    class Stream : public IStream {
    public:
        Stream(std::string id, Type::Type* innerType, VirtualMachine* vm) : _id(std::move(id)), _innerType(innerType), _vm(vm) {
            setKeys();
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

        void setKeys();
        void clearKeys();
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
