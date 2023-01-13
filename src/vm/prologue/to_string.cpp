#include "to_string.h"
#include "../../lang/Type.h"
#include "../ISA.h"

namespace swarmc::Runtime::Prologue {

    void NumberToStringFunctionCall::execute(VirtualMachine*) {
        // At this point, the call should have been type-checked by the runtime
        // So, we're going to assume that the parameters are correct.
        auto opd = (ISA::NumberReference*) _vector.at(0).second;
        setReturn(new ISA::StringReference(std::to_string(opd->value())));
    }

    FormalTypes NumberToStringFunction::paramTypes() const {
        return {Type::Primitive::of(Type::Intrinsic::NUMBER)};
    }

    Type::Type* NumberToStringFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::STRING);
    }

    PrologueFunctionCall* NumberToStringFunction::call(CallVector vector) const {
        auto returnType = Type::Primitive::of(Type::Intrinsic::STRING);
        return new NumberToStringFunctionCall(_provider, vector, returnType);
    }

    void BooleanToStringFunctionCall::execute(VirtualMachine*) {
        auto opd = (ISA::BooleanReference*) _vector.at(0).second;
        if ( opd->value() ) setReturn(new ISA::StringReference("true"));
        else setReturn(new ISA::StringReference("false"));
    }

    FormalTypes BooleanToStringFunction::paramTypes() const {
        return {Type::Primitive::of(Type::Intrinsic::BOOLEAN)};
    }

    Type::Type* BooleanToStringFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::STRING);
    }

    PrologueFunctionCall* BooleanToStringFunction::call(CallVector vector) const {
        auto returnType = Type::Primitive::of(Type::Intrinsic::STRING);
        return new BooleanToStringFunctionCall(_provider, vector, returnType);
    }

}
