# `swarmlang/swarm`

> Swarm is still a work-in-progress and is frequently in a broken or half-functional state.
> For a rough list of ongoing and future work, see `TODO.md`.

Swarm is a parallel/distributed programming language with modular, swappable parallel drivers. It is intended for everything
from multi-threaded local computation on a single machine to large scientific computations split across many nodes in
a cluster.

The Swarm language provides built-in shared variables with guaranteed access safety. It abstracts parallel and background
execution so the programmer need not reason about it explicitly.

## Architecture
The Swarm programming language is parsed into the Swarm AST using a Bison-generated parser. This AST is then compiled
to SVI, Swarm's virtual ISA. The SVI executes on the Swarm Virtual Machine (SVM), which provides the abstractions for
parallel & asynchronous function calls, shared variables, and synchronization.


## Examples
Here's a contrived example of a sum computed in parallel:

```text
enumeration<number> valuesToSum = [6, 4, 55, 21098, 38, 5858, 33, 4, 7, 58];
shared number sum = 0;

enumerate valuesToSum as value {
    sum += value;
}
```

The SVM unrolls the `enumerate` block into 10 jobs which are executed in parallel. The writes to `sum` are synchronized
automatically.

For more complex and less contrived examples (written in both the Swarm language and SVI), see the `demo/` directory.


## Building
To build Swarm, you need `clang`, `make`, `bison`, and `git`.

```shell
# initialize and build external libraries
make binn

# build the production binary, `swarmc`
make

# build the debugging binary, `swarmc_debug`
make debug

# run the test suite
make test
```


## License
This project is licensed under the terms of the MIT license. See the `LICENSE` file for more information.


