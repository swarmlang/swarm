#ifndef SWARMVM_SINGLE_THREADED
#define SWARMVM_SINGLE_THREADED

#include <map>
#include <queue>
#include <random>
#include "../../shared/uuid.h"
#include "interfaces.h"

namespace swarmc::Runtime {
    class VirtualMachine;
}

namespace swarmc::Runtime::SingleThreaded {
    class StorageLock;


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

        std::string toString() const override {
            return "SingleThreaded::GlobalServices<id: " + std::to_string(_id) + ">";
        }

    protected:
        size_t _id = 0;
        std::random_device _rd;
        std::default_random_engine _gen;
        std::uniform_real_distribution<double> _dist;
    };


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

    protected:
        JobID _id;
        JobState _jobState;
        IFunctionCall* _call;
        const ScopeFrame* _scope;
        const State* _vmState;
    };


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

}

#endif //SWARMVM_SINGLE_THREADED
