#ifndef SWARM_044_BINARY_H
#define SWARM_044_BINARY_H

#include <fstream>
#include <sstream>
#include <filesystem>
#include "Test.h"
#include "../vm/isa_meta.h"
#include "../vm/Pipeline.h"
#include "../vm/walk/BinaryISAWalk.h"
#include "../vm/walk/ISABinaryWalk.h"

namespace swarmc::Test {

    class BinarySerializeTest : public Test {
    public:
        BinarySerializeTest() : Test() {}

        bool run() override {
            console->only(nslib::Verbosity::VERBOSE);
            Configuration::VERBOSE = true;
            Configuration::DEBUG = true;

            auto exePath = std::filesystem::canonical("/proc/self/exe").parent_path() / "test/044-binary/test.svi";
            std::ifstream testCode(exePath);
            if ( testCode.bad() ) {
                console->error("Unable to open SVI code for testing: " + exePath.string());
                return false;
            }

            // Execute the instructions directly from the SVI, capturing the output
            console->header("Interpreted SVI")->capture();
            VM::Pipeline pipeline1(&testCode);
            pipeline1.targetSingleThreaded()->execute();
            testCode.seekg(0, std::ifstream::beg);
            std::string sviOutput = console->endCapture();

            VM::Pipeline pipeline4(&testCode);
            std::stringstream sviISA;
            pipeline4.targetISARepresentation(sviISA);
            testCode.seekg(0, std::ifstream::beg);

//            console->output(sviOutput);

            // Serialize the instructions into an SBI binary
            console->header("Serialize SVI -> SBI");
            VM::Pipeline pipeline2(&testCode);
            auto binary = pipeline2.targetBinaryRepresentation();
            testCode.seekg(0, std::ifstream::beg);

            auto binPath = std::filesystem::canonical("/proc/self/exe").parent_path() / "run_tests/044-binary/test.sbi";
            auto fh = fopen(binPath.c_str(), "w");
            if ( fh == nullptr ) {
                console->error("Unable to open temporary file for writing: " + binPath.string());
                return true;
            }
            fwrite(binn_ptr(binary), binn_size(binary), 1, fh);
            fclose(fh);

            // Execute the instructions from the SBI binary
            console->header("Interpreted SBI")->capture();
            std::ifstream binCode(binPath);
            VM::Pipeline pipeline3(&binCode, true);
            pipeline3.targetSingleThreaded()->execute();
            std::string sbiOutput = console->endCapture();
            binCode.seekg(0, std::ifstream::beg);

            VM::Pipeline pipeline5(&binCode, true);
            std::stringstream sbiISA;
            pipeline4.targetISARepresentation(sbiISA);

//            console->output(sbiOutput);

            return (sviOutput == sbiOutput) && (sviISA.str() == sbiISA.str());
        }
    };

}

#endif //SWARM_044_BINARY_H
