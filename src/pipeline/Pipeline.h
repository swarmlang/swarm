#ifndef SWARMC_PIPELINE_H
#define SWARMC_PIPELINE_H

#include <iostream>
#include <string>
#include <sstream>
#include "../shared/nslib.h"
#include "../errors/ParseError.h"
#include "../lang/Scanner.h"
#include "../bison/grammar.hh"
#include "../lang/AST.h"
#include "../lang/Walk/PrintWalk.h"
#include "../lang/Walk/ToISAWalk.h"
#include "../lang/Walk/NameAnalysisWalk.h"
#include "../lang/Walk/TypeAnalysisWalk.h"
#include "../vm/ISAParser.h"
#include "../cfg/cfg.h"

using namespace nslib;

namespace swarmc {

    enum PipelineTarget {
        TOKENS = 0,
        PARSE_RESULT,
        AST_RAW,
        AST_SYMBOLIC,
        AST_SYMBOLIC_TYPED,
    };

    class Pipeline : public IStringable {
    public:
        explicit Pipeline(std::istream* input) {
            _input = input;
            _scanner = new Lang::Scanner(input);
            _root = nullptr;
            _parser = new Lang::Parser(*_scanner, &_root);
            _isa = nullptr;
        }

        ~Pipeline() override {
            delete _scanner;
            delete _parser;

            if ( _root != nullptr) {
                for ( auto stmt : *_root->body() ) delete stmt;
                delete _root->body();
            }

            delete _root;
            delete _isa;
        }

        std::string toString() const override {
            return "Pipeline<>";
        }

        void targetTokenRepresentation(std::ostream& out) {
            _scanner->outputTokens(out);
        }

        int targetParse() {
            return _parser->parse();
        }

        Lang::ProgramNode* targetASTRaw() {
            int parseResult = targetParse();
            if ( parseResult != 0 ) {
                throw Errors::ParseError(parseResult);
            }

            return _root;
        }

        Lang::ProgramNode* targetASTSymbolic() {
            targetASTRaw();

            Lang::Walk::NameAnalysisWalk nA;
            bool nameAnalysisResult = nA.walk(_root);
            if ( !nameAnalysisResult ) {
                throw Errors::ParseError();
            }

            return _root;
        }

        Lang::ProgramNode* targetASTSymbolicTyped() {
            targetASTSymbolic();

            Lang::Walk::TypeAnalysisWalk tA;
            bool typeAnalysisResult = tA.walk(_root);
            if ( !typeAnalysisResult ) {
                throw Errors::ParseError();
            }

            return _root;
        }

        void targetASTRepresentation(std::ostream& out) {
            targetASTSymbolicTyped();
            Lang::Walk::PrintWalk::print(out, _root);
        }

        ISA::Instructions* targetISA() {
            targetASTSymbolicTyped();

            Lang::Walk::ToISAWalk isaWalk;
            _isa = isaWalk.walk(_root);

            return _isa;
        }

        void targetISARepresentation(std::ostream& out) {
            targetISA();

            for ( auto instr : *_isa ) {
                out << instr->toString() << "\n";
            }
        }

        void targetCFGRepresentation(std::ostream& out) {
            CFG::ControlFlowGraph c(targetISA());

            c.serialize(out);
        }

    protected:
        std::istream* _input;
        Lang::Scanner* _scanner;
        Lang::Parser* _parser;
        Lang::ProgramNode* _root;
        ISA::Instructions* _isa;
    };

}

#endif
