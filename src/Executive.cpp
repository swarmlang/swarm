#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <chrono>
#include "Configuration.h"
#include "Executive.h"
#include "pipeline/Pipeline.h"
#include "errors/ParseError.h"
#include "test/Runner.h"
#include "runtime/queue/Waiter.h"

int Executive::run(int argc, char **argv) {
    // Set up the console. Enable debugging output, if we want:
    if ( Configuration::DEBUG ) {
        console->verbose();
    }

    auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    util::generator.seed(epoch);

    std::vector<std::string> params(argv + 1, argv + argc);

    console->debug("Debugging output enabled.");

    // Load in the CLI args. Fail if necessary.
    if ( !this->parseArgs(params) ) return 1;

    int result = 0;

    console->debug("Got input file: " + inputFile);

    if ( flagRunTest ) {
        return runTest();
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

    if ( flagOutputSerialize ) {
        int serializeResult = debugOutputSerialize();
        if ( serializeResult != 0 ) {
            result = serializeResult;
        }
    }

    if ( flagOutputDeSerialize ) {
        int deserializeResult = debugOutputDeSerialize();
        if ( deserializeResult != 0 ) {
            result = deserializeResult;
        }
    }

    int interpretResult = interpret();
    if ( interpretResult != 0 ) {
        result = interpretResult;
    }

    delete console;
    if ( _input != nullptr ) delete _input;
    return result;
}

void Executive::cleanup() {
    Configuration::THREAD_EXIT = true;
    swarmc::Runtime::Waiter::join();
}

// Parse the command line arguments and set up class properties
bool Executive::parseArgs(std::vector<std::string>& params) {

    bool failed = false;
    bool usage = false;
    bool gotInputFile = false;
    bool noInputFile = false;

    bool skipOne = false;
    for ( size_t i = 0; i < params.size(); i += 1 ) {
        if ( skipOne ) {
            skipOne = false;
            continue;
        }

        std::string arg = params.at(i);
        console->debug("Parsing argument: " + arg);

        if ( arg == "--help" ) {
            usage = true;
        } else if ( failed ) {
            continue;
        } else if ( arg == "--dbg-output-tokens-to" ) {
            if ( i+1 >= params.size() ) {
                console->error("Missing required parameter for --dbg-output-tokens-to. Pass --help for more info.");
                failed = true;
                continue;
            }

            std::string outfile = params.at(i+1);
            console->debug("Token output file: " + outfile);

            flagOutputTokens = true;
            flagOutputTokensTo = outfile;
            skipOne = true;
        } else if ( arg == "--dbg-parse" ) {
            flagParseAndStop = true;
        } else if ( arg == "--dbg-output-parse-to" ) {
            if ( i+1 >= params.size() ) {
                console->error("Missing required parameter for --dbg-output-parse-to. Pass --help for more info.");
                failed = true;
                continue;
            }

            std::string outfile = params.at(i+1);
            console->debug("Parse output file: " + outfile);

            flagOutputParse = true;
            flagOutputParseTo = outfile;
            skipOne = true;
        } else if ( arg == "--dbg-output-serialize-to" ) {
            if ( i+1 >= params.size() ) {
                console->error("Missing required parameter for --dbg-output-serialize-to. Pass --help for more info.");
                failed = true;
                continue;
            }

            std::string outfile = params.at(i+1);
            console->debug("Serialized AST output file: " + outfile);

            flagOutputSerialize = true;
            flagOutputSerializeTo = outfile;
            skipOne = true;
        } else if ( arg == "--dbg-output-deserialize-to" ) {
            if ( i+1 >= params.size() ) {
                console->error("Missing required parameter for --dbg-output-deserialize-to. Pass --help for more info.");
                failed = true;
                continue;
            }

            std::string outfile = params.at(i+1);
            console->debug("DeSerialized AST output file: " + outfile);

            flagOutputDeSerialize = true;
            flagOutputDeSerializeTo = outfile;
            skipOne = true;
        } else if ( arg == "--run-test" ) {
            if ( i+1 >= params.size() ) {
                console->error("Missing required parameter for --run-test. Pass --help for more info.");
                failed = true;
                continue;
            }


            std::string name = params.at(i+1);
            console->debug("Will run test: " + name);

            flagRunTest = true;
            flagRunTestName = name;
            skipOne = true;
            noInputFile = true;
        } else if ( arg == "--locally" ) {
            flagInterpretLocally = true;
            Configuration::FORCE_LOCAL = true;
            console->debug("Will interpret locally.");
        } else if ( arg == "--output-to" ) {
            if ( i+1 >= params.size() ) {
                console->error("Missing required parameter for --run-test. Pass --help for more info.");
                failed = true;
                continue;
            }

            std::string name = params.at(i+1);
            console->debug("Will output results to: " + name);
            outputResultTo = name;
        } else if ( arg == "--dbg-use-d-guid" ) {
            util::USE_DETERMINISTIC_UUIDS = true;
        } else {
            // Is this the input file?
            if ( gotInputFile ) {
                console->error("Unknown or invalid argument: " + arg + ". Pass --help for more info.");
                failed = true;
                continue;
            }

            gotInputFile = true;
            inputFile = arg;
        }
    }

    if ( !failed && !(gotInputFile || noInputFile) ) {
        console->error("Missing input file argument. Pass --help for more info.");
        failed = true;
    }

    if ( usage ) {
        printUsage();
    }

    /** Try to alloc and open the input file. */
    _input = new std::ifstream(inputFile);
    if ( _input->bad() ) {
        console->error("Could not open input file: " + inputFile);

        delete _input;
        _input = nullptr;

        failed = true;
    }

    return !failed && !usage;
}

void Executive::printUsage() {
    console->bold()->print("swarmc")->reset()->line(" - compiler for the swarm language");
    console->bold()->print("USAGE: ")->reset()->line("swarmc [...OPTIONS] <INPUT FILE>");
    console->line();
    console->bold()->line("OPTIONS:");

    console->bold()->print("  --help  :  ", true)
        ->line("Show usage information and exit.")
        ->line();

    console->bold()->print("  --locally  :  ", true)
        ->line("Evaluate the given Swarm program without connecting to remote executors.")
        ->line();

    console->bold()->print("  --output-to <OUTFILE>  :  ", true)
        ->line("Print the result of the evaluation to the given file.")
        ->line("                                       If the output file is \"--\", will write to stdout (default).")
        ->line();

    console->bold()->print("  --run-test <NAME>  :  ", true)
        ->line("Run the C++-based test with the given name.")
        ->line();

    console->debug();
        console->bold()->print("  --dbg-output-tokens-to <OUTFILE>  :  ", true)
            ->line("Lex the input and output the tokens to the specified file.")
            ->line("                                       If the output file is \"--\", will write to stdout.")
            ->line();

        console->bold()->print("  --dbg-parse  :  ", true)
            ->line("Lex and parse the input, then stop.")
            ->line();

        console->bold()->print("  --dbg-output-parse-to <OUTFILE>  :  ", true)
            ->line("Parse the input and output the AST to the specified file.")
            ->line("                                      If the output file is \"--\", will write to stdout.")
            ->line();

        console->bold()->print("  --dbg-output-serialize-to <OUTFILE>  :  ", true)
            ->line("Parse the input and output the serialized AST to the specified file.")
            ->line("                                      If the output file is \"--\", will write to stdout.")
            ->line();

        console->bold()->print("  --dbg-output-deserialize-to <OUTFILE>  :  ", true)
            ->line("Parse the input and output the deserialized AST to the specified file.")
            ->line("                                      If the output file is \"--\", will write to stdout.")
            ->line();

        console->bold()->print("  --dbg-use-d-guid  :  ", true)
            ->line("Use deterministic UUIDs (for test output).")
            ->line();
    console->end();

    // Output in debugging mode only:
    console->debug()
            ->line("Built with debugging enabled.")
        ->end();
}

int Executive::debugOutputTokens() {
    swarmc::Pipeline pipeline(_input);
    std::ostream* stream = nullptr;

    if ( flagOutputTokensTo == "--" ) {
        stream = &std::cout;
    } else {
        stream = new std::ofstream(flagOutputTokensTo);
        if ( stream->bad() ) {
            console->error("Could not open token output file for writing: " + flagOutputTokensTo);
            delete stream;
            return 1;
        }
    }

    pipeline.targetTokenRepresentation(*stream);
    if ( stream != &std::cout ) delete stream;
    return 0;
}

int Executive::debugOutputParse() {
    swarmc::Pipeline pipeline(_input);
    std::ostream* stream = nullptr;

    if ( flagOutputParseTo == "--" ) {
        stream = &std::cout;
    } else {
        stream = new std::ofstream(flagOutputParseTo);
        if ( stream->bad() ) {
            console->error("Could not open parse output file for writing: " + flagOutputParseTo);
            delete stream;
            return 1;
        }
    }

    try {
        pipeline.targetASTRepresentation(*stream);
    } catch (swarmc::Errors::ParseError& e) {
        return e.exitCode;
    }

    console->success("Parsed input program.");
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

    console->success("Parsed input program.");
    return 0;
}

int Executive::debugOutputSerialize() {
    swarmc::Pipeline pipeline(_input);
    std::ostream* stream = nullptr;

    if ( flagOutputSerializeTo == "--" ) {
        stream = &std::cout;
    } else {
        stream = new std::ofstream(flagOutputSerializeTo);
        if ( stream->bad() ) {
            console->error("Could not open serialize output file for writing: " + flagOutputSerializeTo);
            delete stream;
            return 1;
        }
    }
    
    try {
        pipeline.targetSerialOutput(*stream);
    } catch (swarmc::Errors::ParseError& e) {
        return e.exitCode;
    }

    console->success("Serialized input program.");
    if ( stream != &std::cout ) delete stream;
    return 0;
}

int Executive::debugOutputDeSerialize() {
    swarmc::Pipeline pipeline(_input);
    std::ostream* stream = nullptr;

    if ( flagOutputDeSerializeTo == "--" ) {
        stream = &std::cout;
    } else {
        stream = new std::ofstream(flagOutputDeSerializeTo);
        if ( stream->bad() ) {
            console->error("Could not open deserialize output file for writing: " + flagOutputDeSerializeTo);
            delete stream;
            return 1;
        }
    }
    
    try {
        pipeline.targetDeSerialOutput(*stream);
    } catch (swarmc::Errors::ParseError& e) {
        return e.exitCode;
    }

    console->success("Deserialized input program.");
    if ( stream != &std::cout ) delete stream;
    return 0;
}

int Executive::runTest() {
    swarmc::Test::Runner runner;
    bool result = runner.run(flagRunTestName);
    if ( result ) {
        console->success("Test passed: " + flagRunTestName);
        return 0;
    }

    console->error("Test failed: " + flagRunTestName);
    return 1;
}

int Executive::interpret() {
    try {
        swarmc::Pipeline pipeline(_input);
        pipeline.targetEvaluate();
        return 0;
    } catch (...) {
        return 1;
    }
}
