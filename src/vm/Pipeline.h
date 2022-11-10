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

    /**
     * Helper class for configuring instances of the Swarm runtime classes.
     */
    class Pipeline : public IStringable {
    public:
        Pipeline(std::istream* input) {
            _input = input;
            _parser = new ISA::Parser(*input);
        }

        virtual ~Pipeline() {
            delete _parser;
        }

        /** Get a list of loaded tokens from the SVI input stream. */
        std::vector<std::string> targetTokenStream() {
            return _parser->tokenize();
        }

        /** Get a list of parsed instructions from the SVI input stream. */
        ISA::Instructions targetInstructions() {
            return _parser->parse();
        }

        /** Print the loaded tokens from the SVI input stream to the given output stream. */
        void targetTokenRepresentation(std::ostream& out) {
            _parser->outputTokens(out);
        }

        /** Print the parsed instructions from the SVI input stream to the given output stream. */
        void targetISARepresentation(std::ostream& out) {
            _parser->outputParse(out);
        }

        /**
         * Parse the SVI input stream and load the instructions into a VirtualMachine configured with single-threaded drivers.
         * This is primarily used for testing/development via the `--locally` flag.
         */
        VirtualMachine* targetSingleThreaded() {
            auto is = targetInstructions();
            auto vm = new VirtualMachine(new SingleThreaded::GlobalServices());
            vm->addStore(new SingleThreaded::StorageInterface(ISA::Affinity::SHARED));
            vm->addStore(new SingleThreaded::StorageInterface(ISA::Affinity::LOCAL));
            vm->addQueue(new SingleThreaded::Queue(vm));
            vm->useStreamDriver(new SingleThreaded::StreamDriver());

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
