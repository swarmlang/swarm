#ifndef SWARMC_SCANNER_H
#define SWARMC_SCANNER_H

#include "Debugging.h"
#include <cstdint>
#include <iostream>
#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "Token.h"
#include "../bison/grammar.hh"

namespace swarmc {
namespace Lang {

    /**
     * Base class for the Flex-based input lexer.
     */
    class Scanner : public yyFlexLexer {
    public:
        Scanner(std::istream* in) : yyFlexLexer(in) {};
        virtual ~Scanner() {};

        using FlexLexer::yylex;

        virtual int yylex(Parser::semantic_type* const lval);

        /** Creates a new Token instance for the given Parser::token kind. */
        int makeBareToken(int tagIn) {
            size_t length = static_cast<size_t>(yyleng);
            Position* pos = new Position(lineNum, colNum, lineNum, colNum+length);

            yylval->lexeme = new Token(pos, tagIn, Debugging::tokenKindToString(tagIn));
            colNum += length;
            return tagIn;
        }

        /** Lex and output the tokens to the given output stream. Mainly for debugging. **/
        void outputTokens(std::ostream& out) {
            Parser::semantic_type lex;
            int tokenKind;

            while ( true ) {
                tokenKind = this->yylex(&lex);

                if ( tokenKind == Parser::token::END ) {
                    out << "EOF" << std::endl;
                    return;
                }

                out << lex.lexeme->toString() << std::endl;
            }
        }

    protected:
        Parser::semantic_type* yylval = nullptr;
        uint64_t lineNum = 1;
        uint64_t colNum = 1;
    };

}
}

#endif
