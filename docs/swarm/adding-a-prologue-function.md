
- add a new `PrologueFunctionSymbol` in `SymbolTable.cpp`
    - if the swarm function maps to an SVI function, the last argument should be the 
    name of the SVI function
    - if it maps to an SVI instruction, the last argument should be the string `"null"`

- if the swarm function maps to an SVI instruction, you will also need to add how to compile it to `ToISAWalk.cpp`
    - add an entry mapping the swarm function name to its return type it `ToISAWalk::InstructionAsFunc`
    - add an if block to `ToISAWalk::callToInstruction` that walks the arguments and creates the instruction. Using `ToISAWalk::getLastLoc` after walking an argument will be profoundly helpful.