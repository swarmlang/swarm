#include "Position.h"
#include "SymbolTable.h"
#include "Type.h"

namespace swarmc {
namespace Lang {
    ScopeTable* ScopeTable::prologue() {
        ScopeTable* prologueScope = new swarmc::Lang::ScopeTable();
        PrimitiveType* primitiveNumber = PrimitiveType::of(ValueType::TNUM);
        PrimitiveType* primitiveString = PrimitiveType::of(ValueType::TSTRING);
        PrimitiveType* primitiveUnit = PrimitiveType::of(ValueType::TUNIT);
        PrimitiveType* primitiveBool = PrimitiveType::of(ValueType::TBOOL);

        // random()
        FunctionType* randomType = FunctionType::of(primitiveNumber, false);
        ProloguePosition* randomPosition = new ProloguePosition("random");
        prologueScope->addFunction("random", randomType, randomPosition);

        // zero()
        FunctionType* zeroType = FunctionType::of(primitiveNumber, false);
        ProloguePosition* zeroPosition = new ProloguePosition("zero");
        prologueScope->addFunction("zero", zeroType, zeroPosition);

        // zero1(number)
        FunctionType* zero1Type = FunctionType::of(primitiveNumber, false);
        zero1Type->addArgument(primitiveNumber);
        ProloguePosition* zero1Position = new ProloguePosition("zero1");
        prologueScope->addFunction("zero1", zero1Type, zero1Position);

        // zero2(number, number)
        FunctionType* zero2Type = FunctionType::of(primitiveNumber, false);
        zero2Type->addArgument(primitiveNumber);
        zero2Type->addArgument(primitiveNumber);
        ProloguePosition* zero2Position = new ProloguePosition("zero2");
        prologueScope->addFunction("zero2", zero2Type, zero2Position);

        // fileContents(string)
        GenericType* stringResource = GenericType::of(ValueType::TRESOURCE, primitiveString);
        FunctionType* fileContentsType = FunctionType::of(stringResource, false);
        fileContentsType->addArgument(primitiveString);
        ProloguePosition* fileContentsPosition = new ProloguePosition("fileContents");
        prologueScope->addFunction("fileContents", fileContentsType, fileContentsPosition);

        // log(string)
        FunctionType* logType = FunctionType::of(primitiveUnit, false);
        logType->addArgument(primitiveString);
        ProloguePosition* logPosition = new ProloguePosition("log");
        prologueScope->addFunction("log", logType, logPosition);

        // logError(string)
        FunctionType* logErrorType = FunctionType::of(primitiveUnit, false);
        logErrorType->addArgument(primitiveString);
        ProloguePosition* logErrorPosition = new ProloguePosition("logError");
        prologueScope->addFunction("logError", logErrorType, logErrorPosition);

        // numberToString(number)
        FunctionType* numberToStringType = FunctionType::of(primitiveString, false);
        numberToStringType->addArgument(primitiveNumber);
        ProloguePosition* numberToStringPosition = new ProloguePosition("numberToString");
        prologueScope->addFunction("numberToString", numberToStringType, numberToStringPosition);

        // boolToString(boolean)
        FunctionType* boolToStringType = FunctionType::of(primitiveString, false);
        boolToStringType->addArgument(primitiveBool);
        ProloguePosition* boolToStringPosition = new ProloguePosition("boolToString");
        prologueScope->addFunction("boolToString", boolToStringType, boolToStringPosition);

        return prologueScope;
    }
}
}
