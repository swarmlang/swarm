#ifndef SWARM_FILE_H
#define SWARM_FILE_H

#include "IPrologueFunction.h"
#include "../../lang/Type.h"

namespace swarmc {
namespace Runtime {
namespace Prologue {

    class FileContents final : public IPrologueFunction {
    public:
        FileContents() : IPrologueFunction("fileContents") {}

        ExpressionNode* call(ExpressionList *args) override {
            return nullptr;  // FIXME implement
        }

        const FunctionType* type() const override {
            auto rsc = GenericType::of(ValueType::TRESOURCE, PrimitiveType::of(ValueType::TSTRING));
            auto type = FunctionType::of(rsc);
            type->addArgument(PrimitiveType::of(ValueType::TSTRING));
            return type;
        }
    };

}
}
}

#endif //SWARM_FILE_H
