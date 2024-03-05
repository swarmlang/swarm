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
        auto numberToString = new PrologueFunctionSymbol("numberToString", typeNumToStr, new ProloguePosition("numberToString"), "NUMBER_TO_STRING");
        prologueScope->insert(numberToString);

        // booleanToString :: boolean -> string
        auto typeBoolToStr = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::BOOLEAN),
            Type::Primitive::of(Type::Intrinsic::STRING)
        );
        auto booleanToString = new PrologueFunctionSymbol("booleanToString", typeBoolToStr, new ProloguePosition("booleanToString"), "BOOLEAN_TO_STRING");
        prologueScope->insert(booleanToString);

        // sin :: number -> number
        auto typeNumToNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            Type::Primitive::of(Type::Intrinsic::NUMBER)
        );
        auto sin = new PrologueFunctionSymbol("sin", typeNumToNum, new ProloguePosition("sin"), "SIN");
        prologueScope->insert(sin);

        // cos :: number -> number
        auto cos = new PrologueFunctionSymbol("cos", typeNumToNum, new ProloguePosition("cos"), "COS");
        prologueScope->insert(cos);

        // tan :: number -> number
        auto tan = new PrologueFunctionSymbol("tan", typeNumToNum, new ProloguePosition("tan"), "TAN");
        prologueScope->insert(tan);

        // floor :: number -> number
        auto floor = new PrologueFunctionSymbol("floor", typeNumToNum, new ProloguePosition("floor"), "FLOOR");
        prologueScope->insert(floor);

        // ceiling :: number -> number
        auto ceiling = new PrologueFunctionSymbol("ceiling", typeNumToNum, new ProloguePosition("ceiling"), "CEILING");
        prologueScope->insert(ceiling);

        // random :: -> number
        auto nullaryNumber = new Type::Lambda0(
            Type::Primitive::of(Type::Intrinsic::NUMBER)
        );
        auto random = new PrologueFunctionSymbol("random", nullaryNumber, new ProloguePosition("random"), "RANDOM");
        prologueScope->insert(random);

        // time :: -> number
        auto time = new PrologueFunctionSymbol("time", nullaryNumber, new ProloguePosition("time"), "TIME");
        prologueScope->insert(time);

        // randomVector :: number -> (enumerable<number>)
        auto typeEnumNum = new Type::Enumerable(
            Type::Primitive::of(Type::Intrinsic::NUMBER)
        );
        auto typeNumToEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeEnumNum
        );
        auto randomVector = new PrologueFunctionSymbol("randomVector", typeNumToEnumNum, new ProloguePosition("randomVector"), "RANDOM_VECTOR");
        prologueScope->insert(randomVector);

        // zeroVector :: number -> (enumerable<number>)
        auto zeroVector = new PrologueFunctionSymbol("zeroVector", typeNumToEnumNum, new ProloguePosition("zeroVector"), "ZERO_VECTOR");
        prologueScope->insert(zeroVector);

        // randomMatrix :: number -> number -> (enumerable<enumerable<number>>)
        auto typeEnumEnumNum = new Type::Enumerable(typeEnumNum);
        auto typeNumToEnumEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeEnumEnumNum
        );
        auto typeNumToNumToEnumEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeNumToEnumEnumNum
        );
        auto randomMatrix = new PrologueFunctionSymbol("randomMatrix", typeNumToNumToEnumEnumNum, new ProloguePosition("randomMatrix"), "RANDOM_MATRIX");
        prologueScope->insert(randomMatrix);

        // zeroMatrix :: number -> number -> (enumerable<enumerable<number>>)
        auto zeroMatrix = new PrologueFunctionSymbol("zeroMatrix", typeNumToNumToEnumEnumNum, new ProloguePosition("zeroMatrix"), "ZERO_MATRIX");
        prologueScope->insert(zeroMatrix);

        // vectorToString :: (enumerable<number>) -> string
        auto typeEnumNumToString = new Type::Lambda1(
            typeEnumNum,
            Type::Primitive::of(Type::Intrinsic::STRING)
        );
        auto vectorToString = new PrologueFunctionSymbol("vectorToString", typeEnumNumToString, new ProloguePosition("vectorToString"), "VECTOR_TO_STRING");
        prologueScope->insert(vectorToString);

        // matrixToString :: (enumerable<enumerable<number>>) -> string
        auto typeEnumEnumNumToString = new Type::Lambda1(
            typeEnumEnumNum,
            Type::Primitive::of(Type::Intrinsic::STRING)
        );
        auto matrixToString = new PrologueFunctionSymbol("matrixToString", typeEnumEnumNumToString, new ProloguePosition("matrixToString"), "MATRIX_TO_STRING");
        prologueScope->insert(matrixToString);

        // range :: number -> number -> number -> (enumerable<number>)
        auto typeNumToNumToEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeNumToEnumNum
        );
        auto typeNumToNumToNumToEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeNumToNumToEnumNum
        );
        auto range = new PrologueFunctionSymbol("range", typeNumToNumToNumToEnumNum, new ProloguePosition("range"), "RANGE");
        prologueScope->insert(range);

        // nthRoot :: number -> number -> number
        auto typeNumToNumToNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeNumToNum
        );
        auto nthRoot = new PrologueFunctionSymbol("nthRoot", typeNumToNumToNum, new ProloguePosition("nthRoot"), "NTH_ROOT");
        prologueScope->insert(nthRoot);

        auto typeStrVoid = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::STRING),
            Type::Primitive::of(Type::Intrinsic::VOID)
        );
        auto llog = new PrologueFunctionSymbol("lLog", typeStrVoid, new ProloguePosition("lLog"), "null");
        prologueScope->insert(llog);

        auto lerror = new PrologueFunctionSymbol("lError", typeStrVoid, new ProloguePosition("lError"), "null");
        prologueScope->insert(lerror);

        auto slog = new PrologueFunctionSymbol("sLog", typeStrVoid, new ProloguePosition("sLog"), "null");
        prologueScope->insert(slog);

        auto serror = new PrologueFunctionSymbol("sError", typeStrVoid, new ProloguePosition("sError"), "null");
        prologueScope->insert(serror);

        // count :: enumerable<ambiguous> :: number
        auto typeEnumAny = new Type::Enumerable(Type::Ambiguous::of());
        auto typeEnumAnyToNum = new Type::Lambda1(
            typeEnumAny,
            Type::Primitive::of(Type::Intrinsic::NUMBER)
        );
        auto count = new PrologueFunctionSymbol("count", typeEnumAnyToNum, new ProloguePosition("count"), "COUNT");
        prologueScope->insert(count);

        // subVector :: number -> number -> enumerable<number> -> enumerable<number>
        auto typeEnumNumToEnumNum = new Type::Lambda1(
            typeEnumNum,
            typeEnumNum
        );
        auto typeNumToEnumNumToEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeEnumNumToEnumNum
        );
        auto typeNumToNumToEnumNumToEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeNumToEnumNumToEnumNum
        );
        auto subVector = new PrologueFunctionSymbol("subVector", typeNumToNumToEnumNumToEnumNum, new ProloguePosition("subVector"), "SUBVECTOR");
        prologueScope->insert(subVector);

        // subMatrix :: number -> number -> number -> number -> enumerable<number> -> enumerable<number>
        auto typeEnumEnumNumToEnumEnumNum = new Type::Lambda1(
            typeEnumEnumNum,
            typeEnumEnumNum
        );
        auto typeNumToEnumEnumNumToEnumEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeEnumEnumNumToEnumEnumNum
        );
        auto typeNumToNumToEnumEnumNumToEnumEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeNumToEnumEnumNumToEnumEnumNum
        );
        auto typeNumToNumToNumToEnumEnumNumToEnumEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeNumToNumToEnumEnumNumToEnumEnumNum
        );
        auto typeNumToNumToNumToNumToEnumEnumNumToEnumEnumNum = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::NUMBER),
            typeNumToNumToNumToEnumEnumNumToEnumEnumNum
        );
        auto subMatrix = new PrologueFunctionSymbol("subMatrix", typeNumToNumToNumToNumToEnumEnumNumToEnumEnumNum, new ProloguePosition("subMatrix"), "SUBMATRIX");
        prologueScope->insert(subMatrix);

        auto typeStringStringTag = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::STRING),
            new Type::Lambda1(
                Type::Primitive::of(Type::Intrinsic::STRING),
                new Type::Resource(Type::Opaque::of("PROLOGUE::TAG"))
            )
        );
        auto tag = new PrologueFunctionSymbol("tag", typeStringStringTag, new ProloguePosition("tag"), "TAG");
        prologueScope->insert(tag);

        auto socketType = new Type::Resource(Type::Opaque::of("PROLOGUE::SOCKET"));

        auto typeSocketInner = new Type::Lambda1(Type::Primitive::of(Type::Intrinsic::NUMBER), socketType);
        auto typeSocket = new Type::Lambda1(Type::Primitive::of(Type::Intrinsic::NUMBER), typeSocketInner);
        auto getSocket = new PrologueFunctionSymbol("getSocket", typeSocket, new ProloguePosition("getSocket"), "SOCKET");
        prologueScope->insert(getSocket);

        auto typeSocketVoid = new Type::Lambda1(socketType, Type::Primitive::of(Type::Intrinsic::VOID));
        auto openSocket = new PrologueFunctionSymbol("openSocket", typeSocketVoid, new ProloguePosition("openSocket"), "OPEN_SOCKET");
        prologueScope->insert(openSocket);

        auto fileType = new Type::Resource(Type::Opaque::of("PROLOGUE::FILE"));

        auto typeStringFile = new Type::Lambda1(
            Type::Primitive::of(Type::Intrinsic::STRING),
            fileType
        );
        auto file_open = new PrologueFunctionSymbol("open", typeStringFile, new ProloguePosition("open"), "OPEN_FILE");
        prologueScope->insert(file_open);

        auto typeFileString = new Type::Lambda1(
            fileType,
            Type::Primitive::of(Type::Intrinsic::STRING)
        );
        auto file_read = new PrologueFunctionSymbol("read", typeFileString, new ProloguePosition("read"), "READ_FILE");
        prologueScope->insert(file_read);

        auto typeFileStringVoid = new Type::Lambda1(
            fileType,
            new Type::Lambda1(
                Type::Primitive::of(Type::Intrinsic::STRING),
                Type::Primitive::of(Type::Intrinsic::VOID)
            )
        );
        auto file_write = new PrologueFunctionSymbol("write", typeFileStringVoid, new ProloguePosition("write"), "WRITE_FILE");
        prologueScope->insert(file_write);

        auto file_append = new PrologueFunctionSymbol("append", typeFileStringVoid, new ProloguePosition("append"), "APPEND_FILE");
        prologueScope->insert(file_append);

        auto max = new PrologueFunctionSymbol("max", typeNumToNumToNum, new ProloguePosition("max"), "MAX");
        prologueScope->insert(max);

        auto min = new PrologueFunctionSymbol("min", typeNumToNumToNum, new ProloguePosition("min"), "MIN");
        prologueScope->insert(min);

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
            _type = swapref(_type, _value->value());
            return;
        }

        Reporting::typeError(
            _declaredAt,
            "Unable to disambiguate type of variable " + _name
        );
    }

}

namespace nslib {
    [[nodiscard]] std::string s(swarmc::Lang::SemanticSymbolKind kind) {
        if (kind == swarmc::Lang::SemanticSymbolKind::VARIABLE) return "VARIABLE";
        if (kind == swarmc::Lang::SemanticSymbolKind::FUNCTION) return "FUNCTION";
        return "UNKNOWN";
    }
}
