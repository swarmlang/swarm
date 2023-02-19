# Adding an Instruction to the Swarm VM \[WIP\]

- Add your instruction to the `swarmc::ISA::Tag` enum class in `ISA.h`
- Add a new case to `Instruction::tagName(...)` in `ISA.cpp`
- Find or create the relevant file in `src/vm/isa`
  - If creating a new file, add it to `src/vm/isa_meta.h`
- Define the `Instruction` class for your instruction in the `swarmc::ISA` namespace
- Add parsing case for `instructionLeader` in `parseInstruction` in `ISAParser.h`
- Add a case (in `walkOne`) and abstract method for your new instruction class in `ISAWalk.h`
- Implement the abstract method in `walk/ISABinaryWalk.h`
- Add a case and method for your new instruction class in `walk/BinaryISAWalk.h`
- Implement the abstract method in `walk/ExecuteWalk.h`
- Add a test case for your new instruction in the `test` folder and make sure it passes
