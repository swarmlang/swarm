#include <chrono>
#include "time_helpers.h"
#include "../isa_meta.h"
#include "../VirtualMachine.h"

namespace swarmc::Runtime::Prologue {

    void TimeFunctionCall::execute(VirtualMachine* vm) {
        setReturn(new ISA::NumberReference(vm->global()->getCurrentTime()));
    }

    PrologueFunctionCall* TimeFunction::call(CallVector vector) const {
        return new TimeFunctionCall(_provider, vector, returnType());
    }

}
