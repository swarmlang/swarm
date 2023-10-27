#include "../../../src/lib/catch_amalgamated.h"

#include "../../../src/pipeline/Pipeline.h"

TEST_CASE("Parsing torture test", "[frontend][parsing]") {
#include "input.h"
#include "output.h"

    std::istringstream inputStream(test_input);
    std::ostringstream outputStream;

    Framework::boot();

    swarmc::Pipeline pipeline(&inputStream);
    pipeline.targetASTRepresentation(outputStream);

    auto diff = nslib::str::diffTrimmed(outputStream.str(), test_output);
    REQUIRE( diff == "" );
}
