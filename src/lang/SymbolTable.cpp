#include "SymbolTable.h"
#include "AST.h"

namespace swarmc::Lang {

    ScopeTable* ScopeTable::prologue() {
        auto prologueScope = new swarmc::Lang::ScopeTable();

        // numberToString :: number -> string
        auto typeNumToStr = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            Type::Primitive::of(Type::Intrinsic::STRING)
        );
        auto numberToString = new PrologueFunctionSymbol("numberToString", typeNumToStr, new ProloguePosition("numberToString"), false);
        prologueScope->insert(numberToString);

        // booleanToString :: boolean -> string
        auto typeBoolToStr = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::BOOLEAN),
            Type::Primitive::of(Type::Intrinsic::STRING)
        );
        auto booleanToString = new PrologueFunctionSymbol("booleanToString", typeBoolToStr, new ProloguePosition("booleanToString"), false);
        prologueScope->insert(booleanToString);

        // sin :: number -> number
        auto typeNumToNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            Type::Primitive::of(Type::Intrinsic::STRING)
        );
        auto sin = new PrologueFunctionSymbol("sin", typeNumToNum, new ProloguePosition("sin"), false);
        prologueScope->insert(sin);

        // cos :: number -> number
        auto cos = new PrologueFunctionSymbol("cos", typeNumToNum, new ProloguePosition("cos"), false);
        prologueScope->insert(cos);

        // tan :: number -> number
        auto tan = new PrologueFunctionSymbol("tan", typeNumToNum, new ProloguePosition("tan"), false);
        prologueScope->insert(tan);

        // random :: -> number
        auto nullaryNumber = new Type::Lambda0(
            Type::Primitive::of(Type::Intrinsic::NUMBER)
        );
        auto random = new PrologueFunctionSymbol("random", nullaryNumber, new ProloguePosition("random"), false);
        prologueScope->insert(random);

        // randomVector :: number -> (enumerable<number>)
        auto typeEnumNum = new Type::Enumerable(
            Type::Primitive::of(Type::Intrinsic::NUMBER)
        );
        auto typeNumToEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeEnumNum
        );
        auto randomVector = new PrologueFunctionSymbol("randomVector", typeNumToEnumNum, new ProloguePosition("randomVector"), false);
        prologueScope->insert(randomVector);

        // randomMatrix :: number -> number -> (enumerable<enumerable<number>>)
        auto typeEnumEnumNum = new Type::Enumerable(typeEnumNum);
        auto typeNumToEnumEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeEnumEnumNum
        );
        auto randomMatrix = new PrologueFunctionSymbol("randomMatrix", typeNumToEnumEnumNum, new ProloguePosition("randomMatrix"), false);
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
        auto range = new PrologueFunctionSymbol("range", typeNumToNumToNumToEnumNum, new ProloguePosition("range"), false);
        prologueScope->insert(range);

        return prologueScope;
    }

    void VariableSymbol::setObjectType(TypeLiteral* type) {
        if ( _value != nullptr ) {
            Reporting::typeError(
                declaredAt(),
                "Attempt to rebind value of type variable. "
            );
            return;
        }
        _value = useref(type);
    }

    void VariableSymbol::disambiguateType()  {
        if ( _type->isAmbiguous() && _value != nullptr ) {
            _type = useref(_value->value());
            return;
        }

        Reporting::typeError(
            _declaredAt,
            "Unable to disambiguate type of variable " + _name
        );
    }

}
