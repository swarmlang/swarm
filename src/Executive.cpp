#include <fstream>
#include "Executive.h"

int Executive::run(int argc, char **argv) {
    // Set up the console. Enable debugging output, if we want:
#ifdef SWARM_DEBUG
        console->verbose();
#endif

    console->debug("Debugging output enabled.");

    // Load in the CLI args. Fail if necessary.
    if ( !this->parse_args(argc, argv) ) return 1;

    return 0;
}

// Parse the command line arguments and set up class properties
bool Executive::parse_args(int argc, char **argv) {
    return true;
}
