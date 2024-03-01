#ifndef SWARMC_NAME_ANALYSIS_WALK_H
#define SWARMC_NAME_ANALYSIS_WALK_H

#include "Walk.h"
#include "../../Reporting.h"
#include "../SymbolTable.h"

namespace swarmc::Lang::Walk {

class NameAnalysisWalk : public Walk<bool> {
public:
    NameAnalysisWalk() : Walk<bool>(), _symbols(new SymbolTable()) {}
    ~NameAnalysisWalk() override {
        delete _symbols;
    }
protected:
    bool walkProgramNode(ProgramNode* node) override {
        bool flag = true;
        // Enter the global scope
        _symbols->enter();

        for ( auto stmt : *node->body() ) {
            flag = walk(stmt) && flag;
            Reporting::nameDebug(stmt->position(), "finished " + s(stmt));
        }

        // Exit the global scope
        _symbols->leave();
        return flag;
    }

    bool walkExpressionStatementNode(ExpressionStatementNode* node) override {
        return walk(node->expression());
    }

    bool walkIdentifierNode(IdentifierNode* node) override {
        if ( node->_symbol != nullptr ) {
            return true;
        }

        std::string name = node->name();
        node->_symbol = useref(_symbols->lookup(name));

        if ( node->_symbol == nullptr ) {
            Reporting::nameError(node->position(), "Use of free identifier \"" + name + "\".");
            return false;
        }

        return true;
    }

    bool walkMapAccessNode(MapAccessNode* node) override {
        return walk(node->path());
    }

    bool walkEnumerableAccessNode(EnumerableAccessNode* node) override {
        bool flag = walk(node->path());
        return walk(node->index()) && flag;
    }

    bool walkTypeLiteral(swarmc::Lang::TypeLiteral *node) override {
        return walkType(node->value());
    }

    bool walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) override {
        return true;
    }

    bool walkVariableDeclarationNode(VariableDeclarationNode* node) override {
        if ( node->id()->symbol() != nullptr ) {
            return walk(node->value());
        }
        bool flag = walk(node->typeNode());
        flag = node->typeNode()->disambiguateValue() && flag;
        std::string name = node->id()->name();

        Type::Type* type = node->typeNode()->value();

        // Make sure the name isn't already declared in this scope
        if ( _symbols->isClashing(name) ) {
            SemanticSymbol* existing = _symbols->lookup(name);
            Reporting::nameError(node->position(), "Redeclaration of identifier \"" + name + "\" first declared at " + existing->declaredAt()->start() + ".");
            return false;
        }

        bool valueResult;
        if ( node->value()->getName() == "FunctionNode" || node->typeNode()->value()->intrinsic() == Type::Intrinsic::TYPE ) {
            // Add the declaration to the current scope
            _symbols->addVariable(name, type, node->position(), node->shared());

            if ( node->typeNode()->value()->intrinsic() == Type::Intrinsic::TYPE ) {
                // attach actual value of type to the symbol, for disambiguation of instances of this type
                TypeLiteral* objtype = nullptr;
                if ( node->value()->getName() == "TypeLiteral" || node->value()->getName() == "TypeBodyNode" ) {
                    objtype = (TypeLiteral*)node->value();
                } else if ( node->value()->getName() == "IdentifierNode" ) {
                    assert(((IdentifierNode*)node->value())->symbol()->kind() == SemanticSymbolKind::VARIABLE);
                    objtype = ((VariableSymbol*)((IdentifierNode*)node->value())->symbol())->getObjectType();
                } else {
                    Reporting::typeError(
                        node->value()->position(),
                        "Attempt to assign nontrivial value to a type variable."
                    );
                    return false;
                }
                auto sym = (VariableSymbol*)_symbols->lookup(name);
                sym->setObjectType(objtype);
            }

            // Call this to attach the Symbol to the IdentifierNode
            walk(node->id());
            // Check the RHS of the assignment
            valueResult = walk(node->value());
        } else {
            // Check the RHS of the assignment
            valueResult = walk(node->value());
            // Add the declaration to the current scope
            _symbols->addVariable(name, type, node->position(), node->shared());
            // Call this to attach the Symbol to the IdentifierNode
            walk(node->id());
        }

        return flag && valueResult;
    }

    bool walkCallExpressionNode(CallExpressionNode* node) override {
        bool flag = walk(node->func());

        for ( auto arg : *node->args() ) {
            flag = walk(arg) && flag;
        }

        return flag;
    }

    bool walkDeferCallExpressionNode(DeferCallExpressionNode* node) override {
        return walkCallExpressionNode(node->call());
    }

    bool walkIIFExpressionNode(IIFExpressionNode* node) override {
        bool flag = walk(node->expression());

        for ( auto arg : *node->args() ) {
            flag = walk(arg) && flag;
        }

        return flag;
    }

