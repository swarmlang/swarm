#include <cmath>
#include "../../errors/SwarmError.h"
#include "../../lang/Type.h"
#include "../isa_meta.h"
#include "trig.h"

namespace swarmc::Runtime::Prologue {

    void TrigFunctionCall::execute() {
        auto opd = (ISA::NumberReference*) _vector.at(0).second;

        double result;
        if ( _op == TrigOperation::SIN ) result = sin(opd->value());
        else if ( _op == TrigOperation::COS ) result = cos(opd->value());
        else if ( _op == TrigOperation::TAN ) result = tan(opd->value());
        else throw Errors::SwarmError("Invalid trigonometric operation.");

        setReturn(new ISA::NumberReference(result));
    }

    FormalTypes TrigFunction::paramTypes() const {
        return {Type::Primitive::of(Type::Intrinsic::NUMBER)};
    }

    const Type::Type* TrigFunction::returnType() const {
        return Type::Primitive::of(Type::Intrinsic::NUMBER);
    }

    PrologueFunctionCall* TrigFunction::call(CallVector vector) const {
        return new TrigFunctionCall(_op, _provider, vector, returnType());
    }

}
