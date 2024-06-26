#include "ToISAWalk.h"
#include "../DeferredLocationScope.h"
#include "../SymbolRemap.h"

namespace swarmc::Lang::Walk {
    ToISAWalk::ToISAWalk() : Walk<ISA::Instructions*>("AST->ISA Walk"), _deferredResults(new DeferredLocationScope(nullptr)), _symbolRemap(new SymbolRemapScope(nullptr)) {
        _locMap[ISA::Affinity::LOCAL] = std::map<std::string, ISA::LocationReference*>();
        _locMap[ISA::Affinity::SHARED] = std::map<std::string, ISA::LocationReference*>();
        _locMap[ISA::Affinity::FUNCTION] = std::map<std::string, ISA::LocationReference*>();
        _locMap[ISA::Affinity::OBJECTPROP] = std::map<std::string, ISA::LocationReference*>();
    }

    ToISAWalk::~ToISAWalk() {
        for ( auto aff : _locMap ) {
            for ( auto loc : aff.second ) freeref(loc.second);
        }
        for ( auto t : _typeMap ) {
            freeref(t.second);
        }
        delete _deferredResults;
        delete _symbolRemap;
    }

    ISA::Instructions* ToISAWalk::walkProgramNode(ProgramNode* node) {
        auto instrs = new ISA::Instructions();

        while ( node->body()->size() ) {
            _sharedLocs = SharedLocationsWalk::getLocs(node->body()->front());
            append(instrs, position(node->body()->front()));
            auto i = walk(node->body()->front());
            assert(_sharedLocs.size() == 0);
            append(instrs, i);
            logger->debug(
                s(node->body()->front()->position()) +
                " Finished: " + s(node->body()->front())
            );
            freeref(node->body()->front());
            node->body()->erase(node->body()->begin());
        }

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkExpressionStatementNode(ExpressionStatementNode* node) {
        return walk(node->expression());
    }

    ISA::Instructions* ToISAWalk::walkIdentifierNode(IdentifierNode* node) {
        // self assign
        auto instrs = position(node);

        auto affinity = node->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
        auto id = _symbolRemap->replace(node->symbol());
        if ( !id ) id = makeLocation(affinity, "var_" + node->name() + "_" + node->symbol()->uuid(), nullptr);
        SharedLocations::registerLoc(id, node->symbol());

        if ( node->symbol()->isPrologue() ) {
            auto fref = makeLocation(
                ISA::Affinity::FUNCTION,
                ((Lang::PrologueFunctionSymbol*)node->symbol())->sviName(),
                nullptr
            );
            append(instrs, assignValue(id, fref, false));
        } else if ( node->symbol()->isProperty() ) {
            // only case where variable is property but is raw identifier
            // is inside member function
            append(instrs, new ISA::ScopeOf(id));
            append(instrs, assignEval(id, new ISA::ObjGet(
                makeLocation(
                    ISA::Affinity::LOCAL,
                    TO_ISA_OBJECT_INSTANCE + s(scanConstructing(node->name())),
                    nullptr
                ),
                makeLocation(ISA::Affinity::OBJECTPROP, node->name(), nullptr)
            )));
        } else {
            // necessary for instructions that pull the location from the bottommost instruction
            append(instrs, assignValue(id, id, true));
        }

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkEnumerableAccessNode(EnumerableAccessNode* node) {
        auto instrs = position(node->path());
        append(instrs, walk(node->path()));
        auto enumeration = getLastLoc(instrs);
        append(instrs, position(node->index()));
        append(instrs, walk(node->index()));
        auto index = getLastLoc(instrs);

        append(instrs, position(node));
        append(instrs, assignEval(
            makeTmp(ISA::Affinity::LOCAL, instrs),
            new ISA::EnumGet(enumeration, index)
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkEnumerableAppendNode(EnumerableAppendNode* node) {
        auto instrs = position(node->path());
        append(instrs, walk(node->path()));
        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkMapAccessNode(MapAccessNode* node) {
        auto instrs = position(node->path());
        append(instrs, walk(node->path()));

        append(instrs, position(node));
        auto value = new ISA::MapGet(
            new ISA::StringReference(TO_ISA_MAP_KEY_PREFIX + node->end()->name()),
            getLastLoc(instrs)
        );

        append(instrs, assignEval(
            makeTmp(ISA::Affinity::LOCAL, instrs),
            value
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkClassAccessNode(ClassAccessNode* node) {
        auto instrs = position(node->path());
        append(instrs, walk(node->path()));

        auto path = getLastLoc(instrs);
        auto loc = makeTmp(ISA::Affinity::LOCAL, instrs);
        append(instrs, position(node));
        append(instrs, assignEval(loc, new ISA::ObjGet(
            path,
            makeLocation(ISA::Affinity::OBJECTPROP, node->end()->name(), nullptr)
        )));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkIncludeStatementNode(IncludeStatementNode* node) {
        // FIXME: actually implement this one day
        return new ISA::Instructions();
    }

    ISA::Instructions* ToISAWalk::walkTypeLiteral(swarmc::Lang::TypeLiteral* node) {
        auto instrs = position(node);

        auto ref = getTypeRef(node->value());
        append(instrs, assignValue(makeTmp(ISA::Affinity::LOCAL, instrs), ref, false));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
        auto instrs = position(node);

        append(instrs, assignValue(
            makeTmp(ISA::Affinity::LOCAL, _depth ? instrs : nullptr),
            new ISA::BooleanReference(node->value()),
            false
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
        auto instrs = position(node);

        append(instrs, assignValue(
            makeTmp(ISA::Affinity::LOCAL, _depth ? instrs : nullptr),
            new ISA::StringReference(node->value()),
            false
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
        auto instrs = position(node);

        append(instrs, assignValue(
            makeTmp(ISA::Affinity::LOCAL, _depth ? instrs : nullptr),
            new ISA::NumberReference(node->value()),
            false
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
        auto instrs = position(node);

        // enuminit
        auto innerType = ((Type::Enumerable*)node->type())->values();
        auto enumLoc = makeTmp(ISA::Affinity::LOCAL, instrs);
        append(instrs, assignEval(
            enumLoc,
            new ISA::EnumInit(getTypeRef(innerType))
        ));

        // contents
        for ( auto i : *node->actuals() ) {
            append(instrs, position(i));
            append(instrs, walk(i));
            append(instrs, new ISA::EnumAppend(getLastLoc(instrs), enumLoc));
        }

        // so that `getLastLoc` can be used
        append(instrs, assignValue(enumLoc, enumLoc, true));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkMapStatementNode(MapStatementNode* node) {
        auto instrs = position(node->value());
        append(instrs, walk(node->value()));
        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkMapNode(MapNode* node) {
        auto instrs = position(node);

        // create map
        auto innerType = ((Type::Map*)node->type())->values();
        auto mapLoc = makeTmp(ISA::Affinity::LOCAL, instrs);
        append(instrs, assignEval(mapLoc, new ISA::MapInit(getTypeRef(innerType))));

        // set values
        for ( auto stmt : *node->body()) {
            append(instrs, position(stmt));
            append(instrs, walk(stmt));
            auto val = getLastLoc(instrs);
            append(instrs, new ISA::MapSet(
                new ISA::StringReference(TO_ISA_MAP_KEY_PREFIX + stmt->id()->name()), val, mapLoc));
        }

        append(instrs, assignValue(makeTmp(ISA::Affinity::LOCAL, instrs), mapLoc, false));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkAssignExpressionNode(AssignExpressionNode* node) {
        auto instrs = position(node->value());
        append(instrs, walk(node->value()));
        auto value = getLastLoc(instrs);

        if ( node->dest()->getTag() == ASTNodeTag::IDENTIFIER ) {
            auto id = (IdentifierNode*)node->dest();
            auto affinity = id->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
            auto loc = _symbolRemap->replace(id->symbol());
            if ( !loc ) loc = makeLocation(
                affinity,
                TO_ISA_VARIABLE_PREFIX + id->name() + "_" + id->symbol()->uuid(),
                nullptr
            );
            SharedLocations::registerLoc(loc, id->symbol());
            append(instrs, position(node->dest()));

            if ( node->value()->getTag() == ASTNodeTag::DEFERCALL ) {
                auto jobid = getLastLoc(
                    instrs, 
                    (instrs->at(instrs->size() - 2)->tag() == ISA::Tag::SCOPEOF) ? 2 : 1
                );
                // loc is the variable the result will eventually be stored in, value is the context
                logger->debug(s(id->position()) + " Marking " + s(loc) + " as the return location of a deferred call");
                _deferredResults->add(loc, jobid, value);
            } else if ( id->symbol()->isProperty() && _constructing.size() > 0 ) {
                // the only case in which a raw identifier is an object property
                // is if we are inside a function defined in a type
                append(instrs, position(node));
                append(instrs, new ISA::ObjSet(
                    makeLocation(
                        ISA::Affinity::LOCAL,
                        TO_ISA_OBJECT_INSTANCE + s(scanConstructing(id->name())),
                        nullptr
                    ),
                    makeLocation(ISA::Affinity::OBJECTPROP, id->name(), nullptr),
                    value
                ));
            } else {
                append(instrs, assignValue(loc, value, false));
            }
        } else if ( node->dest()->getTag() == ASTNodeTag::ENUMERABLEACCESS ) {
            auto e = (EnumerableAccessNode*)node->dest();

            append(instrs, position(e->path()));
            append(instrs, walk(e->path()));
            auto path = getLastLoc(instrs);

            append(instrs, position(e->index()));
            append(instrs, walk(e->index()));
            auto index = getLastLoc(instrs);

            append(instrs, position(node));
            append(instrs, new ISA::EnumSet(path, index, value));
        } else if ( node->dest()->getTag() == ASTNodeTag::ENUMERABLEAPPEND ) {
            auto e = (EnumerableAppendNode*) node->dest();

            append(instrs, position(e));
            append(instrs, walk(e));
            auto enumeration = getLastLoc(instrs);

            append(instrs, position(node));
            append(instrs, new ISA::EnumAppend(value, enumeration));
        } else if ( node->dest()->getTag() == ASTNodeTag::MAPACCESS ) {
            auto m = (MapAccessNode*)node->dest();

            append(instrs, position(m->path()));
            append(instrs, walk(m->path()));
            auto path = getLastLoc(instrs);

            append(instrs, position(node));
            append(instrs, new ISA::MapSet(
                new ISA::StringReference(TO_ISA_MAP_KEY_PREFIX + m->end()->name()),
                value,
                path
            ));
        } else if ( node->dest()->getTag() == ASTNodeTag::CLASSACCESS ) {
            auto obj = (ClassAccessNode*)node->dest();

            append(instrs, position(obj->path()));
            append(instrs, walk(obj->path()));
            auto path = getLastLoc(instrs);

            append(instrs, position(node));
            append(instrs, new ISA::ObjSet(
                path,
                makeLocation(ISA::Affinity::OBJECTPROP, obj->end()->name(), nullptr),
                value
            ));
        }

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkVariableDeclarationNode(VariableDeclarationNode* node) {
        auto instrs = new ISA::Instructions();

        // explicit ScopeOf, since AssignmentExpressionNode wont do it
        if ( _depth > 0 ) {
            auto affinity = node->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
            append(instrs, new ISA::ScopeOf(
                makeLocation(affinity, "var_" + node->id()->name() + "_" + node->id()->symbol()->uuid(), nullptr)
            ));
        }

        append(instrs, walkAssignExpressionNode(node->assignment()));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) {
        throw Errors::SwarmError("Unresolved UninitVarDeclNode in ToISAWalk");
    }

    ISA::Instructions* ToISAWalk::walkUseNode(UseNode* node) {
        throw Errors::SwarmError("Unresolved UseNode in ToISAWalk");
    }

    ISA::Instructions* ToISAWalk::walkReturnStatementNode(ReturnStatementNode* node) {
        auto instrs = position(node);

        // set the variable that prevents execution of subfunctions
        if ( !_functionOuterScope ) {
            append(instrs, assignValue(
                makeLocation(ISA::Affinity::LOCAL, TO_ISA_FUNC_CONTROL_FLOW_BREAKER, nullptr),
                new ISA::BooleanReference(true),
                false
            ));
        }
        // add cfb for loops  if needed
        if ( _loopDepth > 0 ) {
            append(instrs, assignValue(
                makeLocation(ISA::Affinity::LOCAL, TO_ISA_LOOP_CONTROL_FLOW_BREAKER, nullptr),
                new ISA::BooleanReference(true),
                false
            ));
            append(instrs, assignValue(
                makeLocation(ISA::Affinity::LOCAL, TO_ISA_BREAK, nullptr),
                new ISA::BooleanReference(true),
                false
            ));
        }
        // set `retVal`
        if ( node->value() != nullptr ) {
            append(instrs, position(node->value()));
            append(instrs, walk(node->value()));
            append(instrs, assignValue(
                makeLocation(ISA::Affinity::LOCAL, TO_ISA_RETURN_LOCATION, nullptr),
                getLastLoc(instrs),
                false
            ));
        }

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkFunctionNode(FunctionNode* node) {
        // get return type
        Type::Type* retType = ((Type::Lambda*)node->type())->returns();
        auto offset = (node->type()->intrinsic() == Type::Intrinsic::LAMBDA0) ? 0 : 1;
        for ( auto i = 0; i < node->formals()->size() - offset; i++ ) {
            assert(retType->isCallable());
            retType = ((Type::Lambda*)retType)->returns();
        }

        auto name = TO_ISA_FUNCTION_PREFIX + s(_tempCounter++);
        auto tempLoop = _loopDepth;
        _loopDepth = 0;
        auto instrs = position(node);

        // modify type signature to maintain static scope
        auto formals = extractFormals(node->formals());
        if ( node->usedSymbols()->size() > 0 ) {
            for ( auto sy = node->usedSymbols()->rbegin(); sy != node->usedSymbols()->rend(); sy++ ) {
                auto sym = *sy;
                if ( node->varDecldTo() && node->varDecldTo()->symbol() == sym ) continue;
                ISAFormalList* nf = nullptr;
                if ( sym->isProperty() ) {
                    auto osym = (ObjectPropertySymbol*)sym;
                    nf = new ISAFormalList({
                        {
                            osym->partOf(),
                            ISA::Affinity::LOCAL,
                            TO_ISA_OBJECT_INSTANCE + s(osym->partOf()->getId()),
                            sym
                        }
                    });
                } else {
                    nf = new ISAFormalList({
                        {
                            sym->type(),
                            ISA::Affinity::LOCAL,
                            TO_ISA_VARIABLE_PREFIX + sym->name() + "_" + sym->uuid(),
                            sym
                        }
                    });
                }
                formals = mergeFormals(nf, formals);
            }
        }

        _symbolRemap = _symbolRemap->enter();
        if ( node->varDecldTo() ) { // statically evaluate fixpoint
            _symbolRemap->registerSymbol(
                node->varDecldTo()->symbol(), 
                makeLocation(ISA::Affinity::FUNCTION, name, nullptr)
            );
        }
        append(instrs, makeFunction(name, formals, retType, node, false, true, false));
        _symbolRemap = _symbolRemap->leave();
        _loopDepth = tempLoop;

        // curry usedSymbols here
        auto floc = makeLocation(ISA::Affinity::FUNCTION, name, nullptr);
        for ( auto sym : *node->usedSymbols() ) {
            if ( node->varDecldTo() && node->varDecldTo()->symbol() == sym ) continue;
            auto loc = sym->isProperty() 
                ? makeLocation(ISA::Affinity::LOCAL, TO_ISA_OBJECT_INSTANCE + s(scanConstructing(sym->name())), nullptr)
                : makeLocation(
                    sym->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL,
                    TO_ISA_VARIABLE_PREFIX + sym->name() + "_" + sym->uuid(),
                    nullptr
                );
            append(instrs, assignEval(
                makeTmp(ISA::Affinity::LOCAL, instrs),
                new ISA::Curry(floc, loc)
            ));
            floc = getLastLoc(instrs);
        }

        append(instrs, assignValue(
            makeTmp(ISA::Affinity::LOCAL, _depth ? instrs : nullptr),
            floc,
            false
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkConstructorNode(ConstructorNode* node) {
        auto instrs = position(node);
        auto type = _constructing.back().first;
        auto instLoc = makeLocation(ISA::Affinity::LOCAL, TO_ISA_OBJECT_INSTANCE + s(node->partOf()->getId()), nullptr);
        _depth++;
        _deferredResults = _deferredResults->enter();

        // function header
        append(instrs, new ISA::BeginFunction(node->name(), getTypeRef(type)));
        // modify type signature to maintain static scope
        auto formals = extractFormals(node->func()->formals());
        if ( node->func()->usedSymbols()->size() > 0 ) {
            for ( auto sy = node->func()->usedSymbols()->rbegin(); sy != node->func()->usedSymbols()->rend(); sy++ ) {
                auto sym = *sy;
                ISAFormalList* nf = nullptr;
                if ( sym->isProperty() ) {
                    auto osym = (ObjectPropertySymbol*)sym;
                    nf = new ISAFormalList({
                        {
                            osym->partOf(),
                            ISA::Affinity::LOCAL,
                            TO_ISA_OBJECT_INSTANCE + s(osym->partOf()->getId()),
                            sym
                        }
                    });
                } else {
                    nf = new ISAFormalList({
                        {
                            sym->type(),
                            ISA::Affinity::LOCAL,
                            TO_ISA_VARIABLE_PREFIX + sym->name() + "_" + sym->uuid(),
                            sym
                        }
                    });
                }
                formals = mergeFormals(nf, formals);
            }
        }

        for ( const auto& f : *formals ) {
            auto ploc = makeLocation(std::get<1>(f), std::get<2>(f), nullptr);
            SharedLocations::registerLoc(ploc, std::get<3>(f));
            append(instrs, new ISA::FunctionParam(getTypeRef(std::get<0>(f)), ploc));
        }
        delete formals;
        // add inst as final parameter
        append(instrs, new ISA::FunctionParam(getTypeRef(type), instLoc));

        // cfb
        for ( auto stmt : *node->func()->body() ) {
            if ( stmt->isBlock() && hasReturn()->walk(stmt).value_or(false) ) {
                auto cfb = makeLocation(ISA::Affinity::LOCAL, TO_ISA_FUNC_CONTROL_FLOW_BREAKER, instrs);
                append(instrs, assignValue(cfb, new ISA::BooleanReference(false), false));
                break;
            }
        }

        for ( auto parent : *node->parentConstructors() ) {
            append(instrs, position(parent));
            append(instrs, parentCall((CallExpressionNode*)parent));
        }

        // default values
        // FIXME: position annotations?
        for ( auto d : _constructing.back().second ) {
            auto defloc = d.second;
            // if function, curry object instance
            if ( _constructing.back().first->getProperty(d.first)->isCallable() ) {
                defloc = makeTmp(ISA::Affinity::LOCAL, instrs);
                append(instrs, assignEval(defloc, new ISA::Curry(d.second, instLoc)));
            }
            append(instrs, new ISA::ObjSet(
                instLoc,
                makeLocation(ISA::Affinity::OBJECTPROP, d.first, nullptr),
                defloc
            ));
        }

        append(instrs, walkStatementList(node->func(), false, true, false));
        append(instrs, new ISA::Return1(instLoc));

        auto floc = makeLocation(ISA::Affinity::FUNCTION, node->name(), nullptr);
        for ( auto sym : *node->func()->usedSymbols() ) {
            auto loc = sym->isProperty() 
                ? makeLocation(ISA::Affinity::LOCAL, TO_ISA_OBJECT_INSTANCE + s(scanConstructing(sym->name())), nullptr)
                : makeLocation(
                    sym->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL,
                    TO_ISA_VARIABLE_PREFIX + sym->name() + "_" + sym->uuid(),
                    nullptr
                );
            append(instrs, assignEval(
                makeTmp(ISA::Affinity::LOCAL, instrs),
                new ISA::Curry(floc, loc)
            ));
            floc = getLastLoc(instrs);
        }

        _constructorLoc.insert({ node->name(), floc });

        _depth--;
        _deferredResults = _deferredResults->leave();

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkTypeBodyNode(TypeBodyNode* node) {
        auto instrs = position(node);
        assert(node->value()->intrinsic() == Type::Intrinsic::OBJECT);
        auto objtype = (Type::Object*)node->value();

        // get default values of properties, to be assigned at the beginning of every constructor
        _constructing.push_back({ objtype, {} });
        for ( auto vd : *node->declarations() ) {
            if ( vd->getTag() == ASTNodeTag::VARIABLEDECLARATION ) {
                auto decl = (VariableDeclarationNode*)vd;
                ISA::LocationReference* vloc = nullptr;

                if ( decl->value()->getTag() == ASTNodeTag::FUNCTION ) {
                    auto func = (FunctionNode*)decl->value();
                    // prepend instance of type as parameter
                    auto defFormals = extractFormals(func->formals());

                    // get new return type
                    Type::Type* retType = ((Type::Lambda*)func->type())->returns();
                    auto offset = (func->type()->intrinsic() == Type::Intrinsic::LAMBDA0) ? 0 : 1;
                    for ( auto i = 0; i < defFormals->size() - offset; i++ ) {
                        assert(retType->isCallable());
                        retType = ((Type::Lambda*)retType)->returns();
                    }

                    auto formals = new ISAFormalList({ {
                        objtype,
                        ISA::Affinity::LOCAL,
                        TO_ISA_OBJECT_INSTANCE + s(objtype->getId()),
                        nullptr
                    } });
                    formals->insert(
                        formals->end(),
                        std::make_move_iterator(defFormals->begin()),
                        std::make_move_iterator(defFormals->end())
                    );
                    delete defFormals;

                    auto fnname = TO_ISA_FUNCTION_PREFIX + s(_tempCounter++);
                    append(instrs, makeFunction(
                        fnname, formals, retType,
                        func, false, true, false
                    ));
                    vloc = makeLocation(ISA::Affinity::FUNCTION, fnname, nullptr);
                } else {
                    append(instrs, walk(decl->assignment()));

                    // remove bad assign
                    auto instr = instrs->back();
                    instrs->pop_back();
                    freeref(instr);

                    vloc = getLastLoc(instrs);
                }

                // create default value location for
                auto defloc = makeLocation(
                    ISA::Affinity::LOCAL,
                    TO_ISA_OBJECT_DEFAULT_PREFIX + s(objtype->getId()) + "_" + decl->id()->name(),
                    instrs
                );
                append(instrs, assignValue(defloc, vloc, false));
                _constructing.back().second.insert({ decl->id()->name(), defloc });
            }
        }

        // walk constructors
        for ( auto c : *node->constructors() ) {
            append(instrs, walk(c));
        }
        _constructing.pop_back();

        // Ive already built the type in the compiler, no need to use the otype instructions
        append(instrs, assignValue(
            makeTmp(ISA::Affinity::LOCAL, instrs),
            getTypeRef(objtype),
            false
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkCallExpressionNode(CallExpressionNode* node) {
        // TODO: remove this once import-based prologue gets implemented
        // translates Swarm functions that have instruction equivalents in svi
        if ( node->func()->getTag() == ASTNodeTag::IDENTIFIER ) {
            if ( ToISAWalk::getInstructionAsFunc().count(((IdentifierNode*)node->func())->name()) ) {
                return callToInstruction(node);
            }
        }

        ISA::Instructions* instrs = position(node);
        ISA::LocationReference* func = nullptr;

        if ( node->constructor() ) {
            func = _constructorLoc[node->constructor()->name()];
        } else {
            append(instrs, walk(node->func()));
            func = getLastLoc(instrs);
        }

        // evaluate args
        auto temp = _parentCall;
        _parentCall = false;
        std::list<ISA::LocationReference*> arglocs;
        for ( auto arg : *node->args() ) {
            append(instrs, position(arg));
            if ( temp && arg->getTag() == ASTNodeTag::FUNCTION ) {
                auto func = (FunctionNode*)arg;
                // prepend instance of type as parameter
                auto defFormals = extractFormals(func->formals());

                // get new return type
                Type::Type* retType = ((Type::Lambda*)func->type())->returns();
                auto offset = (func->type()->intrinsic() == Type::Intrinsic::LAMBDA0) ? 0 : 1;
                for ( auto i = 0; i < defFormals->size() - offset; i++ ) {
                    assert(retType->isCallable());
                    retType = ((Type::Lambda*)retType)->returns();
                }

                auto formals = new ISAFormalList({ {
                    _constructing.back().first,
                    ISA::Affinity::LOCAL,
                    TO_ISA_OBJECT_INSTANCE + s(_constructing.back().first->getId()),
                    nullptr
                } });
                formals->insert(
                    formals->end(),
                    std::make_move_iterator(defFormals->begin()),
                    std::make_move_iterator(defFormals->end())
                );
                delete defFormals;

                auto fnname = TO_ISA_FUNCTION_PREFIX + s(_tempCounter++);
                append(instrs, makeFunction(
                    fnname, formals, retType,
                    func, false, true, false
                ));
                append(instrs, assignEval(
                    makeTmp(ISA::Affinity::LOCAL, instrs),
                    new ISA::Curry(
                        makeLocation(ISA::Affinity::FUNCTION, fnname, nullptr),
                        makeLocation(ISA::Affinity::LOCAL, TO_ISA_OBJECT_INSTANCE + s(_constructing.back().first->getId()), nullptr)
                    )
                ));
            } else {
                append(instrs, walk(arg));
            }
            arglocs.push_back(getLastLoc(instrs));
        }

        auto initLoc = temp
            ? makeLocation(ISA::Affinity::LOCAL, TO_ISA_OBJECT_INSTANCE + s(_constructing.back().first->getId()), nullptr) 
            : makeTmp(ISA::Affinity::LOCAL, instrs);
        if ( node->constructor() ) {
            arglocs.push_back(initLoc);
        }
        append(instrs, position(node));

        // curry
        while ( arglocs.size() > 1 ) {
            auto curried = makeTmp(ISA::Affinity::LOCAL, instrs);
            append(instrs, assignEval(curried, new ISA::Curry(func, arglocs.front())));
            func = curried;
            arglocs.pop_front();
        }
        _parentCall = temp;

        // call with remaining parameter if exists
        ISA::Instruction* call = nullptr;
        if ( arglocs.size() == 0 ) {
            call = new ISA::Call0(func);
        } else {
            call = new ISA::Call1(func, arglocs.front());
        }

        if ( !node->constructor() && node->type()->intrinsic() == Type::Intrinsic::VOID ) {
            append(instrs, call);
            return instrs;
        }

        if ( node->constructor() ) {
            if ( _parentCall ) {
                append(instrs, assignEval(initLoc, call));
                return instrs;
            }

            append(instrs, assignEval(initLoc, new ISA::ObjInit(getTypeRef(node->constructor()->partOf()))));
            append(instrs, assignEval(initLoc, call));
            append(instrs, assignEval(initLoc, new ISA::ObjInstance(initLoc)));
            return instrs;
        }

        append(instrs, assignEval(initLoc, call));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkDeferCallExpressionNode(DeferCallExpressionNode* node) {
        auto instrs = position(node->call());
        append(instrs, walk(node->call()));

        // TODO: remove when import based prologue exists
        if ( node->call()->func()->getTag() == ASTNodeTag::IDENTIFIER ) {
            std::string name = ((IdentifierNode*)node->call()->func())->name();
            if ( ToISAWalk::getInstructionAsFunc().count(name) ) {
                auto retType = ToISAWalk::getInstructionAsFunc().at(name);
                auto call = instrs->back();
                instrs->pop_back();
                auto funcName = TO_ISA_FUNCTION_PREFIX + s(_tempCounter++);
                append(instrs, new ISA::BeginFunction(funcName, getTypeRef(retType)));
                append(instrs, call);
                auto funcLoc = makeLocation(ISA::Affinity::FUNCTION, funcName, nullptr);
                if ( retType->intrinsic() == Type::Intrinsic::VOID ) {
                    append(instrs, new ISA::Return0());
                    append(instrs, new ISA::Call0(funcLoc));
                } else {
                    append(instrs, new ISA::Return1(getLastLoc(instrs)));
                    append(instrs, assignEval(
                        makeTmp(ISA::Affinity::LOCAL, instrs),
                        new ISA::Call0(funcLoc)
                    ));
                }

            }
        }

        // remove call so we can replace with a pushcall
        auto last = instrs->back();
        GC_LOCAL_REF(last)
        instrs->pop_back();

        // get the call
        bool returnsValue = false;
        if ( last->tag() == ISA::Tag::ASSIGNEVAL ) {
           last = ((ISA::AssignEval*)last)->second();
           returnsValue = true;
        }
        append(instrs, position(node));

        // enter new context
        append(instrs, new ISA::EnterContext());

        // pushcall
        ISA::Instruction* pushcall = nullptr;
        if ( last->tag() == ISA::Tag::CALL0 ) {
            auto call = (ISA::Call0*)last;
            pushcall = new ISA::PushCall0( call->first() );
        } else if ( last->tag() == ISA::Tag::CALL1 ) {
            auto call = (ISA::Call1*)last;
            pushcall = new ISA::PushCall1(call->first(), call->second());
        }

        if ( returnsValue ) {
            append(instrs, assignEval(
                makeTmp(ISA::Affinity::LOCAL, instrs),
                pushcall
            ));
            append(instrs, assignEval(
                makeTmp(ISA::Affinity::LOCAL, instrs),
                new ISA::PopContext()
            ));
        } else {
            append(instrs, pushcall);
            append(instrs, new ISA::PopContext());
        }


        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkAndNode(AndNode* node) {
        return walkBinaryExpressionNode(node);
    }

    ISA::Instructions* ToISAWalk::walkOrNode(OrNode* node) {
        return walkBinaryExpressionNode(node);
    }

    ISA::Instructions* ToISAWalk::walkEqualsNode(EqualsNode* node) {
        return walkBinaryExpressionNode(node);
    }

    ISA::Instructions* ToISAWalk::walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
        return walkBinaryExpressionNode(node);
    }

    ISA::Instructions* ToISAWalk::walkNotEqualsNode(NotEqualsNode* node) {
        auto instrs = walkBinaryExpressionNode(node);
        auto eq = getLastLoc(instrs);

        append(instrs, assignEval(
            makeTmp(ISA::Affinity::LOCAL, instrs),
            new ISA::Not(eq)
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkAddNode(AddNode* node) {
        return walkBinaryExpressionNode(node);
    }

    ISA::Instructions* ToISAWalk::walkSubtractNode(SubtractNode* node) {
        return walkBinaryExpressionNode(node);
    }

    ISA::Instructions* ToISAWalk::walkMultiplyNode(MultiplyNode* node) {
        return walkBinaryExpressionNode(node);
    }

    ISA::Instructions* ToISAWalk::walkDivideNode(DivideNode* node) {
        return walkBinaryExpressionNode(node);
    }

    ISA::Instructions* ToISAWalk::walkModulusNode(ModulusNode* node) {
        return walkBinaryExpressionNode(node);
    }

    ISA::Instructions* ToISAWalk::walkPowerNode(PowerNode* node) {
        return walkBinaryExpressionNode(node);
    }

    ISA::Instructions* ToISAWalk::walkNthRootNode(NthRootNode* node) {
        auto instrs = position(node->left());
        append(instrs, walk(node->left()));
        auto n = getLastLoc(instrs);
        
        append(instrs, position(node->right()));
        append(instrs, walk(node->right()));
        auto exp = getLastLoc(instrs);

        auto curried = makeTmp(ISA::Affinity::LOCAL, instrs);
        append(instrs, position(node));
        append(instrs, assignEval(
            curried,
            new ISA::Curry(makeLocation(ISA::Affinity::FUNCTION, "NTH_ROOT", nullptr), n)
        ));
        append(instrs, assignEval(
            makeTmp(ISA::Affinity::LOCAL, instrs),
            new ISA::Call1(curried, exp)
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkNegativeExpressionNode(NegativeExpressionNode* node) {
        return walkUnaryExpressionNode(node);
    }

    ISA::Instructions* ToISAWalk::walkNotNode(NotNode* node) {
        return walkUnaryExpressionNode(node);
    }

    ISA::Instructions* ToISAWalk::walkEnumerationConcatNode(EnumerationConcatNode* node) {
        return walkBinaryExpressionNode(node);
    }

    ISA::Instructions* ToISAWalk::walkEnumerationStatement(EnumerationStatement* node) {
        auto instrs = position(node->enumerable());
        append(instrs, walk(node->enumerable()));
        auto enumLoc = getLastLoc(instrs);

        // create formals to pass to makeFunction
        auto name = "ENUM_" + s(_tempCounter++);
        ISAFormalList* formals = new ISAFormalList({
            {
                node->local()->type(),
                node->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL,
                TO_ISA_VARIABLE_PREFIX + node->local()->name() + "_" + node->local()->symbol()->uuid(),
                node->local()->symbol()
            },
            {
                Type::Primitive::of(Type::Intrinsic::NUMBER),
                (node->index() != nullptr && node->index()->shared())
                    ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL,
                (node->index() == nullptr) ? "index" : TO_ISA_VARIABLE_PREFIX + node->index()->name() + "_" + node->index()->symbol()->uuid(),
                (node->index() == nullptr) ? nullptr : node->index()->symbol()
            }
        });
        // make function
        auto tempLoop = _loopDepth;
        _loopDepth = 0;
        auto func = makeFunction(
            name,
            formals,
            Type::Primitive::of(Type::Intrinsic::VOID),
            node, false, false, false
        );
        _loopDepth = tempLoop;
        append(instrs, func);
        append(instrs, position(node));

        append(instrs, new ISA::Enumerate(
            getTypeRef(node->local()->type()),
            enumLoc,
            makeLocation(ISA::Affinity::FUNCTION, name, nullptr)
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkWithStatement(WithStatement* node) {
        auto instrs = position(node->resource());
        append(instrs, walk(node->resource()));
        auto resLoc = getLastLoc(instrs);

        ISAFormalList* formals = new ISAFormalList({
            {
                node->resource()->type(),
                node->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL,
                TO_ISA_VARIABLE_PREFIX + node->local()->name() + "_" + node->local()->symbol()->uuid(),
                node->local()->symbol()
            }
        });

        auto name = TO_ISA_WITH_PREFIX + s(_tempCounter++);
        append(instrs, makeFunction(
            name,
            formals,
            Type::Primitive::of(Type::Intrinsic::VOID),
            node, false, false, true
        ));

        append(instrs, position(node));
        append(instrs, new ISA::With(
            resLoc,
            makeLocation(ISA::Affinity::FUNCTION, name, nullptr)
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkIfStatement(IfStatement* node) {
        auto instrs = position(node->condition());
        append(instrs, walk(node->condition()));
        auto conditionLoc = getLastLoc(instrs);

        // create function
        auto name = "IF_" + s(_tempCounter++);
        auto func = makeFunction(
            name, nullptr,
            Type::Primitive::of(Type::Intrinsic::VOID),
            node, false, false, false
        );
        append(instrs, func);
        append(instrs, position(node));

        append(instrs, new ISA::CallIf0(
            conditionLoc,
            makeLocation(ISA::Affinity::FUNCTION, name, nullptr)
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkWhileStatement(WhileStatement* node) {
        // Create condition function
        auto tc = s(_tempCounter++);
        auto instrs = position(node);
        // Create while condition. If there is a break or return, we have to do it goofy
        bool hb = false;
        if ( (hb = hasBreak()->walkStatementList(node).value_or(false)) || hasReturn()->walkStatementList(node).value_or(false) ) {
            auto condLoc = makeLocation(ISA::Affinity::LOCAL, TO_ISA_WHILE_COND_LOCATION, nullptr);
            // to make sure we check the "break" flag first, we actually
            // have to wrap the entire condition function in *another* function
            // which only gets called if break flag is false

            auto brk = makeLocation(ISA::Affinity::LOCAL, TO_ISA_BREAK, instrs);
            append(instrs, assignValue(brk, new ISA::BooleanReference(false), false));

            // inner function (actual condition)
            append(instrs, new ISA::BeginFunction(
                TO_ISA_WHILE_COND_INNER_PREFIX + tc,
                getTypeRef(Type::Primitive::of(Type::Intrinsic::VOID))
            ));
            append(instrs, position(node->condition()));
            append(instrs, walk(node->condition()));
            append(instrs, assignValue(condLoc, getLastLoc(instrs), false));
            append(instrs, new ISA::Return0());
            auto condInnerFunc = makeLocation(ISA::Affinity::FUNCTION, TO_ISA_WHILE_COND_INNER_PREFIX + tc, nullptr);

            // outer function (checks "break" flag)
            append(instrs, new ISA::BeginFunction(
                TO_ISA_WHILE_COND_OUTER_PREFIX + tc,
                getTypeRef(Type::Primitive::of(Type::Intrinsic::BOOLEAN))
            ));
            append(instrs, new ISA::ScopeOf(condLoc));
            append(instrs, new ISA::CallElse0(brk, condInnerFunc));
            append(instrs, new ISA::Return1(condLoc));
        } else {
            // no break/return means no inner function necessary
            append(instrs, new ISA::BeginFunction(
                TO_ISA_WHILE_COND_OUTER_PREFIX + tc,
                getTypeRef(Type::Primitive::of(Type::Intrinsic::BOOLEAN))
            ));
            append(instrs, position(node->condition()));
            append(instrs, walk(node->condition()));
            append(instrs, new ISA::Return1(getLastLoc(instrs)));
        }

        // while function
        _loopDepth++;
        append(instrs, makeFunction(
            TO_ISA_WHILE_PREFIX + tc, nullptr,
            Type::Primitive::of(Type::Intrinsic::VOID),
            node, true, false, false
        ));
        _loopDepth--;
        append(instrs, position(node));
        append(instrs, new ISA::While(
            makeLocation(ISA::Affinity::FUNCTION, TO_ISA_WHILE_COND_OUTER_PREFIX + tc, nullptr),
            makeLocation(ISA::Affinity::FUNCTION, TO_ISA_WHILE_PREFIX + tc, nullptr)
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkContinueNode(ContinueNode* node) {
        auto instrs = position(node);
        append(instrs, assignValue(
            makeLocation(ISA::Affinity::LOCAL, TO_ISA_LOOP_CONTROL_FLOW_BREAKER, nullptr),
            new ISA::BooleanReference(true),
            false
        ));
        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkBreakNode(BreakNode* node) {
        auto instrs = position(node);
        append(instrs, assignValue(
            makeLocation(ISA::Affinity::LOCAL, TO_ISA_BREAK, nullptr),
            new ISA::BooleanReference(true),
            false
        ));
        append(instrs, assignValue(
            makeLocation(ISA::Affinity::LOCAL, TO_ISA_LOOP_CONTROL_FLOW_BREAKER, nullptr),
            new ISA::BooleanReference(true),
            false
        ));
        return instrs;
    }


    ISA::Instructions* ToISAWalk::walkUnaryExpressionNode(UnaryExpressionNode* node) {
        auto instrs = position(node->exp());
        append(instrs, walk(node->exp()));
        auto exp = getLastLoc(instrs);

        ISA::Instruction* instr = nullptr;
        if ( node->getTag() == ASTNodeTag::NOT ) instr = new ISA::Not(exp);
        else if ( node->getTag() == ASTNodeTag::NEGATIVE ) instr = new ISA::Negative(exp);

        append(instrs, position(node));
        append(instrs, assignEval(
            makeTmp(ISA::Affinity::LOCAL, instrs),
            instr
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkBinaryExpressionNode(BinaryExpressionNode* node) {
        auto instrs = position(node->left());
        append(instrs, walk(node->left()));
        auto right = walk(node->right());

        auto leftLoc = getLastLoc(instrs);
        auto rightLoc = getLastLoc(right);
        append(instrs, position(node->right()));
        append(instrs, right);

        ISA::Instruction* instr = nullptr;
        if ( node->getTag() == ASTNodeTag::AND ) instr = new ISA::And(leftLoc, rightLoc);
        else if ( node->getTag() == ASTNodeTag::OR ) instr = new ISA::Or(leftLoc, rightLoc);
        else if ( node->getTag() == ASTNodeTag::ADD ) {
            if ( ((AddNode*)node)->concatenation() ) instr = new ISA::StringConcat(leftLoc, rightLoc);
            else instr = new ISA::Plus(leftLoc, rightLoc);
        }
        else if ( node->getTag() == ASTNodeTag::SUBTRACT ) instr = new ISA::Minus(leftLoc, rightLoc);
        else if ( node->getTag() == ASTNodeTag::MULTIPLY ) instr = new ISA::Times(leftLoc, rightLoc);
        else if ( node->getTag() == ASTNodeTag::DIVIDE ) instr = new ISA::Divide(leftLoc, rightLoc);
        else if ( node->getTag() == ASTNodeTag::MODULUS ) instr = new ISA::Mod(leftLoc, rightLoc);
        else if ( node->getTag() == ASTNodeTag::POWER ) instr = new ISA::Power(leftLoc, rightLoc);
        else if ( node->getTag() == ASTNodeTag::EQUALS
                || node->getTag() == ASTNodeTag::NOTEQUALS ) instr = new ISA::IsEqual(leftLoc, rightLoc);
        else if ( node->getTag() == ASTNodeTag::NUMERICCOMPARISON ) {
            auto ctype = ((NumericComparisonExpressionNode*)node)->comparisonType();
            if ( ctype == NumberComparisonType::GREATER_THAN ) instr = new ISA::GreaterThan(leftLoc, rightLoc);
            else if ( ctype == NumberComparisonType::GREATER_THAN_OR_EQUAL ) instr = new ISA::GreaterThanOrEqual(leftLoc, rightLoc);
            else if ( ctype == NumberComparisonType::LESS_THAN ) instr = new ISA::LessThan(leftLoc, rightLoc);
            else instr = new ISA::LessThanOrEqual(leftLoc, rightLoc);
        } else if ( node->getTag() == ASTNodeTag::ENUMERABLECONCAT ) instr = new ISA::EnumConcat(leftLoc, rightLoc);

        append(instrs, position(node));
        append(instrs, assignEval(
            makeTmp(ISA::Affinity::LOCAL, instrs),
            instr
        ));

        return instrs;
    }

    ISA::Instructions* ToISAWalk::walkStatementList(StatementListWrapper* block, bool loop, bool newScope, bool with) {
        auto instrs = new ISA::Instructions();

        // we only need to create subfunctions for control flow diversions
        // if said diversion contains a relevant return, continue, or break
        std::size_t i = 0;
        for ( ; i < block->body()->size(); i++ ) {
            auto instr = block->body()->at(i);
            // prevent setting cfb if return occurs in function outermost scope
            if ( newScope && instr->getTag() == ASTNodeTag::RETURN ) _functionOuterScope = true;
            _sharedLocs = SharedLocationsWalk::getLocs(block->body()->at(i));
            auto stmtISA = walk(instr);
            assert(_sharedLocs.size() == 0);
            _functionOuterScope = false;
            append(instrs, position(instr));
            append(instrs, stmtISA);
            if ( (loop && hasContinue()->combine(*hasBreak(), ASTMapReduce<bool>::CombineSkipType::FIRST)
                .walk(instr).value_or(false))
                || hasReturn()->walk(instr).value_or(false) )
            {
                i++;
                break;
            }
        }

        //subfunc
        if ( i != block->body()->size() ) {
            std::string name = TO_ISA_SUBFUNCTION_PREFIX + s(_tempCounter++);
            auto remainder = new StatementListWrapper(block->body()->begin() + i, block->body()->end());
            auto func = makeFunction(
                name,
                nullptr,
                Type::Primitive::of(Type::Intrinsic::VOID),
                remainder,
                loop, false, with
            );
            delete remainder;
            append(instrs, func);
            std::string cname = loop ? TO_ISA_LOOP_CONTROL_FLOW_BREAKER : TO_ISA_FUNC_CONTROL_FLOW_BREAKER;
            append(instrs, new ISA::CallElse0(
                makeLocation(ISA::Affinity::LOCAL, cname, nullptr),
                makeLocation(ISA::Affinity::FUNCTION, name, nullptr)
            ));
        }

        return instrs;
    }

    ISA::LocationReference* ToISAWalk::getLastLoc(ISA::Instructions* instrs, std::size_t offset) const {
        assert(instrs->size() > 0 + offset);
        auto instr = instrs->at(instrs->size() - (1 + offset));
        assert(instr->tag() == ISA::Tag::ASSIGNVALUE || instr->tag() == ISA::Tag::ASSIGNEVAL);
        if (instr->tag() == ISA::Tag::ASSIGNEVAL) {
            return ((ISA::AssignEval*)instr)->first();
        }
        return ((ISA::AssignValue*)instr)->first();
    }

    ISA::LocationReference* ToISAWalk::makeTmp(ISA::Affinity affinity, ISA::Instructions* instrs) {
        return makeLocation(affinity, TO_ISA_TMP_PREFIX + s(_tempCounter++), instrs);
    }

    ISA::LocationReference* ToISAWalk::makeLocation(ISA::Affinity affinity, std::string name, ISA::Instructions* instrs) {
        if ( _locMap[affinity].count(name) == 0 ) {
            _locMap[affinity][name] = useref(new ISA::LocationReference(affinity, name));
        }
        ISA::LocationReference* loc = _locMap[affinity][name];
        if ( _depth > 0 && instrs != nullptr ) {
            instrs->push_back(useref(new ISA::ScopeOf(loc)));
            _deferredResults->remove(loc);
        }
        return loc;
    }

    ISA::Instructions* ToISAWalk::makeFunction(std::string name, ISAFormalList* formals, Type::Type* retType, StatementListWrapper* node, bool loop, bool newScope, bool with) {
        auto instrs = new ISA::Instructions();
        ISA::Instruction* ret;
        if (!with) _deferredResults = _deferredResults->enter();
        _depth++;

        // beginfn
        append(instrs, new ISA::BeginFunction(name, getTypeRef(retType)));

        // fnparams
        if ( formals != nullptr ) {
            for ( auto i = 0; i < formals->size(); i ++ ) {
                auto formal = formals->at(i);
                auto loc = makeLocation(std::get<1>(formal), std::get<2>(formal), nullptr);
                SharedLocations::registerLoc(loc, std::get<3>(formal));
                _symbolRemap->registerSymbol(std::get<3>(formal), loc);
                append(instrs, new ISA::FunctionParam(getTypeRef(std::get<0>(formal)), loc));
                _deferredResults->remove(loc);
            }
        }
        delete formals;

        if ( retType->intrinsic() != Type::Intrinsic::VOID ) {
            append(instrs, new ISA::ScopeOf(
                makeLocation(ISA::Affinity::LOCAL, TO_ISA_RETURN_LOCATION, nullptr)
            ));
        }

        // check whether CFB will be used
        if ( loop ) {
            if ( hasContinue()->combine(*hasBreak(), ASTMapReduce<bool>::CombineSkipType::FIRST)
                .combine(*hasReturn(), ASTMapReduce<bool>::CombineSkipType::SECOND)
                .walkStatementList(node).value_or(false) )
            {
                auto cfb = makeLocation(ISA::Affinity::LOCAL, TO_ISA_LOOP_CONTROL_FLOW_BREAKER, instrs);
                append(instrs, assignValue(cfb, new ISA::BooleanReference(false), false));
            }
        } else if ( newScope ) {
            // only need to add cfb if a return appears in a lower scope
            for ( auto stmt : *node->body() ) {
                if ( stmt->isBlock() && hasReturn()->walk(stmt).value_or(false) ) {
                    auto cfb = makeLocation(ISA::Affinity::LOCAL, TO_ISA_FUNC_CONTROL_FLOW_BREAKER, instrs);
                    append(instrs, assignValue(cfb, new ISA::BooleanReference(false), false));
                    break;
                }
            }
        }

        // body
        auto block = walkStatementList(node, loop, newScope, with);
        append(instrs, block);

        //return
        if ( retType->intrinsic() == Type::Intrinsic::VOID ) {
            ret = new ISA::Return0();
        } else {
            auto retLoc = makeLocation(ISA::Affinity::LOCAL, TO_ISA_RETURN_LOCATION, nullptr);
            ret = new ISA::Return1(retLoc);
        }
        append(instrs, ret);
        _depth--;
        if ( !with ) _deferredResults = _deferredResults->leave();

        return instrs;
    }

    ISA::Instructions* ToISAWalk::callToInstruction(CallExpressionNode* node) {
        assert(node->func()->getTag() == ASTNodeTag::IDENTIFIER);
        std::string name = ((IdentifierNode*)node->func())->name();
        if ( ToISAWalk::getInstructionAsFunc().count(name) == 0 ) return nullptr;
        auto instrs = new ISA::Instructions();
        ISA::Instruction* instr = nullptr;
        if ( name == "lLog" ) {
            append(instrs, walk(node->args()->at(0)));
            instr = new ISA::StreamPush(
                makeLocation(ISA::Affinity::LOCAL, "STDOUT", nullptr),
                getLastLoc(instrs)
            );
        }
        else if ( name == "sLog" ) {
            append(instrs, walk(node->args()->at(0)));
            instr = new ISA::StreamPush(
                makeLocation(ISA::Affinity::SHARED, "STDOUT", nullptr),
                getLastLoc(instrs)
            );
        }
        else if ( name == "lErr" ) {
            append(instrs, walk(node->args()->at(0)));
            instr = new ISA::StreamPush(
                makeLocation(ISA::Affinity::LOCAL, "STDERR", nullptr),
                getLastLoc(instrs)
            );
        }
        else if ( name == "sErr" ) {
            append(instrs, walk(node->args()->at(0)));
            instr = new ISA::StreamPush(
                makeLocation(ISA::Affinity::SHARED, "STDERR", nullptr),
                getLastLoc(instrs)
            );
        }
        // do assign if ret type not void
        append(instrs, position(node));
        if ( ToISAWalk::getInstructionAsFunc().at(name)->intrinsic() != Type::Intrinsic::VOID ) {
            append(instrs, assignEval(
                makeTmp(ISA::Affinity::LOCAL, instrs),
                instr
            ));
        } else {
            append(instrs, instr);
        }

        return instrs;
    }

    ISA::Instructions* ToISAWalk::parentCall(CallExpressionNode* call) {
        _parentCall = true;
        auto i = walk(call);
        _parentCall = false;
        return i;
    }

    ISAFormalList* ToISAWalk::extractFormals(FormalList* formals) const {
        auto isaFormals = new ISAFormalList();
        for ( auto p : *formals ) {
            isaFormals->push_back({
                p.first->value(),
                p.second->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL,
                TO_ISA_VARIABLE_PREFIX + p.second->name() + "_" + p.second->symbol()->uuid(),
                p.second->symbol()
            });
        }
        return isaFormals;
    }

    ISAFormalList* ToISAWalk::mergeFormals(ISAFormalList* first, ISAFormalList* second) const {
        first->insert(
            first->end(),
            std::make_move_iterator(second->begin()),
            std::make_move_iterator(second->end())
        );
        delete second;
        return first;
    }

    ISA::TypeReference* ToISAWalk::getTypeRef(Type::Type* type, Type::Type* comp) {
        auto tp = thisify(type, comp);
        std::size_t id = tp->getId();
        if ( !_typeMap.count(id) ) {
            ISA::TypeReference* ref = nullptr;
            if ( type->intrinsic() == Type::Intrinsic::OBJECT ) {
                ref = new ISA::ObjectTypeReference((Type::Object*)type);
            } else {
                ref = new ISA::TypeReference(type);
            }
            _typeMap.insert({ id, useref(ref) });
        }
        return _typeMap[id];
    }

    Type::Type* ToISAWalk::thisify(Type::Type* type, Type::Type* comp) const {
        if (comp == nullptr) return type;
        if (type->isAssignableTo(comp)) return Type::Primitive::of(Type::Intrinsic::THIS);
        auto t = type->copy();
        t->transform([comp](Type::Type* t) -> Type::Type* {
            if (t->isAssignableTo(comp)) return Type::Primitive::of(Type::Intrinsic::THIS);
            return t;
        });
        return t;
    }

    std::size_t ToISAWalk::scanConstructing(std::string name) const {
        for ( auto i = _constructing.rbegin(); i != _constructing.rend(); i++ ) {
            if ( (*i).first->getProperty(name) != nullptr ) return (*i).first->getId();
        }
        throw Errors::SwarmError("Unable to find an instance of type that has property: " + name);
    }

    void ToISAWalk::append(ISA::Instructions* first, ISA::Instructions* second) const {
        first->insert(
            first->end(),
            std::make_move_iterator(second->begin()),
            std::make_move_iterator(second->end())
        );
        delete second;
    }

    void ToISAWalk::append(ISA::Instructions* instrs, ISA::Instruction* instr) {
        ISA::DeferrableLocations dl = _combLocations.walkOne(instr);
        auto isl = _sharedLocsWalkISA.walkOne(instr);

        for ( auto loc : dl ) {
            if ( _deferredResults->contains(loc) ) {
                // determine if we need to lock prematurely for the retmapget
                bool lock = _sharedLocs.has(loc) && !_sharedLocs.locked(loc);

                auto jobdata = _deferredResults->drain(loc);
                auto retMap = makeTmp(ISA::Affinity::LOCAL, instrs);
                append(instrs, new ISA::ResumeContext(jobdata.second));
                append(instrs, assignEval(retMap, new ISA::Drain()));
                if ( lock ) {
                    instrs->push_back(useref(new ISA::Lock(loc)));
                    _sharedLocs.dec(loc);
                }
                append(instrs, assignEval(loc, new ISA::RetMapGet(retMap, jobdata.first)));
                append(instrs, new ISA::PopContext());
            }
        }

        for ( auto i : isl ) { // lock all unlocked shared locs that appear in this instr
            if ( _sharedLocs.has(i) ) {
                if ( !_sharedLocs.locked(i) ) instrs->push_back(useref(new ISA::Lock(i)));
                _sharedLocs.dec(i);
            }
        }
        instrs->push_back(useref(instr));

        ISA::LocationReference* ll = nullptr;
        if ( instrs->back()->tag() == ISA::Tag::ASSIGNEVAL || instrs->back()->tag() == ISA::Tag::ASSIGNVALUE ) {
            ll = getLastLoc(instrs);
        }

        for ( auto i : isl ) { // unlock all locked shared locs whose last appearance was this instr
            if ( _sharedLocs.has(i) && _sharedLocs.shouldUnlock(i) ) {
                instrs->push_back(useref(new ISA::Unlock(i)));
                _sharedLocs.remove(i);
            }
        }

        // because of getLastLoc
        if ( ll != nullptr && instrs->back()->tag() == ISA::Tag::UNLOCK ) {
            append(instrs, assignValue(ll, ll, true));
        }
    }

    ISA::Instructions* ToISAWalk::position(ASTNode* node) const {
#ifdef SWARM_DEBUG
        return new ISA::Instructions({ useref(new ISA::PositionAnnotation(
            new ISA::StringReference(node->position()->file()),
            new ISA::NumberReference(node->position()->startLine()),
            new ISA::NumberReference(node->position()->startCol())
        )) });
#else
        return new ISA::Instructions();
#endif
    }

    ISA::Instructions* ToISAWalk::assignEval(ISA::LocationReference* dest, ISA::Instruction* instr) {
        auto instrs = new ISA::Instructions();

        append(instrs, new ISA::AssignEval(dest, instr));
        _deferredResults->remove(dest);

        return instrs;
    }

    ISA::Instructions* ToISAWalk::assignValue(ISA::LocationReference* dest, ISA::Reference* val, bool selfassign) {
        auto instrs = new ISA::Instructions();

        if ( selfassign ) {
            // would hate to defer and then immediately wait when doing one of those obligatory self assigns
            instrs->push_back(useref(new ISA::AssignValue(dest, val)));
        } else {
            append(instrs, new ISA::AssignValue(dest, val));
            _deferredResults->remove(dest);
        }

        return instrs;
    }

    const std::map<std::string, Type::Type*>& ToISAWalk::getInstructionAsFunc() {
        static const std::map<std::string, Type::Type*> instructionAsFunc = {
            { "lLog", Type::Primitive::of(Type::Intrinsic::VOID) },
            { "lError", Type::Primitive::of(Type::Intrinsic::VOID) },
            { "sLog", Type::Primitive::of(Type::Intrinsic::VOID) },
            { "sError", Type::Primitive::of(Type::Intrinsic::VOID) },
        };

        return instructionAsFunc;
    }
}
