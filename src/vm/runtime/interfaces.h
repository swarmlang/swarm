#ifndef SWARMVM_INTERFACES
#define SWARMVM_INTERFACES

#include <utility>
#include <vector>
#include <map>
#include <algorithm>
#include "../../shared/IStringable.h"
#include "../../shared/util/Console.h"

namespace swarmc::ISA {
    class Reference;
    class LocationReference;
    enum class Affinity: size_t;
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

    using SchedulingFilters = std::map<std::string, std::string>;
    using JobID = size_t;
    using QueueContextID = std::string;
    using Stores = std::vector<IStorageInterface*>;
    using Locks = std::vector<IStorageLock*>;
    using Queues = std::vector<IQueue*>;

    /** Tracks the status of a queued function call. */
    enum class JobState: size_t {
        UNKNOWN = 2 << 0,
        PENDING = 2 << 1,
        RUNNING = 2 << 2,
        COMPLETE = 2 << 3,
        ERROR = 2 << 4,
    };

    /**
     * The VM requires some basic generators for IDs/random numbers, which may require
     * different logic to ensure uniqueness depending on which drivers are used for the
     * runtime.
     *
     * IGlobalServices abstracts these operations.
     */
    class IGlobalServices : public IStringable {
    public:
        virtual ~IGlobalServices() = default;

        /** This should return a UUIDv4-formatted string, globally unique. */
        virtual std::string getUuid() = 0;

        /** This should return a globally-monotonic, unique numeric identifier. */
        virtual size_t getId() = 0;

        /** This should use a source of randomness to generate a random double on [0,1]. */
        virtual double random() = 0;

        virtual std::string getNodeId() = 0;

        virtual SchedulingFilters getSchedulingFilters() const { return _filters; }

        virtual void applySchedulingFilter(const std::string& key, std::string value) {
            Console::get()->debug("Apply scheduling filter: " + key + " -> " + value);
            _filters[key] = std::move(value);
        }

        virtual void applySchedulingFilters(SchedulingFilters filters) {
            Console::get()->debug("Apply bulk scheduling filters.");
            _filters = std::move(filters);
        }

        virtual void clearSchedulingFilters() {
            Console::get()->debug("Clear scheduling filters.");
            _filters.clear();
        }

        virtual SchedulingFilters getContextFilters() const { return _context; }

        virtual void applyContextFilter(const std::string& key, std::string value) {
            _context[key] = std::move(value);
        }

        virtual void clearContextFilters() { _context.clear(); }

    protected:
        SchedulingFilters _filters;
        SchedulingFilters _context;
    };

    /** Represents a lock acquired by some control. */
    class IStorageLock : public IStringable {
    public:
        virtual ~IStorageLock() = default;

        /** Get the location this lock covers. */
        virtual ISA::LocationReference* location() const = 0;

        /** Release this lock. */
        virtual void release() = 0;
    };

    /**
     * Interface for VM variable storage backends.
     */
    class IStorageInterface : public IStringable {
    public:
        virtual ~IStorageInterface() = default;

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
        virtual void typify(ISA::LocationReference*, const Type::Type*) = 0;

        /**
         * Attempt to acquire a lock for a particular storage location.
         * If the lock is acquired, an IStorageLock is returned. It not,
         * then nullptr is returned and the caller should retry.
         */
        virtual IStorageLock* acquire(ISA::LocationReference*) = 0;

        /** Forget all stored variables. */
        virtual void clear() = 0;

        virtual IStorageInterface* copy() = 0;
    };


    /** Tracking class for a single deferred function call. */
    class IQueueJob : public IStringable {
    public:
        virtual ~IQueueJob() = default;

        /** Get the tracking ID for this job. */
        virtual JobID id() const = 0;

        /** Get the current status of this job. */
        virtual JobState state() const = 0;

        virtual IFunctionCall* getCall() const = 0;

        virtual const ScopeFrame* getScope() const = 0;

        virtual const State* getState() const = 0;

        virtual void setFilters(SchedulingFilters) = 0;

        virtual SchedulingFilters getFilters() const = 0;

        virtual bool matchesFilters(const SchedulingFilters& current) const {
            auto filters = getFilters();

            return std::all_of(filters.begin(), filters.end(), [current](const std::pair<std::string, std::string>& filter) {
                auto result = current.find(filter.first);
                return result != current.end() && (*result).second == filter.second;
            });
        }
    };


    /** Interface for a deferred function call queue. */
    class IQueue : public IStringable {
    public:
        virtual ~IQueue() = default;

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
        virtual IQueueJob* build(IFunctionCall*, const ScopeFrame*, const State*) = 0;

        /** Push a call onto this queue. */
        virtual void push(IQueueJob*) = 0;

        /** Remove the next pending job from the queue and return it. */
        virtual IQueueJob* pop() = 0;

        /** Returns true if there are no pending jobs. */
        virtual bool isEmpty() = 0;
    };


    class IStream : public IStringable {
    public:
        virtual ~IStream() = default;

        virtual void open() = 0;

        virtual void close() = 0;

        virtual bool isOpen() = 0;

        virtual const Type::Type* innerType() = 0;

        virtual void push(ISA::Reference* value) = 0;

        virtual ISA::Reference* pop() = 0;

        virtual bool isEmpty() = 0;

        virtual std::string id() const = 0;
    };


    class IStreamDriver : public IStringable {
    public:
        virtual IStream* open(const std::string&, const Type::Type*) = 0;
    };


    class IResource : public IStringable {
    public:
        virtual bool isOpen() = 0;

        virtual void open() = 0;

        virtual void close() = 0;

        virtual const Type::Type* innerType() = 0;

        virtual ISA::Reference* innerValue() = 0;
    };

}

#endif //SWARMVM_INTERFACES
