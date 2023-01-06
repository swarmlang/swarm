#include "ignite.h"
#include "../isa_meta.h"
#include "../VirtualMachine.h"
#include "../Wire.h"

using namespace swarmc::ISA;

namespace swarmc::Runtime::Ignite {

    StorageInterface::StorageInterface(Runtime *runtime, VirtualMachine* vm, ISA::Affinity affinity) : _runtime(runtime), _vm(vm), _affinity(affinity)  {
        _uuid = nslib::uuid();
        _cache = _runtime->getQualifiedCache("store::" + _uuid);
    }

    Reference* StorageInterface::load(LocationReference* loc) {
        // FIXME: check type?
        std::string str;
        _runtime->asRetryableClusterTransaction([&str, loc, this](auto) {
            str = _cache.Get("val::" + loc->fqName());
        });

        auto cstr = new char[str.length() + 1];
        strcpy(cstr, str.c_str());

        auto buf = static_cast<void*>(cstr);
        auto binn = binn_open(buf);
        auto ref = Wire::references()->produce(binn, _vm);

        delete[] cstr;
        return ref;
    }

    void StorageInterface::store(LocationReference* loc, Reference* value) {
        auto str = _ref->walkToString(value);
        auto tstr = _ref->walkToString(value->type());
        _runtime->asRetryableClusterTransaction([&str, &tstr, loc, this](auto) {
            _cache.Put("val::" + loc->fqName(), str);
            _cache.Put("type::" + loc->fqName(), tstr);
        });
    }

    bool StorageInterface::has(LocationReference* loc) {
        bool has = false;
        _runtime->asRetryableClusterTransaction([&has, loc, this](auto) {
            has = _cache.ContainsKey("val::" + loc->fqName());
        });
        return has;
    }

    bool StorageInterface::manages(LocationReference* loc) {
        return loc->affinity() == _affinity;
    }

    void StorageInterface::drop(LocationReference* loc) {
        _runtime->asRetryableClusterTransaction([loc, this](auto) {
            _cache.Remove("val::" + loc->fqName());
            _cache.Remove("type::" + loc->fqName());
        });
    }

    const Type::Type* StorageInterface::typeOf(LocationReference* loc) {
        std::string str;
        _runtime->asRetryableClusterTransaction([&str, loc, this](auto) {
            str = _cache.Get("type::" + loc->fqName());
        });

        auto cstr = new char[str.length() + 1];
        strcpy(cstr, str.c_str());

        auto buf = static_cast<void*>(cstr);
        auto binn = binn_open(buf);
        auto ref = _bin->walkType(binn);

        delete[] cstr;
        return ref;
    }

    void StorageInterface::typify(LocationReference* loc, const Type::Type* type) {
        auto tstr = _ref->walkToString(type);
        _runtime->asRetryableClusterTransaction([&tstr, loc, this](auto) {
            _cache.Put("type::" + loc->fqName(), tstr);
        });
    }

    IStorageLock* StorageInterface::acquire(LocationReference* loc) {} // FIXME

    void StorageInterface::clear() {} // FIXME

    IStorageInterface* StorageInterface::copy() {} // FIXME



    LocationReference* StorageLock::location() const {
        return _loc;
    }

    void StorageLock::release() {}  // FIXME

    std::string StorageLock::toString() const {
        return "Ignite::StorageLock<loc: " + s(_loc) + ">";
    }



    // QUEUE JOB



    Queue::Queue(Runtime *runtime, VirtualMachine *vm): _runtime(runtime), _vm(vm) {
    }

    Queue::~Queue() {
        delete _bin;
        delete _ref;
    }

    QueueJob* Queue::build(IFunctionCall* call, const ScopeFrame* scope, const State* state) {

    }

}
