#include "prologue_provider.h"
#include "to_string.h"

namespace swarmc::Runtime::Prologue {

    PrologueFunction* Provider::loadFunction(std::string name) {
        if ( name == "NUMBER_TO_STRING" ) return new NumberToStringFunction(this);
        if ( name == "BOOLEAN_TO_STRING" ) return new BooleanToStringFunction(this);
        return nullptr;
    }

    void Provider::call(IProviderFunctionCall* call) {
        call->execute();
    }

}
