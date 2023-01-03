#include <climits>
#include "Executive.h"

int main(int argc, char** argv) {
    Executive executive;
    auto code = executive.run(argc, argv);
    executive.cleanup();
    return code;
}

