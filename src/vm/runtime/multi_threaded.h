#ifndef SWARMVM_MULTI_THREADED_H
#define SWARMVM_MULTI_THREADED_H

#include <mutex>
#include <queue>
#include "../../shared/nslib.h"
#include "../ISA.h"
#include "single_threaded.h"
#include "interfaces.h"

namespace swarmc::Runtime {
    class VirtualMachine;
}

namespace swarmc::Runtime::MultiThreaded {
    class StorageLock;

    class GlobalServices : public SingleThreaded::GlobalServices {
    public:
        std::string getNodeId() override {
            return "multithreaded::localhost";
        }

        [[nodiscard]] std::string toString() const override {
            return "MultiThreaded::GlobalServices<>";
        }
    };

    class SharedStorageInterface : public SingleThreaded::StorageInterface {
    public:
        explicit SharedStorageInterface() : StorageInterface(ISA::Affinity::SHARED) {}

        IStorageLock* acquire(ISA::LocationReference*) override;

        void clear() override;

        bool shouldLockAccesses() const override { return true; }
    protected:
        std::map<std::string, std::mutex> _mutexes;

        friend class StorageLock;
    };

    class StorageLock : public IStorageLock {
    public:
        StorageLock(SharedStorageInterface* store, ISA::LocationReference* loc, std::mutex& m) : _store(store), _loc(loc) {
            _lock = new std::lock_guard<std::mutex>(m);
        }

        [[nodiscard]] ISA::LocationReference* location() const override;

        void release() override;

        [[nodiscard]] std::string toString() const override;
    protected:
        SharedStorageInterface* _store = nullptr;
        ISA::LocationReference* _loc = nullptr;
        std::lock_guard<std::mutex>* _lock;
    };

    class QueueJob : public IQueueJob {
    public:
        QueueJob(JobID id, JobState jobState, IFunctionCall *call, VirtualMachine* vm);

        ~QueueJob() noexcept override;

        [[nodiscard]] JobID id() const override { return _id; }

        [[nodiscard]] JobState state() const override { return _jobState; }

        void setState(JobState state) override { _jobState = state; }

        [[nodiscard]] IFunctionCall* getCall() const override { return _call; }

        void setFilters(SchedulingFilters filters) override { _filters = filters; }

        [[nodiscard]] SchedulingFilters getFilters() const override { return _filters; }

        [[nodiscard]] VirtualMachine* getVM() const { return _vm; }

        [[nodiscard]] std::string toString() const override {
            return "MultiThreaded::QueueJob<id: " + std::to_string(_id) + ", call: " + _call->toString() + ">";
        }
    protected:
        JobID _id;
        JobState _jobState;
        IFunctionCall* _call;
        SchedulingFilters _filters;
        VirtualMachine* _vm;
    };

    class Queue : public IQueue {
    public:
        explicit Queue(VirtualMachine* vm);

        void setContext(QueueContextID ctx) override {
            _context = ctx;
        }

        QueueContextID getContext() override { return _context; }

        bool shouldHandle(IFunctionCall*) override;

        IQueueJob* build(IFunctionCall*) override;

        void push(IQueueJob* job) override;

        IQueueJob* pop() override;

        bool isEmpty() override { return _queue.empty(); }

        [[nodiscard]] std::string toString() const override {
            return "MultiThreaded::Queue<ctx: " + _context + ">";
        }

        void tick() override;

    protected:
        VirtualMachine* _vm;
        JobID _nextId = 0;
        QueueContextID _context;
        std::mutex _queueMutex;
        std::queue<IQueueJob*> _queue;
        std::mutex _threadMutex;
        std::vector<IThreadContext*> _threads;

        bool _shouldExit = false;

        void spawnThreads();
    };

    class Stream : public IStream {
    public:
        Stream(std::string id, Type::Type* innerType) : _id(std::move(id)), _innerType(innerType) {}

        ~Stream() noexcept override;

        void open() override {}

        void close() override {}

        bool isOpen() override { return true; }

        Type::Type* innerType() override { return _innerType; }

        void push(ISA::Reference* value) override;

        ISA::Reference* pop() override;

        bool isEmpty() override { return _items.empty(); }

        [[nodiscard]] std::string id() const override { return _id; }

        [[nodiscard]] std::string toString() const override {
            return "MultiThreaded::Stream<of: " + _innerType->toString() + ">";
        }
    protected:
        std::string _id;
        Type::Type* _innerType;
        std::queue<ISA::Reference*> _items;
    };


    class StreamDriver : public IStreamDriver {
    public:
        IStream* open(const std::string&, Type::Type*) override;

        [[nodiscard]] std::string toString() const override {
            return "MultiThreaded::StreamDriver<>";
        }
    };
}

#endif
