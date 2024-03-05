#ifndef SWARMC_VALID_CONSTRUCTOR_WALK_H
#define SWARMC_VALID_CONSTRUCTOR_WALK_H

#include "Walk.h"

namespace swarmc::Lang::Walk {

class ValidConstructorWalk : Walk<void> {
public:
    static bool isValidConstructor(TypeBodyNode* type, ConstructorNode* constructor) {
        ValidConstructorWalk vcw;
        vcw._console->debug("Validating constructor");
        for ( auto d : *type->declarations() ) {
            if ( d->getTag() == ASTNodeTag::UNINITIALIZEDVARIABLEDECLARATION ) {
                // get symbols that need to be initialized
                auto uninit = ((UninitializedVariableDeclarationNode*)d);
                vcw._console->debug("Adding " + uninit->id()->name() + " to list of required initializations.");
                vcw._symbols.insert(uninit->id()->symbol());
            } else if ( d->getTag() == ASTNodeTag::VARIABLEDECLARATION ) {
                // get default values of functions
                auto v = ((VariableDeclarationNode*)d);
                if ( v->typeNode()->value()->isCallable() ) {
                    vcw._console->debug("Attempt to get default value of function " + v->id()->name() + ".");
                    vcw.setPossibleFunctions(v->id()->symbol(), v->value());
                }
            }
        }
        vcw.walk(constructor->func());
        if (!vcw._symbols.empty()) {
            std::string s = "";
            for (auto sym : vcw._symbols) s += sym->name() + ", ";
            Reporting::typeError(
                constructor->position(),
                "Unable to determine value of { " + s.substr(0, s.size() - 2) + " } in type constructor."
            );
        }
        vcw._console->debug("Constructor verified.");
        return vcw._symbols.empty() && vcw._goodReturns;
    }
protected:
    explicit ValidConstructorWalk() : Walk<void>(), _console(Console::get()) {}

    void walkProgramNode(ProgramNode* node) override {
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
    }

    void walkExpressionStatementNode(ExpressionStatementNode* node) override {
        walk(node->expression());
    }

    void walkIdentifierNode(IdentifierNode* node) override {}

    void walkEnumerableAccessNode(EnumerableAccessNode* node) override {}

    void walkEnumerableAppendNode(EnumerableAppendNode* node) override {}

    void walkMapAccessNode(MapAccessNode* node) override {}

    void walkClassAccessNode(ClassAccessNode* node) override {}

    void walkIncludeStatementNode(IncludeStatementNode* node) override {}

    void walkTypeLiteral(swarmc::Lang::TypeLiteral *node) override {}

    void walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) override {}

