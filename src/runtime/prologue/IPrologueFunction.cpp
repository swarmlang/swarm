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
    table->addPrologueFunction(random.name(), (FunctionType*) random.type()->copy(), random.position()->copy());

    Log log;
    table->addPrologueFunction(log.name(), (FunctionType*) log.type()->copy(), log.position()->copy());

    LogError logError;
    table->addPrologueFunction(logError.name(), (FunctionType*) logError.type()->copy(), logError.position()->copy());

    NumberToString numberToString;
    table->addPrologueFunction(numberToString.name(), (FunctionType*) numberToString.type()->copy(), numberToString.position()->copy());

    BoolToString boolToString;
    table->addPrologueFunction(boolToString.name(), (FunctionType*) boolToString.type()->copy(), boolToString.position()->copy());

    Min min;
    table->addPrologueFunction(min.name(), (FunctionType*) min.type()->copy(), min.position()->copy());

    Max max;
    table->addPrologueFunction(max.name(), (FunctionType*) max.type()->copy(), max.position()->copy());

    FileContents fileContents;
    table->addPrologueFunction(fileContents.name(), (FunctionType*) fileContents.type()->copy(), fileContents.position()->copy());
}
