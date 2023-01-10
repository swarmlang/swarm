#ifndef SWARMVM_FABRIC
#define SWARMVM_FABRIC

#include "../../../mod/binn/src/binn.h"
#include "../../shared/nslib.h"
#include "../../errors/RuntimeError.h"
#include "./interfaces.h"

namespace swarmc::Runtime {

    class VirtualMachine;

    enum class ResourceCategory: std::size_t {
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
        [[nodiscard]] virtual SchedulingFilters getSchedulingFilters() const { return {}; }

        virtual void acquire(VirtualMachine*) {}
        virtual void release(VirtualMachine*) {}

        virtual void replicateLocally(VirtualMachine*) {
            throw Errors::RuntimeError(Errors::RuntimeExCode::AttemptedCloneOfNonReplicableResource, "Cannot clone non-replicable resource: " + s(this));
        }
    };


    class Fabric : public IStringable, public IRefCountable {
    public:
        explicit Fabric(IGlobalServices* global) : _global(useref(global)) {}

        ~Fabric() override {
            freeref(_global);
        }

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
