#ifndef SWARMC_DEBUGGING_H
#define SWARMC_DEBUGGING_H

#include <string>

#include "../bison/grammar.hh"

namespace swarmc {
namespace Lang {


    /**
     * Some helpful debugging utilities.
     */
    class Debugging {
    public:
        /** Given a Parser::token value, get its string representation. */
        static std::string tokenKindToString(int kind) {
            if (kind == Parser::token::END) return "EOF";
            if (kind == Parser::token::ENUMERATE) return "ENUMERATE";

            return "UNKNOWN";
        }
    };

}
}

#endif
