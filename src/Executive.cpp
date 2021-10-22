#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "Executive.h"
#include "lang/Scanner.h"

int Executive::run(int argc, char **argv) {
    // Set up the console. Enable debugging output, if we want:
#ifdef SWARM_DEBUG
        console->verbose();
#endif

    std::vector<std::string> params(argv + 1, argv + argc);

    console->debug("Debugging output enabled.");

    // Load in the CLI args. Fail if necessary.
    if ( !this->parseArgs(params) ) return 1;

    int result = 0;

    console->debug("Got input file: " + inputFile);

    if ( flagOutputTokens ) {
        result = debugOutputTokens();
    }

    delete console;
    if ( _input != nullptr ) delete _input;
    return result;
}

// Parse the command line arguments and set up class properties
bool Executive::parseArgs(std::vector<std::string>& params) {

    bool failed = false;
    bool usage = false;
    bool gotInputFile = false;

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

    if ( !failed && !gotInputFile ) {
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

    console->bold()->print("  --dbg-output-tokens-to <OUTFILE>  :  ", true)
        ->line("Lex the input and output the tokens to the specified file.")
        ->line("                                       If the output file is \"--\", will write to stdout.")
        ->line();
    
    // Output in debugging mode only:
    console->debug()
            ->line("Built with debugging enabled.")
        ->end();
}

int Executive::debugOutputTokens() {
    swarmc::Lang::Scanner scanner(_input);

    if ( flagOutputTokensTo == "--" ) {
        scanner.outputTokens(std::cout);
    } else {
        std::ofstream outfile(flagOutputTokensTo);
        if ( outfile.bad() ) {
            console->error("Could not open token output file for writing: " + flagOutputTokensTo);
            return 1;
        }

        scanner.outputTokens(outfile);
    }

    return 0;
}
