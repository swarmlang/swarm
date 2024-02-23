#include <optional>
#include <cassert>
#include "../../Configuration.h"
#include "../../errors/RuntimeError.h"
#include "fabric.h"
#include "../../lang/Type.h"
#include "./VirtualMachine.h"
#include "../walk/binary_const.h"

namespace swarmc::Runtime {

    InlineRefHandle<Type::Type> IResource::innerTypei() const {
        return inlineref<Type::Type>(innerType());
    }


    ResourceOperationFrame TunneledResource::performOperation(VirtualMachine* vm, OperationName op, ResourceOperationFrame params) {
        IFunction* fn = new TunneledResourceOperationFunction;
        GC_LOCAL_REF(fn)

        // 3 params: the ID of the resource, the name of the operation, and a list of parameters to the operation
        fn = fn->curry(new ISA::StringReference(_id));
        GC_LOCAL_REF(fn)

        fn = fn->curry(new ISA::StringReference(op));
        GC_LOCAL_REF(fn)

        auto frame = new ISA::EnumerationReference(Type::Ambiguous::of());
        frame->reserve(params.size());
        for ( auto p : params ) {
            frame->append(p);
        }

        fn = fn->curry(frame);

        auto call = fn->call();  // fixme: need to set execution context filters
        auto job = vm->pushCall(call);
        while ( !job->hasFinished() ) {
            vm->whileWaitingForDrain();
        }

        // fixme: error handling
        auto result = job->getCall()->getReturn();
        auto enumAnyT = new Type::Enumerable(Type::Ambiguous::of());
        GC_LOCAL_REF(enumAnyT);
        assert(result->typei()->isAssignableTo(enumAnyT));

        auto enumResult = dynamic_cast<ISA::EnumerationReference*>(result);
        ResourceOperationFrame resultFrame;
        frame->reserve(sizeof(ISA::Reference*) * enumResult->length());
        for ( std::size_t i = 0; i < enumResult->length(); i += 1 ) {
            resultFrame.push_back(enumResult->get(i));
        }

        return resultFrame;
    }


    bool Fabric::shouldPublish(IResource* resource) {
        if ( resource->owner() != _vm->global()->getNodeId() ) {
            return false;
        }

        auto key = Configuration::FABRIC_PREFIX + resource->id();
        if ( _vm->global()->getKeyValue(key) != std::nullopt ) {
            return false;
        }

        return true;
    }


    void Fabric::publish(IResource* resource) {
        // We can only publish resources we own
        if ( resource->owner() != _vm->global()->getNodeId() ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidPrivilegedResourceOperation,
                "Cannot publish resource owned by a different node (owner: " + s(resource->owner()) + ", current: " + s(_vm->global()->getNodeId()) + ")"
            );
        }

        // We can't "re-publish" existing resources
        auto key = Configuration::FABRIC_PREFIX + resource->id();
        if ( _vm->global()->getKeyValue(key) != std::nullopt ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::RepublishExistingResource,
                "Cannot publish resource (" + s(resource) + "): the resource is already published"
            );
        }

        binn* info = binn_map();
        binn_map_set_str(info, BC_OWNER, strdup(resource->owner().c_str()));
        binn_map_set_str(info, BC_NAME, strdup(resource->name().c_str()));
        binn_map_set_map(info, BC_TYPE, Wire::types()->reduce(resource->innerType(), nullptr));
        binn_map_set_uint64(info, BC_CATEGORY, (uint64_t) resource->category());

        std::string serialized((char*) binn_ptr(info), binn_size(info));
        _vm->global()->putKeyValue(key, serialized);
    }

    void Fabric::unpublish(IResource* resource) {
        // We can only unpublish resources we own
        if ( resource->owner() != _vm->global()->getNodeId() ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidPrivilegedResourceOperation,
                "Cannot un-publish resource owned by a different node (owner: " + s(resource->owner()) + ", current: " + s(_vm->global()->getNodeId()) + ")"
            );
        }

        auto key = Configuration::FABRIC_PREFIX + resource->id();
        _vm->global()->dropKeyValue(key);
    }

    IResource* Fabric::load(const std::string& id) {
        // Look up the owner from the KV
        auto key = Configuration::FABRIC_PREFIX + id;
        auto serialized = _vm->global()->getKeyValue(key);
        if ( serialized == std::nullopt ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidPrivilegedResourceOperation,
                "Unable to load unpublished or invalid resource (id: " + id + ")"
            );
        }


        std::stringstream ss;
        ss << *serialized;
        void* buf = malloc(sizeof(char) * (*serialized).size());
        ss.read(static_cast<char*>(buf), static_cast<std::streamsize>((*serialized).size()));
        auto info = binn_open((char*)buf);

        std::string owner = binn_map_str(info, BC_OWNER);
        std::string name = binn_map_str(info, BC_NAME);
        auto type = Wire::types()->produce((binn*) binn_map_map(info, BC_TYPE), nullptr);
        auto category = (ResourceCategory) binn_map_uint64(info, BC_CATEGORY);

        if ( category == ResourceCategory::TUNNELED ) {
            return new TunneledResource(id, owner, name, type);
        }

        throw std::runtime_error("Invalid resource category (currently only TUNNELED is supported)");
    }

}
