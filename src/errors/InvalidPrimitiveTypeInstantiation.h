#ifndef SWARMC_INVALIDPRIMITIVETYPEINSTANTIATION_H
#define SWARMC_INVALIDPRIMITIVETYPEINSTANTIATION_H

#include "SwarmError.h"

namespace swarmc::Errors {

    class InvalidPrimitiveTypeInstantiationError : public SwarmError {
    public:
        InvalidPrimitiveTypeInstantiationError() : SwarmError("Tried to instantiate PrimitiveType with non-primitive ValueType.") {}
    };

}

#endif
