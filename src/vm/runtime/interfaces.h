#ifndef SWARMVM_INTERFACES
#define SWARMVM_INTERFACES

#include <utility>
#include <vector>
#include <map>
#include <algorithm>
#include "../../shared/nslib.h"

using namespace nslib;

namespace swarmc::ISA {
    class Reference;
    class LocationReference;
    enum class Affinity: std::size_t;
}

namespace swarmc::Type {
    class Type;
}

namespace swarmc::Runtime {

    class IFunctionCall;
    class ScopeFrame;
    class State;

    class IStorageLock;
    class IStorageInterface;
    class IQueue;

    class VirtualMachine;

    using SchedulingFilters = std::map<std::string, std::string>;
    using JobID = std::size_t;
    using QueueContextID = std::string;
    using Stores = std::vector<IStorageInterface*>;
    using Locks = std::vector<IStorageLock*>;
    using Queues = std::vector<IQueue*>;
    using NodeID = std::string;

    /** Tracks the status of a queued function call. */
    enum class JobState: std::size_t {
        UNKNOWN = 1 << 0,
        PENDING = 1 << 1,
        RUNNING = 1 << 2,
        COMPLETE = 1 << 3,
        ERROR = 1 << 4,
    };

    /**
     * The VM requires some basic generators for IDs/random numbers, which may require
     * different logic to ensure uniqueness depending on which drivers are used for the
     * runtime.
     *
     * IGlobalServices abstracts these operations.
     */
    class IGlobalServices : public IStringable, public IRefCountable {
    public:
        ~IGlobalServices() override = default;

        /** This should return a UUIDv4-formatted string, globally unique. */
        virtual std::string getUuid() = 0;

        /** This should return a globally-monotonic, unique numeric identifier. */
        virtual std::size_t getId() = 0;

        /** This should use a source of randomness to generate a random double on [0,1]. */
        virtual double random() = 0;

        /** Get the current UNIX time in fractional seconds. */
        virtual double getCurrentTime() = 0;

        virtual NodeID getNodeId() = 0;

        virtual void putKeyValue(const std::string& key, const std::string& value) = 0;

        virtual std::optional<std::string> getKeyValue(const std::string& key) = 0;

        virtual void dropKeyValue(const std::string& key) = 0;

        [[nodiscard]] virtual SchedulingFilters getSchedulingFilters() const { return _filters; }

        virtual void applySchedulingFilter(const std::string& key, std::string value);

        virtual void applySchedulingFilters(SchedulingFilters filters);

        virtual void clearSchedulingFilters();

        [[nodiscard]] virtual SchedulingFilters getContextFilters() const { return _context; }

        virtual void applyContextFilter(const std::string& key, std::string value);

        virtual void clearContextFilters();

    protected:
        SchedulingFilters _filters;
        SchedulingFilters _context;
    };

    /** Represents a lock acquired by some control. */
    class IStorageLock : public IStringable, public IRefCountable {
    public:
        ~IStorageLock() override = default;

        /** Get the location this lock covers. */
        [[nodiscard]] virtual ISA::LocationReference* location() const = 0;

        /** Release this lock. */
        virtual void release() = 0;
    };

    /**
     * Interface for VM variable storage backends.
     */
    class IStorageInterface : public IStringable, public IRefCountable, public nslib::serial::ISerializable {
    public:
        ~IStorageInterface() override = default;

        /** Load the actual value of a variable. */
        virtual ISA::Reference* load(ISA::LocationReference*) = 0;

        /** Store a value into a variable. */
        virtual void store(ISA::LocationReference*, ISA::Reference*) = 0;

        /** Returns true if this backend has a value for the given variable. */
        virtual bool has(ISA::LocationReference*) = 0;

        /** Returns true if this backend should be used to lock/store the given variable. */
        virtual bool manages(ISA::LocationReference*) = 0;

        /** Make the backend forget the value of the given variable, if it exists. */
        virtual void drop(ISA::LocationReference*) = 0;

