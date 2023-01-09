#ifndef SWARMVM_PIPELINE
#define SWARMVM_PIPELINE

#include <iostream>
#include <utility>
#include "../shared/nslib.h"
#include "isa_meta.h"
#include "ISAParser.h"
#include "runtime/single_threaded.h"
#include "VirtualMachine.h"
#include "prologue/prologue_provider.h"
#include "walk/ISABinaryWalk.h"
#include "walk/BinaryISAWalk.h"
#include "runtime/external.h"

using namespace nslib;

namespace swarmc::VM {

    using namespace Runtime;

    /**
     * Helper class for configuring instances of the Swarm runtime classes.
     */
    class Pipeline : public IStringable {
    public:
        explicit Pipeline(std::istream* input, bool isBinary = false) {
            _input = input;
            _parser = new ISA::Parser(*input);
            _isBinary = isBinary;
        }

        ~Pipeline() override {
            delete _parser;
        }

        void setExternalProviders(std::vector<std::string> p) {
            _externalProviders = std::move(p);
        }

        /** Get a list of loaded tokens from the SVI input stream. */
        std::vector<std::string> targetTokenStream() {
            if ( _isBinary ) {
                throw Errors::SwarmError("Cannot parse token stream from binary input.");
            }

            return _parser->tokenize();
        }

        /** Get a list of parsed instructions from the SVI input stream. */
        ISA::Instructions targetInstructions() {
            if ( _isBinary ) {
                return ISA::BinaryISAWalk::fromInput(*_input);
            }

            return _parser->parse();
        }

        /** Get a binary-serialized form of the given instructions. */
        binn* targetBinaryRepresentation() {
            if ( _isBinary ) {
                return ISA::BinaryISAWalk::readInput(*_input);
            }

            return ISA::ISABinaryWalk::serialize(targetInstructions(), nullptr);  // FIXME: nullptr for vm?
        }

        /** Print the loaded tokens from the SVI input stream to the given output stream. */
        void targetTokenRepresentation(std::ostream& out) {
            if ( _isBinary ) {
                throw Errors::SwarmError("Cannot output tokens from binary input source.");
            }

            _parser->outputTokens(out);
        }

        /** Print the parsed instructions from the SVI input stream to the given output stream. */
        void targetISARepresentation(std::ostream& out) {
            auto is = targetInstructions();
            for ( auto i : is ) {
                out << i->toString() << std::endl;
            }
            _parser->dispose(is);
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

            for ( const auto& path : _externalProviders ) {
                vm->addExternalProvider(path);
            }

            vm->initialize(is);
            return vm;
        }

        void targetInteractiveDebugger() {
            Debug::Debugger d;

            auto vm = targetSingleThreaded();  // TODO: support other targets for interactive debugging
            vm->attachDebugger(&d);

            Debug::Debugger::launchInteractive(vm);

            delete vm;
        }

        [[nodiscard]] std::string toString() const override {
            return "VM::Pipeline<>";
        }
    protected:
        std::istream* _input;
        ISA::Parser* _parser;
        bool _isBinary;
        std::vector<std::string> _externalProviders;
    };

}

#endif //SWARMVM_PIPELINE
