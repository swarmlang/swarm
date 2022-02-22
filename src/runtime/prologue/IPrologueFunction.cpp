#include "IPrologueFunction.h"
#include "../../errors/SwarmError.h"
#include "Random.h"
#include "Log.h"
#include "ToString.h"
#include "MinMax.h"
#include "File.h"

using namespace swarmc::Runtime::Prologue;

IPrologueFunction* IPrologueFunction::resolveByName(std::string name) {
    if ( name == "random" ) return new Random;
    if ( name == "log" ) return new Log;
    if ( name == "logError" ) return new LogError;
    if ( name == "numberToString" ) return new NumberToString;
    if ( name == "boolToString" ) return new BoolToString;
    if ( name == "min" ) return new Min;
    if ( name == "max" ) return new Max;
    if ( name == "fileContents" ) return new FileContents;

    throw Errors::SwarmError("Invalid Prologue function: " + name);
}

void IPrologueFunction::buildScope(Lang::ScopeTable* table) {
    Random random;
    table->addFunction(random.name(), (FunctionType*) random.type()->copy(), random.position()->copy());

    Log log;
    table->addFunction(log.name(), (FunctionType*) log.type()->copy(), log.position()->copy());

    LogError logError;
    table->addFunction(logError.name(), (FunctionType*) logError.type()->copy(), logError.position()->copy());

    NumberToString numberToString;
    table->addFunction(numberToString.name(), (FunctionType*) numberToString.type()->copy(), numberToString.position()->copy());

    BoolToString boolToString;
    table->addFunction(boolToString.name(), (FunctionType*) boolToString.type()->copy(), boolToString.position()->copy());

    Min min;
    table->addFunction(min.name(), (FunctionType*) min.type()->copy(), min.position()->copy());

    Max max;
    table->addFunction(max.name(), (FunctionType*) max.type()->copy(), max.position()->copy());

    FileContents fileContents;
    table->addFunction(fileContents.name(), (FunctionType*) fileContents.type()->copy(), fileContents.position()->copy());
}
