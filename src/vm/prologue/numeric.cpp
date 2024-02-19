#include <cmath>
#include "numeric.h"
#include "../isa_meta.h"

namespace swarmc::Runtime::Prologue {

    void CeilingFunctionCall::execute(VirtualMachine*) {
        auto num = (ISA::NumberReference*) _vector.at(0).second;
        setReturn(new ISA::NumberReference(ceil(num->value())));
    }

    PrologueFunctionCall* CeilingFunction::call(CallVector vector) const {
        return new CeilingFunctionCall(_provider, vector, returnType());
    }

    void FloorFunctionCall::execute(VirtualMachine*) {
        auto num = (ISA::NumberReference*) _vector.at(0).second;
        setReturn(new ISA::NumberReference(floor(num->value())));
    }

    PrologueFunctionCall* FloorFunction::call(CallVector vector) const {
        return new FloorFunctionCall(_provider, vector, returnType());
    }

    void NthRootFunctionCall::execute(VirtualMachine*) {
        auto nth = (ISA::NumberReference*) _vector.at(0).second;
        auto num = (ISA::NumberReference*) _vector.at(1).second;
        setReturn(new ISA::NumberReference(std::pow(num->value(), 1.0 / nth->value())));
    }

    PrologueFunctionCall* NthRootFunction::call(CallVector vector) const {
        return new NthRootFunctionCall(_provider, vector, returnType());
    }

    void MaxFunctionCall::execute(VirtualMachine*) {
        auto num1 = ((ISA::NumberReference*) _vector.at(0).second)->value();
        auto num2 = ((ISA::NumberReference*) _vector.at(1).second)->value();
        setReturn(new ISA::NumberReference(num1 > num2 ? num1 : num2));
    }

    PrologueFunctionCall* MaxFunction::call(CallVector vector) const {
        return new MaxFunctionCall(_provider, vector, returnType());
    }

    void MinFunctionCall::execute(VirtualMachine*) {
        auto num1 = ((ISA::NumberReference*) _vector.at(0).second)->value();
        auto num2 = ((ISA::NumberReference*) _vector.at(1).second)->value();
        setReturn(new ISA::NumberReference(num1 < num2 ? num1 : num2));
    }

    PrologueFunctionCall* MinFunction::call(CallVector vector) const {
        return new MinFunctionCall(_provider, vector, returnType());
    }

}
