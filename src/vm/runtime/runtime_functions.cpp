#include "runtime_functions.h"
#include "../isa_meta.h"

namespace swarmc::Runtime {

    std::string CurriedFunction::toString() const {
        return "CurriedFunction<f: " + _upstream->toString() + ", ref: " + _ref->toString() + ">";
    }

    std::string InlineFunction::toString() const {
        return "InlineFunction<f:" + _name + ", rt: " + _returnType->toString() + ">";
    }

}
