#include <string>
#include <vector>
#include <chrono>
#include "../lib/json.hpp"
#include "Configuration.h"
#include "Executive.h"
#include "pipeline/Pipeline.h"
#include "errors/ParseError.h"
#include "test/Runner.h"
#include "vm/Pipeline.h"

int Executive::run(int argc, char **argv) {
    Framework::boot();

    // Set up the console. Enable debugging output, if we want:
    if ( Configuration::DEBUG ) {
        console->setVerbosity(nslib::Verbosity::VERBOSE);
        Logging::get()->setVerbosity(nslib::Verbosity::VERBOSE);
    }

    std::vector<std::string> params(argv + 1, argv + argc);

    logger->debug("Debugging output enabled.");

    // Load in the CLI args. Fail if necessary.
    if ( !this->parseArgs(params) ) {
        Framework::shutdown();
        return 1;
    }

    int result = 0;

    logger->debug("Got input file: " + inputFile);

    if ( flagRunTest ) {
        auto c = runTest();
        Framework::shutdown();
        return c;
    }

    if ( flagOutputTokens ) {
        int lexResult = debugOutputTokens();
        if ( lexResult != 0 ) {
            result = lexResult;
        }
    }

    if ( flagOutputParse ) {
        int parseResult = debugOutputParse();
        if ( parseResult != 0 ) {
            result = parseResult;
        }
    }

    if ( flagParseAndStop ) {
        int parseResult = debugParseAndStop();
        if ( parseResult != 0 ) {
            result = parseResult;
        }
    }

    if ( flagOutputISA ) {
        int isaResult = debugOutputISA();
        if ( isaResult != 0 ) {
            result = isaResult;
        }
    }

    if ( flagOutputCFG ) {
        int cfgResult = debugOutputCFG();
        if ( cfgResult != 0 ) {
            result = cfgResult;
        }
    }

    if ( (flagSingleThreaded || flagMultiThreaded) && flagSVI ) {
        int executeResult = executeLocalSVI(flagMultiThreaded);
        if ( executeResult != 0 ) {
            result = executeResult;
        }
    }

    if ( flagDebugger ) {
        swarmc::Runtime::Debug::Debugger::launchInteractive();
    }

    if ( flagOutputBinary ) {
        int binResult = emitBinary();
        if ( binResult != 0 ) {
            result = binResult;
        }
    }

    delete console;
    delete _input;
    Framework::shutdown();
    return result;
}

void Executive::cleanup() {
    Configuration::THREAD_EXIT = true;
}

