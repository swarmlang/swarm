#include "runtime_functions.h"
#include "../isa_meta.h"

namespace nslib {
    std::string s(swarmc::Runtime::FunctionBackend v) {
        if ( v == swarmc::Runtime::FunctionBackend::FB_INLINE ) return "FunctionBackend(INLINE)";
        if ( v == swarmc::Runtime::FunctionBackend::FB_PROVIDER ) return "FunctionBackend(PROVIDER)";
        return "FunctionBackend(UNKNOWN)";
    }
}

namespace swarmc::Runtime {

    std::string CurriedFunction::toString() const {
        return "CurriedFunction<f: " + _upstream->toString() + ", ref: " + _ref->toString() + ">";
    }

    std::string InlineFunction::toString() const {
        return "InlineFunction<f:" + _name + ", rt: " + _returnType->toString() + ">";
    }

}
