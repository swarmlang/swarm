#include "string_helpers.h"

namespace swarmc::Runtime::Prologue {

    void CharCountFunctionCall::execute(VirtualMachine*) {
        auto str = (ISA::StringReference*) _vector.at(0).second;
        setReturn(new ISA::NumberReference(static_cast<double>(str->value().size())));
    }

    PrologueFunctionCall* CharCountFunction::call(CallVector vector) const {
        return new CharCountFunctionCall(_provider, vector, returnType());
    }

    void CharAtFunctionCall::execute(VirtualMachine*) {
        auto str = (ISA::StringReference*) _vector.at(0).second;
        auto idx = (ISA::NumberReference*) _vector.at(1).second;

        std::stringstream s;
        s << str->value().at(idx->value());

        setReturn(new ISA::StringReference(s.str()));
    }

    PrologueFunctionCall* CharAtFunction::call(CallVector vector) const {
        return new CharAtFunctionCall(_provider, vector, returnType());
    }

}
