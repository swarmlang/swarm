#include <string>
#include "Type.h"

namespace swarmc {
namespace Lang {
    bool Type::isPrimitiveValueType(ValueType t) {
        return (
            t == ValueType::TSTRING
            || t == ValueType::TNUM
            || t == ValueType::TBOOL
        );
    }

    std::string Type::valueTypeToString(ValueType t) {
        if ( t == ValueType::TSTRING ) return "TSTRING";
        if ( t == ValueType::TBOOL ) return "TBOOL";
        if ( t == ValueType::TENUMERABLE ) return "TENUMERABLE";
        if ( t == ValueType::TMAP ) return "TMAP";
        if ( t == ValueType::TNUM ) return "TNUM";
        return "TUNKNOWN";
    }
}
}
