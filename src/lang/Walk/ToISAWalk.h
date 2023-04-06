#ifndef SWARMC_TO_ISA_WALK_H
#define SWARMC_TO_ISA_WALK_H

#include <fstream>
#include <stack>
#include <queue>
#include "Walk.h"
#include "../../vm/isa_meta.h"
#include "../../errors/SwarmError.h"

namespace swarmc::Lang::Walk {


class ToISAWalk : public Walk<ISA::Instructions*> {
public:
    ToISAWalk() : Walk<ISA::Instructions*>(),
        _tempCounter(0), _inFunction(0), _whileConds(new std::stack<std::string>()) {}

    ~ToISAWalk() override {
        delete _whileConds;
        for (auto p : _typeMap) freeref(p.second);
        for (auto p : _locMap) freeref(p.second);
    }
protected:
    ISA::Instructions* walkProgramNode(ProgramNode* node) override {
        auto instrs = new ISA::Instructions();
        for ( auto stmt : *node->body() ) {
            auto i = walk(stmt);
            instrs->insert(instrs->end(), i->begin(), i->end());
            delete i;
        }
        return instrs;
    }

    ISA::Instructions* walkExpressionStatementNode(ExpressionStatementNode* node) override {
        return walk(node->expression());
    }

    ISA::Instructions* walkIdentifierNode(IdentifierNode* node) override {
        auto instrs = new ISA::Instructions();
        ISA::Affinity affinity = node->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
        auto ref = makeLocation(affinity, "var_" + node->name());
        instrs->push_back(useref(new ISA::AssignValue(ref, ref)));
        return instrs;
    }

