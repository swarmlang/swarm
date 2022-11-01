#include <cassert>
#include "single_threaded.h"
#include "../isa_meta.h"

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
        return true;  // TODO: allow configuring affinity?
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
}
