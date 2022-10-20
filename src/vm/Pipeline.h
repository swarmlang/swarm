#ifndef SWARMVM_PIPELINE
#define SWARMVM_PIPELINE

#include <iostream>
#include "../shared/IStringable.h"
#include "isa_meta.h"
#include "ISAParser.h"

namespace swarmc::VM {

    class Pipeline : public IStringable {
    public:
        Pipeline(std::istream* input) {
            _input = input;
            _parser = new ISA::Parser(*input);
        }

        virtual ~Pipeline() {
            delete _parser;
        }

        std::vector<std::string> targetTokenStream() {
            return _parser->tokenize();
        }

        ISA::Instructions targetInstructions() {
            return _parser->parse();
        }

        void targetTokenRepresentation(std::ostream& out) {
            _parser->outputTokens(out);
        }

        void targetISARepresentation(std::ostream& out) {
            _parser->outputParse(out);
        }

        std::string toString() const override {
            return "VM::Pipeline<>";
        }
    protected:
        std::istream* _input;
        ISA::Parser* _parser;
    };

}

#endif //SWARMVM_PIPELINE
