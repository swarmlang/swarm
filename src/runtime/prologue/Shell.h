#ifndef SWARM_SHELL_H
#define SWARM_SHELL_H

#include "../../lang/AST.h"
#include "IPrologueFunction.h"
#include "../../lib/subprocess.hpp"

namespace swarmc {
namespace Runtime {
namespace Prologue {

    class Shell : public IPrologueFunction {
    public:
        Shell() : IPrologueFunction("shell") {}

        ExpressionNode* call(ExpressionList *args) override {
            auto cmd = (StringLiteralExpressionNode*) args->at(0);

            console->debug("+ " + cmd->value());

            std::stringstream output;
            subprocess::popen sh("bash", {"-c", cmd->value()});
            output << sh.stdout().rdbuf();
            sh.wait();

            return new StringLiteralExpressionNode(getNewPosition(), output.str());
        }

        const FunctionType* type() const override {
            auto ft = FunctionType::of(PrimitiveType::of(ValueType::TSTRING));
            ft->addArgument(PrimitiveType::of(ValueType::TSTRING));
            return ft;
        }
    };

}
}
}

#endif //SWARM_SHELL_H
