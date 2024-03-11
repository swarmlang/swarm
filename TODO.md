- fix memory leaks
  - Type objects and semantic symbols
- fix lexer position counter to account for escaped characters
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
    - ~~Redis implementation~~
    - ~~pthread implementation~~
    - ...others?
- minimal set of used locations in a function walk
  - identify pure functions
  - fix scoping issue
    - `fn foo = (n: number): ->number => (): number => n;` does not compile correctly, but should
      be allowed. Potential compile-time fix is to identify all unscoped locations in a function
      literal and modify the signature to curry in all of those locations.
- modify Type::Object (and subsequently the compiler walks) to support multiinheritance
- add syntax to swarm for (or otherwise implicitly support) currying a 1 argument function into a 0 argument function instead of immediately calling functions once they have no more arguments. Consider as an implicit example:
```swarmc
fn ex = (n: number): number => n + 1;

-- this is the existing behavior
number called = ex(5);

-- explicitly writing the `->number` type could inform the compiler to curry the last argument instead of calling it.

->number curried = ex(5);
```
This would mostly be finagling with type analysis to allow something of type `T->T` to be assigned to both `T` and `->T`, and setting a flag for which one to compile to.

- ast optimization pass
  - removing statements coming after a return/continue/break
  - simplification of trivial expressions
  - name mangling to prevent imported modules from compiling to objects
- assembly optimization pass
  - `X += -Y` can be optimized in the assembly
  - Propagate primitives values
  - Dead code elimination
  - beginfn followed by return0
- Runtime
  - threaded listener for `s:STDOUT` and `s:STDERR` streams
- ~~remove RESOURCE type from lexing (added so I could test WITH statements)~~
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
- make it so constructor nodes don't require rebuilding the entire instruction vector in ToISAWalk
- update syntax highlighting
- ~~Syntax for deferred function calls~~
- FUTURE: (hilariously low priority) blockchain-based distributed drivers
- FUTURE: separate fetch/execute/writeback threads for runtime
- FUTURE: Serialize ISA to SVI code
- FUTURE: Swarm module package manager?
- Fabric: rewrite Stream as a resource
- SVM: automatic parallelization of SVI code and wait on join
- SVM: only lock reads if there are writes in the instruction
- Function to create a zeroed matrix of a certain type
