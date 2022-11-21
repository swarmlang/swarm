#ifndef SWARMVM_SINGLE_THREADED
#define SWARMVM_SINGLE_THREADED

#include <map>
#include <queue>
#include <random>
#include <utility>
#include "../../shared/uuid.h"
#include "interfaces.h"

/*
 * This file contains a single-threaded, synchronous implementation of the
 * runtime drivers. These are used for development/testing because they don't
 * require any external services like Redis. However, they offer NO parallelism.
 */

namespace swarmc::Runtime {
    class VirtualMachine;
}

namespace swarmc::Runtime::SingleThreaded {
    class StorageLock;


    /** A single-threaded IGlobalServices using local variables, `<random>`, and util's `uuid4`. */
    class GlobalServices : public IGlobalServices {
    public:
        GlobalServices() {
            _gen = std::default_random_engine(_rd());
            _dist = std::uniform_real_distribution<double>(0.0, 1.0);
        }

        std::string getUuid() override {
            return util::uuid4();
        }

        size_t getId() override {
            return _id++;
        }

        double random() override {
            return _dist(_gen);
        }

        std::string getNodeId() override {
            return "singlethreaded::localhost";
        }

        std::string toString() const override {
            return "SingleThreaded::GlobalServices<id: " + std::to_string(_id) + ">";
        }

    protected:
        size_t _id = 0;
        std::random_device _rd;
        std::default_random_engine _gen;
        std::uniform_real_distribution<double> _dist;
    };


    /** A single-threaded storage driver using `std::map`. */
    class StorageInterface : public IStorageInterface {
    public:
        StorageInterface(ISA::Affinity affinity) : _affinity(affinity) {}

        ISA::Reference* load(ISA::LocationReference* loc) override;

        void store(ISA::LocationReference* loc, ISA::Reference* value) override;

        bool has(ISA::LocationReference* loc) override;

        bool manages(ISA::LocationReference* loc) override;

        void drop(ISA::LocationReference* loc) override;

        const Type::Type* typeOf(ISA::LocationReference* loc) override;

        void typify(ISA::LocationReference* loc, const Type::Type* type) override;

        IStorageLock* acquire(ISA::LocationReference* loc) override;

        void clear() override;

        IStorageInterface* copy() override;

        std::string toString() const override {
            return "SingleThreaded::StorageInterface<#loc: " + std::to_string(_map.size()) + ">";
        }

    protected:
        ISA::Affinity _affinity;
        std::map<std::string, ISA::Reference*> _map;
        std::map<std::string, const Type::Type*> _types;
        std::map<std::string, StorageLock*> _locks;

        friend class StorageLock;
    };


    /**
     * A trivial storage lock implementation.
     * Locks are useless in a single-threaded environment, so this is just
     * provided to satisfy the interfaces.
     */
    class StorageLock : public IStorageLock {
    public:
        StorageLock(StorageInterface* store, ISA::LocationReference* loc) {
            _loc = loc;
            _store = store;
        }

        ISA::LocationReference* location() const override;

        void release() override;

        std::string toString() const override;
    protected:
        ISA::LocationReference* _loc = nullptr;
        StorageInterface* _store = nullptr;
    };


    /**
     * A trivial queue job implementation.
     * In a single threaded environment, jobs are executed immediately when they
     * are pushed onto the queue, so this also exists just to satisfy the interfaces.
     */
    class QueueJob : public IQueueJob {
    public:
        QueueJob(JobID id, JobState jobState, IFunctionCall* call, const ScopeFrame* scope, const State* vmState):
            _id(id), _jobState(jobState), _call(call), _scope(scope), _vmState(vmState) {}

        JobID id() const override { return _id; }

        JobState state() const override { return _jobState; }

        IFunctionCall* getCall() const override { return _call; }

        const ScopeFrame* getScope() const override { return _scope; }

        const State* getState() const override { return _vmState; }

        std::string toString() const override;

        void setFilters(SchedulingFilters filters) override { _filters = std::move(filters); }

        SchedulingFilters getFilters() const override { return _filters; }

    protected:
        JobID _id;
        JobState _jobState;
        IFunctionCall* _call;
        SchedulingFilters _filters;
        const ScopeFrame* _scope;
        const State* _vmState;
    };


    /**
     * A single-threaded queue implementation.
     * This queue is not a queue at all, actually. It is always empty
     * and executes jobs as soon as they are pushed onto the queue.
     */
    class Queue : public IQueue {
    public:
        Queue(VirtualMachine* vm) : _vm(vm) {}

        void setContext(QueueContextID ctx) override {
            _context = ctx;
        }

        QueueContextID getContext() override { return _context; }

        bool shouldHandle(IFunctionCall* call) override { return true; }

        QueueJob* build(IFunctionCall* call, const ScopeFrame* scope, const State* state) override {
            return new QueueJob(_nextId++, JobState::PENDING, call, scope, state);
        }

        void push(IQueueJob* job) override;

        IQueueJob* pop() override {
            return nullptr;
        }

        bool isEmpty() override {
            return true;
        }

        std::string toString() const override {
            return "SingleThreaded::Queue<ctx: " + _context + ">";
        }

    protected:
        VirtualMachine* _vm;
        JobID _nextId = 0;
        QueueContextID _context;
    };


    class Stream : public IStream {
    public:
        Stream(std::string id, const Type::Type* innerType) : _id(std::move(id)), _innerType(innerType) {}

        void open() override {}

        void close() override {}

        bool isOpen() override { return true; }

        const Type::Type* innerType() override { return _innerType; }

        void push(ISA::Reference* value) override;

        ISA::Reference* pop() override;

        bool isEmpty() override;

        std::string id() const override { return _id; }

        std::string toString() const override;

    protected:
        std::string _id;
        const Type::Type* _innerType;
        std::queue<ISA::Reference*> _items;
    };


    class StreamDriver : public IStreamDriver {
    public:
        IStream* open(const std::string& id, const Type::Type* innerType) override;

        std::string toString() const override {
            return "SingleThreaded::StreamDriver<>";
        }
    };

}

#endif //SWARMVM_SINGLE_THREADED
