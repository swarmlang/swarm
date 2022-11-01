#ifndef SWARMVM_SINGLE_THREADED
#define SWARMVM_SINGLE_THREADED

#include <map>
#include <queue>
#include "../../shared/uuid.h"
#include "interfaces.h"

namespace swarmc::Runtime::SingleThreaded {
    class StorageLock;


    class GlobalServices : public IGlobalServices {
    public:
        std::string getUuid() override {
            return util::uuid4();
        }

        size_t getId() override {
            return _id++;
        }

        std::string toString() const override {
            return "SingleThreaded::GlobalServices<id: " + std::to_string(_id) + ">";
        }

    protected:
        size_t _id = 0;
    };


    class StorageInterface : public IStorageInterface {
    public:
        ISA::Reference* load(ISA::LocationReference* loc) override;

        void store(ISA::LocationReference* loc, ISA::Reference* value) override;

        bool has(ISA::LocationReference* loc) override;

        bool manages(ISA::LocationReference* loc) override;

        void drop(ISA::LocationReference* loc) override;

        const Type::Type* typeOf(ISA::LocationReference* loc) override;

        void typify(ISA::LocationReference* loc, const Type::Type* type) override;

        IStorageLock* acquire(ISA::LocationReference* loc) override;

        void clear() override;

        std::string toString() const override {
            return "SingleThreaded::StorageInterface<#loc: " + std::to_string(_map.size()) + ">";
        }

    protected:
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
        QueueJob(JobID id, JobState jobState, const IFunctionCall* call, const ScopeFrame* scope, const State* vmState):
            _id(id), _jobState(jobState), _call(call), _scope(scope), _vmState(vmState) {}

        JobID id() const override { return _id; }

        JobState state() const override { return _jobState; }

        const IFunctionCall* getCall() const override { return _call; }

        const ScopeFrame* getScope() const override { return _scope; }

        const State* getState() const override { return _vmState; }

        std::string toString() const override;

    protected:
        JobID _id;
        JobState _jobState;
        const IFunctionCall* _call;
        const ScopeFrame* _scope;
        const State* _vmState;
    };


    class Queue : public IQueue {
    public:
        void setContext(QueueContextID ctx) override {
            _context = ctx;
            if ( _queues.find(ctx) == _queues.end() ) _queues[ctx] = std::queue<IQueueJob*>();
        }

        QueueContextID getContext() override { return _context; }

        bool shouldHandle(IFunctionCall* call) override { return true; }

        QueueJob* build(const IFunctionCall* call, const ScopeFrame* scope, const State* state) override {
            return new QueueJob(_nextId++, JobState::PENDING, call, scope, state);
        }

        void push(IQueueJob* job) override {
            _queues[_context].push(job);
        }

        IQueueJob* pop() override {
            auto job = _queues[_context].front();
            _queues[_context].pop();
            return job;
        }

        bool isEmpty() override {
            return _queues[_context].empty();
        }

    protected:
        JobID _nextId = 0;
        QueueContextID _context;
        std::map<QueueContextID, std::queue<IQueueJob*>> _queues;
    };

}

#endif //SWARMVM_SINGLE_THREADED
