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
            if (kind == Parser::token::ID) return "ID";
            if (kind == Parser::token::AS) return "AS";
            if (kind == Parser::token::WITH) return "WITH";
            if (kind == Parser::token::LBRACE) return "LBRACE";
            if (kind == Parser::token::RBRACE) return "RBRACE";
            if (kind == Parser::token::LBRACKET) return "LBRACKET";
            if (kind == Parser::token::RBRACKET) return "RBRACKET";
            if (kind == Parser::token::LARROW) return "LARROW";
            if (kind == Parser::token::RARROW) return "RARROW";
            if (kind == Parser::token::SEMICOLON) return "SEMICOLON";
            if (kind == Parser::token::COMMA) return "COMMA";
            if (kind == Parser::token::ASSIGN) return "ASSIGN";
            if (kind == Parser::token::STRING) return "STRING";
            if (kind == Parser::token::NUMBER) return "NUMBER";
            if (kind == Parser::token::BOOL) return "BOOL";
            if (kind == Parser::token::MAP) return "MAP";
            if (kind == Parser::token::ENUMERABLE) return "ENUMERABLE";

            return "UNKNOWN";
        }
    };

}
}

#endif
