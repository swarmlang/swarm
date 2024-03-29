#include "prologue_provider.h"
#include "to_string.h"
#include "trig.h"
#include "rand.h"
#include "range.h"
#include "FileResource.h"
#include "TagResource.h"
#include "resource.h"
#include "type_construction.h"
#include "numeric.h"
#include "count.h"
#include "time_helpers.h"
#include "vectors.h"
#include "SocketResource.h"
#include "string_helpers.h"


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
        if ( name == "TAG" ) return new TagFunction(this);
        if ( name == "OPEN_FILE" ) return new OpenFileFunction(this);
        if ( name == "READ_FILE" ) return new ReadFileFunction(this);
        if ( name == "WRITE_FILE" ) return new WriteFileFunction(this);
        if ( name == "APPEND_FILE" ) return new AppendFileFunction(this);
        if ( name == "RESOURCE_T" ) return new ResourceTFunction(this);
        if ( name == "FILE_T" ) return new FileTFunction(this);
        if ( name == "TAG_T" ) return new TagTFunction(this);
        if ( name == "LAMBDA0_T" ) return new Lambda0Function(this);
        if ( name == "LAMBDA1_T" ) return new Lambda1Function(this);
        if ( name == "CONTEXT_ID_T" ) return new ContextIdFunction(this);
        if ( name == "JOB_ID_T" ) return new JobIdFunction(this);
        if ( name == "RETURN_VALUE_MAP_T" ) return new ReturnValueMapFunction(this);
        if ( name == "FLOOR" ) return new FloorFunction(this);
        if ( name == "CEILING" ) return new CeilingFunction(this);
        if ( name == "NTH_ROOT" ) return new NthRootFunction(this);
        if ( name == "MAX" ) return new MaxFunction(this);
        if ( name == "MIN" ) return new MinFunction(this);
        if ( name == "COUNT" ) return new CountFunction(this);
        if ( name == "TIME" ) return new TimeFunction(this);
        if ( name == "ZERO_VECTOR" ) return new ZeroVectorFunction(this);
        if ( name == "ZERO_MATRIX" ) return new ZeroMatrixFunction(this);
        if ( name == "VECTOR_TO_STRING" ) return new VectorToStringFunction(this);
        if ( name == "MATRIX_TO_STRING" ) return new MatrixToStringFunction(this);
        if ( name == "SUBVECTOR" ) return new SubVectorFunction(this);
        if ( name == "SUBMATRIX" ) return new SubMatrixFunction(this);
        if ( name == "SOCKET_T" ) return new SocketTFunction(this);
        if ( name == "SOCKET" ) return new SocketFunction(this);
        if ( name == "OPEN_SOCKET" ) return new OpenSocketFunction(this);
        if ( name == "ACCEPT_SOCKET_CONNECTION" ) return new AcceptSocketConnectionFunction(this);
        if ( name == "READ_FROM_CONNECTION" ) return new ReadFromConnectionFunction(this);
        if ( name == "CHAR_COUNT" ) return new CharCountFunction(this);
        if ( name == "CHAR_AT" ) return new CharAtFunction(this);

        return nullptr;
    }

    void Provider::call(VirtualMachine* vm, IProviderFunctionCall* call) {
        GC_LOCAL_REF(call)
        call->execute(vm);
    }

}
