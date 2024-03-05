#ifndef SWARMVM_STRING_HELPERS
#define SWARMVM_STRING_HELPERS

#include <utility>

#include "prologue_provider.h"
#include "../isa_meta.h"

namespace swarmc::Runtime::Prologue {

    class CharCountFunctionCall : public PrologueFunctionCall {
    public:
        CharCountFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "CHAR_COUNT", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "CharCountFunctionCall<>";
        }
    };

    class CharCountFunction : public PrologueFunction {
    public:
        explicit CharCountFunction(IProvider* provider) : PrologueFunction("CHAR_COUNT", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override {
            return {Type::Primitive::of(Type::Intrinsic::STRING)};
        }

        [[nodiscard]] Type::Type* returnType() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "CharCountFunction<>";
        }
    };

    class CharAtFunctionCall : public PrologueFunctionCall {
    public:
        CharAtFunctionCall(IProvider* provider, const CallVector& vector, Type::Type* returnType):
            PrologueFunctionCall(provider, "CHAR_AT", vector, returnType) {}

        void execute(VirtualMachine*) override;

        [[nodiscard]] std::string toString() const override {
            return "CharAtFunctionCall<>";
        }
    };

    class CharAtFunction : public PrologueFunction {
    public:
        explicit CharAtFunction(IProvider* provider) : PrologueFunction("CHAR_AT", provider) {}

        [[nodiscard]] FormalTypes paramTypes() const override {
            return {Type::Primitive::of(Type::Intrinsic::STRING), Type::Primitive::of(Type::Intrinsic::NUMBER)};
        }

        [[nodiscard]] Type::Type* returnType() const override {
            return Type::Primitive::of(Type::Intrinsic::STRING);
        }

        [[nodiscard]] PrologueFunctionCall* call(CallVector) const override;

        [[nodiscard]] std::string toString() const override {
            return "CharAtFunction<>";
        }
    };

}

#endif //SWARMVM_STRING_HELPERS
