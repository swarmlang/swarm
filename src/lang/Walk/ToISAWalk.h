#ifndef SWARMC_TO_ISA_WALK_H
#define SWARMC_TO_ISA_WALK_H

#include <fstream>
#include <stack>
#include <queue>
#include "Walk.h"
#include "../../vm/isa_meta.h"

namespace swarmc {
namespace Lang {
namespace Walk {


class ToISAWalk : public Walk<ISA::Instructions*> {
public:
    ToISAWalk() : Walk<ISA::Instructions*>(), 
        _tempCounter(0), _inFunction(0), _whileConds(new std::stack<std::string>()) {}

    ~ToISAWalk() {
        delete _whileConds;
    }
protected:
    virtual ISA::Instructions* walkProgramNode(ProgramNode* node) {
        auto instrs = new ISA::Instructions();
        for ( auto stmt : *node->body() ) {
            auto i = walk(stmt);
            instrs->insert(instrs->end(), i->begin(), i->end());
            delete i;
        }
        return instrs;
    }

    virtual ISA::Instructions* walkExpressionStatementNode(ExpressionStatementNode* node) {
        return walk(node->expression());
    }

    virtual ISA::Instructions* walkIdentifierNode(IdentifierNode* node) {
        auto instrs = new ISA::Instructions();
        ISA::Affinity affinity = node->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
        auto ref = new ISA::LocationReference(affinity, "var_" + node->name());
        instrs->push_back(new ISA::AssignValue(ref, ref));
        return instrs;
    }

    virtual ISA::Instructions* walkMapAccessNode(MapAccessNode* node) {
        auto instrs = walk(node->path());
        auto mapget = new ISA::MapGet(
            new ISA::LocationReference(ISA::Affinity::LOCAL, "mkey_" + node->end()->name()), getLocFromAssign(instrs->back()));
        instrs->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), mapget));
        return instrs;
    }

