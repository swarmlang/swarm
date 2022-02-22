#ifndef SWARM_LOG_H
#define SWARM_LOG_H

#include "../../lang/AST.h"
#include "../ISwarmFunction.h"
#include "../../shared/util/Console.h"

namespace swarmc {
namespace Runtime {
namespace Prologue {

    class Log : public ISwarmFunction {
    public:
        virtual bool validateCall(const std::vector<Lang::ExpressionNode*>* args) const {
            if ( args->size() != 1 ) return false;
            return (
                args->size() == 1
                && args->at(0)->isValue()
                && args->at(0)->type()->is(Lang::PrimitiveType::of(Lang::ValueType::TSTRING))
            );
        }

        virtual Lang::ExpressionNode* call(std::vector<Lang::ExpressionNode*>* args) {
            Console::get()->info(((Lang::StringLiteralExpressionNode*) args->at(0))->value());
            return getUnit();
        }
    };

    class LogError : public ISwarmFunction {
    public:
        virtual bool validateCall(const std::vector<Lang::ExpressionNode*>* args) const {
            if ( args->size() != 1 ) return false;
            return (
                args->size() == 1
                && args->at(0)->isValue()
                && args->at(0)->type()->is(Lang::PrimitiveType::of(Lang::ValueType::TSTRING))
            );
        }

        virtual Lang::ExpressionNode* call(std::vector<Lang::ExpressionNode*>* args) {
            Console::get()->error(((Lang::StringLiteralExpressionNode*) args->at(0))->value());
            return getUnit();
        }
    };

}
}
}

#endif //SWARM_LOG_H
