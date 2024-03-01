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
        _tempCounter(0), _inFunction(0),
        _typeMap(new std::map<std::size_t, ISA::TypeReference*>()),
        _locMap(new std::map<std::string, ISA::LocationReference*>()),
        _deferredRetLocs(new std::set<ISA::LocationReference*>()) {}

    ~ToISAWalk() override {
        for (auto p : *_typeMap) freeref(p.second);
        for (auto p : *_locMap) freeref(p.second);
        for (auto l : *_deferredRetLocs) freeref(l);
        delete _typeMap;
        delete _locMap;
        delete _deferredRetLocs;
    }
protected:
    ISA::Instructions* walkProgramNode(ProgramNode* node) override {
        auto instrs = new ISA::Instructions();
        while ( node->body()->size() ) {
            auto i = walk(node->body()->front());
            instrs->insert(instrs->end(), i->begin(), i->end());
            freeref(node->body()->front());
            node->body()->erase(node->body()->begin());
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
        auto ref = makeLocation(affinity, "var_" + node->name(), nullptr);
        // TODO: remove first case once import-based prologue is implemented
        if ( node->symbol()->isPrologue() ) {
            auto fref = makeLocation(ISA::Affinity::FUNCTION, ((Lang::PrologueFunctionSymbol*)node->symbol())->sviName(), nullptr);
            instrs->push_back(useref(new ISA::AssignValue(ref, fref)));
        } else if ( node->symbol()->isProperty() ) { // use objget to get property values
            instrs->push_back(useref(new ISA::AssignEval(ref, new ISA::ObjGet(
                _constructing.top().first, 
                makeLocation(ISA::Affinity::OBJECTPROP, node->name(), nullptr))
            )));
        } else {
            // necessary for instructions that pull the location from the bottommost instruction
            instrs->push_back(useref(new ISA::AssignValue(ref, ref)));
        }
        return instrs;
    }

    ISA::Instructions* walkMapAccessNode(MapAccessNode* node) override {
        auto instrs = walk(node->path());
        auto mapget = new ISA::MapGet(
            new ISA::StringReference("mkey_" + node->end()->name()), getLocFromAssign(instrs->back()));
        instrs->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, instrs), mapget)));
        return instrs;
    }

    ISA::Instructions* walkEnumerableAccessNode(EnumerableAccessNode* node) override {
        auto instrs = walk(node->index());
        auto llval = walk(node->path());
        auto enumget = new ISA::EnumGet(getLocFromAssign(llval->back()), getLocFromAssign(instrs->back()));
        instrs->insert(instrs->end(), llval->begin(), llval->end());
        instrs->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, instrs), enumget)));
        return instrs;
    }

    ISA::Instructions* walkTypeLiteral(swarmc::Lang::TypeLiteral *node) override {
        auto instrs = new ISA::Instructions();
        auto ref = getTypeRef(node->value());
        instrs->push_back(useref(new ISA::AssignValue(makeNewTmp(ISA::Affinity::LOCAL, instrs), ref)));
        return instrs;
    }

    ISA::Instructions* walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) override {
        auto instrs = new ISA::Instructions();
        auto ref = new ISA::BooleanReference(node->value());
        instrs->push_back(useref(new ISA::AssignValue(makeNewTmp(ISA::Affinity::LOCAL, instrs), ref)));
        return instrs;
    }

    ISA::Instructions* walkVariableDeclarationNode(VariableDeclarationNode* node) override {
        auto instrs = walk(node->value());
        auto vloc = getLocFromAssign(instrs->back());

        // Create location from variable name
        auto aff = node->id()->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
        auto loc = makeLocation(aff, "var_" + node->id()->name(), instrs);

        // acquire temporary location from previous instruction and assign
        instrs->push_back(useref(new ISA::AssignValue(loc, vloc)));

        return instrs;
    }

    ISA::Instructions* walkCallExpressionNode(CallExpressionNode* node) override {
        ISA::Instructions* instrs;
        ISA::LocationReference* floc;
        Type::Type* fnType;

        // TODO: remove this once import-based prologue gets implemented
        if ( node->func()->getName() == "IdentifierNode" ) {
            std::string name = ((IdentifierNode*)node->func())->name();
            if ( ToISAWalk::FuncToLocation.count(name) != 0 ) {
                instrs = walk(node->args()->at(0));
                instrs->push_back(useref(new ISA::StreamPush(
                    ToISAWalk::FuncToLocation.at(name),
                    getLocFromAssign(instrs->back())
                )));
                return instrs;
            }
        }

        if (node->constructor()) {
            instrs = new ISA::Instructions();
            floc = makeLocation(ISA::Affinity::LOCAL, node->constructor()->name(), nullptr);
            fnType = node->constructor()->func()->type();
        } else {
            instrs = walk(node->func());
            floc = getLocFromAssign(instrs->back());
            fnType = node->func()->type();
        }

        if ( node->args()->empty() ) {
            auto call = new ISA::Call0(floc);
            if (node->type()->intrinsic() == Type::Intrinsic::VOID) {
                instrs->push_back(useref(call));
            } else {
                instrs->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, instrs), call)));
            }
        } else {
            for ( auto arg : *node->args() ) {
                auto evalarg = walk(arg);
                instrs->insert(instrs->end(), evalarg->begin(), evalarg->end());
                assert(fnType->isCallable());

                // curry or call func in sequence
                if (((Type::Lambda*)fnType)->returns()->intrinsic() == Type::Intrinsic::VOID && !node->constructor()) {
                    auto call = new ISA::Call1(floc, getLocFromAssign(evalarg->back()));
                    instrs->push_back(useref(call));
                } else if (((Type::Lambda*)fnType)->returns()->isCallable()) {
                    auto call = new ISA::Curry(floc, getLocFromAssign(evalarg->back()));
                    auto loc2 = makeNewTmp(ISA::Affinity::LOCAL, instrs);
                    instrs->push_back(useref(new ISA::AssignEval(loc2, call)));
                    floc = loc2;
                } else {
                    auto call = new ISA::Call1(floc, getLocFromAssign(evalarg->back()));
                    auto loc2 = makeNewTmp(ISA::Affinity::LOCAL, instrs);
                    instrs->push_back(useref(new ISA::AssignEval(loc2, call)));
                    floc = loc2;
                }

                fnType = ((Type::Lambda*)fnType)->returns();
                delete evalarg;
            }
        }

        return instrs;
    }

    ISA::Instructions* walkDeferCallExpressionNode(DeferCallExpressionNode* node) override {
        ISA::Instructions* instrs;
        ISA::LocationReference* floc;
        Type::Type* fnType;

        // Idk why you would ever do this, but this handles the case of
        // deferring functions that arent actually functions in SVI
        if ( node->call()->func()->getName() == "IdentifierNode" ) {
            std::string name = ((IdentifierNode*)node->call()->func())->name();
            if ( ToISAWalk::FuncToLocation.count(name) != 0 ) {
                instrs = new ISA::Instructions();
                // build function
                if ( ToISAWalk::FuncToFunc.count(name) == 0 ) {
                    auto finstrs = new ISA::Instructions();
                    auto param = makeLocation(ISA::Affinity::LOCAL, "var_content", nullptr);
                    finstrs->push_back(new ISA::BeginFunction(name, getTypeRef(Type::Primitive::of(Type::Intrinsic::VOID))));
                    finstrs->push_back(new ISA::FunctionParam(
                        getTypeRef(Type::Primitive::of(Type::Intrinsic::STRING)),
                        param
                    ));
                    finstrs->push_back(useref(new ISA::StreamPush(
                        ToISAWalk::FuncToLocation.at(name),
                        param
                    )));
                    finstrs->push_back(new ISA::Return0());
                    ToISAWalk::FuncToFunc.insert({ name, finstrs });
                    instrs->insert(instrs->begin(), finstrs->begin(), finstrs->end());
                }
                instrs = walk(node->call()->args()->at(0));
                instrs->push_back(new ISA::PushCall1(
                    makeLocation(ISA::Affinity::FUNCTION, name, nullptr),
                    getLocFromAssign(instrs->back())
                ));
                return instrs;
            }
        }

        if (node->call()->constructor()) {
            instrs = new ISA::Instructions();
            floc = makeLocation(ISA::Affinity::LOCAL, node->call()->constructor()->name(), nullptr);
            fnType = node->call()->constructor()->func()->type();
        } else {
            instrs = walk(node->call()->func());
            floc = getLocFromAssign(instrs->back());
            fnType = node->call()->func()->type();
        }

        if ( node->call()->args()->empty() ) {
            auto call = new ISA::PushCall0(floc);
            if (node->type()->intrinsic() == Type::Intrinsic::VOID) {
                instrs->push_back(useref(call));
            } else {
                auto retLoc = makeNewTmp(ISA::Affinity::LOCAL, instrs);
                instrs->push_back(useref(new ISA::AssignEval(retLoc, call)));
                _deferredRetLocs->insert(useref(retLoc));
            }
        } else {
            for ( auto arg : *node->call()->args() ) {
                auto evalarg = walk(arg);
                instrs->insert(instrs->end(), evalarg->begin(), evalarg->end());
                assert(fnType->isCallable());

                // curry or call func in sequence
                if (((Type::Lambda*)fnType)->returns()->intrinsic() == Type::Intrinsic::VOID && !node->call()->constructor()) {
                    auto call = new ISA::PushCall1(floc, getLocFromAssign(evalarg->back()));
                    instrs->push_back(useref(call));
                } else if (((Type::Lambda*)fnType)->returns()->isCallable()) {
                    auto call = new ISA::Curry(floc, getLocFromAssign(evalarg->back()));
                    auto loc2 = makeNewTmp(ISA::Affinity::LOCAL, instrs);
                    instrs->push_back(useref(new ISA::AssignEval(loc2, call)));
                    floc = loc2;
                } else {
                    auto call = new ISA::PushCall1(floc, getLocFromAssign(evalarg->back()));
                    auto loc2 = makeNewTmp(ISA::Affinity::LOCAL, instrs);
                    instrs->push_back(useref(new ISA::AssignEval(loc2, call)));
                    floc = loc2;
                    _deferredRetLocs->insert(loc2);
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
                instrs->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, instrs), call)));
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
                    auto loc2 = makeNewTmp(ISA::Affinity::LOCAL, instrs);
                    instrs->push_back(useref(new ISA::AssignEval(loc2, call)));
                    floc = loc2;
                } else {
                    auto call = new ISA::Call1(floc, getLocFromAssign(evalarg->back()));
                    auto loc2 = makeNewTmp(ISA::Affinity::LOCAL, instrs);
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
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, left), new ISA::And(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkOrNode(OrNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, left), new ISA::Or(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkEqualsNode(EqualsNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, left), new ISA::IsEqual(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkNotEqualsNode(NotEqualsNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        auto loc = makeNewTmp(ISA::Affinity::LOCAL, left);
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
            left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, left), new ISA::StringConcat(lhs, rhs))));
        } else {
            left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, left), new ISA::Plus(lhs, rhs))));
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
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, left), new ISA::Minus(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkMultiplyNode(MultiplyNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, left), new ISA::Times(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkDivideNode(DivideNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, left), new ISA::Divide(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkModulusNode(ModulusNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, left), new ISA::Mod(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkPowerNode(PowerNode* node) override {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, left), new ISA::Power(lhs, rhs))));
        delete right;
        return left;
    }

    ISA::Instructions* walkNegativeExpressionNode(NegativeExpressionNode* node) override {
        auto instrs = walk(node->exp());
        auto loc = getLocFromAssign(instrs->back());
        instrs->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, instrs), new ISA::Negative(loc))));
        return instrs;
    }

    ISA::Instructions* walkSqrtNode(SqrtNode* node) override {
        auto instrs = walk(node->exp());
        auto loc = getLocFromAssign(instrs->back());
        auto curried = makeNewTmp(ISA::Affinity::LOCAL, instrs);
        instrs->push_back(useref(new ISA::AssignEval(curried, new ISA::Curry(
            makeLocation(ISA::Affinity::FUNCTION, "NTH_ROOT", nullptr),
            new ISA::NumberReference(2)
        ))));
        instrs->push_back(useref(new ISA::AssignEval(
            makeNewTmp(ISA::Affinity::LOCAL, instrs),
            new ISA::Call1(curried, loc)
        )));
        return instrs;
    }

    ISA::Instructions* walkNotNode(NotNode* node) override {
        auto instrs = walk(node->exp());
        auto loc = getLocFromAssign(instrs->back());
        instrs->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, instrs), new ISA::Not(loc))));
        return instrs;
    }

    ISA::Instructions* walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) override {
        auto instrs = new ISA::Instructions();
        auto loc = makeNewTmp(ISA::Affinity::LOCAL, instrs);
        assert(node->type()->intrinsic() == Type::Intrinsic::ENUMERABLE);
        auto innerType = ((Type::Enumerable*)node->type())->values();
        auto map = new ISA::AssignEval(loc, new ISA::EnumInit(getTypeRef(innerType)));
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
        auto elemType = getTypeRef(((Type::Enumerable*)node->enumerable()->type())->values());

        std::string name = "ENUM_" + std::to_string(_tempCounter++);

        _inFunction++;
        instrs->push_back(useref(new ISA::BeginFunction(name, getTypeRef(Type::Primitive::of(Type::Intrinsic::VOID)))));
        auto elemShared = node->local()->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
        instrs->push_back(useref(new ISA::FunctionParam(elemType, makeLocation(elemShared, "var_" + node->local()->name(), nullptr))));
        if ( node->index() != nullptr ) {
            auto idxShared = node->index()->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
            instrs->push_back(useref(new ISA::FunctionParam(
                getTypeRef(Type::Primitive::of(Type::Intrinsic::NUMBER)),
                makeLocation(idxShared, "var_" + node->index()->name(), nullptr)
            )));
        } else {
            instrs->push_back(useref(new ISA::FunctionParam(
                getTypeRef(Type::Primitive::of(Type::Intrinsic::NUMBER)),
                makeLocation(ISA::Affinity::LOCAL, "UNUSEDvar_idx", nullptr)
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
            makeLocation(ISA::Affinity::FUNCTION, name, nullptr)
        )));

        return instrs;
    }

    ISA::Instructions* walkWithStatement(WithStatement* node) override {
        auto instrs = walk(node->resource());
        auto resLoc = getLocFromAssign(instrs->back());
        static auto resFunc = new ISA::LocationReference(ISA::Affinity::FUNCTION, "RESOURCE_T");

        // get yield type
        assert(node->local()->type()->intrinsic() == Type::Intrinsic::RESOURCE);
        auto resource = (Type::Resource*)node->local()->type();
        auto opaque = ((Type::Opaque*)resource->yields())->toString();
        auto tagLoc = makeNewTmp(ISA::Affinity::LOCAL, instrs);
        instrs->push_back(useref(new ISA::AssignEval(tagLoc, new ISA::Call0(_opaqueVals.at(opaque)))));
        auto typeLoc = makeNewTmp(ISA::Affinity::LOCAL, instrs);
        instrs->push_back(useref(new ISA::AssignEval(typeLoc, new ISA::Call1(resFunc, tagLoc))));
        std::string name = "WITH_" + std::to_string(_tempCounter++);

        _inFunction++;
        instrs->push_back(useref(new ISA::BeginFunction(name, getTypeRef(Type::Primitive::of(Type::Intrinsic::VOID)))));
        auto localShared = node->local()->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
        instrs->push_back(useref(new ISA::FunctionParam(typeLoc, makeLocation(localShared, "var_" + node->local()->name(), nullptr))));

        // walk body
        for ( auto stmt : *node->body() ) {
            auto s = walk(stmt);
            instrs->insert(instrs->end(), s->begin(), s->end());
            delete s;
        }

        instrs->push_back(useref(new ISA::Return0()));
        _inFunction--;

        instrs->push_back(useref(new ISA::With(resLoc, makeLocation(ISA::Affinity::FUNCTION, name, nullptr))));

        return instrs;
    }

    ISA::Instructions* walkIfStatement(IfStatement* node) override {
        auto instrs = new ISA::Instructions();

        // build IF function (potentially need global scope? add to map if so)
        std::string name = "IFBODY_" + std::to_string(_tempCounter++);
        _inFunction++;
        auto begin = new ISA::BeginFunction(name, getTypeRef(Type::Primitive::of(Type::Intrinsic::VOID)));
        instrs->push_back(useref(begin));
        for ( auto stmt : *node->body() ) {
            auto i = walk(stmt);
            instrs->insert(instrs->end(), i->begin(), i->end());
            delete i;
        }
        instrs->push_back(useref(new ISA::Return0()));
        _inFunction--;
        auto func = makeLocation(ISA::Affinity::FUNCTION, name, nullptr);

        // walk condition and callif
        auto condition = walk(node->condition());
        instrs->insert(instrs->end(), condition->begin(), condition->end());
        instrs->push_back(useref(new ISA::CallIf0(getLocFromAssign(instrs->back()), func)));

        return instrs;
    }

    ISA::Instructions* walkWhileStatement(WhileStatement* node) override {
        auto instrs = new ISA::Instructions();

        // condition function
        std::string condName = "WHILECOND_" + std::to_string(_tempCounter++);
        auto brk = makeLocation(ISA::Affinity::LOCAL, "break", instrs);
        _inFunction++;
        instrs->push_back(useref(new ISA::BeginFunction(condName, getTypeRef(Type::Primitive::of(Type::Intrinsic::BOOLEAN)))));
        auto cond = walk(node->condition());
        instrs->insert(instrs->end(), cond->begin(), cond->end());
        instrs->push_back(useref(new ISA::Return1(getLocFromAssign(cond->back()))));
        _inFunction--;

        // While function header
        std::string name = "WHILE_" + std::to_string(_tempCounter++);
        auto retType = getTypeRef(Type::Primitive::of(Type::Intrinsic::VOID));
        _inFunction++;
        instrs->push_back(useref(new ISA::BeginFunction(name, retType)));

        auto cfb = makeLocation(ISA::Affinity::LOCAL, "CFBWhile", instrs);
        instrs->push_back(useref(new ISA::AssignValue(cfb, new ISA::BooleanReference(false))));
        instrs->push_back(useref(new ISA::AssignValue(brk, new ISA::BooleanReference(false))));

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
                auto voidtype = getTypeRef(Type::Primitive::of(Type::Intrinsic::VOID));
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
                auto loc = makeLocation(ISA::Affinity::FUNCTION, funcnames.front(), nullptr);
                instrs->push_back(useref(new ISA::CallElse0(cfb, loc)));
            } else {
                delete subf->at(0);
            }
            funcnames.pop();
        }

        // end of function return
        instrs->push_back(useref(new ISA::Return0()));
        _inFunction--;

        instrs->push_back(useref(new ISA::While(
            makeLocation(ISA::Affinity::FUNCTION, condName, nullptr),
            makeLocation(ISA::Affinity::FUNCTION, name, nullptr)
        )));

        return instrs;
    }

    ISA::Instructions* walkContinueNode(ContinueNode* node) override {
        auto instrs = new ISA::Instructions();
        // skip rest of function
        instrs->push_back(useref(new ISA::AssignValue(
            makeLocation(ISA::Affinity::LOCAL, "CFBWhile", nullptr),
            new ISA::BooleanReference(true)
        )));
        return instrs;
    }

    ISA::Instructions* walkBreakNode(BreakNode* node) override {
        auto instrs = new ISA::Instructions();
        // skip rest of function
        instrs->push_back(useref(new ISA::AssignValue(
            makeLocation(ISA::Affinity::LOCAL, "CFBWhile", nullptr),
            new ISA::BooleanReference(true)
        )));
        instrs->push_back(useref(new ISA::AssignValue(
            makeLocation(ISA::Affinity::LOCAL, "break", nullptr),
            new ISA::BooleanReference(true)
        )));
        Console::get()->warn("Hi. If you're reading this, it means you used a break; statement in your code. I realized that because while conditions can have side effects due to function calls, compiling break was gonna get real goofy. I decided I only wanted to do it once and not have to figure it out in this legacy ToISAWalk, so it currently doesnt work");
        return instrs;
    }

    ISA::Instructions* walkReturnStatementNode(ReturnStatementNode* node) override {
        auto instrs = new ISA::Instructions();
        instrs->push_back(useref(new ISA::AssignValue(makeLocation(ISA::Affinity::LOCAL, "CFB", nullptr), new ISA::BooleanReference(true))));
        if ( node->value() != nullptr ) {
            auto exp = walk(node->value());
            auto retval = getLocFromAssign(exp->back());
            instrs->insert(instrs->end(), exp->begin(), exp->end());
            instrs->push_back(useref(new ISA::AssignValue(makeLocation(ISA::Affinity::LOCAL, "retVal", instrs), retval)));
        }
        return instrs;
    }

    ISA::Instructions* walkMapStatementNode(MapStatementNode* node) override {
        return walk(node->value());
    }

    ISA::Instructions* walkMapNode(MapNode* node) override {
        auto instrs = new ISA::Instructions();
        auto loc = makeNewTmp(ISA::Affinity::LOCAL, instrs);
        assert(node->type()->intrinsic() == Type::Intrinsic::MAP);
        auto innerType = ((Type::Map*)node->type())->values();
        auto map = new ISA::AssignEval(loc, new ISA::MapInit(getTypeRef(innerType)));
        instrs->push_back(useref(map));
        for ( auto stmt : *node->body() ) {
            ISA::Reference* id = new ISA::StringReference("mkey_" + stmt->id()->name());
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
        instrs->push_back(useref(new ISA::AssignValue(makeNewTmp(ISA::Affinity::LOCAL, instrs), ref)));
        return instrs;
    }

    ISA::Instructions* walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) override {
        auto instrs = new ISA::Instructions();
        auto ref = new ISA::NumberReference(node->value());
        instrs->push_back(useref(new ISA::AssignValue(makeNewTmp(ISA::Affinity::LOCAL, instrs), ref)));
        return instrs;
    }

    ISA::Instructions* walkAssignExpressionNode(AssignExpressionNode* node) override {
        auto instrs = walk(node->value());
        auto value = getLocFromAssign(instrs->back());
        if ( node->dest()->getName() == "IdentifierNode" ) {
            auto id = (IdentifierNode*) node->dest();
            auto aff = id->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
            if ( id->symbol()->isProperty() ) {
                assert(!_constructing.empty());
                instrs->push_back(useref(new ISA::ObjSet(
                    _constructing.top().first,
                    makeLocation(ISA::Affinity::OBJECTPROP, id->name(), nullptr),
                    value
                )));
            } else {
                instrs->push_back(useref(new ISA::AssignValue(
                    makeLocation(aff, "var_" + id->name(),  nullptr),
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
                    makeLocation(ISA::Affinity::LOCAL, "mkey_" + ma->end()->name(), nullptr),
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
                    makeLocation(ISA::Affinity::OBJECTPROP, ca->end()->name(), nullptr),
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

        auto retType = getTypeRef(fnType);
        _inFunction++;
        instrs->push_back(useref(new ISA::BeginFunction(name, retType)));
        if ( !_isMemberFunctionOf.empty() ) {
            _constructing.push(std::make_pair(makeNewTmp(ISA::Affinity::LOCAL, nullptr), _isMemberFunctionOf.top()));
            instrs->push_back(useref(new ISA::FunctionParam(
                getTypeRef(_isMemberFunctionOf.top()),
                _constructing.top().first
            )));
        }

        // formals
        for ( auto f : *node->formals() ) {
            auto type = getTypeRef(f.first->value());
            auto ref = makeLocation(ISA::Affinity::LOCAL, "var_" + f.second->name(), nullptr);
            instrs->push_back(useref(new ISA::FunctionParam(type, ref)));
        }

        // bring return value trackers in scope
        auto retVar = makeLocation(ISA::Affinity::LOCAL, "retVal", nullptr);
        auto cfb = makeLocation(ISA::Affinity::LOCAL, "CFB", instrs);
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
                auto voidtype = getTypeRef(Type::Primitive::of(Type::Intrinsic::VOID));
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
                auto loc = makeLocation(ISA::Affinity::FUNCTION, funcnames.front(), nullptr);
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
        if ( !_isMemberFunctionOf.empty() ) _constructing.pop();
        _inFunction--;

        // needed for assignment
        instrs->push_back(useref(new ISA::AssignValue(makeNewTmp(ISA::Affinity::LOCAL, instrs), makeLocation(ISA::Affinity::FUNCTION, name, nullptr))));

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
        left->push_back(useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, left), instr)));
        delete right;
        return left;
    }

    ISA::Instructions* walkTypeBodyNode(TypeBodyNode* node) override {
        auto instrs = new ISA::Instructions();

        auto proto = makeNewTmp(ISA::Affinity::LOCAL, instrs);
        instrs->push_back(useref(new ISA::AssignEval(proto, new ISA::OTypeInit())));
        for (auto prop : *node->declarations()) {
            std::string name = "";
            Type::Type* type = nullptr;
            if (prop->getName() == "VariableDeclarationNode") {
                name = ((VariableDeclarationNode*)prop)->id()->name();
                type = ((VariableDeclarationNode*)prop)->typeNode()->value();
            } else if (prop->getName() == "UninitializedVariableDeclarationNode") {
                name = ((UninitializedVariableDeclarationNode*)prop)->id()->name();
                type = ((UninitializedVariableDeclarationNode*)prop)->typeNode()->value();
            }
            instrs->push_back(useref(new ISA::OTypeProp(
                proto,
                makeLocation(ISA::Affinity::OBJECTPROP, name, nullptr),
                getTypeRef(type, node->value())
            )));
        }

        // create finalized type
        auto ofinal = makeNewTmp(ISA::Affinity::LOCAL, instrs);
        instrs->push_back(useref(
            new ISA::AssignEval(ofinal, new ISA::OTypeFinalize(proto))
        ));

        // create type reference
        _typeMap->insert({ node->value()->getId(), useref(new ISA::ObjectTypeReference((Type::Object*) node->value())) });

        // get default values
        _isMemberFunctionOf.push((Type::Object*)node->value());
        for ( auto p : *node->declarations() ) {
            if (p->getName() == "VariableDeclarationNode") {
                auto pvd = (VariableDeclarationNode*)p;
                auto eval = walk(pvd);

                // remove bad assign to variable name
                freeref(eval->back());
                eval->pop_back();

                instrs->insert(instrs->end(), eval->begin(), eval->end());
                auto defname = "deval_" + std::to_string(node->value()->getId()) + "_" + pvd->id()->name();
                _objDefaults.insert(defname);
                instrs->push_back(useref(new ISA::AssignValue(
                    makeLocation(ISA::Affinity::LOCAL, defname, nullptr),
                    getLocFromAssign(instrs->back())
                )));
                delete eval;
            }
        }
        _isMemberFunctionOf.pop();

        // walk constructors
        for (auto c : *node->constructors()) {
            auto cinstrs = walk(c);
            instrs->insert(instrs->end(), cinstrs->begin(), cinstrs->end());
            delete cinstrs;
        }

        instrs->push_back(useref(new ISA::AssignValue(ofinal, ofinal)));

        return instrs;
    }

    ISA::Instructions* walkClassAccessNode(ClassAccessNode* node) override {
        auto instrs = walk(node->path());
        auto obj = getLocFromAssign(instrs->back());
        instrs->push_back(useref(
            new ISA::AssignEval(
                makeNewTmp(ISA::Affinity::LOCAL, instrs),
                new ISA::ObjGet(obj, makeLocation(ISA::Affinity::OBJECTPROP, node->end()->name(), nullptr))
            )
        ));
        return instrs;
    }

    ISA::Instructions* walkIncludeStatementNode(IncludeStatementNode* node) override {
        //throw Errors::SwarmError("Unresolved IncludeStatementNode in ToISAWalk");
        return new ISA::Instructions();
    }

    ISA::Instructions* walkConstructorNode(ConstructorNode* node) override {
        auto obj = makeNewTmp(ISA::Affinity::LOCAL, nullptr);

        // add obj to stack so assignments know what obj to use
        _constructing.push(std::make_pair(obj, node->partOf()));
        auto function = walk(node->func());
        _constructing.pop();

        assert(node->partOf() != nullptr);
        auto rettype = node->partOf();

        auto transformedFunction = new std::vector<ISA::Instruction*>;

        _inFunction++;
        // change ret type to object from void
        transformedFunction->push_back(useref(new ISA::BeginFunction(
            ((ISA::BeginFunction*)function->front())->first()->name(),
            getTypeRef(rettype)
        )));
        freeref(function->front());
        function->erase(function->begin());

        // fnparams
        while ( true ) {
            if ( function->empty() ) break;
            auto front = *function->begin();
            if ( front->tag() == ISA::Tag::FNPARAM ) {
                transformedFunction->push_back(front);
                function->erase(function->begin());
                continue;
            }

            break;
        }
        transformedFunction->push_back(useref(new ISA::ScopeOf(
            makeLocation(ISA::Affinity::LOCAL, "retVal", nullptr)
        )));

        // initialize object
        transformedFunction->push_back(useref(new ISA::ScopeOf(obj)));
        transformedFunction->push_back(useref(new ISA::AssignEval(obj, new ISA::ObjInit(new ISA::ObjectTypeReference(rettype)))));

        // insert default values
        for ( const auto& prop : rettype->getProperties() ) {
            std::string defname = "deval_" + std::to_string(rettype->getId()) + "_" + prop.first;
            if ( _objDefaults.count(defname) ) {
                auto p = makeLocation(ISA::Affinity::OBJECTPROP, prop.first, nullptr);
                auto deval = makeLocation(ISA::Affinity::LOCAL, defname, nullptr);
                if ( prop.second->isCallable() ) {
                    auto a = useref(new ISA::AssignEval(makeNewTmp(ISA::Affinity::LOCAL, transformedFunction), new ISA::Curry(deval, obj)));
                    transformedFunction->push_back(a);
                    auto b = useref(new ISA::ObjSet(obj, p, a->first()));
                    transformedFunction->push_back(b);
                } else {
                    transformedFunction->push_back(new ISA::ObjSet(obj, p, deval));
                }
            }
        }

        while ( !function->empty() ) {
            transformedFunction->push_back(*function->begin());
            function->erase(function->begin());
        }

        // insert ObjInstance instruction right before the return
        auto j = transformedFunction->rbegin();
        do { j++; } while ( (*j)->tag() != ISA::Tag::RETURN0 );
        transformedFunction->insert((j + 1).base(), useref(new ISA::AssignEval(
            makeLocation(ISA::Affinity::LOCAL, "retVal", nullptr),
            new ISA::ObjInstance(obj)
        )));
        // change return0 to return retval
        freeref(*(j - 1));
        *(j - 1) = useref(new ISA::Return1(
            makeLocation(ISA::Affinity::LOCAL, "retVal", nullptr)
        ));
        _inFunction--;

         // remove unnecessary temp assignment
        freeref(transformedFunction->back());
        transformedFunction->pop_back();
        _tempCounter--;

        transformedFunction->push_back(useref(new ISA::AssignValue(
            makeLocation(ISA::Affinity::LOCAL, node->name(), transformedFunction),
            ((ISA::BeginFunction*)transformedFunction->front())->first()
        )));

        return transformedFunction;
    }

    ISA::Instructions* walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) override {
        throw Errors::SwarmError("Unresolved UninitVarDeclNode in ToISAWalk");
    }

    [[nodiscard]] std::string toString() const override {
        return "ToISAWalk<>";
    }

private:
    int _tempCounter, _inFunction;

    ISA::LocationReference* makeNewTmp(ISA::Affinity affinity, ISA::Instructions* instrs) {
        return makeLocation(affinity, "tmp" + std::to_string(_tempCounter++), instrs);
    }

    static ISA::LocationReference* getLocFromAssign(ISA::Instruction* instr) {
        assert(instr->tag() == ISA::Tag::ASSIGNEVAL || instr->tag() == ISA::Tag::ASSIGNVALUE);
        if (instr->tag() == ISA::Tag::ASSIGNEVAL) {
            return ((ISA::AssignEval*)instr)->first();
        }
        return ((ISA::AssignValue*)instr)->first();
    }

    ISA::TypeReference* getTypeRef(Type::Type* type, Type::Type* comp=nullptr) const {
        auto tp = thisify(type, comp);
        std::size_t id = tp->getId();
        if (_typeMap->count(id)) return _typeMap->at(id);
        auto t = new ISA::TypeReference(tp);
        _typeMap->insert({ id, useref(t) });
        return t;
    }

    Type::Type* thisify(Type::Type* type, Type::Type* comp) const {
        if (comp == nullptr) return type;
        if (type->isAssignableTo(comp)) return Type::Primitive::of(Type::Intrinsic::THIS);
        type->transform([comp](Type::Type* t) -> Type::Type* {
            if (t->isAssignableTo(comp)) return Type::Primitive::of(Type::Intrinsic::THIS);
            return t;
        });
        return type;
    }

    ISA::LocationReference* makeLocation(ISA::Affinity aff, std::string name, ISA::Instructions* instrs) {
        std::string n = ISA::LocationReference::affinityString(aff) + ":" + name;
        if ( _locMap->count(n) ) {
            if ( _inFunction && instrs != nullptr ) {
                instrs->push_back(useref(new ISA::ScopeOf(_locMap->at(n))));
            }
            return _locMap->at(n);
        }
        auto tmp = useref(new ISA::LocationReference(aff, name));
        _locMap->insert({ tmp->fqName(), tmp });
        if ( _inFunction && instrs != nullptr ) {
            instrs->push_back(useref(new ISA::ScopeOf(tmp)));
        }
        return tmp;
    }

    std::stack<Type::Object*> _isMemberFunctionOf;
    std::map<std::size_t, ISA::TypeReference*>* _typeMap;
    std::map<std::string, ISA::LocationReference*>* _locMap;
    std::set<ISA::LocationReference*>* _deferredRetLocs;

    // used for keeping track of which type members have default values
    std::set<std::string> _objDefaults;

    // used for assignments in constructors
    std::stack<std::pair<ISA::LocationReference*,Type::Object*>> _constructing;

    static const std::unordered_map<std::string, ISA::LocationReference*> _opaqueVals;
    static const std::unordered_map<std::string, ISA::LocationReference*> FuncToLocation;
    static std::unordered_map<std::string, ISA::Instructions*> FuncToFunc;
};

}

#endif
