#ifndef SWARMVM_PIPELINE
#define SWARMVM_PIPELINE

#include <iostream>
#include <utility>
#include "../shared/nslib.h"
#include "isa_meta.h"
#include "ISAParser.h"
#include "runtime/single_threaded.h"
#include "runtime/multi_threaded.h"
#include "runtime/redis_driver.h"
#include "VirtualMachine.h"
#include "prologue/prologue_provider.h"
#include "walk/ISABinaryWalk.h"
#include "walk/BinaryISAWalk.h"
#include "runtime/external.h"
#include "runtime/Worker.h"

using namespace nslib;

namespace swarmc::VM {

    using namespace Runtime;

    /**
     * Helper class for configuring instances of the Swarm runtime classes.
     */
    class Pipeline : public IStringable {
    public:
        Pipeline() : _input(nullptr), _parser(nullptr), _isBinary(false) {}
        explicit Pipeline(std::istream* input) {
            _input = input;
            _parser = new ISA::Parser(*input);
            std::string header = "\x7fSVI";
            for (int i = 0; i < 4; i++) {
                input->seekg(i, std::istream::beg);
                _isBinary = input->peek() == header[i];
            }
            input->seekg(0, std::istream::beg);
        }

        ~Pipeline() override {
            delete _parser;
        }

        void setExternalProviders(std::vector<std::string> p) {
            _externalProviders = std::move(p);
        }

        /** Get a list of loaded tokens from the SVI input stream. */
        std::vector<std::string> targetTokenStream() {
            if ( _input == nullptr ) {
                throw Errors::SwarmError("Cannot parse token stream without an input");
            }

            if ( _isBinary ) {
                throw Errors::SwarmError("Cannot parse token stream from binary input.");
            }

            return _parser->tokenize();
        }

        /** Get a list of parsed instructions from the SVI input stream. */
        ISA::Instructions targetInstructions() {
            if ( _input == nullptr ) {
                throw Errors::SwarmError("Cannot parse ISA without an input to parse.");
            }

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
            if ( _input == nullptr ) {
                throw Errors::SwarmError("Cannot output tokens without an input.");
            }

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
            vm->addQueue(new SingleThreaded::Queue());
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

        VirtualMachine* targetMultiThreaded() {
            auto is = targetInstructions();
            auto vm = new VirtualMachine(new MultiThreaded::GlobalServices());
            vm->addStore(new MultiThreaded::SharedStorageInterface());
            vm->addStore(new SingleThreaded::StorageInterface(ISA::Affinity::LOCAL));
            vm->addQueue(new MultiThreaded::Queue(vm));
            vm->useStreamDriver(new MultiThreaded::StreamDriver());

            if ( Configuration::WITH_PROLOGUE ) {
                vm->addProvider(new Prologue::Provider(vm->global()));
            }

            for ( const auto& path : _externalProviders ) {
                vm->addExternalProvider(path);
            }

            vm->initialize(is);
            return vm;
        }

        VirtualMachine* targetRedis() {
            auto is = targetInstructions();
            auto vm = new VirtualMachine(new RedisDriver::GlobalServices());
            vm->addStore(new RedisDriver::RedisStorageInterface(vm));
            vm->addStore(new SingleThreaded::StorageInterface(ISA::Affinity::LOCAL));
            auto rq = new RedisDriver::RedisQueue(vm);
            vm->addQueue(rq);
            vm->useStreamDriver(new RedisDriver::RedisStreamDriver(vm));

            if ( Configuration::WITH_PROLOGUE ) {
                vm->addProvider(new Prologue::Provider(vm->global()));
            }

            for ( const auto& path : _externalProviders ) {
                vm->addExternalProvider(path);
            }

            vm->initialize(is);
            rq->initialize();
            return vm;
        }

        void targetRedisWorker() {
            Framework::registerSignalHandler(SIGINT, [](int signal) -> void {
                Configuration::THREAD_EXIT = true;
            });

            auto vm = new VirtualMachine(new RedisDriver::GlobalServices());
            vm->addStore(new RedisDriver::RedisStorageInterface(vm));
            auto rq = new RedisDriver::RedisQueue(vm);
            vm->addQueue(rq);
            vm->useStreamDriver(new RedisDriver::RedisStreamDriver(vm));

            if ( Configuration::WITH_PROLOGUE ) {
                vm->addProvider(new Prologue::Provider(vm->global()));
            }

            for ( const auto& path : _externalProviders ) {
                vm->addExternalProvider(path);
            }

            vm->initializeWorker();
            rq->initialize();
            Worker worker(vm->global(), rq);
            worker.wait();
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
