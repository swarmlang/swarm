#ifndef SWARMC_AST_H
#define SWARMC_AST_H

#include "../shared/IStringable.h"
#include "Position.h"

namespace swarmc {
namespace Lang {

    class ASTNode : public IStringable {
    public:
        ASTNode(Position* pos) : _pos(pos) {};
        virtual std::string toString() const = 0;
        virtual Position* position() const {
            return _pos;
        };

    private:
        Position* _pos = nullptr;
    };



    class ProgramNode : public ASTNode {
    public:
        ProgramNode() : ASTNode(new Position(0, 0, 0, 0)) {}
        std::string toString() const override {
            return "ProgramNode<>";
        }
    };

}
}


#endif