    bool walkAndNode(AndNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkOrNode(OrNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkEqualsNode(EqualsNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkNotEqualsNode(NotEqualsNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkAddNode(AddNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkSubtractNode(SubtractNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkMultiplyNode(MultiplyNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkDivideNode(DivideNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkModulusNode(ModulusNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkPowerNode(PowerNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkNegativeExpressionNode(NegativeExpressionNode* node) override {
        return walk(node->exp());
    }

    bool walkSqrtNode(SqrtNode* node) override {
        return walk(node->exp());
    }

    bool walkNotNode(NotNode* node) override {
        return walk(node->exp());
    }

    bool walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) override {
        bool flag = true;

        for ( auto actual : *node->actuals() ) {
            flag = walk(actual) && flag;
        }

        return flag;
    }

    virtual bool walkBlockStatementNode(BlockStatementNode* node) {
        bool flag = true;

        for ( auto stmt : *node->body() ) {
            flag = walk(stmt) && flag;
        }

        return flag;
    }

    bool walkEnumerationStatement(EnumerationStatement* node) override {
        bool flag = walk(node->enumerable());

        // Need to register the block-local variable
        // Its type is implicit as the generic type of the enumerable,
        // but we don't know it until type analysis
        std::string name = node->local()->name();
        Position* pos = node->local()->position();

        // Start a new scope in the body and add the local
        _symbols->enter();
        _symbols->addVariable(name, nullptr, pos, node->shared());

        // Add the index symbol if it exists. Symbol was created during parsing
        auto i = node->index();
        if (i != nullptr) {
            _symbols->insert(i->symbol());
        }

        flag = walk(node->local()) && flag;
        flag = walkBlockStatementNode(node) && flag;
        _symbols->leave();
        return flag;
    }

    bool walkWithStatement(WithStatement* node) override {
        bool flag = walk(node->resource());

        // need to register the block-local variable
        // Its type is implicit as the result of the expression
        bool inScope = false;
        if ( node->local()->symbol() == nullptr ) {
            std::string name = node->local()->name();
            Position* pos = node->local()->position();
            Type::Type* type = nullptr;

            // Note that the type of the local depends on the type of the expression,
            // since it is implicitly defined. This is handled in typeAnalysis.

            // Start a new scope in the body and add the local
            _symbols->enter();
            inScope = true;
            _symbols->addVariable(name, type, pos, node->shared());
        }

        flag = walk(node->local()) && flag;
        flag = walkBlockStatementNode(node) && flag;
        if ( inScope ) _symbols->leave();
        return flag;
    }

    bool walkIfStatement(IfStatement* node) override {
        bool flag = walk(node->condition());
        _symbols->enter();
        flag = walkBlockStatementNode(node) && flag;
        _symbols->leave();
        return flag;
    }

    bool walkWhileStatement(WhileStatement* node) override {
        bool flag = walk(node->condition());
        _symbols->enter();
        flag = walkBlockStatementNode(node) && flag;
        _symbols->leave();
        return flag;
    }

    bool walkContinueNode(ContinueNode* node) override {
        return true;
    }

    bool walkBreakNode(BreakNode* node) override {
        return true;
    }

    bool walkReturnStatementNode(ReturnStatementNode* node) override {
        if ( node->value() == nullptr ) {
            return true;
        }

        return walk(node->value());
    }

    bool walkMapStatementNode(MapStatementNode* node) override {
        return walk(node->value());
    }

    bool walkMapNode(MapNode* node) override {
        bool flag = true;
        // Check each entry in the map
        for ( auto entry : *node->body() ) {
            // Check for duplicate name
            size_t nameCount = 0;
            for ( auto subentry : *node->body() ) {
                if ( subentry->id()->name() == entry->id()->name() && nameCount < 2) {
                    nameCount += 1;
                }
            }

            if ( nameCount > 1 ) {
                Reporting::nameError(
                    entry->position(),
                    "Duplicate map key: \"" + entry->id()->name() + "\""
                );

                flag = false;
            }

            // Check the value expression
            flag = walk(entry) && flag;
        }

        return flag;
    }

    bool walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) override {
        return true;
    }

    bool walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) override {
        return true;
    }

    bool walkAssignExpressionNode(AssignExpressionNode* node) override {
        bool lvalResult = walk(node->dest());
        bool rvalResult = walk(node->value());
        return lvalResult && rvalResult;
    }

    bool walkUnitNode(UnitNode* node) override {
        return true;
    }

    bool walkFunctionNode(FunctionNode* node) override {
        bool flag = true;
        _symbols->enter();
        for ( auto formal : *node->formals() ) {
            bool t = formal.first->disambiguateValue() && flag;
            if ( !t ) {
                Reporting::nameError(formal.first->position(), "Identifier at " + formal.first->position()->toString() + " does not refer to a type!");
                flag = false;
            }
            flag = walk(formal.first) && flag;
            std::string name = formal.second->name();
            Type::Type* type = formal.first->value();

            // Make sure the name isn't already declared in this scope
            if ( _symbols->isClashing(name) ) {
                SemanticSymbol* existing = _symbols->lookup(name);
                Reporting::nameError(node->position(), "Redeclaration of identifier \"" + name + "\" first declared at " + existing->declaredAt()->start() + ".");
                flag = false;
            }

            // Add the declaration to the current scope
            _symbols->addVariable(name, type, formal.second->position(), false);

            // Call this to attach the Symbol to the IdentifierNode
            walk(formal.second);
        }
        walk(node->typeNode());
        flag = node->typeNode()->disambiguateValue() && flag;

        for ( auto stmt : *node->body() ) {
            flag = walk(stmt) && flag;
        }

        _symbols->leave();

        return flag;
    }

    bool walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) override {
        bool leftResult = walk(node->left());
        bool rightResult = walk(node->right());
        return leftResult && rightResult;
    }

    bool walkTypeBodyNode(TypeBodyNode* node) override {
        walkType(node->value());
        node->value()->transform([](Type::Type* v) -> Type::Type* {
            return v->disambiguateStatically();
        });
        _symbols->enter();
        bool flag = true;
        for (auto decl : *node->declarations()) {
            if ( decl->getName() == "VariableDeclarationNode" ) {
                auto d = (VariableDeclarationNode*)decl;
                if (d->shared()) {
                    Reporting::nameError(
                        d->position(),
                        "Attempted to create a shared variable " + d->id()->name() + " within type body."
                    );
                    flag = false;
                }
                flag = walk(d->typeNode()) && flag;
                flag = d->typeNode()->disambiguateValue() && flag;
                _symbols->addObjectProperty(d->id()->name(), d->typeNode()->value(), d->position());
                walk(d->id());
            } else if ( decl->getName() == "UninitializedVariableDeclarationNode" ) {
                auto d = (UninitializedVariableDeclarationNode*)decl;
                flag = walk(d->typeNode()) && flag;
                flag = d->typeNode()->disambiguateValue() && flag;
                _symbols->addObjectProperty(d->id()->name(), d->typeNode()->value(), d->position());
                walk(d->id());
            }
        }
        for (auto c : *node->declarations()) {
            if (c->getName() == "VariableDeclarationNode") {
                flag = walk(((VariableDeclarationNode*)c)->value()) && flag;
            } else {
                flag = walk(c) && flag;
            }
        }
        for (auto c : *node->constructors()) {
            flag = walk(c) && flag;
        }
        _symbols->leave();
        return flag;
    }

    bool walkClassAccessNode(ClassAccessNode* node) override {
        return walk(node->path()); // property names cant be checked until type analysis
    }

    bool walkIncludeStatementNode(IncludeStatementNode* node) override {
        return true;
    }

    bool walkConstructorNode(ConstructorNode* node) override {
        return walk(node->func());
    }

    bool walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) override {
        return true; // symbols should be added to scope during type body
    }

    [[nodiscard]] std::string toString() const override {
        return "NameAnalysisWalk<>";
    }
private:
    SymbolTable* _symbols;

    bool walkType(Type::Type* type) {
        std::set<std::size_t> visited;
        return walkTypeRec(type, visited);
    }

    bool walkTypeRec(Type::Type* type, std::set<std::size_t>& visited) {
        if ( stl::contains(visited, type->getId()) ) return true;
        visited.insert(type->getId());
        // attach symbol to ambiguous types
        if ( type->isAmbiguous() ) {
            auto a = (Type::Ambiguous*)type;
            assert(a->id() != nullptr);
            return walk(a->id());
        }
        if ( type->intrinsic() == Type::Intrinsic::ENUMERABLE ) {
            return walkTypeRec(((Type::Enumerable*)type)->values(), visited);
        }
        if ( type->intrinsic() == Type::Intrinsic::MAP ) {
            return walkTypeRec(((Type::Map*)type)->values(), visited);
        }
        if ( type->intrinsic() == Type::Intrinsic::LAMBDA0 ) {
            return walkTypeRec(((Type::Lambda0*)type)->returns(), visited);
        }
        if ( type->intrinsic() == Type::Intrinsic::LAMBDA1 ) {
            auto l = (Type::Lambda1*)type;
            return walkTypeRec(l->param(), visited) && walkTypeRec(l->returns(), visited);
        }
        if ( type->intrinsic() == Type::Intrinsic::OBJECT ) {
            auto obj = (Type::Object*)type;
            bool flag = true;
            for ( auto p : obj->getProperties() ) {
                flag = walkTypeRec(p.second, visited) && flag;
            }
            return flag;
        }
        return true;
    }
};

}

#endif
