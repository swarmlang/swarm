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
            if (kind == Parser::token::LPAREN) return "LPAREN";
            if (kind == Parser::token::RPAREN) return "RPAREN";
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
            if (kind == Parser::token::NUMBERLITERAL) return "NUMBERLITERAL";
            if (kind == Parser::token::STRINGLITERAL) return "STRINGLITERAL";
            if (kind == Parser::token::MAP) return "MAP";
            if (kind == Parser::token::ENUMERABLE) return "ENUMERABLE";
            if (kind == Parser::token::IF) return "IF";
            if (kind == Parser::token::TRUE) return "TRUE";
            if (kind == Parser::token::FALSE) return "FALSE";
            if (kind == Parser::token::AND) return "AND";
            if (kind == Parser::token::OR) return "OR";
            if (kind == Parser::token::NOT) return "NOT";
            if (kind == Parser::token::EQUAL) return "EQUAL";
            if (kind == Parser::token::NOTEQUAL) return "NOTEQUAL";
            if (kind == Parser::token::ADD) return "ADD";
            if (kind == Parser::token::SUBTRACT) return "SUBTRACT";
            if (kind == Parser::token::MULTIPLY) return "MULTIPLY";
            if (kind == Parser::token::DIVIDE) return "DIVIDE";
            if (kind == Parser::token::ADDASSIGN) return "ADDASSIGN";
            if (kind == Parser::token::MULTIPLYASSIGN) return "MULTIPLYASSIGN";
            if (kind == Parser::token::MODULUS) return "MODULUS";
            if (kind == Parser::token::POWER) return "POWER";

            return "UNKNOWN";
        }
    };

}
}

#endif