    void walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) override {}

    void walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) override {}

    void walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) override {}

    void walkMapStatementNode(MapStatementNode* node) override {}

    void walkMapNode(MapNode* node) override {}

    void walkAssignExpressionNode(AssignExpressionNode* node) override {
        if ( node->dest()->getTag() == ASTNodeTag::IDENTIFIER ) {
            auto dest = (IdentifierNode*)node->dest();
            if ( _symbols.count(dest->symbol()) ) {
                _symbols.erase(dest->symbol());
            }

            if ( dest->symbol()->type()->isCallable() ) {
                setPossibleFunctions(dest->symbol(), node->value());
            }
        }
    }

    void walkVariableDeclarationNode(VariableDeclarationNode* node) override {
        if ( node->typeNode()->value()->isCallable() ) {
            setPossibleFunctions(node->id()->symbol(), node->value());
        }
    }

    void walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) override {}

    void walkReturnStatementNode(ReturnStatementNode* node) override {
        if ( node->value() != nullptr ) walk(node->value());

        if ( !_symbols.empty() ) {
            std::string s = "";
            for (auto sym : _symbols) s += sym->name() + ", ";
            Reporting::typeError(
                node->position(),
                "Unable to determine value of { " + s.substr(0, s.size() - 2) + " } in type constructor."
            );
            _goodReturns = false;
        }
    }

    void walkFunctionNode(FunctionNode* node) override {
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
    }

    void walkConstructorNode(ConstructorNode* node) override {}

    void walkTypeBodyNode(TypeBodyNode* node) override {}

    void walkCallExpressionNode(CallExpressionNode* node) override {
        if ( node->func()->getTag() == ASTNodeTag::IDENTIFIER ) {
            auto id = (IdentifierNode*)node->func();
            if ( node->constructor() ) {
                // walk the constructor being called
                walk(node->constructor());
            } else if ( _possibleFunctions.count(id->symbol()) ) {
                _console->debug("Walking all possible values for " + id->name());
                auto prewalk = _symbols;
                std::vector<std::set<SemanticSymbol*>> postwalks;

                // walk every possible function
                for (auto f : _possibleFunctions.at(id->symbol())) {
                    walk(f);
                    postwalks.push_back(_symbols);
                    _symbols = prewalk;
                }

                // get symbols that were not assigned in at least 1 possible function
                _symbols = std::set<SemanticSymbol*>();
                for (auto& p : postwalks) {
                    _symbols.insert(p.begin(), p.end());
                }
            } else {
                _console->debug("Function " + id->name() + " is too ambiguous for constructor validation.");
            }
        }
    }

    void walkDeferCallExpressionNode(DeferCallExpressionNode* node) override {
        walkCallExpressionNode(node->call());
    }

    void walkAndNode(AndNode* node) override {}

    void walkOrNode(OrNode* node) override {}

    void walkEqualsNode(EqualsNode* node) override {}

    void walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) override {}

    void walkNotEqualsNode(NotEqualsNode* node) override {}

    void walkAddNode(AddNode* node) override {}

    void walkSubtractNode(SubtractNode* node) override {}

    void walkMultiplyNode(MultiplyNode* node) override {}

    void walkDivideNode(DivideNode* node) override {}

    void walkModulusNode(ModulusNode* node) override {}

    void walkPowerNode(PowerNode* node) override {}

    void walkNegativeExpressionNode(NegativeExpressionNode* node) override {}

    void walkSqrtNode(SqrtNode* node) override {}

    void walkNotNode(NotNode* node) override {}

    void walkEnumerationStatement(EnumerationStatement* node) override {}

    void walkWithStatement(WithStatement* node) override {
        // this remains in the top layer because its body executes unconditionally
        for ( auto stmt : *node->body() ) {
            walk(stmt);
        }
    }

    void walkIfStatement(IfStatement* node) override {
        walkBlockStatementNode(node);
    }

    void walkWhileStatement(WhileStatement* node) override {
        walkBlockStatementNode(node);
    }

    void walkContinueNode(ContinueNode* node) override {}

    void walkBreakNode(BreakNode* node) override {}

    [[nodiscard]] std::string toString() const override {
        return "ValidConstructorWalk<>";
    }

    void walkBlockStatementNode(BlockStatementNode* node) {
        _inTopLayer = false;
        auto before = _symbols;
        for (auto stmt : *node->body()) {
            walk(stmt);
        }
        _symbols = before;
        _inTopLayer = true;
    }
private:
    void setPossibleFunctions(SemanticSymbol* destSym, ExpressionNode* value) {
        if ( !_possibleFunctions.count(destSym) ) {
            _possibleFunctions.insert({ destSym, std::vector<FunctionNode*>() });
        }
        if ( _inTopLayer ) {
            _possibleFunctions.at(destSym).clear();
        }
        if ( value->getTag() == ASTNodeTag::FUNCTION ) {
            _console->debug("Giving " + destSym->name() + " the possible value of " + value->toString());
            _possibleFunctions.at(destSym).push_back((FunctionNode*)value);
        } else if ( value->getTag() == ASTNodeTag::IDENTIFIER
                && _possibleFunctions.count(((IdentifierNode*) value)->symbol()) )
        {
            for (auto f : _possibleFunctions.at(((IdentifierNode*) value)->symbol())) {
                _console->debug("Giving " + destSym->name() + " the possible value of " + f->toString());
                _possibleFunctions.at(destSym).push_back(f);
            }
        } else {
            _console->debug(value->toString() + " is too ambiguous for constructor validation. " + destSym->name() + " is now ambiguous.");
            _possibleFunctions.erase(destSym);
        }
    }

    std::set<SemanticSymbol*> _symbols;
    std::map<SemanticSymbol*, std::vector<FunctionNode*>> _possibleFunctions;
    bool _inTopLayer = true, _goodReturns = true;
    Console* _console;
};

}

#endif
