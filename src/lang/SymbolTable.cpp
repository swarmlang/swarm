#include "SymbolTable.h"

namespace swarmc::Lang {

    ScopeTable* ScopeTable::prologue() {
        auto prologueScope = new swarmc::Lang::ScopeTable();

        // numberToString :: number -> string
        auto typeNumToStr = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            Type::Primitive::of(Type::Intrinsic::STRING)
        );
        auto numberToString = new PrologueFunctionSymbol("numberToString", typeNumToStr, new ProloguePosition("numberToString"));
        prologueScope->insert(numberToString);

        // booleanToString :: boolean -> string
        auto typeBoolToStr = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::BOOLEAN),
            Type::Primitive::of(Type::Intrinsic::STRING)
        );
        auto booleanToString = new PrologueFunctionSymbol("booleanToString", typeBoolToStr, new ProloguePosition("booleanToString"));
        prologueScope->insert(booleanToString);

        // sin :: number -> number
        auto typeNumToNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            Type::Primitive::of(Type::Intrinsic::STRING)
        );
        auto sin = new PrologueFunctionSymbol("sin", typeNumToNum, new ProloguePosition("sin"));
        prologueScope->insert(sin);

        // cos :: number -> number
        auto cos = new PrologueFunctionSymbol("cos", typeNumToNum, new ProloguePosition("cos"));
        prologueScope->insert(cos);

        // tan :: number -> number
        auto tan = new PrologueFunctionSymbol("tan", typeNumToNum, new ProloguePosition("tan"));
        prologueScope->insert(tan);

        // random :: -> number
        auto nullaryNumber = new Type::Lambda0(
            Type::Primitive::of(Type::Intrinsic::NUMBER)
        );
        auto random = new PrologueFunctionSymbol("random", nullaryNumber, new ProloguePosition("random"));
        prologueScope->insert(random);

        // randomVector :: number -> (enumerable<number>)
        auto typeEnumNum = new Type::Enumerable(
            Type::Primitive::of(Type::Intrinsic::NUMBER)
        );
        auto typeNumToEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeEnumNum
        );
        auto randomVector = new PrologueFunctionSymbol("randomVector", typeNumToEnumNum, new ProloguePosition("randomVector"));
        prologueScope->insert(randomVector);

        // randomMatrix :: number -> number -> (enumerable<enumerable<number>>)
        auto typeEnumEnumNum = new Type::Enumerable(typeEnumNum);
        auto typeNumToEnumEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeEnumEnumNum
        );
        auto randomMatrix = new PrologueFunctionSymbol("randomMatrix", typeNumToEnumEnumNum, new ProloguePosition("randomMatrix"));
        prologueScope->insert(randomMatrix);

        // range :: number -> number -> number -> (enumerable<number>)
        auto typeNumToNumToEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeNumToEnumNum
        );
        auto typeNumToNumToNumToEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeNumToNumToEnumNum
        );
        auto range = new PrologueFunctionSymbol("range", typeNumToNumToNumToEnumNum, new ProloguePosition("range"));
        prologueScope->insert(range);

        return prologueScope;
    }

}