    ISA::Instructions* walkMapAccessNode(MapAccessNode* node) override {
        auto instrs = walk(node->path());
        auto mapget = new ISA::MapGet(
            makeLocation(ISA::Affinity::LOCAL, "mkey_" + node->end()->name()), getLocFromAssign(instrs->back()));
        instrs->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), mapget)));
        return instrs;
    }

    ISA::Instructions* walkEnumerableAccessNode(EnumerableAccessNode* node) override {
        auto instrs = walk(node->index());
        auto llval = walk(node->path());
        auto enumget = new ISA::EnumGet(getLocFromAssign(llval->back()), getLocFromAssign(instrs->back()));
        instrs->insert(instrs->end(), llval->begin(), llval->end());
        instrs->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), enumget)));
        return instrs;
    }

    ISA::Instructions* walkTypeLiteral(swarmc::Lang::TypeLiteral *node) override {
        auto instrs = new ISA::Instructions();
        auto ref = new ISA::TypeReference(node->value());
        instrs->push_back(useref(new ISA::AssignValue(makeNewTmp(ISA::Affinity::LOCAL), ref)));
        return instrs;
    }

    ISA::Instructions* walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) override {
        auto instrs = new ISA::Instructions();
        auto ref = new ISA::BooleanReference(node->value());
        instrs->push_back(useref(new ISA::AssignValue(makeNewTmp(ISA::Affinity::LOCAL), ref)));
        return instrs;
    }

    ISA::Instructions* walkVariableDeclarationNode(VariableDeclarationNode* node) override {
        auto instrs = walk(node->value());
        auto vloc = getLocFromAssign(instrs->back());

        // Create location from variable name
        auto aff = node->id()->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
        auto loc = makeLocation(aff, "var_" + node->id()->name());

        // if in a function, add a scopeof just in case
        if ( _inFunction ) {
            auto scopeOf = new ISA::Instructions();
            scopeOf->push_back(useref(new ISA::ScopeOf(loc)));
            scopeOf->insert(scopeOf->end(), instrs->begin(), instrs->end());
            delete instrs;
            instrs = scopeOf;
        }

        // acquire temporary location from previous instruction and assign
        instrs->push_back(useref(new ISA::AssignValue(loc, vloc)));

        return instrs;
    }

    ISA::Instructions* walkCallExpressionNode(CallExpressionNode* node) override {
        ISA::Instructions* instrs;
        ISA::LocationReference* floc;
        Type::Type* fnType;
        if (!node->calling()) {
            instrs = walk(node->func());
            floc = getLocFromAssign(instrs->back());
            fnType = node->func()->type();
        } else {
            instrs = new ISA::Instructions();
            auto aff = node->calling()->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
            floc = makeLocation(aff, node->calling()->name());
            fnType = node->calling()->func()->type();
        }

        if ( node->args()->empty() ) {
            auto call = new ISA::Call0(floc);
            if (node->type()->intrinsic() == Type::Intrinsic::VOID) {
                instrs->push_back(useref(call));
            } else {
                instrs->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), call)));
            }
        } else {
            for ( auto arg : *node->args() ) {
                auto evalarg = walk(arg);
                instrs->insert(instrs->end(), evalarg->begin(), evalarg->end());
                assert(fnType->isCallable());

                // curry or call func in sequence
                if (((Type::Lambda*)fnType)->returns()->intrinsic() == Type::Intrinsic::VOID) {
                    auto call = new ISA::Call1(floc, getLocFromAssign(evalarg->back()));
                    instrs->push_back(useref(call));
                } else if (((Type::Lambda*)fnType)->returns()->isCallable()) {
                    auto call = new ISA::Curry(floc, getLocFromAssign(evalarg->back()));
                    auto loc2 = makeNewTmp(ISA::Affinity::LOCAL);
                    instrs->push_back(useref(new ISA::AssignEval(loc2, call)));
                    floc = loc2;
                } else {
                    auto call = new ISA::Call1(floc, getLocFromAssign(evalarg->back()));
                    auto loc2 = makeNewTmp(ISA::Affinity::LOCAL);
                    instrs->push_back(useref(new ISA::AssignEval(loc2, call)));
                    floc = loc2;
                }

                fnType = ((Type::Lambda*)fnType)->returns();
                delete evalarg;
            }
        }

        return instrs;
    }

    ISA::Instructions* walkIIFExpressionNode(IIFExpressionNode* node) override {
        auto instrs = walk(node->expression());
        auto floc = getLocFromAssign(instrs->back());

        if ( node->args()->empty() ) {
            auto call = new ISA::Call0(floc);
            if (node->type()->intrinsic() == Type::Intrinsic::VOID) {
                instrs->push_back(useref(call));
            } else {
                instrs->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), call)));
            }
        } else {
            auto fnType = node->expression()->type();
            for ( auto arg : *node->args() ) {
                auto evalarg = walk(arg);
                instrs->insert(instrs->end(), evalarg->begin(), evalarg->end());
                assert(fnType->isCallable());
                if (((Type::Lambda*)fnType)->returns()->intrinsic() == Type::Intrinsic::VOID) {
                    auto call = new ISA::Call1(floc, getLocFromAssign(evalarg->back()));
                    instrs->push_back(useref(call));
                } else if (((Type::Lambda*)fnType)->returns()->isCallable()) {
                    auto call = new ISA::Curry(floc, getLocFromAssign(evalarg->back()));
                    auto loc2 = makeNewTmp(ISA::Affinity::LOCAL);
                    instrs->push_back(useref(new ISA::AssignEval(loc2, call)));
                    floc = loc2;
                } else {
                    auto call = new ISA::Call1(floc, getLocFromAssign(evalarg->back()));
                    auto loc2 = makeNewTmp(ISA::Affinity::LOCAL);
                    instrs->push_back(useref(new ISA::AssignEval(loc2, call)));
                    floc = loc2;
                }
                fnType = ((Type::Lambda*)fnType)->returns();
                delete evalarg;
            }
        }

        return instrs;
    }

    ISA::Instructions* walkAndNode(AndNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), new ISA::And(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkOrNode(OrNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), new ISA::Or(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkEqualsNode(EqualsNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), new ISA::IsEqual(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkNotEqualsNode(NotEqualsNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        auto loc = makeNewTmp(ISA::Affinity::LOCAL);
        left->push_back(useref(new ISA::AssignEval(loc, new ISA::IsEqual(lhs, rhs))));
        left->push_back(useref(new ISA::AssignEval(loc, new ISA::Not(loc))));
        delete right;
        return left;
    }

    ISA::Instructions* walkAddNode(AddNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        if ( node->concatenation() ) {
            left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), new ISA::StringConcat(lhs, rhs))));
        } else {
            left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), new ISA::Plus(lhs, rhs))));
        }
        delete right;
        return left;
    }

    ISA::Instructions* walkSubtractNode(SubtractNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), new ISA::Minus(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkMultiplyNode(MultiplyNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), new ISA::Times(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkDivideNode(DivideNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), new ISA::Divide(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkModulusNode(ModulusNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), new ISA::Mod(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkPowerNode(PowerNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), new ISA::Power(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkNegativeExpressionNode(NegativeExpressionNode* node) override {
        auto instrs = walk(node->exp());
        auto loc = getLocFromAssign(instrs->back());
        instrs->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), new ISA::Negative(loc))));
        return instrs;
    }

    ISA::Instructions* walkNotNode(NotNode* node) override {
        auto instrs = walk(node->exp());
        auto loc = getLocFromAssign(instrs->back());
        instrs->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), new ISA::Not(loc))));
        return instrs;
    }

    ISA::Instructions* walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) override {
        auto instrs = new ISA::Instructions();
        auto loc = makeNewTmp(ISA::Affinity::LOCAL);
        assert(node->type()->intrinsic() == Type::Intrinsic::ENUMERABLE);
        auto innerType = ((Type::Enumerable*)node->type())->values();
        auto map = new ISA::AssignEval(loc, new ISA::EnumInit(new ISA::TypeReference(innerType)));
        instrs->push_back(useref(map));
        for ( auto exp : *node->actuals() ) {
            auto expinstrs = walk(exp);
            auto enumapp = new ISA::EnumAppend(getLocFromAssign(expinstrs->back()), loc);
            instrs->insert(instrs->end(), expinstrs->begin(), expinstrs->end());
            instrs->push_back(useref(enumapp));
            delete expinstrs;
        }
        // need to end with an assign because future operations will look at the end of instrs for the lloc
        instrs->push_back(useref(new ISA::AssignValue(loc, loc)));
        return instrs;
    }

    ISA::Instructions* walkEnumerationStatement(EnumerationStatement* node) override {
        auto instrs = walk(node->enumerable());
        auto enumLoc = getLocFromAssign(instrs->back());

        assert(node->enumerable()->type()->intrinsic() == Type::Intrinsic::ENUMERABLE);
        auto elemType = new ISA::TypeReference(((Type::Enumerable*)node->enumerable()->type())->values());

        std::string name = "ENUM_" + std::to_string(_tempCounter++);

        _inFunction++;
        instrs->push_back(useref(new ISA::BeginFunction(name, new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::VOID)))));
        auto elemShared = node->local()->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
        instrs->push_back(useref(new ISA::FunctionParam(elemType, makeLocation(elemShared, "arg_" + node->local()->name()))));
        if ( node->index() != nullptr ) {
            auto idxShared = node->index()->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
            instrs->push_back(useref(new ISA::FunctionParam(
                new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::NUMBER)),
                makeLocation(idxShared, "arg_" + node->index()->name())
            )));
        } else {
            instrs->push_back(useref(new ISA::FunctionParam(
                new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::NUMBER)),
                makeLocation(ISA::Affinity::LOCAL, "UNUSEDarg_idx")
            )));
        }

        for ( auto stmt : *node->body() ) {
            auto s = walk(stmt);
            instrs->insert(instrs->end(), s->begin(), s->end());
            delete s;
        }

        instrs->push_back(useref(new ISA::Return0()));
        _inFunction--;

        instrs->push_back(useref(new ISA::Enumerate(
            elemType,
            enumLoc,
            makeLocation(ISA::Affinity::FUNCTION, name)
        )));

        return instrs;
    }

    ISA::Instructions* walkWithStatement(WithStatement* node) override {
        auto instrs = walk(node->resource());
        auto resLoc = getLocFromAssign(instrs->back());

        // get yield type
        auto type = new ISA::TypeReference(node->local()->type()->copy());
        auto typeLoc = makeNewTmp(ISA::Affinity::LOCAL);
        instrs->push_back(useref(new ISA::AssignValue(typeLoc, type)));
        std::string name = "WITH_" + std::to_string(_tempCounter++);

        _inFunction++;
        instrs->push_back(useref(new ISA::BeginFunction(name, new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::VOID)))));
        auto localShared = node->local()->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
        instrs->push_back(useref(new ISA::FunctionParam(typeLoc, makeLocation(localShared, "res_" + node->local()->name()))));

        // walk body
        for ( auto stmt : *node->body() ) {
            auto s = walk(stmt);
            instrs->insert(instrs->end(), s->begin(), s->end());
            delete s;
        }

        instrs->push_back(useref(new ISA::Return0()));
        _inFunction--;

        instrs->push_back(useref(new ISA::With(resLoc, makeLocation(ISA::Affinity::FUNCTION, name))));

        return instrs;
    }

    ISA::Instructions* walkIfStatement(IfStatement* node) override {
        auto instrs = new ISA::Instructions();

        // build IF function (potentially need global scope? add to map if so)
        std::string name = "IFBODY_" + std::to_string(_tempCounter++);
        _inFunction++;
        auto begin = new ISA::BeginFunction(name, new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::VOID)));
        instrs->push_back(useref(begin));
        for ( auto stmt : *node->body() ) {
            auto i = walk(stmt);
            instrs->insert(instrs->end(), i->begin(), i->end());
            delete i;
        }
        instrs->push_back(useref(new ISA::Return0()));
        _inFunction--;
        auto func = makeLocation(ISA::Affinity::FUNCTION, name);

        // walk condition and callif
        auto condition = walk(node->condition());
        instrs->insert(instrs->end(), condition->begin(), condition->end());
        instrs->push_back(useref(new ISA::CallIf0(getLocFromAssign(instrs->back()), func)));

        return instrs;
    }

    ISA::Instructions* walkWhileStatement(WhileStatement* node) override {
        auto instrs = new ISA::Instructions();

        instrs->push_back(useref(new ISA::AssignValue(
            makeLocation(ISA::Affinity::LOCAL, "whileCond"),
            new ISA::BooleanReference(false))
        ));
        // condition function
        _whileConds->push("WHILECOND_" + std::to_string(_tempCounter++));
        _inFunction++;
        instrs->push_back(useref(new ISA::BeginFunction(_whileConds->top(), new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::BOOLEAN)))));
        auto cond = walk(node->condition());
        instrs->insert(instrs->end(), cond->begin(), cond->end());
        instrs->push_back(useref(new ISA::Return1(getLocFromAssign(instrs->back()))));
        _inFunction--;

        // While function header
        std::string name = "WHILE_" + std::to_string(_tempCounter++);
        auto retType = new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::VOID));
        auto cfb = makeLocation(ISA::Affinity::LOCAL, "CFBWhile");
        _inFunction++;
        instrs->push_back(useref(new ISA::BeginFunction(name, retType)));

        // bring control flow breaker tracker in scope
        instrs->push_back(useref(new ISA::ScopeOf(cfb)));
        instrs->push_back(useref(new ISA::AssignValue(cfb, new ISA::BooleanReference(false))));

        std::queue<ISA::Instructions*> subfuncs;
        std::queue<std::string> funcnames;
        subfuncs.push(instrs);

        // Walk body, creating subfunctions from the body after every diversion of control flow
        for ( auto stmt : *node->body() ) {
            auto walkstmt = walk(stmt);
            subfuncs.back()->insert(subfuncs.back()->end(), walkstmt->begin(), walkstmt->end());
            if ( stmt->isBlock() ) {
                auto subf = new ISA::Instructions();
                auto funcname = "SUBFUNC_" + std::to_string(_tempCounter++);
                auto voidtype = new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::VOID));
                _inFunction++;
                subf->push_back(useref(new ISA::BeginFunction(funcname, voidtype)));
                subfuncs.push(subf);
                funcnames.push(funcname);
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
                instrs->push_back(useref(new ISA::Return0()));
                _inFunction--;
                auto loc = makeLocation(ISA::Affinity::FUNCTION, funcnames.front());
                instrs->push_back(useref(new ISA::CallElse0(cfb, loc)));
            } else {
                delete subf->at(0);
            }
            funcnames.pop();
        }

        // reevaluate condition
        instrs->push_back(useref(new ISA::CallElse0(cfb, makeLocation(ISA::Affinity::FUNCTION, _whileConds->top()))));

        // end of function return
        instrs->push_back(useref(new ISA::Return0()));
        _inFunction--;

        //evaluate condition
        instrs->push_back(useref(new ISA::AssignEval(
            makeLocation(ISA::Affinity::LOCAL, "whileCond"),
            new ISA::Call0(makeLocation(ISA::Affinity::FUNCTION, _whileConds->top()))
        )));

        instrs->push_back(useref(new ISA::While(
            makeLocation(ISA::Affinity::LOCAL, "whileCond"),
            makeLocation(ISA::Affinity::FUNCTION, name)
        )));

        _whileConds->pop();
        return instrs;
    }

    ISA::Instructions* walkContinueNode(ContinueNode* node) override {
        auto instrs = new ISA::Instructions();
        // evaluate condition
        instrs->push_back(useref(new ISA::AssignEval(
            makeLocation(ISA::Affinity::LOCAL, "whileCond"),
            new ISA::Call0(makeLocation(ISA::Affinity::FUNCTION, _whileConds->top()))
        )));
        // skip rest of function
        instrs->push_back(useref(new ISA::AssignValue(
            makeLocation(ISA::Affinity::LOCAL, "CFBWhile"),
            new ISA::BooleanReference(true)
        )));
        return instrs;
    }

    ISA::Instructions* walkBreakNode(BreakNode* node) override {
        auto instrs = new ISA::Instructions();
        // set condition to false
        instrs->push_back(useref(new ISA::AssignValue(
            makeLocation(ISA::Affinity::LOCAL, "whileCond"),
            new ISA::BooleanReference(false)
        )));
        // skip rest of function
        instrs->push_back(useref(new ISA::AssignValue(
            makeLocation(ISA::Affinity::LOCAL, "CFBWhile"),
            new ISA::BooleanReference(true)
        )));
        return instrs;
    }

    ISA::Instructions* walkReturnStatementNode(ReturnStatementNode* node) override {
        auto instrs = new ISA::Instructions();
        instrs->push_back(useref(new ISA::AssignValue(makeLocation(ISA::Affinity::LOCAL, "CFB"), new ISA::BooleanReference(true))));
        if ( node->value() != nullptr ) {
            auto exp = walk(node->value());
            auto retval = getLocFromAssign(exp->back());
            instrs->insert(instrs->end(), exp->begin(), exp->end());
            instrs->push_back(useref(new ISA::AssignValue(makeLocation(ISA::Affinity::LOCAL, "retVal"), retval)));
        }
        return instrs;
    }

    ISA::Instructions* walkMapStatementNode(MapStatementNode* node) override {
        return walk(node->value());
    }

    ISA::Instructions* walkMapNode(MapNode* node) override {
        auto instrs = new ISA::Instructions();
        auto loc = makeNewTmp(ISA::Affinity::LOCAL);
        assert(node->type()->intrinsic() == Type::Intrinsic::MAP);
        auto innerType = ((Type::Map*)node->type())->values();
        auto map = new ISA::AssignEval(loc, new ISA::MapInit(new ISA::TypeReference(innerType)));
        instrs->push_back(useref(map));
        for ( auto stmt : *node->body() ) {
            auto id = makeLocation(ISA::Affinity::LOCAL, "mkey_" + stmt->id()->name());
            auto mapstmt = walk(stmt);
            auto mapset = new ISA::MapSet(id, getLocFromAssign(mapstmt->back()), loc);
            instrs->insert(instrs->end(), mapstmt->begin(), mapstmt->end());
            instrs->push_back(useref(mapset));
            delete mapstmt;
        }
        // need to end with an assign because future operations will look at the end of instrs for the lloc
        instrs->push_back(useref(new ISA::AssignValue(loc, loc)));
        return instrs;
    }

    ISA::Instructions* walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) override {
        auto instrs = new ISA::Instructions();
        auto ref = new ISA::StringReference(node->value());
        instrs->push_back(useref(new ISA::AssignValue(makeNewTmp(ISA::Affinity::LOCAL), ref)));
        return instrs;
    }

    ISA::Instructions* walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) override {
        auto instrs = new ISA::Instructions();
        auto ref = new ISA::NumberReference(node->value());
        instrs->push_back(useref(new ISA::AssignValue(makeNewTmp(ISA::Affinity::LOCAL), ref)));
        return instrs;
    }

    ISA::Instructions* walkAssignExpressionNode(AssignExpressionNode* node) override {
        auto instrs = walk(node->value());
        auto value = getLocFromAssign(instrs->back());
        if ( node->dest()->getName() == "IdentifierNode" ) {
            auto id = (IdentifierNode*) node->dest();
            auto aff = id->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
            if ( id->symbol()->isProperty() ) {
                instrs->push_back(useref(new ISA::ObjSet(
                    determineType(((ObjectPropertySymbol*)id->symbol())->propertyOf()),
                    makeLocation(ISA::Affinity::OBJECTPROP, id->name()),
                    value
                )));
            } else {
                instrs->push_back(useref(new ISA::AssignValue(
                    makeLocation(aff, "var_" + id->name()), 
                    value
                )));
            }
        } else if ( node->dest()->getName() == "EnumerableAccessNode" ) {
            auto en = (EnumerableAccessNode*) node->dest();
            auto index = walk(en->index());
            auto path = walk(en->path());
            instrs->insert(instrs->end(), index->begin(), index->end());
            instrs->insert(instrs->end(), path->begin(), path->end());
            instrs->push_back(useref(
                new ISA::EnumSet(
                    getLocFromAssign(path->back()),
                    getLocFromAssign(index->back()),
                    value)
            ));
        } else if ( node->dest()->getName() == "MapAccessNode" ) {
            auto ma = (MapAccessNode*) node->dest();
            auto path = walk(ma->path());
            instrs->insert(instrs->end(), path->begin(), path->end());
            instrs->push_back(useref(
                new ISA::MapSet(
                    makeLocation(ISA::Affinity::LOCAL, "mkey_" + ma->end()->name()),
                    value,
                    getLocFromAssign(path->back()))
            ));
        } else if ( node->dest()->getName() == "ClassAccessNode" ) {
            auto ca = (ClassAccessNode*) node->dest();
            auto path = walk(ca->path());
            instrs->insert(instrs->end(), path->begin(), path->end());
            instrs->push_back(useref(
                new ISA::ObjSet(
                    getLocFromAssign(path->back()),
                    makeLocation(ISA::Affinity::OBJECTPROP, ca->end()->name()),
                    value
                )
            ));
        }
        return instrs;
    }

    ISA::Instructions* walkUnitNode(UnitNode* node) override {
        // this should never be called
        return new ISA::Instructions();
    }

    ISA::Instructions* walkFunctionNode(FunctionNode* node) override {
        auto instrs = new ISA::Instructions();

        std::string name = "FUNC_" + std::to_string(_tempCounter++);
        auto fnType = node->type();
        if (node->formals()->empty()) {
            assert( fnType->isCallable() );
            fnType = ((Type::Lambda*)fnType)->returns();
        } else {
            for ( size_t i = 0; i < node->formals()->size(); i++ ) {
                assert( fnType->isCallable() );
                fnType = ((Type::Lambda*)fnType)->returns();
            }
        }
        auto retType = determineType(fnType);
        auto retVar = makeLocation(ISA::Affinity::LOCAL, "retVal");
        auto cfb = makeLocation(ISA::Affinity::LOCAL, "CFB");
        _inFunction++;
        instrs->push_back(useref(new ISA::BeginFunction(name, retType)));

        // formals
        for ( auto f : *node->formals() ) {
            auto type = determineType(f.first->value());
            auto ref = makeLocation(ISA::Affinity::LOCAL, "arg_" + f.second->name());
            instrs->push_back(useref(new ISA::FunctionParam(type, ref)));
        }

        // bring return value trackers in scope
        if ( fnType->intrinsic() != Type::Intrinsic::VOID ) {
            instrs->push_back(useref(new ISA::ScopeOf(retVar)));
        }
        instrs->push_back(useref(new ISA::ScopeOf(cfb)));
        instrs->push_back(useref(new ISA::AssignValue(cfb, new ISA::BooleanReference(false))));

        std::queue<ISA::Instructions*> subfuncs;
        std::queue<std::string> funcnames;
        subfuncs.push(instrs);

        // Walk body, creating subfunctions after every control flow diversion
        for ( auto stmt : *node->body() ) {
            auto walkstmt = walk(stmt);
            subfuncs.back()->insert(subfuncs.back()->end(), walkstmt->begin(), walkstmt->end());
            if ( stmt->isBlock() ) {
                auto subf = new ISA::Instructions();
                auto funcname = "SUBFUNC_" + std::to_string(_tempCounter++);
                auto voidtype = new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::VOID));
                _inFunction++;
                subf->push_back(useref(new ISA::BeginFunction(funcname, voidtype)));
                subfuncs.push(subf);
                funcnames.push(funcname);
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
                instrs->push_back(useref(new ISA::Return0()));
                _inFunction--;
                auto loc = makeLocation(ISA::Affinity::FUNCTION, funcnames.front());
                instrs->push_back(useref(new ISA::CallElse0(cfb, loc)));
            } else {
                delete subf->at(0);
            }
            funcnames.pop();
        }

        // end of function return
        if ( fnType->intrinsic() == Type::Intrinsic::VOID ) {
            instrs->push_back(useref(new ISA::Return0()));
        } else {
            instrs->push_back(useref(new ISA::Return1(retVar)));
        }
        _inFunction--;

        // needed for assignment
        instrs->push_back(useref(new ISA::AssignValue(makeNewTmp(ISA::Affinity::LOCAL), makeLocation(ISA::Affinity::FUNCTION, name))));

        return instrs;
    }

    ISA::Instructions* walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) override {
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
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), instr)));
        delete right;
        return left;
    }

    ISA::Instructions* walkTypeBodyNode(TypeBodyNode* node) override {
        auto instrs = new ISA::Instructions();

        auto proto = makeNewTmp(ISA::Affinity::LOCAL);
        instrs->push_back(useref(new ISA::AssignEval(proto, new ISA::OTypeInit())));
        for (auto prop : *node->declarations()) {
            std::string name = "";
            Type::Type* type = nullptr;
            if (prop->getName() == "VariableDeclarationNode") {
                name = ((VariableDeclarationNode*)prop)->id()->name();
                type = ((VariableDeclarationNode*)prop)->typeNode()->value();

                // evaluate default value when the type is defined
                auto eval = walk(((VariableDeclarationNode*)prop)->value());
                instrs->insert(instrs->end(), eval->begin(), eval->end());
                auto defname = "deval_" + std::to_string(node->value()->getId()) + "_" + name;
                _objDefaults.insert(defname);
                instrs->push_back(useref(new ISA::AssignValue(
                    makeLocation(ISA::Affinity::LOCAL, defname),
                    getLocFromAssign(instrs->back())
                )));
                delete eval;
            } else if (prop->getName() == "UninitializedVariableDeclarationNode") {
                name = ((UninitializedVariableDeclarationNode*)prop)->id()->name();
                type = ((UninitializedVariableDeclarationNode*)prop)->typeNode()->value();
            }
            instrs->push_back(useref(new ISA::OTypeProp(
                proto, 
                makeLocation(ISA::Affinity::OBJECTPROP, name),
                determineType(type)
            )));
        }

        // create finalized type
        instrs->push_back(useref(
            new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL), new ISA::OTypeFinalize(proto))
        ));

        for (auto c : *node->constructors()) {
            auto cinstrs = walk(c);
            // remove unnecessary temp assignment
            freeref(cinstrs->back());
            cinstrs->pop_back();
            _tempCounter--;
            
            instrs->insert(instrs->end(), cinstrs->begin(), cinstrs->end());
            auto aff = c->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
            instrs->push_back(useref(new ISA::AssignValue(
                makeLocation(aff, c->name()),
                ((ISA::BeginFunction*)cinstrs->front())->first()
            )));
            delete cinstrs;
        }

        // create type reference
        _typeMap.insert({ node->value(), useref(new ISA::TypeReference(node->value())) });

        return instrs;
    }

    ISA::Instructions* walkClassAccessNode(ClassAccessNode* node) override {
        auto instrs = walk(node->path());
        auto obj = getLocFromAssign(instrs->back());
        instrs->push_back(useref(
            new ISA::AssignEval(
                makeNewTmp(ISA::Affinity::LOCAL), 
                new ISA::ObjGet(obj, makeLocation(ISA::Affinity::OBJECTPROP, node->end()->name()))
            )
        ));
        return instrs;
    }

    ISA::Instructions* walkIncludeStatementNode(IncludeStatementNode* node) override {
        //throw Errors::SwarmError("Unresolved IncludeStatementNode in ToISAWalk");
        return new ISA::Instructions();
    }

    ISA::Instructions* walkConstructorNode(ConstructorNode* node) override {
        // FIXME: std::bad_alloc happens in this function when using test 030
        auto function = walk(node->func());
        auto type = node->func()->type();
        while ( type->isCallable() ) {
            type = ((Type::Lambda*)type)->returns();
        }
        assert(type->intrinsic() == Type::Intrinsic::OBJECT);
        auto rettype = (Type::Object*)type;

        // get insertion point
        auto i = function->begin() + 1;
        while ( (*i)->tag() == ISA::Tag::FNPARAM ) i++;

        // initialize object
        auto obj = makeNewTmp(ISA::Affinity::LOCAL);
        function->insert(i, useref(new ISA::AssignEval(obj, new ISA::ObjInit(new ISA::TypeReference(rettype)))));
        std::cout << "\t\tinserted objinit\n";

        // insert default values
        for ( auto prop : ((Type::Object*)rettype)->getProperties() ) {
            std::string defname = "deval_" + std::to_string(rettype->getId()) + "_" + prop.first;
            if ( _objDefaults.count(defname) ) {
                function->insert(i++, useref(new ISA::ObjSet(
                    obj, 
                    makeLocation(ISA::Affinity::OBJECTPROP, prop.first),
                    makeLocation(ISA::Affinity::LOCAL, defname)
                )));
            }
        }
        std::cout << "\t\tinserted def values\n";

        // insert ObjInstance instruction right before the return
        auto j = function->rbegin();
        while ( (*j)->tag() != ISA::Tag::RETURN1 ) j++;
        function->insert((++j).base(), useref(new ISA::AssignEval(
            makeLocation(ISA::Affinity::LOCAL, "retVal"),
            new ISA::ObjInstance(obj)
        )));
        std::cout << "\t\tinserted objinstance\n";

        return function;
    }

    ISA::Instructions* walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) override {
        throw Errors::SwarmError("Unresolved UninitVarDeclNode in ToISAWalk");
    }

    [[nodiscard]] std::string toString() const override {
        return "ToISAWalk<>";
    }