// Parse the command line arguments and set up class properties
bool Executive::parseArgs(std::vector<std::string>& params) {

    bool failed = false;
    bool usage = false;
    bool gotInputFile = false;
    bool noInputFile = false;

    bool skipOne = false;
    for ( std::size_t i = 0; i < params.size(); i += 1 ) {
        if ( skipOne ) {
            skipOne = false;
            continue;
        }

        std::string arg = params.at(i);
        logger->debug("Parsing argument: " + arg);

        if ( arg == "--help" ) {
            usage = true;
        } else if ( failed ) {
            continue;
        } else if ( arg == "--dbg-output-tokens-to" ) {
            if ( i+1 >= params.size() ) {
                logger->error("Missing required parameter for --dbg-output-tokens-to. Pass --help for more info.");
                failed = true;
                continue;
            }

            std::string outfile = params.at(i+1);
            logger->debug("Token output file: " + outfile);

            flagOutputTokens = true;
            flagOutputTokensTo = outfile;
            skipOne = true;
        } else if ( arg == "--dbg-parse" ) {
            flagParseAndStop = true;
        } else if ( arg == "--dbg-output-parse-to" ) {
            if ( i+1 >= params.size() ) {
                logger->error("Missing required parameter for --dbg-output-parse-to. Pass --help for more info.");
                failed = true;
                continue;
            }

            std::string outfile = params.at(i+1);
            logger->debug("Parse output file: " + outfile);

            flagOutputParse = true;
            flagOutputParseTo = outfile;
            skipOne = true;
        } else if ( arg == "--debugger" ) {
            flagDebugger = true;
            noInputFile = true;
        } else if ( arg == "--interactive-debug" ) {
            flagInteractiveDebug = true;
            flagSingleThreaded = true;
            logger->warn("--interactive-debug implies --locally. Will use single-threaded runtime drivers.");
        } else if ( arg == "--run-test" ) {
            if ( i+1 >= params.size() ) {
                logger->error("Missing required parameter for --run-test. Pass --help for more info.");
                failed = true;
                continue;
            }


            std::string name = params.at(i+1);
            logger->debug("Will run test: " + name);

            flagRunTest = true;
            flagRunTestName = name;
            skipOne = true;
            noInputFile = true;
        } else if ( arg == "--external-provider" ) {
            if ( i+1 >= params.size() ) {
                logger->error("Missing required parameter for --external-provider. Pass --help for more info.");
                failed = true;
                continue;
            }

            std::string path = params.at(i+1);
            logger->debug("Will load external provider from: " + path);
            externalProviders.push_back(path);
            skipOne = true;
        } else if ( arg == "--binary" ) {
            if ( i+1 >= params.size() ) {
                logger->error("Missing required parameter for --binary. Pass --help for more info.");
                failed = true;
                continue;
            }

            std::string name = params.at(i+1);
            logger->debug("Will output binary to: " + name);
            flagOutputBinary = true;
            outputBinaryTo = name;
            skipOne = true;
        } else if ( arg == "--locally" ) {
            Configuration::FORCE_LOCAL = true;
            flagSingleThreaded = !flagMultiThreaded;
            logger->debug("Will execute locally.");
        } else if ( arg == "--locally-multithreaded" ) {
            flagMultiThreaded = true;
            flagSingleThreaded = false;
            logger->debug("Will execute multithreaded");
        } else if ( arg == "--verbose" ) {
            flagVerbose = true;
            Configuration::DEBUG = true;
            Configuration::VERBOSE = true;
            console->setVerbosity(nslib::Verbosity::VERBOSE);
            Logging::get()->setVerbosity(nslib::Verbosity::VERBOSE);
        } else if ( arg == "--output-to" ) {
            if ( i+1 >= params.size() ) {
                logger->error("Missing required parameter for --output-to. Pass --help for more info.");
                failed = true;
                continue;
            }

            std::string name = params.at(i+1);
            logger->debug("Will output results to: " + name);
            outputResultTo = name;
        } else if ( arg == "--dbg-use-d-guid" ) {
            nslib::priv::USE_DETERMINISTIC_UUIDS = true;
        } else if ( arg == "--svi" ) {
            flagSVI = true;
        } else if ( arg == "--without-prologue" ) {
            flagWithPrologue = false;
            Configuration::WITH_PROLOGUE = false;
        } else if ( arg == "--log-target" ) {
            if ( i+1 >= params.size() ) {
                logger->error("Missing required parameter for --log-target. Pass --help for more info.");
                failed = true;
                continue;
            }

            std::string name = params.at(i+1);
            Logging::get()->onlyEnabledLoggers();
            Logging::get()->configureLoggerTag(name);
            skipOne = true;
        } else if ( arg == "--log-file" ) {
            if ( i+1 >= params.size() ) {
                logger->error("Missing required parameter for --log-file. Pass --help for more info.");
                failed = true;
                continue;
            }

            std::string path = params.at(i+1);
            Logging::get()->addTarget(new LogFileTarget(path));
            skipOne = true;
        } else if ( arg == "--dbg-output-isa-to" ) {
            if ( i+1 >= params.size() ) {
                logger->error("Missing required parameter for --debug-output-isa-to. Pass --help for more info.");
                failed = true;
                continue;
            }

            outputISATo = params.at(i+1);
            logger->debug("Output file name: " + outputISATo);
            skipOne = true;
            flagOutputISA = true;
        } else if ( arg == "--dbg-output-cfg-to" ) {
            if ( i+1 >= params.size() ) {
                logger->error("Missing required parameter for --debug-output-cfg-to. Pass --help for more info.");
                failed = true;
                continue;
            }

            outputCFGTo = params.at(i+1);
            logger->debug("Output file name: " + outputCFGTo);
            skipOne = true;
            flagOutputCFG = true;
        } else if ( arg == "--no-remove-self-assigns" ) {
            flagRemoveSelfAssign = false;
        } else if ( arg == "--no-constant-propagation" ) {
            flagConstProp = false;
        } else {
            // Is this the input file?
            if ( gotInputFile ) {
                logger->error("Unknown or invalid argument: " + arg + ". Pass --help for more info.");
                failed = true;
                continue;
            }

            gotInputFile = true;
            inputFile = arg;
        }
    }

    if ( !failed && !(gotInputFile || noInputFile) ) {
        logger->error("Missing input file argument. Pass --help for more info.");
        failed = true;
    }

    if ( usage ) {
        printUsage();
    }

    /** Try to alloc and open the input file. */
    if ( !noInputFile ) {
        _input = new std::ifstream(inputFile);
        if ( _input->bad() ) {
            logger->error("Could not open input file: " + inputFile);

            delete _input;
            _input = nullptr;

            failed = true;
        }
    }

    return !failed && !usage;
}

