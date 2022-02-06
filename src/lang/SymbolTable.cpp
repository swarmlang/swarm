#include "Position.h"
#include "SymbolTable.h"
#include "Type.h"

namespace swarmc {
namespace Lang {
    ScopeTable* ScopeTable::prologue() {
        ScopeTable* prologueScope = new swarmc::Lang::ScopeTable();
        PrimitiveType* primitiveNumber = PrimitiveType::of(ValueType::TNUM);

        // random()
        FunctionType* randomType = FunctionType::of(primitiveNumber);
        ProloguePosition* randomPosition = new ProloguePosition("random");
        prologueScope->addFunction("random", randomType, randomPosition);

        // zero()
        FunctionType* zeroType = FunctionType::of(primitiveNumber);
        ProloguePosition* zeroPosition = new ProloguePosition("zero");
        prologueScope->addFunction("zero", zeroType, zeroPosition);

        // zero1(number)
        FunctionType* zero1Type = FunctionType::of(primitiveNumber);
        zero1Type->addArgument(primitiveNumber);
        ProloguePosition* zero1Position = new ProloguePosition("zero1");
        prologueScope->addFunction("zero1", zero1Type, zero1Position);

        // zero2(number, number)
        FunctionType* zero2Type = FunctionType::of(primitiveNumber);
        zero2Type->addArgument(primitiveNumber);
        zero2Type->addArgument(primitiveNumber);
        ProloguePosition* zero2Position = new ProloguePosition("zero2");
        prologueScope->addFunction("zero2", zero2Type, zero2Position);

        return prologueScope;
    }
}
}