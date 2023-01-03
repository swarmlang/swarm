#ifndef SWARMVM_SINGLE_THREADED
#define SWARMVM_SINGLE_THREADED

#include <map>
#include <queue>
#include <random>
#include <utility>
#include "../../shared/nslib.h"
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
        GlobalServices() = default;

        std::string getUuid() override {
            return nslib::uuid();
        }

        std::size_t getId() override {
            return _id++;
        }

        double random() override {
            return nslib::rand();
        }

        std::optional<std::string> getKeyValue(const std::string &key) override {
            auto value = _map.find(key);
            return value == _map.end() ? std::nullopt : std::make_optional(value->second);
        }

        void putKeyValue(const std::string& key, const std::string& value) override {
            _map[key] = value;
        }

        void dropKeyValue(const std::string& key) override {
            _map.erase(key);
        }

        std::string getNodeId() override {
            return "singlethreaded::localhost";
        }

        [[nodiscard]] std::string toString() const override {
            return "SingleThreaded::GlobalServices<id: " + std::to_string(_id) + ">";
        }

    protected:
        std::size_t _id = 0;
        std::map<std::string, std::string> _map;
    };


    /** A single-threaded storage driver using `std::map`. */
    class StorageInterface : public IStorageInterface {
    public:
        explicit StorageInterface(ISA::Affinity affinity) : _affinity(affinity) {}

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

        [[nodiscard]] std::string toString() const override {
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

        [[nodiscard]] ISA::LocationReference* location() const override;

        void release() override;

        [[nodiscard]] std::string toString() const override;
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

        [[nodiscard]] JobID id() const override { return _id; }

        [[nodiscard]] JobState state() const override { return _jobState; }

        [[nodiscard]] IFunctionCall* getCall() const override { return _call; }

        [[nodiscard]] const ScopeFrame* getScope() const override { return _scope; }

        [[nodiscard]] const State* getState() const override { return _vmState; }

        [[nodiscard]] std::string toString() const override;

        void setFilters(SchedulingFilters filters) override { _filters = std::move(filters); }

        [[nodiscard]] SchedulingFilters getFilters() const override { return _filters; }

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
        explicit Queue(VirtualMachine* vm) : _vm(vm) {}

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

        [[nodiscard]] std::string toString() const override {
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

        [[nodiscard]] std::string id() const override { return _id; }

        [[nodiscard]] std::string toString() const override;

    protected:
        std::string _id;
        const Type::Type* _innerType;
        std::queue<ISA::Reference*> _items;
    };


    class StreamDriver : public IStreamDriver {
    public:
        IStream* open(const std::string& id, const Type::Type* innerType) override;

        [[nodiscard]] std::string toString() const override {
            return "SingleThreaded::StreamDriver<>";
        }
    };

}

#endif //SWARMVM_SINGLE_THREADED