void Executive::printUsage() {
    console->bold()->print("swarmc")->reset()->println(" - compiler/interpreter for the swarm language");
    console->bold()->print("USAGE: ")->reset()->println("swarmc [...OPTIONS] <INPUT FILE>");
    console->println();
    console->bold()->println("OPTIONS:");

    console->bold()->print("  --help  :  ", true)
        ->println("Show usage information and exit.")
        ->println();

    console->bold()->print("  --locally  :  ", true)
        ->println("Evaluate the given Swarm program without connecting to remote executors.")
        ->println();

    console->bold()->print("  --locally-multithreaded", true)
        ->println("Evaluate the given Swarm program without connecting to remote executors, but multithreaded.")
        ->println();

    console->bold()->print("  --binary  :  ", true)
        ->println("Compile the given program and output the binary at the specified file.")
        ->println();

    console->bold()->print("  --verbose  :  ", true)
        ->println("Enable verbose/debugging output.")
        ->println();

    console->bold()->print("  --clear-queue  :  ", true)
        ->println("Force clear any jobs in the Redis queue.")
        ->println();

    console->bold()->print("  --work-queue <FILTERS> :  ", true)
        ->println("Listen for and execute jobs in the Redis queue, with the filters from the given file. ")
        ->println();

    console->bold()->print("  --output-to <OUTFILE>  :  ", true)
        ->println("Print the result of the evaluation to the given file.")
        ->println("                                       If the output file is \"--\", will write to stdout (default).")
        ->println();

    console->bold()->print("  --run-test <NAME>  :  ", true)
        ->println("Run the C++-based test with the given name.")
        ->println();

    console->bold()->print("  --svi  :  ", true)
        ->println("Read the input file as SVI code.")
        ->println();

    console->bold()->print("  --without-prologue  :  ", true)
        ->println("Exclude the Prologue provider from the runtime")
        ->println();

    console->bold()->print("  --external-provider <PATH>  :  ", true)
        ->println("Load an external provider from the given path into the runtime")
        ->println();

    console->bold()->print("  --log-target <NAME>  :  ", true)
            ->println("Enable a logging target. This flag can be specified multiple times")
            ->println();

    console->bold()->print("  --log-file <PATH>  :  ", true)
            ->println("Enable logging to the specified file")
            ->println();

    console->debug();
        console->bold()->print("  --dbg-output-tokens-to <OUTFILE>  :  ", true)
            ->println("Lex the input and output the tokens to the specified file.")
            ->println("                                       If the output file is \"--\", will write to stdout.")
            ->println();

        console->bold()->print("  --dbg-parse  :  ", true)
            ->println("Lex and parse the input, then stop.")
            ->println();

        console->bold()->print("  --dbg-output-parse-to <OUTFILE>  :  ", true)
            ->println("Parse the input and output the AST to the specified file.")
            ->println("                                      If the output file is \"--\", will write to stdout.")
            ->println();

        console->bold()->print("  --dbg-use-d-guid  :  ", true)
            ->println("Use deterministic UUIDs (for test output).")
            ->println();

        console->bold()->print("  --dbg-output-isa-to <OUTFILE> :  ", true)
            ->println("Set the name of the ISA output file")
            ->println();

        console->bold()->print("  --dbg-output-cfg-to <OUTFILE> :  ", true)
            ->println("Set the name of the CFG output file")
            ->println();

        console->bold()->print("  --no-remove-self-assigns: ", true)
            ->println("Disable self-assignment removal in the ISA")
            ->println();

        console->bold()->print("  --no-constant-propagation: ", true)
            ->println("Disable constant propagation in the ISA")
            ->println();
    console->end();

    console->bold()->println("FILTERS:")
        ->println("Workers can have key-value filters applied to them so that tagged code regions can be")
        ->println("selectively executed on nodes matching the appropriate filters.")
        ->println()
        ->println("To do this, pass the path to a JSON file containing an object of string-string mappings to")
        ->println("the --work-queue flag.")
        ->println();

    // Output in debugging mode only:
    console->debug()
            ->println("Built with debugging enabled.")
        ->end();
}

int Executive::debugOutputTokens() {
    std::ostream* stream = nullptr;

    if ( flagOutputTokensTo == "--" ) {
        stream = &std::cout;
    } else {
        stream = new std::ofstream(flagOutputTokensTo);
        if ( stream->bad() ) {
            logger->error("Could not open token output file for writing: " + flagOutputTokensTo);
            delete stream;
            return 1;
        }
    }

    if ( flagSVI ) {
        swarmc::VM::Pipeline pipeline(_input);
        pipeline.targetTokenRepresentation(*stream);
    } else {
        swarmc::Pipeline pipeline(_input);
        pipeline.targetTokenRepresentation(*stream);
    }

    if ( stream != &std::cout ) delete stream;
    return 0;
}

