#ifndef SWARMVM_FABRIC
#define SWARMVM_FABRIC

#include "../../../mod/binn/src/binn.h"
#include "../../shared/nslib.h"
#include "./interfaces.h"

namespace swarmc::Runtime {

    enum class ResourceCategory: size_t {
        TUNNELED = 1 << 0,
        REPLICATED = 1 << 1,
        EXCLUSIVE = 1 << 2,
    };


    class IResource : public IStringable {
    public:
        [[nodiscard]] virtual ResourceCategory category() const = 0;
        [[nodiscard]] virtual std::string id() const = 0;
        [[nodiscard]] virtual NodeID owner() const = 0;
        [[nodiscard]] virtual std::string name() const = 0;
        [[nodiscard]] virtual const Type::Type* innerType() const = 0;
    };


    class Fabric : public IStringable {
    public:
        explicit Fabric(IGlobalServices* global) : _global(global) {}

        virtual void publish(IResource*);

        virtual void unpublish(IResource*);

        virtual IResource* load(const std::string& id);

        [[nodiscard]] std::string toString() const override {
            return "Fabric<>";
        }

    protected:
        IGlobalServices* _global;
    };

}

#endif //SWARMVM_FABRIC
