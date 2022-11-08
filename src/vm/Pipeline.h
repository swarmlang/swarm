#ifndef SWARMVM_PIPELINE
#define SWARMVM_PIPELINE

#include <iostream>
#include "../shared/IStringable.h"
#include "isa_meta.h"
#include "ISAParser.h"
#include "runtime/single_threaded.h"
#include "VirtualMachine.h"
#include "prologue/prologue_provider.h"

namespace swarmc::VM {

    using namespace Runtime;

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

        VirtualMachine* targetSingleThreaded() {
            auto is = targetInstructions();
            auto vm = new VirtualMachine(new SingleThreaded::GlobalServices());
            vm->addStore(new SingleThreaded::StorageInterface(ISA::Affinity::SHARED));
            vm->addStore(new SingleThreaded::StorageInterface(ISA::Affinity::LOCAL));
            vm->addQueue(new SingleThreaded::Queue(vm));

            if ( Configuration::WITH_PROLOGUE ) {
                vm->addProvider(new Prologue::Provider(vm->global()));
            }

            vm->initialize(is);
            return vm;
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
