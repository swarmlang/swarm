#ifndef SWARMC_SCANNER_H
#define SWARMC_SCANNER_H

#include <cstdint>
#include <iostream>
#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "Debugging.h"
#include "Token.h"
#include "../bison/grammar.hh"

namespace swarmc::Lang {

    /**
     * Base class for the Flex-based input lexer.
     */
    class Scanner : public yyFlexLexer {
    public:
        explicit Scanner(std::istream* in) : yyFlexLexer(in) {};
        ~Scanner() override = default;

        using FlexLexer::yylex;

        virtual int yylex(Parser::semantic_type* const lval);

        /** Creates a new Token instance for the given Parser::token kind. */
        int makeBareToken(int tagIn) {
            auto length = static_cast<size_t>(yyleng);
            auto pos = new Position(this->lineNum, this->lineNum, this->colNum, this->colNum+length);

            yylval->lexeme = new Token(pos, tagIn, Debugging::tokenKindToString(tagIn));
            colNum += length;
            return tagIn;
        }

        int makeStringLiteralToken() {
            int tagIn = Parser::token::STRINGLITERAL;
            auto length = static_cast<size_t>(yyleng);
            auto pos = new Position(this->lineNum, this->lineNum, this->colNum, this->colNum+length);

            std::string text(yytext);
            yylval->lexeme = new StringLiteralToken(pos, tagIn, Debugging::tokenKindToString(tagIn) + ":" + text, text.substr(1, text.size() - 2));
            colNum += length;
            return tagIn;
        }

        int makeNumberLiteralToken() {
            int tagIn = Parser::token::NUMBERLITERAL;
            auto length = static_cast<size_t>(yyleng);
            auto pos = new Position(this->lineNum, this->lineNum, this->colNum, this->colNum+length);

            std::string text(yytext);
            yylval->lexeme = new NumberLiteralToken(pos, tagIn, Debugging::tokenKindToString(tagIn) + ":" + text, std::stod(yytext));
            colNum += length;
            return tagIn;
        }

        int makeIDToken() {
            int tagIn = Parser::token::ID;
            auto length = static_cast<size_t>(yyleng);
            auto pos = new Position(this->lineNum, this->lineNum, this->colNum, this->colNum+length);

            std::string text(yytext);
            yylval->lexeme = new IDToken(pos, tagIn, Debugging::tokenKindToString(tagIn) + ":" + text, yytext);
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
        size_t lineNum = 1;
        size_t colNum = 1;
    };

}

#endif