int Executive::debugOutputParse() {
    std::ostream* stream;
    if ( flagOutputParseTo == "--" ) {
        stream = &std::cout;
    } else {
        stream = new std::ofstream(flagOutputParseTo);
        if ( stream->bad() ) {
            logger->error("Could not open parse output file for writing: " + flagOutputParseTo);
            delete stream;
            return 1;
        }
    }

    if ( flagSVI ) {
        try {
            swarmc::VM::Pipeline pipeline(_input);
            pipeline.targetISARepresentation(*stream);
        } catch (swarmc::Errors::SwarmError& e) {
            logger->error(e.what());
            return 1;
        }
    } else {
        try {
            swarmc::Pipeline pipeline(_input);
            pipeline.targetASTRepresentation(*stream);
        } catch (swarmc::Errors::ParseError& e) {
            return e.exitCode;
        }
    }

    logger->success("Parsed input program.");
    if ( stream != &std::cout ) delete stream;
    return 0;
}

int Executive::debugParseAndStop() {
    swarmc::Pipeline pipeline(_input);

    try {
        pipeline.targetASTSymbolic();
    } catch (swarmc::Errors::ParseError& e) {
        return e.exitCode;
    }

    logger->success("Parsed input program.");
    return 0;
}

int Executive::runTest() {
    swarmc::Test::Runner runner;
    bool result = runner.run(flagRunTestName);
    if ( result ) {
        logger->success("Test passed: " + flagRunTestName);
        return 0;
    }

    logger->error("Test failed: " + flagRunTestName);
    return 1;
}

int Executive::parseFilters() {
    std::string filters, temp;
    std::ifstream input = std::ifstream(flagFilterFile);
    while (input >> temp) filters += temp;

    Configuration::QUEUE_FILTERS = nlohmann::json::parse(filters);
    return 0;
}

int Executive::debugOutputISA() {
    std::ostream* stream;
    if ( outputISATo == "--" ) {
        stream = &std::cout;
    } else {
        stream = new std::ofstream(outputISATo);
        if ( stream->bad() ) {
            logger->error("Could not open parse output file for writing: " + outputISATo);
            delete stream;
            return 1;
        }
    }

    swarmc::Pipeline pipeline(_input);

    try {
        pipeline.targetISARepresentation(*stream, flagRemoveSelfAssign, flagConstProp);
    } catch (swarmc::Errors::ParseError& e) {
        return e.exitCode;
    }

    logger->success("Compiled to ISA.");
    return 0;
}

int Executive::debugOutputCFG() {
    std::ostream* stream;
    if ( outputCFGTo == "--" ) {
        stream = &std::cout;
    } else {
        stream = new std::ofstream(outputCFGTo);
        if ( stream->bad() ) {
            logger->error("Could not open parse output file for writing: " + outputCFGTo);
            delete stream;
            return 1;
        }
    }

    swarmc::Pipeline pipeline(_input);

    try {
        pipeline.targetCFGRepresentation(*stream, flagRemoveSelfAssign, flagConstProp);
    } catch (swarmc::Errors::ParseError& e) {
        return e.exitCode;
    }

    logger->success("Output CFG.");
    return 0;
}

int Executive::executeLocalSVI(bool multithreaded) {
    swarmc::VM::Pipeline pipeline(_input);
    pipeline.setExternalProviders(externalProviders);

    if ( flagInteractiveDebug ) {
        pipeline.targetInteractiveDebugger();
        return 0;
    }

    auto vm = multithreaded ? pipeline.targetMultiThreaded() : pipeline.targetSingleThreaded();
    vm->execute();
    vm->cleanup();
    delete vm;
    return 0;
}

int Executive::emitBinary() {
    if ( flagSVI ) {
        swarmc::VM::Pipeline pipeline(_input);
        auto fh = fopen(outputBinaryTo.c_str(), "w");
        if ( fh == nullptr ) {
            logger->error("Could not open binary output file for writing: " + outputBinaryTo);
            return 1;
        }

        auto binary = pipeline.targetBinaryRepresentation();
        fwrite(binn_ptr(binary), binn_size(binary), 1, fh);
        fclose(fh);
        return 0;
    }

    // FIXME: compile Swarm source to SVI, then emit as binary
    return 1;
}
