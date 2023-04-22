#include "count.h"

namespace swarmc::Runtime::Prologue {

    void CountFunctionCall::execute(VirtualMachine*) {
        auto enumerable = (ISA::EnumerationReference*) _vector.at(0).second;
        setReturn(new ISA::NumberReference(static_cast<double>(enumerable->length())));
    }

    PrologueFunctionCall* CountFunction::call(CallVector vector) const {
        return new CountFunctionCall(_provider, vector, returnType());
    }

}
