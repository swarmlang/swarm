#ifndef SWARMC_SCANNER_H
#define SWARMC_SCANNER_H

#include <cstdint>
#include <iostream>
#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "Token.h"
#include "../bison/grammar.hh"

namespace swarmc::Lang {

    [[nodiscard]] std::string tokenTagString(swarmc::Lang::Parser::token::token_kind_type token);

    /**
     * Base class for the Flex-based input lexer.
     */
    class Scanner : public yyFlexLexer {
    public:
        explicit Scanner(std::istream* in, std::string file) : yyFlexLexer(in), _file(std::move(file)) {};
        ~Scanner() override {
            for ( auto token : _tokens ) {
                freeref(token);
            }
        }

        using FlexLexer::yylex;

        virtual int yylex(Parser::semantic_type* const lval);

        /** Creates a new Token instance for the given Parser::token kind. */
        int makeBareToken(swarmc::Lang::Parser::token::token_kind_type tagIn) {
            auto length = static_cast<size_t>(yyleng);
            auto pos = new Position(_file, this->lineNum, this->lineNum, this->colNum, this->colNum+length);

            yylval->lexeme = new Token(pos, tagIn, tokenTagString(tagIn));
            _tokens.push_back(useref(yylval->lexeme));
            setColNum(colNum + length);
            return tagIn;
        }

        int makeStringLiteralToken() {
            auto tagIn = Parser::token::STRINGLITERAL;
            auto length = static_cast<size_t>(yyleng);
            auto pos = new Position(_file, this->lineNum, this->lineNum, this->colNum, this->colNum+length);

            std::string text(yytext);
            // replace "escape characters" with the actual ascii escape characters
            // i stg i tried doing this with std::regex and couldnt get it to work
            for ( auto i = 1; i < text.size() - 2; i++ ) {
                if ( text[i] != '\\' ) continue;
                text.erase(i, 1);
                switch ( text[i] ) {
                case 'n':  text[i] = '\n'; break;
                case 't':  text[i] = '\t'; break;
                case 'r':  text[i] = '\r'; break;
                case '\'': text[i] = '\''; break;
                case '\"': text[i] = '\"'; break;
                case '\\': text[i] = '\\'; break;
                default: // this not possible
                    break;
                }
            }

            yylval->lexeme = new StringLiteralToken(pos, tagIn, tokenTagString(tagIn) + ":" + text, text.substr(1, text.size() - 2));
            _tokens.push_back(useref(yylval->lexeme));
            setColNum(colNum + length);
            return tagIn;
        }

        int makeNumberLiteralToken() {
            auto tagIn = Parser::token::NUMBERLITERAL;
            auto length = static_cast<size_t>(yyleng);
            auto pos = new Position(_file, this->lineNum, this->lineNum, this->colNum, this->colNum+length);

            std::string text(yytext);
            yylval->lexeme = new NumberLiteralToken(pos, tagIn, tokenTagString(tagIn) + ":" + text, std::stod(yytext));
            _tokens.push_back(useref(yylval->lexeme));
            setColNum(colNum + length);
            return tagIn;
        }

        int makeIDToken() {
            auto tagIn = Parser::token::ID;
            auto length = static_cast<size_t>(yyleng);
            auto pos = new Position(_file, this->lineNum, this->lineNum, this->colNum, this->colNum+length);

            std::string text(yytext);
            yylval->lexeme = new IDToken(pos, tagIn, tokenTagString(tagIn) + ":" + text, yytext);
            _tokens.push_back(useref(yylval->lexeme));
            setColNum(colNum + length);
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

                out << s(lex.lexeme) << std::endl;
            }
        }

        Position* currentPos() const {
            return new Position(_file, prevLine, lineNum, prevCol, colNum);
        }

        void setLineNum(size_t line) {
            prevLine = lineNum;
            lineNum = line;
        }

        void setColNum(size_t col) {
            prevCol = colNum;
            colNum = col;
        }

    protected:
        Parser::semantic_type* yylval = nullptr;
        std::string _file;
        size_t prevLine = 0;
        size_t prevCol = 0;
        size_t lineNum = 1;
        size_t colNum = 1;

        std::list<Token*> _tokens;
    };

}

#endif
