#include <cassert>
#include "../../errors/InvalidStoreLocationError.h"
#include "single_threaded.h"
#include "../isa_meta.h"
#include "../VirtualMachine.h"
#include "../walk/binary_const.h"

namespace swarmc::Runtime::SingleThreaded {

    StorageInterface::~StorageInterface() noexcept {
        for ( const auto& e : _map ) freeref(e.second);
        for ( const auto& e : _types ) freeref(e.second);
        for ( const auto& e : _locks ) freeref(e.second);
    }

    ISA::Reference* StorageInterface::load(ISA::LocationReference* loc) {
        auto iter = _map.find(loc->fqName());
        if ( iter == _map.end() ) throw Errors::InvalidStoreLocationError(s(loc), s(this));
        return iter->second;
    }

    void StorageInterface::store(ISA::LocationReference* loc, ISA::Reference* value) {
        if ( _types.find(loc->fqName()) == _types.end() ) _types[loc->fqName()] = useref(value->type());
        assert(value->typei()->isAssignableTo(_types[loc->fqName()]));

        auto existing = _map.find(loc->fqName());
        if ( existing != _map.end() && existing->second != value ) freeref(existing->second);

        _map[loc->fqName()] = useref(value);
    }

    bool StorageInterface::has(ISA::LocationReference* loc) {
        return _map.find(loc->fqName()) != _map.end();
    }

    bool StorageInterface::manages(ISA::LocationReference* loc) {
        return loc->affinity() == _affinity;
    }

    void StorageInterface::drop(ISA::LocationReference* loc) {
        auto mapIter = _map.find(loc->fqName());
        if ( mapIter == _map.end() ) return;

        freeref(mapIter->second);
        _map.erase(mapIter);

        auto typeIter = _types.find(loc->fqName());
        if ( typeIter != _types.end() ) {
            freeref(typeIter->second);
            _types.erase(typeIter);
        }
    }

    const Type::Type* StorageInterface::typeOf(ISA::LocationReference* loc) {
        auto iter = _types.find(loc->fqName());
        if ( iter == _types.end() ) return nullptr;
        return iter->second;
    }

    void StorageInterface::typify(ISA::LocationReference* loc, Type::Type* type) {
        _types[loc->fqName()] = useref(type);
    }

    IStorageLock* StorageInterface::acquire(ISA::LocationReference* loc) {
        if ( _locks.find(loc->fqName()) != _locks.end() ) return nullptr;
        _locks[loc->fqName()] = useref(new StorageLock(this, loc));
        return _locks[loc->fqName()];
    }

    void StorageInterface::clear() {
        for ( const auto& e : _map ) freeref(e.second);
        _map.clear();

        for ( const auto& e : _types ) freeref(e.second);
        _types.clear();
    }

    IStorageInterface* StorageInterface::copy() {
        // In a single threaded environment, shared variables are singleton,
        // so we "copy" a shared variable store by re-using the reference
        if ( _affinity == ISA::Affinity::SHARED ) return this;

        // Otherwise, duplicate the store
        auto copy = new StorageInterface(_affinity);

        copy->_map = _map;
        for ( const auto& e : copy->_map ) useref(e.second);

        copy->_types = _types;
        for ( const auto& e : copy->_types ) useref(e.second);

        // (don't duplicate locks, since the recipient won't hold them)
        return copy;
    }

    StorageLock::StorageLock(StorageInterface* store, ISA::LocationReference* loc) {
        _store = useref(store);
        _loc = useref(loc);
    }

    StorageLock::~StorageLock() noexcept {
        freeref(_store);
        freeref(_loc);
    }

    ISA::LocationReference* StorageLock::location() const {
        return _loc;
    }

    void StorageLock::release() {
        auto lockIter = _store->_locks.find(_loc->fqName());
        freeref((*lockIter).second);
        _store->_locks.erase(lockIter);
    }

    std::string StorageLock::toString() const {
        return "SingleThreaded::StorageLock<loc: " + _loc->toString() + ">";
    }


    QueueJob::QueueJob(
        JobID id, JobState jobState, IFunctionCall* call):
            _id(id), _jobState(jobState), _call(useref(call)) {}

    QueueJob::~QueueJob() noexcept {
        freeref(_call);
    }


    std::string QueueJob::toString() const {
        return "SingleThreaded::QueueJob<id: " + std::to_string(_id) + ", call: " + _call->toString() + ">";
    }


    QueueJob* Queue::build(VirtualMachine*, IFunctionCall* call) {
        return new QueueJob(_nextId++, JobState::PENDING, call);
    }

    void Queue::push(VirtualMachine* vm, IQueueJob* job) {
        ISA::Reference* ret = nullptr;
        vm->copy([job, &ret](VirtualMachine* clonedVm) {
            // FIXME: handle errors
            Console::get()->debug("Got VM from queue: " + clonedVm->toString());
            clonedVm->executeCall(job->getCall());
            ret = job->getCall()->getReturn();
            job->setState(JobState::COMPLETE);
        });
        setJobReturn(job->id(), ret);
    }


    Stream::Stream(std::string id, Type::Type *innerType) : _id(std::move(id)), _innerType(useref(innerType)) {}

    Stream::~Stream() noexcept {
        while ( !_items.empty() ) {
            freeref(_items.front());
            _items.pop();
        }
    }

    void Stream::push(ISA::Reference* value) {
        assert(value->type()->isAssignableTo(_innerType));
        _items.push(useref(value));
    }

    ISA::Reference* Stream::pop() {
        assert(!_items.empty());
        auto top = _items.front();
        _items.pop();
        releaseref(top);
        return top;
    }

    bool Stream::isEmpty() {
        return _items.empty();
    }

    std::string Stream::toString() const {
        return "SingleThreaded::Stream<of: " + _innerType->toString() + ">";
    }


    IStream* StreamDriver::open(const std::string &id, Type::Type* innerType) {
        return new Stream(id, innerType);
    }
}
