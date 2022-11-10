#include <cassert>
#include "single_threaded.h"
#include "../isa_meta.h"
#include "../VirtualMachine.h"

namespace swarmc::Runtime::SingleThreaded {

    ISA::Reference* StorageInterface::load(ISA::LocationReference* loc) {
        auto iter = _map.find(loc->fqName());
        if ( iter == _map.end() ) return nullptr;
        return iter->second;
    }

    void StorageInterface::store(ISA::LocationReference* loc, ISA::Reference* value) {
        if ( _types.find(loc->fqName()) == _types.end() ) _types[loc->fqName()] = value->type();
        assert(value->type()->isAssignableTo(_types[loc->fqName()]));
        _map[loc->fqName()] = value;
    }

    bool StorageInterface::has(ISA::LocationReference* loc) {
        return _map.find(loc->fqName()) != _map.end();
    }

    bool StorageInterface::manages(ISA::LocationReference* loc) {
        return loc->affinity() == _affinity;
    }

    void StorageInterface::drop(ISA::LocationReference* loc) {
        _map.erase(loc->fqName());
        _types.erase(loc->fqName());
    }

    const Type::Type* StorageInterface::typeOf(ISA::LocationReference* loc) {
        auto iter = _types.find(loc->fqName());
        if ( iter == _types.end() ) return nullptr;
        return iter->second;
    }

    void StorageInterface::typify(ISA::LocationReference* loc, const Type::Type* type) {
        _types[loc->fqName()] = type;
    }

    IStorageLock* StorageInterface::acquire(ISA::LocationReference* loc) {
        if ( _locks.find(loc->fqName()) != _locks.end() ) return nullptr;
        _locks[loc->fqName()] = new StorageLock(this, loc);
        return _locks[loc->fqName()];
    }

    void StorageInterface::clear() {
        _map.clear();
        _types.clear();
    }

    IStorageInterface* StorageInterface::copy() {
        // In a single threaded environment, shared variables are singleton,
        // so we "copy" a shared variable store by re-using the reference
        if ( _affinity == ISA::Affinity::SHARED ) return this;

        // Otherwise, duplicate the store
        auto copy = new StorageInterface(_affinity);
        copy->_map = _map;
        copy->_types = _types;
        // (don't duplicate locks, since the recipient won't hold them)

        return copy;
    }


    ISA::LocationReference* StorageLock::location() const {
        return _loc;
    }

    void StorageLock::release() {
        _store->_locks.erase(_loc->fqName());
    }

    std::string StorageLock::toString() const {
        return "SingleThreaded::StorageLock<loc: " + _loc->toString() + ">";
    }


    std::string QueueJob::toString() const {
        return "SingleThreaded::QueueJob<id: " + std::to_string(_id) + ", call: " + _call->toString() + ">";
    }


    void Queue::push(IQueueJob* job) {
        auto vm = _vm->copy();
        Console::get()->debug("Got VM from queue: " + vm->toString());
        vm->restore(job->getScope()->copy(), job->getState()->copy());
        vm->executeCall(job->getCall());
        delete vm;
    }


    void Stream::push(ISA::Reference* value) {
        assert(value->type()->isAssignableTo(_innerType));
        _items.push(value);
    }

    ISA::Reference* Stream::pop() {
        assert(!_items.empty());
        auto top = _items.front();
        _items.pop();
        return top;
    }

    bool Stream::isEmpty() {
        return _items.empty();
    }

    std::string Stream::toString() const {
        return "SingleThreaded::Stream<of: " + _innerType->toString() + ">";
    }


    IStream* StreamDriver::open(const std::string &id, const Type::Type* innerType) {
        return new Stream(id, innerType);
    }
}
