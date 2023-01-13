#include <optional>
#include <cassert>
#include "../../Configuration.h"
#include "../../errors/RuntimeError.h"
#include "fabric.h"
#include "../../lang/Type.h"

namespace swarmc::Runtime {

    InlineRefHandle<Type::Type> IResource::innerTypei() const {
        return inlineref<Type::Type>(innerType());
    }


    void Fabric::publish(IResource* resource) {
        // We can only publish resources we own
        if ( resource->owner() != _global->getNodeId() ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidPrivilegedResourceOperation,
                "Cannot publish resource owned by a different node (owner: " + s(resource->owner()) + ", current: " + s(_global->getNodeId()) + ")"
            );
        }

        // We can't "re-publish" existing resources
        auto key = Configuration::FABRIC_PREFIX + resource->id();
        if ( _global->getKeyValue(key) != std::nullopt ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::RepublishExistingResource,
                "Cannot publish resource (" + s(resource) + "): the resource is already published"
            );
        }

        _global->putKeyValue(key, resource->owner());
    }

    void Fabric::unpublish(IResource* resource) {
        // We can only unpublish rsources we own
        if ( resource->owner() != _global->getNodeId() ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidPrivilegedResourceOperation,
                "Cannot un-publish resource owned by a different node (owner: " + s(resource->owner()) + ", current: " + s(_global->getNodeId()) + ")"
            );
        }

        auto key = Configuration::FABRIC_PREFIX + resource->id();
        _global->dropKeyValue(key);
    }

    IResource* Fabric::load(const std::string& id) {
        // Look up the owner from the KV
        return nullptr;  // fixme
    }

}
