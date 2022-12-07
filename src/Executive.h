#ifndef SWARMC_EXECUTIVE_H
#define SWARMC_EXECUTIVE_H

#include <istream>
#include <string>
#include <fstream>
#include <chrono>
#include <vector>

#include "shared/nslib.h"

using namespace nslib;

/**
 * Executive class for CLI applications. Handles arg parsing and manages runtime globals.
 */
class Executive : public IUsesConsole, public IUsesLogger {
public:
    Executive() : IUsesConsole(), IUsesLogger("main") {};

    int run(int argc, char **argv);
    void cleanup();
    bool parseArgs(std::vector<std::string>&);
    void printUsage();

protected:
    // CLI options
    bool flagOutputTokens = false;
    bool flagParseAndStop = false;
    bool flagOutputParse = false;
    bool flagRunTest = false;
    bool flagSingleThreaded = false;
    bool flagDebugger = false;
    bool flagInteractiveDebug = false;
    bool flagVerbose = false;
    bool flagClearQueue = false;
    bool flagWorkQueue = false;
    bool flagWithPrologue = true;
    bool flagSVI = false;
    bool flagOutputISA = false;
    bool flagOutputCFG = false;
    bool flagRemoveSelfAssign = true;
    bool flagConstProp = true;
    bool flagOutputBinary = false;
    std::string flagOutputTokensTo;
    std::string flagOutputParseTo;
    std::string flagOutputSerializeTo;
    std::string flagOutputDeSerializeTo;
    std::string flagRunTestName;
    std::string outputResultTo = "--";
    std::string outputISATo = "--";
    std::string outputBinaryTo;
    std::string outputCFGTo;
    std::string flagFilterFile;
    std::string inputFile;
    std::istream* _input = nullptr;

    int debugOutputTokens();
    int debugOutputParse();
    int debugParseAndStop();
    int debugOutputISA();
    int debugOutputCFG();
    int runTest();
    int parseFilters();
    int executeLocalSVI();
    int emitBinary();
};


#endif //SWARMC_EXECUTIVE_H
