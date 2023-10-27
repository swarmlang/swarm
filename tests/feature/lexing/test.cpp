#include "../../../src/lib/catch_amalgamated.h"

#include "../../../src/pipeline/Pipeline.h"

TEST_CASE("Lexing every .swarm token", "[frontend][lexing]") {
#include "input.h"
#include "output.h"

    std::istringstream inputStream(test_input);
    std::ostringstream outputStream;

    Framework::boot();

    swarmc::Pipeline pipeline(&inputStream);
    pipeline.targetTokenRepresentation(outputStream);

    auto diff = nslib::str::diffTrimmed(outputStream.str(), test_output);
    REQUIRE( diff == "" );
}

TEST_CASE("[WIP] Bad escape characters", "[frontend][lexing]") {
    std::istringstream inputStream(R"(string t3 = "\'\r\n\t\"\\" + "\";)");
    std::ostringstream outputStream;

    Framework::boot();
    nslib::Fatal::shouldThrow();

    swarmc::Pipeline pipeline(&inputStream);

    try {
        pipeline.targetTokenRepresentation(outputStream);
    } catch (nslib::FatalException& e) {
        REQUIRE( e.code() == 1 );
        REQUIRE( e.message() == R"(Lexing error, unexpected character " at position [1, 30])" );
        return;
    }

    REQUIRE( 1 == 0 );
}
