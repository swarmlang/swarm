- fix memory leaks
  - Type objects and semantic symbols
- not dogshit test suite
  - fuzzy testing with American Fuzzy Lop
  - runtime tests
    - resources
    - exception handling
    - function currying
    - locking
- module system / import system
- language-native testing system
  - e.g. define named test blocks which are ignored during normal runtime
- standard library for swarm
- Implement IStorageInterface, IQueue, IQueueJob, IStreamDriver, IStream, and IGlobalServices
    - Redis implementation
    - pthread implementation
    - ...others?
- ast optimization pass
  - removing statements coming after a return
  - simplification of trivial expressions
- assembly optimization pass
  - `X += -Y` can be optimized in the assembly
  - Propagate primitives values
  - Dead code elimination
  - beginfn followed by return0
- Runtime
  - threaded listener for `s:STDOUT` and `s:STDERR` streams
- remove RESOURCE type from lexing (added so I could test WITH statements)
- change map access back to [] (check for lval type in name analysis to avoid parsing conflict)
- some form of exception/error system (e.g. exceptions, error values, ...?)
- Exceptions: show call stack for debugging, map back to original Position data using Metadata
- Garbage collection / reference counting
- More generic logging class that can centralize logs from workers?
- `mapvalues` instruction? Parallel to `mapkeys`.
- Currently, `typeof` only works when a variable has a value stored in it
  - This means the following will fail:
    ```txt
    typify $l:a p:NUMBER
    typeof $l:a
    ```
  - Do we care to support this?
- Providers:
  - Custom types (opaque only?)
  - Custom resource & stream implementations
- Tiered call queues (e.g. a local one for fast, multi-thread calls and a distributed one for longer batch jobs)
- Sci-comp natives
  - Map-reduce
  - Parallel matrix operations
  - Parallel sorting
  - Machine learning
- Limit jobs w/ serialized resources to the nodes that owns the resource
- SVI: optimizations for deferred/parallel pure function calls in the VM
- Support for signed binaries
- Support building `swarmc` binary without the C++ test suite
- Support automatically determining which variables should be shared?
- add unset instruction
- generic types :(
- update syntax highlighting
- Syntax for deferred function calls
  - ```
    wait<string> a = ~test();
    ...
    log(a);
    ```
- FUTURE: (hilariously low priority) blockchain-based distributed drivers
- FUTURE: separate fetch/execute/writeback threads for runtime
- FUTURE: Serialize ISA to SVI code
- FUTURE: Swarm module package manager?
- Fabric: rewrite Stream as a resource