        /** Get the type of the given location. */
        virtual const Type::Type* typeOf(ISA::LocationReference*) = 0;

        /** Constrain the given location to a specific type. */
        virtual void typify(ISA::LocationReference*, Type::Type*) = 0;

        /**
         * Attempt to acquire a lock for a particular storage location.
         * If the lock is acquired, an IStorageLock is returned. It not,
         * then nullptr is returned and the caller should retry.
         */
        virtual IStorageLock* acquire(ISA::LocationReference*) = 0;

        /** Forget all stored variables. */
        virtual void clear() = 0;

        virtual IStorageInterface* copy() = 0;

        /** Returns true if the VM should acquire locks before accessing variables in this store. */
        [[nodiscard]] virtual bool shouldLockAccesses() const { return true; }
    };


    /** Tracking class for a single deferred function call. */
    class IQueueJob : public IStringable, public IRefCountable {
    public:
        ~IQueueJob() override = default;

        /** Get the tracking ID for this job. */
        [[nodiscard]] virtual JobID id() const = 0;

        /** Get the current status of this job. */
        [[nodiscard]] virtual JobState state() const = 0;

        [[nodiscard]] virtual bool hasFinished() const {
            auto currentState = state();
            return currentState == JobState::COMPLETE || currentState == JobState::ERROR;
        }

        virtual void setState(JobState) = 0;

        [[nodiscard]] virtual IFunctionCall* getCall() const = 0;

        virtual void setFilters(SchedulingFilters) = 0;

        [[nodiscard]] virtual SchedulingFilters getFilters() const = 0;

        [[nodiscard]] virtual bool matchesFilters(const SchedulingFilters& current) const;
    };


    /** Interface for a deferred function call queue. */
    class IQueue : public IStringable, public IRefCountable {
    public:
        ~IQueue() override = default;

        /**
         * Focuses the queue on a particular context.
         * Contexts provide a way for the VM to isolate jobs in batches.
         * e.g. an `enumerate` instruction will produce a batch of jobs, one for each element.
         * The VM will push this batch into its own context so they can be awaited
         * independently.
         */
        virtual void setContext(QueueContextID) = 0;

        /** Get the ID of the current queue context. */
        virtual QueueContextID getContext() = 0;

        /** Determines whether this queue should execute the given function call. */
        virtual bool shouldHandle(IFunctionCall*) = 0;

        /** Given a function call and context, instantiate a new IQueueJob. */
        virtual IQueueJob* build(VirtualMachine*, IFunctionCall*) = 0;

        /** Push a call onto this queue. */
        virtual void push(VirtualMachine*, IQueueJob*) = 0;

        /** Remove the next pending job from the queue and return it. */
        virtual IQueueJob* pop() = 0;

        /** Returns true if there are no pending jobs. */
        virtual bool isEmpty(QueueContextID) = 0;

        virtual void tick() = 0;
    };


    class IStream : public IStringable, public IRefCountable {
    public:
        ~IStream() override = default;

        virtual void open() = 0;

        virtual void close() = 0;

        virtual bool isOpen() = 0;

        virtual Type::Type* innerType() = 0;

        virtual InlineRefHandle<Type::Type> innerTypei();

        virtual void push(ISA::Reference* value) = 0;

        virtual ISA::Reference* pop() = 0;

        virtual bool isEmpty() = 0;

        [[nodiscard]] virtual std::string id() const = 0;
    };


    class IStreamDriver : public IStringable, public IRefCountable {
    public:
        virtual IStream* open(const std::string&, Type::Type*) = 0;
    };
}

namespace nslib {
    [[nodiscard]] std::string s(swarmc::Runtime::JobState v) {
        if ( v == JobState::COMPLETE ) return "COMPLETE";
        if ( v == JobState::ERROR ) return "ERROR";
        if ( v == JobState::PENDING ) return "PENDING";
        if ( v == JobState::RUNNING ) return "RUNNING";
        return "UNKNOWN";
    }
}

#endif //SWARMVM_INTERFACES
