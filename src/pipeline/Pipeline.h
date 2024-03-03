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
#include "../vm/walk/ISABinaryWalk.h"
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

    enum ISAOptimizationType {
        REMOVESELFASSIGN = 0x1,
        CONSTANTPROPAGATION = 0x2,
        REMOVEDEADCODE = 0x4,
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
            delete _root;
            delete _isa;
        }

        [[nodiscard]] std::string toString() const override {
            return "Pipeline<>";
        }

        void setISAOptimizationLevel(unsigned int flags, bool b) {
            if ( flags & REMOVESELFASSIGN ) flagRemoveSelfAssigns = b;
            if ( flags & CONSTANTPROPAGATION ) flagConstantPropagation = b;
            if ( flags & REMOVEDEADCODE ) flagRemoveDeadCode = b;
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
            delete _root;
            _root = nullptr;

            return _isa;
        }

        void targetISAOptimized() {
            CFG::ControlFlowGraph c(targetISA());

            c.optimize(flagRemoveSelfAssigns, flagConstantPropagation);

            auto isa = c.reconstruct();

            delete _isa;
            _isa = isa;
        }

        void targetISARepresentation(std::ostream& out) {
            targetISAOptimized();

            for ( auto instr : *_isa ) {
                out << instr->toString() << std::endl;
            }
        }

        void targetCFGRepresentation(std::ostream& out) {
            CFG::ControlFlowGraph c(targetISA());

            c.optimize(flagRemoveSelfAssigns, flagConstantPropagation);

            c.serialize(out);

            auto isa = c.reconstruct();

            delete _isa;
            _isa = isa;
        }

        binn* targetBinary() {
            targetISAOptimized();
            return swarmc::ISA::ISABinaryWalk::serialize(*_isa, nullptr);
        }

    protected:
        std::istream* _input;
        Lang::Scanner* _scanner;
        Lang::Parser* _parser;
        Lang::ProgramNode* _root;
        ISA::Instructions* _isa;
        bool flagRemoveSelfAssigns = true;
        bool flagConstantPropagation = true;
        bool flagRemoveDeadCode = true;
    };

}

#endif
