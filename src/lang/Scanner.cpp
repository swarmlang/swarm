#include "Scanner.h"

namespace nslib {
    [[nodiscard]] std::string s(swarmc::Lang::Parser::token::token_kind_type token) {
        if (token == swarmc::Lang::Parser::token::END) return "EOF";
        if (token == swarmc::Lang::Parser::token::ENUMERATE) return "ENUMERATE";
        if (token == swarmc::Lang::Parser::token::SHARED) return "SHARED";
        if (token == swarmc::Lang::Parser::token::ID) return "ID";
        if (token == swarmc::Lang::Parser::token::AS) return "AS";
        if (token == swarmc::Lang::Parser::token::WITH) return "WITH";
        if (token == swarmc::Lang::Parser::token::LBRACE) return "LBRACE";
        if (token == swarmc::Lang::Parser::token::RBRACE) return "RBRACE";
        if (token == swarmc::Lang::Parser::token::LPAREN) return "LPAREN";
        if (token == swarmc::Lang::Parser::token::RPAREN) return "RPAREN";
        if (token == swarmc::Lang::Parser::token::LBRACKET) return "LBRACKET";
        if (token == swarmc::Lang::Parser::token::RBRACKET) return "RBRACKET";
        if (token == swarmc::Lang::Parser::token::LARROW) return "LARROW";
        if (token == swarmc::Lang::Parser::token::RARROW) return "RARROW";
        if (token == swarmc::Lang::Parser::token::SEMICOLON) return "SEMICOLON";
        if (token == swarmc::Lang::Parser::token::COLON) return "COLON";
        if (token == swarmc::Lang::Parser::token::COMMA) return "COMMA";
        if (token == swarmc::Lang::Parser::token::ASSIGN) return "ASSIGN";
        if (token == swarmc::Lang::Parser::token::STRING) return "STRING";
        if (token == swarmc::Lang::Parser::token::NUMBER) return "NUMBER";
        if (token == swarmc::Lang::Parser::token::SOCKET) return "SOCKET";
        if (token == swarmc::Lang::Parser::token::BOOL) return "BOOL";
        if (token == swarmc::Lang::Parser::token::VOID) return "VOID";
        if (token == swarmc::Lang::Parser::token::NUMBERLITERAL) return "NUMBERLITERAL";
        if (token == swarmc::Lang::Parser::token::STRINGLITERAL) return "STRINGLITERAL";
        if (token == swarmc::Lang::Parser::token::MAP) return "MAP";
        if (token == swarmc::Lang::Parser::token::ENUMERABLE) return "ENUMERABLE";
        if (token == swarmc::Lang::Parser::token::IF) return "IF";
        if (token == swarmc::Lang::Parser::token::WHILE) return "WHILE";
        if (token == swarmc::Lang::Parser::token::TTRUE) return "TTRUE";
        if (token == swarmc::Lang::Parser::token::TFALSE) return "TFALSE";
        if (token == swarmc::Lang::Parser::token::CONTINUE) return "CONTINUE";
        if (token == swarmc::Lang::Parser::token::BREAK) return "BREAK";
        if (token == swarmc::Lang::Parser::token::RETURN) return "RETURN";
        if (token == swarmc::Lang::Parser::token::AND) return "AND";
        if (token == swarmc::Lang::Parser::token::OR) return "OR";
        if (token == swarmc::Lang::Parser::token::NOT) return "NOT";
        if (token == swarmc::Lang::Parser::token::EQUAL) return "EQUAL";
        if (token == swarmc::Lang::Parser::token::NOTEQUAL) return "NOTEQUAL";
        if (token == swarmc::Lang::Parser::token::ADD) return "ADD";
        if (token == swarmc::Lang::Parser::token::SUBTRACT) return "SUBTRACT";
        if (token == swarmc::Lang::Parser::token::MULTIPLY) return "MULTIPLY";
        if (token == swarmc::Lang::Parser::token::DIVIDE) return "DIVIDE";
        if (token == swarmc::Lang::Parser::token::ADDASSIGN) return "ADDASSIGN";
        if (token == swarmc::Lang::Parser::token::MULTIPLYASSIGN) return "MULTIPLYASSIGN";
        if (token == swarmc::Lang::Parser::token::SUBTRACTASSIGN) return "SUBTRACTASSIGN";
        if (token == swarmc::Lang::Parser::token::DIVIDEASSIGN) return "DIVIDEASSIGN";
        if (token == swarmc::Lang::Parser::token::MODULUSASSIGN) return "MODULUSASSIGN";
        if (token == swarmc::Lang::Parser::token::POWERASSIGN) return "POWERASSIGN";
        if (token == swarmc::Lang::Parser::token::ANDASSIGN) return "ANDASSIGN";
        if (token == swarmc::Lang::Parser::token::ORASSIGN) return "ORASSIGN";
        if (token == swarmc::Lang::Parser::token::MODULUS) return "MODULUS";
        if (token == swarmc::Lang::Parser::token::POWER) return "POWER";
        if (token == swarmc::Lang::Parser::token::DOT) return "DOT";
        if (token == swarmc::Lang::Parser::token::LARROWEQUALS) return "LARROWEQUALS";
        if (token == swarmc::Lang::Parser::token::RARROWEQUALS) return "RARROWEQUALS";
        if (token == swarmc::Lang::Parser::token::FN) return "FN";
        if (token == swarmc::Lang::Parser::token::ARROW) return "ARROW";
        if (token == swarmc::Lang::Parser::token::FNDEF) return "FNDEF";
        if (token == swarmc::Lang::Parser::token::OF) return "OF";
        if (token == swarmc::Lang::Parser::token::TYPE) return "TYPE";
        if (token == swarmc::Lang::Parser::token::INCLUDE) return "INCLUDE";
        if (token == swarmc::Lang::Parser::token::CONSTRUCTOR) return "CONSTRUCTOR";
        if (token == swarmc::Lang::Parser::token::FROM) return "FROM";
        if (token == swarmc::Lang::Parser::token::ROOT) return "ROOT";
        if (token == swarmc::Lang::Parser::token::WILDCARD) return "WILDCARD";
        if (token == swarmc::Lang::Parser::token::DEFER) return "DEFER";
        if (token == swarmc::Lang::Parser::token::USE) return "USE";

        return "UNKNOWN";
    }
}
