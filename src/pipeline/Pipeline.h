#ifndef SWARMC_PIPELINE_H
#define SWARMC_PIPELINE_H

#include <iostream>
#include <string>
#include <sstream>
#include "../shared/IStringable.h"
#include "../errors/ParseError.h"
#include "../lang/Scanner.h"
#include "../bison/grammar.hh"
#include "../lang/AST.h"
#include "../serialization/SerializeWalk.h"

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
        Pipeline(std::istream* input) {
            _input = input;
            _scanner = new Lang::Scanner(input);
            _root = nullptr;
            _parser = new Lang::Parser(*_scanner, &_root);
        }

        virtual ~Pipeline(){
            delete _scanner;
            delete _parser;
            delete _root;
        }

        virtual std::string toString() const override {
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

            bool nameAnalysisResult = _root->nameAnalysis();
            if ( !nameAnalysisResult ) {
                throw Errors::ParseError();
            }

            return _root;
        }

        Lang::ProgramNode* targetASTSymbolicTyped() {
            targetASTSymbolic();

            bool typeAnalysisResult = _root->typeAnalysis();
            if ( !typeAnalysisResult ) {
                throw Errors::ParseError();
            }

            return _root;
        }

        void targetASTRepresentation(std::ostream& out) {
            targetASTSymbolicTyped();
            _root->printTree(out);
        }

        void targetSerialOutput(std::ostream& out) {
            targetASTSymbolicTyped();

            Serialization::SerializeWalk walk;
            std::string json = walk.toJSON(_root);
            out << json;
        }

    protected:
        std::istream* _input;
        Lang::Scanner* _scanner;
        Lang::Parser* _parser;
        Lang::ProgramNode* _root;
    };

}

#endif