private:
    int _tempCounter, _inFunction;
    std::stack<std::string>* _whileConds;

    ISA::LocationReference* makeNewTmp(ISA::Affinity affinity) {
        auto tmp = makeLocation(affinity, "tmp" + std::to_string(_tempCounter++));
        _locMap.insert({ tmp->fqName() ,tmp });
        return tmp;
    }

    static ISA::LocationReference* getLocFromAssign(ISA::Instruction* instr) {
        assert(instr->tag() == ISA::Tag::ASSIGNEVAL || instr->tag() == ISA::Tag::ASSIGNVALUE);
        if (instr->tag() == ISA::Tag::ASSIGNEVAL) {
            return ((ISA::AssignEval*)instr)->first();
        }
        return ((ISA::AssignValue*)instr)->first();
    }

    ISA::Reference* determineType(Type::Type* type) const {
        if (type->intrinsic() != Type::Intrinsic::OBJECT) return new ISA::TypeReference(type);
        if (_typeMap.count(type)) return _typeMap.at(type);
        return new ISA::TypeReference(Type::Primitive::of(Type::Intrinsic::THIS));
    }

    ISA::LocationReference* makeLocation(ISA::Affinity aff, std::string name) {
        std::string n = ISA::LocationReference::affinityString(aff) + ":" + name;
        if ( _locMap.count(n) ) {
            return _locMap.at(n);
        }
        auto tmp = useref(new ISA::LocationReference(aff, name));
        _locMap.insert({ tmp->fqName(), tmp });
        return tmp;
    }

    std::map<Type::Type*, ISA::TypeReference*> _typeMap;
    std::map<std::string, ISA::LocationReference*> _locMap;

    // used for keeping track of which type members have default values
    std::set<std::string> _objDefaults;
};

}

#endif
