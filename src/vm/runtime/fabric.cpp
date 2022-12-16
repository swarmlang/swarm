#include <optional>
#include <cassert>
#include "../../Configuration.h"
#include "fabric.h"

namespace swarmc::Runtime {

    void Fabric::publish(IResource* resource) {
        // FIXME: these should generate runtime exceptionss

        // We can only publish resources we own
        assert(resource->owner() == _global->getNodeId());

        // We can't "re-publish" existing resources
        auto key = Configuration::FABRIC_PREFIX + resource->id();
        assert(_global->getKeyValue(key) != std::nullopt);

        _global->putKeyValue(key, resource->owner());
    }

    void Fabric::unpublish(IResource* resource) {
        // FIXME: this should generate a runtime exception

        // We can only unpublish rsources we own
        assert(resource->owner() == _global->getNodeId());

        auto key = Configuration::FABRIC_PREFIX + resource->id();
        _global->dropKeyValue(key);
    }

    IResource* Fabric::load(const std::string& id) {
        // Look up the owner from the KV
        return nullptr;  // fixme
    }

}
