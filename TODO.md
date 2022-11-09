- ~~setup docker container for builds~~
- fix memory leaks
  - Type objects and semantic symbols
- not dogshit test suite
  - fuzzy testing with American Fuzzy Lop
  - runtime tests
    - ~~basic execution~~
    - streams
    - resources
    - exception handling
    - ~~single-threaded runtime drivers (isolation, pushcall, &c)~~
    - ~~prologue functions (random\*, range, sin, cos, tan, \*to_string)~~
    - ~~dynamic scoping/shadowing~~
    - function currying
    - locking
    - equality
- module system / import system
- language-native testing system
  - e.g. define named test blocks which are ignored during normal runtime
- Implement IStorageInterface, IQueue, IQueueJob, and IGlobalServices
    - Redis implementation
    - pthread implementation
    - ~~single-threaded implementation~~
    - ...others?
- ~~pass to convert AST to ISA~~
- ast optimization pass
  - removing statements coming after a return
  - simplification of trivial expressions
- assembly optimization pass
  - `X += -Y` can be optimized in the assembly
  - Remove instructions assigning variables to themselves
  - Propagate primitives values
  - Dead code elimination
  - beginfn followed by return0
  - Collapse call instructions
    - Example:
      ```txt
      -- this
      $l:call0 <- curry f:MY_FN 1
      $l:call1 <- curry $l:call0 2
      $l:assn <- call $l:call1

      -- can reduce to
      $l:call0 <- curry f:MY_FN 1
      $l:assn <- call $l:call0 2
      ```
- Runtime
  - streams
  - resources
  - exception handling
- remove RESOURCE type from lexing (added so I could test WITH statements)
- Serialize ISA to SVI code
- change map access back to [] (check for lval type in name analysis to avoid parsing conflict)
- some form of exception/error system (e.g. exceptions, error values, ...?)
- FUTURE: separate fetch/execute/writeback threads for runtime
- Support Position annotations in SVI for better error messages from the VM
- Garbage collection / reference counting
- More generic logging class that can centralize logs from workers?
- More compact binary form of SVI
- `mapvalues` instruction? Parallel to `mapkeys`.
- Currently, `typeof` only works when a variable has a value stored in it
  - This means the following will fail:
    ```txt
    typify $l:a p:NUMBER
    typeof $l:a
    ```
  - Do we care to support this?
- Well-defined C++/native bridge to allow custom `f:XXX` function bindings, as well as custom stream implementations.
- The `call` instruction should curry partial applications. Right now, it just errors.
- Tiered call queues (e.g. a local one for fast, multi-thread calls and a distributed one for longer batch jobs)
- Sci-comp natives
  - Map-reduce
  - Parallel matrix operations
  - Parallel sorting
  - Machine learning