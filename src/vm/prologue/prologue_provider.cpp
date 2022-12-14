#include "prologue_provider.h"
#include "to_string.h"
#include "trig.h"
#include "rand.h"
#include "range.h"
#include "tag.h"
#include "FileResource.h"

namespace swarmc::Runtime::Prologue {

    PrologueFunction* Provider::loadFunction(std::string name) {
        if ( name == "NUMBER_TO_STRING" ) return new NumberToStringFunction(this);
        if ( name == "BOOLEAN_TO_STRING" ) return new BooleanToStringFunction(this);
        if ( name == "SIN" ) return new TrigFunction(TrigOperation::SIN, this);
        if ( name == "COS" ) return new TrigFunction(TrigOperation::COS, this);
        if ( name == "TAN" ) return new TrigFunction(TrigOperation::TAN, this);
        if ( name == "RANDOM" ) return new RandomFunction(this);
        if ( name == "RANDOM_VECTOR" ) return new RandomVectorFunction(this);
        if ( name == "RANDOM_MATRIX" ) return new RandomMatrixFunction(this);
        if ( name == "RANGE" ) return new RangeFunction(this);
//        if ( name == "TAG" ) return new TagFunction(this);
        if ( name == "OPEN_FILE" ) return new OpenFileFunction(this);
        if ( name == "READ_FILE" ) return new ReadFileFunction(this);
        return nullptr;
    }

    void Provider::call(VirtualMachine* vm, IProviderFunctionCall* call) {
        call->execute(vm);
    }

}
