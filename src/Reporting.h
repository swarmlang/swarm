#ifndef SWARMC_REPORTING_H
#define SWARMC_REPORTING_H

#include <string>
#include "lang/Position.h"
#include "errors/SwarmError.h"

namespace swarmc {

    class Reporting {
    public:
        // Prints parsing debug messages
        static void parseDebug(const Lang::Position* pos, const std::string& message);

        // Prints parsing error messages
        static void parseError(const Lang::Position* pos, const std::string& message);

        // Prints name analysis debug messages
        static void nameDebug(const Lang::Position* pos, const std::string& message);

        // Prints name analysis error messages
        static void nameError(const Lang::Position* pos, const std::string& message);

        // Prints type analysis debug messages
        static void typeDebug(const Lang::Position* pos, const std::string& message);

        // Prints type analysis error messages
        static void typeError(const Lang::Position* pos, const std::string& message);

        static void syntaxError(const Lang::Position* pos, const std::string& message);
    
        // Prints ASTToISAWalk Debug messages
        static void toISADebug(const Lang::Position* pos, const std::string& message);
    };

}

#endif