    virtual ISA::Instructions* walkEnumerableAccessNode(EnumerableAccessNode* node) {
        auto instrs = walk(node->index());
        auto llval = walk(node->path());
        auto enumget = new ISA::EnumGet(getLocFromAssign(llval->back()), getLocFromAssign(instrs->back()));
        instrs->insert(instrs->end(), llval->begin(), llval->end());
        instrs->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), enumget));
        return instrs;
    }

    virtual ISA::Instructions* walkTypeLiteral(swarmc::Lang::TypeLiteral *node) {
        auto instrs = new ISA::Instructions();
        auto ref = new ISA::TypeReference(node->value());
        instrs->push_back(new ISA::AssignValue(makeLocation(ISA::Affinity::LOCAL), ref));
        return instrs;
    }

    virtual ISA::Instructions* walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
        auto instrs = new ISA::Instructions();
        auto ref = new ISA::BooleanReference(node->value());
        instrs->push_back(new ISA::AssignValue(makeLocation(ISA::Affinity::LOCAL), ref));
        return instrs;
    }

    virtual ISA::Instructions* walkVariableDeclarationNode(VariableDeclarationNode* node) {
        auto instrs = walk(node->value());

        // Create location from variable name
        auto aff = node->id()->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
        ISA::LocationReference* loc = new ISA::LocationReference(aff, "var_" + node->id()->name());

        // if in a function, add a scopeof just in case
        if ( _inFunction ) {
            auto scopeOf = new ISA::Instructions();
            scopeOf->push_back(new ISA::ScopeOf(loc));
            scopeOf->insert(scopeOf->end(), instrs->begin(), instrs->end());
            delete instrs;
            instrs = scopeOf;
        }

        // acquire temporary location from previous instruction and assign
        instrs->push_back(new ISA::AssignValue(loc, getLocFromAssign(instrs->back())));

        return instrs;
    }

    virtual ISA::Instructions* walkCallExpressionNode(CallExpressionNode* node) {
        auto instrs = walk(node->id());
        auto floc = getLocFromAssign(instrs->back());
        if ( node->args()->size() == 0 ) {
            auto call = new ISA::Call0(floc);
            if (node->type()->intrinsic() == Type::Intrinsic::VOID) {
                instrs->push_back(call);
            } else {
                instrs->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), call));
            }
        } else {
            auto fnType = node->id()->type();
            for ( auto arg : *node->args() ) {
                auto evalarg = walk(arg);
                instrs->insert(instrs->end(), evalarg->begin(), evalarg->end());
                assert(fnType->isCallable());
                if (((Type::Lambda*)fnType)->returns()->intrinsic() == Type::Intrinsic::VOID) {
                    auto call = new ISA::Call1(floc, getLocFromAssign(evalarg->back()));
                    instrs->push_back(call);
                } else if (((Type::Lambda*)fnType)->returns()->isCallable()) {
                    auto call = new ISA::Curry(floc, getLocFromAssign(evalarg->back()));
                    auto loc2 = makeLocation(ISA::Affinity::LOCAL);
                    instrs->push_back(new ISA::AssignEval(loc2, call));
                    floc = loc2;
                } else {
                    auto call = new ISA::Call1(floc, getLocFromAssign(evalarg->back()));
                    auto loc2 = makeLocation(ISA::Affinity::LOCAL);
                    instrs->push_back(new ISA::AssignEval(loc2, call));
                    floc = loc2;
                }
                fnType = ((Type::Lambda*)fnType)->returns();
                delete evalarg;
            }
        }

        return instrs;
    }

    virtual ISA::Instructions* walkIIFExpressionNode(IIFExpressionNode* node) {
        auto instrs = walk(node->expression());
        auto floc = getLocFromAssign(instrs->back());

        if ( node->args()->size() == 0 ) {
            auto call = new ISA::Call0(floc);
            if (node->type()->intrinsic() == Type::Intrinsic::VOID) {
                instrs->push_back(call);
            } else {
                instrs->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), call));
            }
        } else {
            auto fnType = node->expression()->type();
            for ( auto arg : *node->args() ) {
                auto evalarg = walk(arg);
                instrs->insert(instrs->end(), evalarg->begin(), evalarg->end());
                assert(fnType->isCallable());
                if (((Type::Lambda*)fnType)->returns()->intrinsic() == Type::Intrinsic::VOID) {
                    auto call = new ISA::Call1(floc, getLocFromAssign(evalarg->back()));
                    instrs->push_back(call);
                } else if (((Type::Lambda*)fnType)->returns()->isCallable()) {
                    auto call = new ISA::Curry(floc, getLocFromAssign(evalarg->back()));
                    auto loc2 = makeLocation(ISA::Affinity::LOCAL);
                    instrs->push_back(new ISA::AssignEval(loc2, call));
                    floc = loc2;
                } else {
                    auto call = new ISA::Call1(floc, getLocFromAssign(evalarg->back()));
                    auto loc2 = makeLocation(ISA::Affinity::LOCAL);
                    instrs->push_back(new ISA::AssignEval(loc2, call));
                    floc = loc2;
                }
                fnType = ((Type::Lambda*)fnType)->returns();
                delete evalarg;
            }
        }

        return instrs;
    }

    virtual ISA::Instructions* walkAndNode(AndNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::And(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkOrNode(OrNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Or(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkEqualsNode(EqualsNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::IsEqual(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkNotEqualsNode(NotEqualsNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        auto loc = makeLocation(ISA::Affinity::LOCAL);
        left->push_back(new ISA::AssignEval(loc, new ISA::IsEqual(lhs, rhs)));
        left->push_back(new ISA::AssignEval(loc, new ISA::Not(loc)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkAddNode(AddNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Plus(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkSubtractNode(SubtractNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Minus(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkMultiplyNode(MultiplyNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Times(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkDivideNode(DivideNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Divide(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkModulusNode(ModulusNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Mod(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkPowerNode(PowerNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Power(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkConcatenateNode(ConcatenateNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::StringConcat(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkNegativeExpressionNode(NegativeExpressionNode* node) {
        auto instrs = walk(node->exp());
        auto loc = getLocFromAssign(instrs->back());
        instrs->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Negative(loc)));
        return instrs;
    }

    virtual ISA::Instructions* walkNotNode(NotNode* node) {
        auto instrs = walk(node->exp());
        auto loc = getLocFromAssign(instrs->back());
        instrs->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Not(loc)));
        return instrs;
    }

    virtual ISA::Instructions* walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
        auto instrs = new ISA::Instructions();
        auto loc = makeLocation(ISA::Affinity::LOCAL);
        assert(node->type()->intrinsic() == Type::Intrinsic::ENUMERABLE);
        auto innerType = ((Type::Enumerable*)node->type())->values();
        auto map = new ISA::AssignEval(loc, new ISA::EnumInit(new ISA::TypeReference(innerType)));
        instrs->push_back(map);
        for ( auto exp : *node->actuals() ) {
            auto expinstrs = walk(exp);
            auto enumapp = new ISA::EnumAppend(getLocFromAssign(expinstrs->back()), loc);
            instrs->insert(instrs->end(), expinstrs->begin(), expinstrs->end());
            instrs->push_back(enumapp);
            delete expinstrs;
        }
        // need to end with an assign because future operations will look at the end of instrs for the lloc
        instrs->push_back(new ISA::AssignValue(loc, loc));
        return instrs;
    }

    virtual ISA::Instructions* walkEnumerationStatement(EnumerationStatement* node) {
        auto instrs = walk(node->enumerable());
        auto enumLoc = getLocFromAssign(instrs->back());

        assert(node->enumerable()->type()->intrinsic() == Type::Intrinsic::ENUMERABLE);
        auto elemType = new ISA::TypeReference(((Type::Enumerable*)node->enumerable()->type())->values());

        std::string name = "ENUM_" + std::to_string(_tempCounter++);

        _inFunction++;
        instrs->push_back(new ISA::BeginFunction(name, new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::VOID))));
        auto elemShared = node->local()->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
        instrs->push_back(new ISA::FunctionParam(elemType, new ISA::LocationReference(elemShared, "arg_" + node->local()->name())));
        if ( node->index() != nullptr ) {
            auto idxShared = node->index()->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
            instrs->push_back(new ISA::FunctionParam(
                new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::NUMBER)), 
                new ISA::LocationReference(idxShared, "arg_" + node->index()->name())
            ));
        } else {
            instrs->push_back(new ISA::FunctionParam(
                new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::NUMBER)), 
                new ISA::LocationReference(ISA::Affinity::LOCAL, "UNUSEDarg_idx")
            ));
        }

        for ( auto stmt : *node->body() ) {
            auto s = walk(stmt);
            instrs->insert(instrs->end(), s->begin(), s->end());
            delete s;
        }

        instrs->push_back(new ISA::Return0());
        _inFunction--;

        instrs->push_back(new ISA::Enumerate(
            elemType,
            enumLoc,
            new ISA::LocationReference(ISA::Affinity::FUNCTION, name)
        ));

        return instrs;
    }

    virtual ISA::Instructions* walkWithStatement(WithStatement* node) {
        auto instrs = walk(node->resource());
        auto resLoc = getLocFromAssign(instrs->back());

        // get yield type
        auto type = new ISA::TypeReference(node->local()->type());
        auto typeLoc = makeLocation(ISA::Affinity::LOCAL);
        instrs->push_back(new ISA::AssignValue(typeLoc, type));
        std::string name = "WITH_" + std::to_string(_tempCounter++);

        _inFunction++;
        instrs->push_back(new ISA::BeginFunction(name, new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::VOID))));
        auto localShared = node->local()->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
        instrs->push_back(new ISA::FunctionParam(typeLoc, new ISA::LocationReference(localShared, "res_" + node->local()->name())));

        // walk body
        for ( auto stmt : *node->body() ) {
            auto s = walk(stmt);
            instrs->insert(instrs->end(), s->begin(), s->end());
            delete s;
        }

        instrs->push_back(new ISA::Return0());
        _inFunction--;

        instrs->push_back(new ISA::With(resLoc, new ISA::LocationReference(ISA::Affinity::FUNCTION, name)));

        return instrs;
    }

    virtual ISA::Instructions* walkIfStatement(IfStatement* node) {
        auto instrs = new ISA::Instructions();
        
        // build IF function (potentially need global scope? add to map if so)
        std::string name = "IFBODY_" + std::to_string(_tempCounter++);
        _inFunction++;
        auto begin = new ISA::BeginFunction(name, new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::VOID)));
        instrs->push_back(begin);
        for ( auto stmt : *node->body() ) {
            auto i = walk(stmt);
            instrs->insert(instrs->end(), i->begin(), i->end());
            delete i;
        }
        instrs->push_back(new ISA::Return0());
        _inFunction--;
        auto func = new ISA::LocationReference(ISA::Affinity::FUNCTION, name);

        // walk condition and callif
        auto condition = walk(node->condition());
        instrs->insert(instrs->end(), condition->begin(), condition->end());
        instrs->push_back(new ISA::CallIf0(getLocFromAssign(instrs->back()), func));

        return instrs;
    }

    virtual ISA::Instructions* walkWhileStatement(WhileStatement* node) {
        auto instrs = new ISA::Instructions();

        instrs->push_back(new ISA::AssignValue(
            new ISA::LocationReference(ISA::Affinity::LOCAL, "whileCond"), 
            new ISA::BooleanReference(false))
        );
        // condition function
        _whileConds->push("WHILECOND_" + std::to_string(_tempCounter++));
        _inFunction++;
        instrs->push_back(new ISA::BeginFunction(_whileConds->top(), new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::BOOLEAN))));
        auto cond = walk(node->condition());
        instrs->insert(instrs->end(), cond->begin(), cond->end());
        instrs->push_back(new ISA::Return1(getLocFromAssign(instrs->back())));
        _inFunction--;

        // While function header
        std::string name = "WHILE_" + std::to_string(_tempCounter++);
        auto retType = new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::VOID));
        auto cfb = new ISA::LocationReference(ISA::Affinity::LOCAL, "CFBWhile");
        _inFunction++;
        instrs->push_back(new ISA::BeginFunction(name, retType));

        // bring control flow breaker tracker in scope
        instrs->push_back(new ISA::ScopeOf(cfb));
        instrs->push_back(new ISA::AssignValue(cfb, new ISA::BooleanReference(false)));

        std::queue<ISA::Instructions*> subfuncs;
        std::queue<std::string> funcnames;
        subfuncs.push(instrs);

        // Walk body, creating subfunctions from the body after every diversion of control flow
        for ( auto stmt : *node->body() ) {
            auto walkstmt = walk(stmt);
            subfuncs.back()->insert(subfuncs.back()->end(), walkstmt->begin(), walkstmt->end());
            if ( stmt->isBlock() ) {
                auto subf = new ISA::Instructions();
                std::string name = "SUBFUNC_" + std::to_string(_tempCounter++);
                auto voidtype = new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::VOID));
                _inFunction++;
                subf->push_back(new ISA::BeginFunction(name, voidtype));
                subfuncs.push(subf);
                funcnames.push(name);
            }
            delete walkstmt;
        }

        // unload subfunctions
        subfuncs.pop();
        while ( !funcnames.empty() ) {
            auto subf = subfuncs.front();
            subfuncs.pop();
            if ( subf->size() != 1 ) {
                instrs->insert(instrs->end(), subf->begin(), subf->end());
                instrs->push_back(new ISA::Return0());
                _inFunction--;
                auto loc = new ISA::LocationReference(ISA::Affinity::FUNCTION, funcnames.front());
                instrs->push_back(new ISA::CallElse0(cfb, loc));
            } else {
                delete subf->at(0);
            }
            funcnames.pop();
        }

        // reevaluate condition
        instrs->push_back(new ISA::CallElse0(cfb, new ISA::LocationReference(ISA::Affinity::FUNCTION, _whileConds->top())));

        // end of function return
        instrs->push_back(new ISA::Return0());
        _inFunction--;

        //evaluate condition
        instrs->push_back(new ISA::AssignEval(
            new ISA::LocationReference(ISA::Affinity::LOCAL, "whileCond"), 
            new ISA::Call0(new ISA::LocationReference(ISA::Affinity::FUNCTION, _whileConds->top()))
        ));

        instrs->push_back(new ISA::While(
            new ISA::LocationReference(ISA::Affinity::LOCAL, "whileCond"), 
            new ISA::LocationReference(ISA::Affinity::FUNCTION, name)
        ));

        _whileConds->pop();
        return instrs;
    }

    virtual ISA::Instructions* walkContinueNode(ContinueNode* node) {
        auto instrs = new ISA::Instructions();
        // evaluate condition
        instrs->push_back(new ISA::AssignEval(
            new ISA::LocationReference(ISA::Affinity::LOCAL, "whileCond"), 
            new ISA::Call0(new ISA::LocationReference(ISA::Affinity::FUNCTION, _whileConds->top()))
        ));
        // skip rest of function
        instrs->push_back(new ISA::AssignValue(
            new ISA::LocationReference(ISA::Affinity::LOCAL, "CFBWhile"), 
            new ISA::BooleanReference(true)
        ));
        return instrs;
    }

    virtual ISA::Instructions* walkBreakNode(BreakNode* node) {
        auto instrs = new ISA::Instructions();
        // set condition to false
        instrs->push_back(new ISA::AssignValue(
            new ISA::LocationReference(ISA::Affinity::LOCAL, "whileCond"),
            new ISA::BooleanReference(false)
        ));
        // skip rest of function
        instrs->push_back(new ISA::AssignValue(
            new ISA::LocationReference(ISA::Affinity::LOCAL, "CFBWhile"), 
            new ISA::BooleanReference(true)
        ));
        return instrs;
    }

    virtual ISA::Instructions* walkReturnStatementNode(ReturnStatementNode* node) {
        auto instrs = new ISA::Instructions();
        instrs->push_back(new ISA::AssignValue(new ISA::LocationReference(ISA::Affinity::LOCAL, "CFB"), new ISA::BooleanReference(true)));
        if ( node->value() != nullptr ) {
            auto exp = walk(node->value());
            auto retval = getLocFromAssign(exp->back());
            instrs->insert(instrs->end(), exp->begin(), exp->end());
            instrs->push_back(new ISA::AssignValue(new ISA::LocationReference(ISA::Affinity::LOCAL, "retVal"), retval));
        }
        return instrs;
    }

    virtual ISA::Instructions* walkMapStatementNode(MapStatementNode* node) {
        return walk(node->value());
    }

    virtual ISA::Instructions* walkMapNode(MapNode* node) {
        auto instrs = new ISA::Instructions();
        auto loc = makeLocation(ISA::Affinity::LOCAL);
        assert(node->type()->intrinsic() == Type::Intrinsic::MAP);
        auto innerType = ((Type::Map*)node->type())->values();
        auto map = new ISA::AssignEval(loc, new ISA::MapInit(new ISA::TypeReference(innerType)));
        instrs->push_back(map);
        for ( auto stmt : *node->body() ) {
            auto id = new ISA::LocationReference(ISA::Affinity::LOCAL, "mkey_" + stmt->id()->name());
            auto mapstmt = walk(stmt);
            auto mapset = new ISA::MapSet(id, getLocFromAssign(mapstmt->back()), loc);
            instrs->insert(instrs->end(), mapstmt->begin(), mapstmt->end());
            instrs->push_back(mapset);
            delete mapstmt;
        }
        // need to end with an assign because future operations will look at the end of instrs for the lloc
        instrs->push_back(new ISA::AssignValue(loc, loc));
        return instrs;
    }

    virtual ISA::Instructions* walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
        auto instrs = new ISA::Instructions();
        auto ref = new ISA::StringReference(node->value());
        instrs->push_back(new ISA::AssignValue(makeLocation(ISA::Affinity::LOCAL), ref));
        return instrs;
    }

    virtual ISA::Instructions* walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
        auto instrs = new ISA::Instructions();
        auto ref = new ISA::NumberReference(node->value());
        instrs->push_back(new ISA::AssignValue(makeLocation(ISA::Affinity::LOCAL), ref));
        return instrs;
    }

    virtual ISA::Instructions* walkAssignExpressionNode(AssignExpressionNode* node) {
        auto instrs = walk(node->value());
        auto value = getLocFromAssign(instrs->back());
        if ( node->dest()->getName() == "IdentifierNode" ) {
            auto id = (IdentifierNode*) node->dest();
            auto aff = id->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
            instrs->push_back(
                new ISA::AssignValue(
                    new ISA::LocationReference(aff, "var_" + id->name()), value));
        } else if ( node->dest()->getName() == "EnumerableAccessNode" ) {
            auto en = (EnumerableAccessNode*) node->dest();
            auto index = walk(en->index());
            auto path = walk(en->path());
            instrs->insert(instrs->end(), index->begin(), index->end());
            instrs->insert(instrs->end(), path->begin(), path->end());
            instrs->push_back(
                new ISA::EnumSet(
                    getLocFromAssign(path->back()), 
                    getLocFromAssign(index->back()), 
                    value));
        } else {
            auto ma = (MapAccessNode*) node->dest();
            auto path = walk(ma->path());
            instrs->insert(instrs->end(), path->begin(), path->end());
            instrs->push_back(
                new ISA::MapSet(
                    new ISA::LocationReference(ISA::Affinity::LOCAL, "mkey_" + ma->end()->name()), 
                    value,
                    getLocFromAssign(path->back())));
        }
        return instrs;
    }

    virtual ISA::Instructions* walkUnitNode(UnitNode* node) {
        // this should never be called
        return new ISA::Instructions();
    }

    virtual ISA::Instructions* walkFunctionNode(FunctionNode* node) {
        auto instrs = new ISA::Instructions();

        std::string name = "FUNC_" + std::to_string(_tempCounter++);
        auto fnType = node->type();
        if (node->formals()->size() == 0) {
            assert( fnType->isCallable() );
            fnType = ((Type::Lambda*)fnType)->returns();
        } else {
            for ( size_t i = 0; i < node->formals()->size(); i++ ) {
                assert( fnType->isCallable() );
                fnType = ((Type::Lambda*)fnType)->returns();
            }
        }
        auto retType = new ISA::TypeReference((fnType));
        auto retVar = new ISA::LocationReference(ISA::Affinity::LOCAL, "retVal");
        auto cfb = new ISA::LocationReference(ISA::Affinity::LOCAL, "CFB");
        _inFunction++;
        instrs->push_back(new ISA::BeginFunction(name, retType));

        // formals
        for ( auto f : *node->formals() ) {
            auto type = new ISA::TypeReference(f.first->value());
            auto ref = new ISA::LocationReference(ISA::Affinity::LOCAL, "arg_" + f.second->name());
            instrs->push_back(new ISA::FunctionParam(type, ref));
        }

        // bring return value trackers in scope
        if ( fnType->intrinsic() != Type::Intrinsic::VOID ) {
            instrs->push_back(new ISA::ScopeOf(retVar));
        }
        instrs->push_back(new ISA::ScopeOf(cfb));
        instrs->push_back(new ISA::AssignValue(cfb, new ISA::BooleanReference(false)));

        std::queue<ISA::Instructions*> subfuncs;
        std::queue<std::string> funcnames;
        subfuncs.push(instrs);

        // Walk body, creating subfunctions after every control flow diversion
        for ( auto stmt : *node->body() ) {
            auto walkstmt = walk(stmt);
            subfuncs.back()->insert(subfuncs.back()->end(), walkstmt->begin(), walkstmt->end());
            if ( stmt->isBlock() ) {
                auto subf = new ISA::Instructions();
                std::string name = "SUBFUNC_" + std::to_string(_tempCounter++);
                auto voidtype = new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::VOID));
                _inFunction++;
                subf->push_back(new ISA::BeginFunction(name, voidtype));
                subfuncs.push(subf);
                funcnames.push(name);
            }
            delete walkstmt;
        }

        // unload subfunctions
        subfuncs.pop();
        while ( !funcnames.empty() ) {
            auto subf = subfuncs.front();
            subfuncs.pop();
            if ( subf->size() != 1 ) {
                instrs->insert(instrs->end(), subf->begin(), subf->end());
                instrs->push_back(new ISA::Return0());
                _inFunction--;
                auto loc = new ISA::LocationReference(ISA::Affinity::FUNCTION, funcnames.front());
                instrs->push_back(new ISA::CallElse0(cfb, loc));
            } else {
                delete subf->at(0);
            }
            funcnames.pop();
        }

        // end of function return
        if ( fnType->intrinsic() == Type::Intrinsic::VOID ) {
            instrs->push_back(new ISA::Return0());
        } else {
            instrs->push_back(new ISA::Return1(retVar));
        }
        _inFunction--;

        // needed for assignment
        instrs->push_back(new ISA::AssignValue(makeLocation(ISA::Affinity::LOCAL), new ISA::LocationReference(ISA::Affinity::FUNCTION, name)));
        
        return instrs;
    }

    virtual ISA::Instructions* walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());

        // determine which instruction
        ISA::Instruction* instr;
        switch (node->comparisonType()) {
        case NumberComparisonType::GREATER_THAN:
            instr = new ISA::GreaterThan(lhs, rhs);
            break;
        case NumberComparisonType::GREATER_THAN_OR_EQUAL:
            instr = new ISA::GreaterThanOrEqual(lhs, rhs);
            break;
        case NumberComparisonType::LESS_THAN:
            instr = new ISA::LessThan(lhs, rhs);
            break;
        default:
            instr = new ISA::LessThanOrEqual(lhs, rhs);
            break;
        }
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), instr));
        delete right;
        return left;
    }

    virtual std::string toString() const {
        return "ToISAWalk<>";
    }

private:
    int _tempCounter, _inFunction;
    std::stack<std::string>* _whileConds;

    ISA::LocationReference* makeLocation(ISA::Affinity affinity) {
        return new ISA::LocationReference(affinity, "tmp" + std::to_string(_tempCounter++));
    }

    ISA::LocationReference* getLocFromAssign(ISA::Instruction* instr) {
        assert(instr->tag() == ISA::Tag::ASSIGNEVAL || instr->tag() == ISA::Tag::ASSIGNVALUE);
        if (instr->tag() == ISA::Tag::ASSIGNEVAL) {
            return ((ISA::AssignEval*)instr)->first();
        }
        return ((ISA::AssignValue*)instr)->first();
    }
};

}
}
}

#endif