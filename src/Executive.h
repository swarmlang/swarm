#ifndef SWARMC_EXECUTIVE_H
#define SWARMC_EXECUTIVE_H

#include <istream>
#include <string>
#include <fstream>
#include <chrono>

#include "shared/util/Console.h"

class Executive : public IUsesConsole {
public:
    Executive() : IUsesConsole() {};

    int run(int argc, char **argv);
    bool parse_args(int argc, char** argv);
};


#endif //SWARMC_EXECUTIVE_H
