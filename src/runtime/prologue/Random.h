#ifndef SWARM_RANDOM_H
#define SWARM_RANDOM_H

#include "../../shared/rand.h"
#include "../../lang/AST.h"
#include "IPrologueFunction.h"

namespace swarmc {
namespace Runtime {
namespace Prologue {

    class Random : public IPrologueFunction {
    public:
        Random() : IPrologueFunction("random") {}

        ExpressionNode* call(ExpressionList *args) override {
            return new NumberLiteralExpressionNode(getNewPosition(), randomDouble());
        }

        const FunctionType* type() const override {
            return FunctionType::of(PrimitiveType::of(Lang::ValueType::TNUM));
        }
    };

    class RandomVector : public IPrologueFunction {
    public:
        RandomVector() : IPrologueFunction("randomVector") {}

        ExpressionNode* call(ExpressionList* args) override {
            auto node = new EnumerationLiteralExpressionNode(getNewPosition(), new ExpressionList);
            auto nEntries = (NumberLiteralExpressionNode*) args->at(0);
            if ( nEntries->value() <= 0 ) return node;

            for ( size_t i = 0; i < nEntries->value(); i += 1 ) {
                node->push(new NumberLiteralExpressionNode(getNewPosition(), randomDouble()));
            }

            return node;
        }

        const FunctionType* type() const override {
            auto ft = FunctionType::of(GenericType::of(ValueType::TENUMERABLE, PrimitiveType::of(ValueType::TNUM)));
            ft->addArgument(PrimitiveType::of(ValueType::TNUM));
            return ft;
        }
    };

    class RandomMatrix : public IPrologueFunction {
    public:
        RandomMatrix() : IPrologueFunction("randomMatrix") {}

        ExpressionNode* call(ExpressionList* args) override {
            auto node = new EnumerationLiteralExpressionNode(getNewPosition(), new ExpressionList);
            auto nRows = (NumberLiteralExpressionNode*) args->at(0);
            auto nCols = (NumberLiteralExpressionNode*) args->at(1);
            if ( nRows->value() <= 0 ) return node;

            for ( size_t row = 0; row < nRows->value(); row += 1 ) {
                auto rowNode = new EnumerationLiteralExpressionNode(getNewPosition(), new ExpressionList);
                if ( nCols->value() <= 0 ) {
                    node->push(rowNode);
                    continue;
                }

                for ( size_t col = 0; col < nCols->value(); col += 1 ) {
                    rowNode->push(new NumberLiteralExpressionNode(getNewPosition(), randomDouble()));
                }

                node->push(rowNode);
            }

            return node;
        }

        const FunctionType* type() const override {
            auto ft = FunctionType::of(GenericType::of(ValueType::TENUMERABLE, GenericType::of(ValueType::TENUMERABLE, PrimitiveType::of(ValueType::TNUM))));
            ft->addArgument(PrimitiveType::of(ValueType::TNUM));
            ft->addArgument(PrimitiveType::of(ValueType::TNUM));
            return ft;
        }
    };

}
}
}

#endif //SWARM_RANDOM_H
