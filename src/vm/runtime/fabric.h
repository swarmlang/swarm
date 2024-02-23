#ifndef SWARMVM_FABRIC
#define SWARMVM_FABRIC

#include <utility>

#include "../../../mod/binn/src/binn.h"
#include "../../shared/nslib.h"
#include "../../errors/RuntimeError.h"
#include "./interfaces.h"
#include "runtime_provider.h"
#include "../../lang/Type.h"

namespace swarmc::Runtime {

    class VirtualMachine;

    using OperationName = std::string;
    using ResourceOperationFrame = std::vector<ISA::Reference*>;

    enum class ResourceCategory: std::size_t {
        TUNNELED = 1 << 0,
        REPLICATED = 1 << 1,
        EXCLUSIVE = 1 << 2,
    };


    class InvalidResourceOperation : public Errors::SwarmError {
    public:
        InvalidResourceOperation(const std::string& resource, const std::string& operation)
            : Errors::SwarmError("Invalid operation " + operation + " on resource " + resource) {}
    };


    class IResource : public IStringable {
    public:
        [[nodiscard]] virtual ResourceCategory category() const = 0;
        [[nodiscard]] virtual std::string id() const = 0;
        [[nodiscard]] virtual NodeID owner() const = 0;
        [[nodiscard]] virtual std::string name() const = 0;
        [[nodiscard]] virtual Type::Type* innerType() const = 0;
        [[nodiscard]] virtual InlineRefHandle<Type::Type> innerTypei() const;
        [[nodiscard]] virtual SchedulingFilters getSchedulingFilters() const { return {}; }

        virtual ResourceOperationFrame performOperation(VirtualMachine*, OperationName, ResourceOperationFrame) = 0;

        virtual void acquire(VirtualMachine*) {}
        virtual void release(VirtualMachine*) {}

        virtual void replicateLocally(VirtualMachine*) {
            throw Errors::RuntimeError(Errors::RuntimeExCode::AttemptedCloneOfNonReplicableResource, "Cannot clone non-replicable resource: " + s(this));
        }
    };


    class TunneledResourceOperationFunctionCall : public IFunctionCall {
    public:
        TunneledResourceOperationFunctionCall(CallVector vector, Type::Type* returnType) :
            IFunctionCall(FunctionBackend::FB_INTRINSIC, "FABRIC_TUNNEL_RESOURCE_OP", std::move(vector), returnType) {}

        [[nodiscard]] std::string toString() const override {
            return "TunneledResourceOperationFunctionCall<i:" + _name + ">";
        }
    };


    class TunneledResourceOperationFunction : public IFunction {
    public:
        [[nodiscard]] FormalTypes paramTypes() const override {
            // 3 params: the ID of the resource, the name of the operation, and a list of parameters to the operation
            auto stringT = Type::Primitive::of(Type::Intrinsic::STRING);
            auto enumAnyT = new Type::Enumerable(Type::Ambiguous::of());  // yes, this is equivalent to (void*) but in swarm
            return {stringT, stringT, enumAnyT};
        }

        [[nodiscard]] Type::Type* returnType() const override {
            return new Type::Enumerable(Type::Ambiguous::of());
        }

        [[nodiscard]] IFunction* curry(ISA::Reference* ref) override {
            return new CurriedFunction(ref, this);
        }

        [[nodiscard]] CallVector getCallVector() const override {
            return {};
        }

        [[nodiscard]] TunneledResourceOperationFunctionCall* call(CallVector v) const override {
            return new TunneledResourceOperationFunctionCall(v, returnType());
        }

        [[nodiscard]] FunctionBackend backend() const override {
            return FunctionBackend::FB_INTRINSIC;
        }

        [[nodiscard]] std::string name() const override {
            return "FABRIC_TUNNEL_RESOURCE_OP";
        }

        [[nodiscard]] std::string toString() const override {
            return "TunneledResourceOperationFunction<>";
        }
    };


    class TunneledResource : public IResource {
    public:
        TunneledResource(std::string id, NodeID owner, std::string name, Type::Type* type) : _id(std::move(id)), _owner(std::move(owner)), _name(std::move(name)), _type(type) {}

        [[nodiscard]] ResourceCategory category() const override {
            return ResourceCategory::TUNNELED;
        }

        [[nodiscard]] std::string id() const override {
            return _id;
        }

        [[nodiscard]] NodeID owner() const override {
            return _owner;
        }

        [[nodiscard]] std::string name() const override {
            return _name;
        }

        [[nodiscard]] Type::Type* innerType() const override {
            return _type;
        }

        ResourceOperationFrame performOperation(VirtualMachine*, OperationName op, ResourceOperationFrame) override;

        [[nodiscard]] std::string toString() const override {
            return "Runtime::TunneledResource<id: " + _id + ", owner: " + _owner + ", name: " + _name + ">";
        }
    protected:
        std::string _id;
        NodeID _owner;
        std::string _name;
        Type::Type* _type;
    };


    class Fabric : public IStringable, public IRefCountable {
    public:
        explicit Fabric(VirtualMachine* vm) : _vm(vm) {}

        ~Fabric() override {
//            freeref(_vm);
        }

        virtual bool shouldPublish(IResource*);

        virtual void publish(IResource*);

        virtual void unpublish(IResource*);

        virtual IResource* load(const std::string& id);

        [[nodiscard]] std::string toString() const override {
            return "Fabric<>";
        }

    protected:
        VirtualMachine* _vm;
    };

}

#endif //SWARMVM_FABRIC
