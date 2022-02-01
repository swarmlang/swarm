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

    bool IfStatement::nameAnalysis(SymbolTable* symbols) {
        if (!_condition->nameAnalysis(symbols)) {
            return false;
        }

        symbols->enter();
        bool bodyResult = BlockStatementNode::nameAnalysis(symbols);
        symbols->leave();
        return bodyResult;
    }

    bool WhileStatement::nameAnalysis(SymbolTable* symbols) {
        if (!_condition->nameAnalysis(symbols) ) {
            return false;
        }

        symbols->enter();
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



    /************* TYPE ANALYSIS *************/

    bool ProgramNode::typeAnalysis(TypeTable* types) {
        for ( auto stmt : *_body ) {
            if ( !stmt->typeAnalysis(types) ) {
                return false;
            }
        }

        types->setTypeOf(this, PrimitiveType::of(TUNIT));
        return true;
    }

    bool ExpressionStatementNode::typeAnalysis(TypeTable* types) {
        bool expResult = _exp->typeAnalysis(types);

        if ( expResult ) {
            types->setTypeOf(this, PrimitiveType::of(TUNIT));
        }

        return expResult;
    }

    bool IdentifierNode::typeAnalysis(TypeTable* types) {
        if ( _symbol->type() == nullptr ) {
            Reporting::typeError(
                position(),
                "Invalid type of free identifier: " + _name
            );

            return false;
        }

        types->setTypeOf(this, _symbol->type());
        return true;
    }

    bool TypeNode::typeAnalysis(TypeTable* types) {
        types->setTypeOf(this, PrimitiveType::of(TUNIT));
        return true;
    }

    bool BooleanLiteralExpressionNode::typeAnalysis(TypeTable* types) {
        types->setTypeOf(this, PrimitiveType::of(TBOOL));
        return true;
    }

    bool VariableDeclarationNode::typeAnalysis(TypeTable* types) {
        if (
            !_type->typeAnalysis(types)
            || !_id->typeAnalysis(types)
            || !_value->typeAnalysis(types)
        ) {
            return false;
        }

        const Type* typeOfValue = types->getTypeOf(_value);
        if ( !_type->type()->is(typeOfValue) ) {
            Reporting::typeError(
                position(),
                "Attempted to initialize identifier of type " + _type->type()->toString() + " with value of type " + typeOfValue->toString() + "."
            );

            return false;
        }

        types->setTypeOf(this, PrimitiveType::of(TUNIT));
        return true;
    }

    bool AssignExpressionNode::typeAnalysis(TypeTable* types) {
        if (
            !_dest->typeAnalysis(types)
            || !_value->typeAnalysis(types)
        ) {
            return false;
        }

        const Type* typeOfValue = types->getTypeOf(_value);
        const Type* typeOfDest = types->getTypeOf(_dest);

        if ( !typeOfValue->is(typeOfDest) ) {
            Reporting::typeError(
                position(),
                "Attempted to assign value of type " + typeOfValue->toString() + " to lval of type " + typeOfDest->toString() + "."
            );

            return false;
        }

        types->setTypeOf(this, PrimitiveType::of(TUNIT));
        return true;
    }

    bool CallExpressionNode::typeAnalysis(TypeTable* types) {
        // Perform type analysis on the callable
        if ( !_id->typeAnalysis(types) ) {
            return false;
        }

        // Perform type analysis on the arguments
        for ( auto arg : *_args ) {
            if ( !arg->typeAnalysis(types) ) {
                return false;
            }
        }

        // Make sure the callee is actually a function type
        const Type* baseTypeOfCallee = types->getTypeOf(_id);
        if ( baseTypeOfCallee->kind() != KFUNCTION ) {
            Reporting::typeError(
                position(),
                "Attempted to call non-callable type " + baseTypeOfCallee->toString() + "."
            );
        }

        const FunctionType* typeOfCallee = (FunctionType*) baseTypeOfCallee;
        std::vector<Type*>* argTypes = typeOfCallee->getArgumentTypes();

        // Make sure the # of arguments matches
        if ( argTypes->size() != _args->size() ) {
            Reporting::typeError(
                position(),
                "Invalid number of arguments for call (expected: " + std::to_string(argTypes->size()) + ")."
            );

            return false;
        }

        // Make sure the type of each argument matches
        for ( size_t i = 0; i < argTypes->size(); i += 1 ) {
            const Type* expectedType = argTypes->at(i);
            const Type* actualType = types->getTypeOf(_args->at(i));

            if ( !actualType->is(expectedType) ) {
                Reporting::typeError(
                    position(),
                    "Invalid argument of type " + actualType->toString() + " in position " + std::to_string(i) + " (expected: " + expectedType->toString() + ")."
                );

                return false;
            }
        }

        types->setTypeOf(this, typeOfCallee->returnType());
        return true;
    }

    bool PureBinaryExpressionNode::typeAnalysis(TypeTable* types) {
        if (
            !_left->typeAnalysis(types)
            || !_right->typeAnalysis(types)
        ) {
            return false;
        }

        const Type* actualLeftType = types->getTypeOf(_left);
        const Type* actualRightType = types->getTypeOf(_right);

        console->debug("Typing PureBinaryExpressionNode for " + getName());

        if ( !leftType()->is(actualLeftType) ) {
            Reporting::typeError(
                position(),
                "Invalid type " + actualLeftType->toString() + " of left-hand operand to expression (expected: " + leftType()->toString() + ")."
            );

            return false;
        }

        if ( !rightType()->is(actualRightType) ) {
            Reporting::typeError(
                position(),
                "Invalid type " + actualRightType->toString() + " of right-hand operand to expression (expected: " + rightType()->toString() + ")."
            );

            return false;
        }

        types->setTypeOf(this, resultType());
        return true;
    }

    bool EqualsNode::typeAnalysis(TypeTable* types) {
        if (
            !_left->typeAnalysis(types)
            || !_right->typeAnalysis(types)
        ) {
            return false;
        }

        const Type* actualLeftType = types->getTypeOf(_left);
        const Type* actualRightType = types->getTypeOf(_right);
        if ( !actualLeftType->is(actualRightType) ) {
            Reporting::typeError(
                position(),
                "Invalid comparison between left-hand type " + actualLeftType->toString() + " and right-hand type " + actualRightType->toString() + "."
            );

            return false;
        }

        types->setTypeOf(this, PrimitiveType::of(TBOOL));
        return true;
    }

    bool NotEqualsNode::typeAnalysis(TypeTable* types) {
        if (
            !_left->typeAnalysis(types)
            || !_right->typeAnalysis(types)
        ) {
            return false;
        }

        const Type* actualLeftType = types->getTypeOf(_left);
        const Type* actualRightType = types->getTypeOf(_right);
        if ( !actualLeftType->is(actualRightType) ) {
            Reporting::typeError(
                position(),
                "Invalid comparison between left-hand type " + actualLeftType->toString() + " and right-hand type " + actualRightType->toString() + "."
            );

            return false;
        }

        types->setTypeOf(this, PrimitiveType::of(TBOOL));
        return true;
    }

    bool IfStatement::typeAnalysis(TypeTable* types) {
        if (!_condition->typeAnalysis(types)) {
            return false;
        }

        const Type* condType = types->getTypeOf(_condition);
        if ( condType->is(PrimitiveType::of(TBOOL))) {
            Reporting::typeError(
                position(),
                "Condition for if loop not boolean " + condType->toString() + "."
            );
        }
    }

    bool WhileStatement::typeAnalysis(TypeTable* types) {
        if (!_condition->typeAnalysis(types)) {
            return false;
        }

        const Type* condType = types->getTypeOf(_condition);
        if ( condType->is(PrimitiveType::of(TBOOL))) {
            Reporting::typeError(
                position(),
                "Condition for while loop not boolean " + condType->toString() + "."
            );
        }
    }
}
}
