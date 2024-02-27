#include "type_construction.h"
#include "../../lang/Type.h"
#include "../ISA.h"

namespace swarmc::Runtime::Prologue {

    void Lambda0FunctionCall::execute(VirtualMachine*) {
        auto opd = (ISA::TypeReference*) _vector.at(0).second;
        setReturn(new ISA::TypeReference(new Type::Lambda0(opd->value())));
    }

    FormalTypes Lambda0Function::paramTypes() const {
        return {Type::Primitive::of(Type::Intrinsic::TYPE)};
    }

    Type::Type* Lambda0Function::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::TYPE);
    }

    PrologueFunctionCall* Lambda0Function::call(CallVector vector) const {
        auto returnType = Type::Primitive::of(Type::Intrinsic::TYPE);
        return new Lambda0FunctionCall(_provider, vector, returnType);
    }

    void Lambda1FunctionCall::execute(VirtualMachine*) {
        auto arg = (ISA::TypeReference*) _vector.at(0).second;
        auto ret = (ISA::TypeReference*) _vector.at(1).second;
        setReturn(new ISA::TypeReference(new Type::Lambda1(arg->value(), ret->value())));
    }

    FormalTypes Lambda1Function::paramTypes() const {
        return {
            Type::Primitive::of(Type::Intrinsic::TYPE),
            Type::Primitive::of(Type::Intrinsic::TYPE)
        };
    }

    Type::Type* Lambda1Function::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::TYPE);
    }

    PrologueFunctionCall* Lambda1Function::call(CallVector vector) const {
        auto returnType = Type::Primitive::of(Type::Intrinsic::TYPE);
        return new Lambda1FunctionCall(_provider, vector, returnType);
    }

    void ContextIdFunctionCall::execute(VirtualMachine*) {
        setReturn(new ISA::TypeReference(contextIdType()));
    }

    Type::Type* ContextIdFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::TYPE);
    }

    PrologueFunctionCall* ContextIdFunction::call(CallVector vector) const {
        auto returnType = Type::Primitive::of(Type::Intrinsic::TYPE);
        return new ContextIdFunctionCall(_provider, vector, returnType);
    }



}
