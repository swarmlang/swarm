#include <string>
#include <ostream>
#include "AST.h"

#ifndef SWARMC_SPACE
#define SWARMC_SPACE "  "
#endif

namespace swarmc {
namespace Lang {

    void ProgramNode::printTree(std::ostream& out, std::string prefix) const {
        out << prefix << toString() << std::endl;
        for ( auto stmt : *_body ) {
            stmt->printTree(out, prefix + SWARMC_SPACE);
        }
    }

    void ExpressionNode::printTree(std::ostream &out, std::string prefix) const {
        out << prefix << toString() << std::endl;
    }

    void ExpressionStatementNode::printTree(std::ostream &out, std::string prefix) const {
        out << prefix << toString() << std::endl;
        _exp->printTree(out, prefix + SWARMC_SPACE);
    }

    void TypeNode::printTree(std::ostream &out, std::string prefix) const {
        out << prefix << toString() << std::endl;
    }

    void VariableDeclarationNode::printTree(std::ostream &out, std::string prefix) const {
        out << prefix << toString() << std::endl;
        _type->printTree(out, prefix + SWARMC_SPACE);
        _value->printTree(out, prefix + SWARMC_SPACE);
    }

    void AssignExpressionNode::printTree(std::ostream &out, std::string prefix) const {
        out << prefix << toString() << std::endl;
        _value->printTree(out, prefix + SWARMC_SPACE);
    }

    void CallExpressionNode::printTree(std::ostream& out, std::string prefix) const {
        out << prefix << toString() << std::endl;
        for ( auto exp : *_args ) {
            exp->printTree(out, prefix + SWARMC_SPACE);
        }
    }

    void BinaryExpressionNode::printTree(std::ostream &out, std::string prefix) const {
        out << prefix << toString() << std::endl;
        _left->printTree(out, prefix + SWARMC_SPACE);
        _right->printTree(out, prefix + SWARMC_SPACE);
    }

    void UnaryExpressionNode::printTree(std::ostream &out, std::string prefix) const {
        out << prefix << toString() << std::endl;
        _exp->printTree(out, prefix + SWARMC_SPACE);
    }

    void EnumerationLiteralExpressionNode::printTree(std::ostream& out, std::string prefix) const {
        out << prefix << toString() << std::endl;
        for ( auto exp : *_actuals ) {
            exp->printTree(out, prefix + SWARMC_SPACE);
        }
    }

    void BlockStatementNode::printTree(std::ostream& out, std::string prefix) const {
        out << prefix << toString() << std::endl;
        for ( auto stmt : *_body ) {
            stmt->printTree(out, prefix + SWARMC_SPACE);
        }
    }

    void MapStatementNode::printTree(std::ostream &out, std::string prefix) const {
        out << prefix << toString() << std::endl;
        _value->printTree(out, prefix + SWARMC_SPACE);
    }

    void MapNode::printTree(std::ostream &out, std::string prefix) const {
        out << prefix << toString() << std::endl;
        for ( auto entry : *_body ) {
            entry->printTree(out, prefix + SWARMC_SPACE);
        }
    }
}
}

