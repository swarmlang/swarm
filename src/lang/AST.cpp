#include <string>
#include <ostream>
#include "AST.h"
#include "SymbolTable.h"
#include "../Reporting.h"
#include "Type.h"

#ifndef SWARMC_SPACE
#define SWARMC_SPACE "  "
#endif

namespace swarmc {
namespace Lang {

    /************* PRINT TREE *************/

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



    /************* NAME ANALYSIS *************/

    bool ProgramNode::nameAnalysis(SymbolTable* symbols) {
        // Enter the global scope
        symbols->enter();

        for ( auto stmt : *_body ) {
            if ( !stmt->nameAnalysis(symbols) ) {
                symbols->leave();
                return false;
            }
        }

        // Exit the global scope
        symbols->leave();
        return true;
    }

    bool ProgramNode::nameAnalysis() {
        SymbolTable* sym = new SymbolTable();
        return nameAnalysis(sym);
    }

    bool ExpressionStatementNode::nameAnalysis(SymbolTable* symbols) {
        return _exp->nameAnalysis(symbols);
    }

    bool IdentifierNode::nameAnalysis(SymbolTable* symbols) {
        std::string name = _name;
        _symbol = symbols->lookup(name);

        if ( _symbol == nullptr ) {
            Reporting::nameError(position(), "Use of free identifier \"" + name + "\".");
            return false;
        }

        return true;
    }

    bool TypeNode::nameAnalysis(SymbolTable* symbols) {
        return true;
    }

    bool BooleanLiteralExpressionNode::nameAnalysis(SymbolTable* symbols) {
        return true;
    }

    bool VariableDeclarationNode::nameAnalysis(SymbolTable* symbols) {
        std::string name = _id->name();
        Type* type = _type->type();

        // Make sure the name isn't already declared in this scope
        if ( symbols->isClashing(name) ) {
            SemanticSymbol* existing = symbols->lookup(name);
            Reporting::nameError(position(), "Redeclaration of identifier \"" + name + "\" first declared at " + existing->declaredAt()->start() + ".");
            return false;
        }

        // Check the RHS of the assignment
        bool valueResult = _value->nameAnalysis(symbols);

        // Add the declaration to the current scope
        symbols->addVariable(name, type, position());

        // Call this to attach the Symbol to the IdentifierNode
        _id->nameAnalysis(symbols);
        return valueResult;
    }

    bool AssignExpressionNode::nameAnalysis(SymbolTable* symbols) {
        bool lvalResult = _dest->nameAnalysis(symbols);
        bool rvalResult = _value->nameAnalysis(symbols);
        return lvalResult && rvalResult;
    }

    bool CallExpressionNode::nameAnalysis(SymbolTable* symbols) {
        if ( !_id->nameAnalysis(symbols) ) {
            return false;
        }

        for ( auto arg : *_args ) {
            if ( !arg->nameAnalysis(symbols) ) {
                return false;
            }
        }

        return true;
    }

    bool BinaryExpressionNode::nameAnalysis(SymbolTable* symbols) {
        bool leftResult = _left->nameAnalysis(symbols);
        bool rightResult = _right->nameAnalysis(symbols);
        return leftResult && rightResult;
    }

    bool UnaryExpressionNode::nameAnalysis(SymbolTable* symbols) {
        return _exp->nameAnalysis(symbols);
    }

    bool EnumerationLiteralExpressionNode::nameAnalysis(SymbolTable* symbols) {
        for ( auto actual : *_actuals ) {
            if ( !actual->nameAnalysis(symbols) ) {
                return false;
            }
        }

        return true;
    }

    bool BlockStatementNode::nameAnalysis(SymbolTable* symbols) {
        for ( auto stmt : *_body ) {
            if ( !stmt->nameAnalysis(symbols) ) {
                return false;
            }
        }

        return true;
    }

    bool EnumerationStatement::nameAnalysis(SymbolTable* symbols) {
        if ( !_enumerable->nameAnalysis(symbols) ) {
            return false;
        }

        // Need to register the block-local variable
        // Its type is implicit as the generic type of the enumerable
        std::string name = _local->name();
        Position* pos = _local->position();
        Type* type = nullptr;

        // Try to look up the generic type of the enumerable
        const Type* enumType = _enumerable->symbol()->type();
        if ( enumType->kind() == KGENERIC ) {
            GenericType* enumGenericType = (GenericType*) enumType;
            type = enumGenericType->concrete();
        }

        // Start a new scope in the body and add the local
        symbols->enter();
        symbols->addVariable(name, type, pos);

        if ( !_local->nameAnalysis(symbols) ) {
            symbols->leave();
            return false;
        }

        bool bodyResult = BlockStatementNode::nameAnalysis(symbols);
        symbols->leave();
        return bodyResult;
    }

    bool WithStatement::nameAnalysis(SymbolTable* symbols) {
        if ( !_resource->nameAnalysis(symbols) ) {
            return false;
        }

        // need to register the block-local variable
        // Its type is implicit as the result of the expression
        std::string name = _local->name();
        Position* pos = _local->position();
        Type* type = nullptr;

        // FIXME: type of local depends on the type of the expression

        // Start a new scope in the body and add the local
        symbols->enter();
        symbols->addVariable(name, type, pos);

        if ( !_local->nameAnalysis(symbols) ) {
            symbols->leave();
            return false;
        }

        bool bodyResult = BlockStatementNode::nameAnalysis(symbols);
        symbols->leave();
        return bodyResult;
    }

    bool MapStatementNode::nameAnalysis(SymbolTable* symbols) {
        return _value->nameAnalysis(symbols);
    }

    bool MapNode::nameAnalysis(SymbolTable* symbols) {
        // Check each entry in the map
        for ( auto entry : *_body ) {
            // Check for duplicate name
            size_t nameCount = 0;
            for ( auto subentry : *_body ) {
                if ( subentry->id()->name() == entry->id()->name() && nameCount < 2) {
                    nameCount += 1;
                }
            }

            if ( nameCount > 1 ) {
                Reporting::nameError(
                    entry->position(),
                    "Duplicate map key: \"" + entry->id()->name() + "\""
                );

                return false;
            }

            // Check the value expression
            if ( !entry->nameAnalysis(symbols) ) {
                return false;
            }
        }

        return true;
    }
}
}

