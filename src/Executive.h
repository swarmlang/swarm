#ifndef SWARMC_EXECUTIVE_H
#define SWARMC_EXECUTIVE_H

#include <istream>
#include <string>
#include <fstream>
#include <chrono>
#include <vector>

#include "shared/util/Console.h"

/**
 * Executive class for CLI applications. Handles arg parsing and manages runtime globals.
 */
class Executive : public IUsesConsole {
public:
    Executive() : IUsesConsole() {};

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
    bool flagClearQueue = false;
    bool flagWorkQueue = false;
    bool flagSVI = false;
    bool flagOutputISA = false;
    std::string flagOutputTokensTo;
    std::string flagOutputParseTo;
    std::string flagOutputSerializeTo;
    std::string flagOutputDeSerializeTo;
    std::string flagRunTestName;
    std::string outputResultTo = "--";
    std::string outputISATo = "--";
    std::string flagFilterFile;
    std::string inputFile;
    std::istream* _input = nullptr;

    int debugOutputTokens();
    int debugOutputParse();
    int debugParseAndStop();
    int debugOutputISA();
    int runTest();
    int parseFilters();
};


#endif //SWARMC_EXECUTIVE_H
