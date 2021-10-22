#ifndef SWARMC_SCANNER_H
#define SWARMC_SCANNER_H

#include <cstdint>
#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "../bison/grammar.hh"

namespace swarmc {
namespace Lang {

    class Scanner : public yyFlexLexer {
    public:
        Scanner(std::istream* in) : yyFlexLexer(in) {};
        virtual ~Scanner() {};

        using FlexLexer::yylex;

        virtual int yylex(Parser::semantic_type* const lval);

        // makeBareToken
        // tokenKindString
        // outputTokens

    protected:
        Parser::semantic_type* yylval = nullptr;
        uint64_t lineNum = 1;
        uint64_t colNum = 1;
    };

}
}

#endif
