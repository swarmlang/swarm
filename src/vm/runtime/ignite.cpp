#include "ignite.h"
#include "../isa_meta.h"
#include "../VirtualMachine.h"
#include "../walk/ReferenceBinaryWalk.h"
#include "../walk/BinaryReferenceWalk.h"

using namespace swarmc::ISA;

namespace swarmc::Runtime::Ignite {

    StorageInterface::StorageInterface(Runtime *runtime, ISA::Affinity affinity) : _runtime(runtime), _affinity(affinity)  {
        _uuid = nslib::uuid();
        _cache = _runtime->getQualifiedCache("store::" + _uuid);
        _ref = new ReferenceBinaryWalk();
        _bin = new BinaryReferenceWalk();
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
        auto ref = _bin->walk(binn);

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
        _bin = new BinaryReferenceWalk();
        _ref = new ReferenceBinaryWalk();
    }

    Queue::~Queue() {
        delete _bin;
        delete _ref;
    }

    QueueJob* Queue::build(IFunctionCall* call, const ScopeFrame* scope, const State* state) {

    }

    binn* Queue::serializeCall(IFunctionCall* call) {
        auto vectorTypes = binn_list();
        auto vectorValues = binn_list();
        for ( auto pair : call->vector() ) {
            binn_list_add_object(vectorTypes, _ref->walkType(pair.first));
            binn_list_add_object(vectorValues, _ref->walk(pair.second));
        }

        auto binn = binn_map();
        binn_map_set_uint64(binn, BC_BACKEND, (std::size_t) call->backend());
        binn_map_set_str(binn, BC_NAME, strdup(call->name().c_str()));
        binn_map_set_object(binn, BC_TYPE, _ref->walkType(call->returnType()));
        binn_map_set_object(binn, BC_EXTRA, call->getExtraSerialData());
        binn_map_set_list(binn, BC_VECTOR_TYPES, vectorTypes);
        binn_map_set_list(binn, BC_VECTOR_VALUES, vectorValues);
        return binn;
    }

    binn* Queue::serializeScope(const ScopeFrame* scope) {
        auto names = binn_list();
        auto locations = binn_list();
        for ( const auto& pair : scope->nameMap() ) {
            binn_list_add_str(names, strdup(pair.first.c_str()));
            binn_list_add_object(locations, _ref->walk(pair.second));
        }


        auto parent = scope->parent();
        auto call = scope->call();
        auto ret = scope->getReturnCall();
        auto returnTo = scope->getReturnPC();

        auto binn = binn_map();
        binn_map_set_list(binn, BC_NAMES, names);
        binn_map_set_list(binn, BC_LOCATIONS, locations);
        binn_map_set_str(binn, BC_ID, strdup(scope->id().c_str()));
        binn_map_set_bool(binn, BC_IS_EX_FRAME, scope->isExceptionFrame());
        binn_map_set_bool(binn, BC_CAPTURE_RETURN, scope->shouldCaptureReturn());

        binn_map_set_bool(binn, BC_HAS_PARENT, parent != nullptr);
        if ( parent != nullptr ) binn_map_set_object(binn, BC_PARENT, serializeScope(parent));

        binn_map_set_bool(binn, BC_HAS_CALL, call != nullptr);
        if ( call != nullptr ) binn_map_set_object(binn, BC_CALL, serializeCall(call));

        binn_map_set_bool(binn, BC_HAS_RETURN, ret != nullptr);
        if ( ret != nullptr ) binn_map_set_object(binn, BC_RETURN, serializeCall(call));

        binn_map_set_bool(binn, BC_HAS_RETURN_PC, returnTo != std::nullopt);
        if ( returnTo != std::nullopt ) binn_map_set_uint64(binn, BC_RETURN_PC, (std::size_t) *returnTo);
    }

}
