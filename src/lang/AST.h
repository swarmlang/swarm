#ifndef SWARMC_AST_H
#define SWARMC_AST_H

#include "../shared/IStringable.h"
#include "Position.h"

namespace swarmc {
namespace Lang {

    /**
     * Base class for all AST nodes.
     */
    class ASTNode : public IStringable {
    public:
        ASTNode(Position* pos) : _pos(pos) {};

        /** Implements IStringable. */
        virtual std::string toString() const = 0;

        /** Get the node's Position instance. */
        virtual Position* position() const {
            return _pos;
        };

    private:
        Position* _pos = nullptr;
    };


    /**
     * AST node representing the root of the program.
     */
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
