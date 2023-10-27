# Debugging `swarmc`

## Binary Files
This repo contains a Node.js script which can be used to dump compiled Swarm binaries to their `binn` representation for debugging.

To use it, all you need is a reasonably modern version of Node and the compiled binary:

```shell
./bin/binary_dump.js ./path/to/my/compiled.sbi
```

## Test Suite
Swarm contains 2 sets of legacy test suites (`test` and `test_old`). These are being transitioned to a [Catch2](https://github.com/catchorg/Catch2)-based test suite located in the `tests/` directory.

To run the test suite, build the `tests` target. This emits a `./swarmc_tests` binary with the standard Catch2 options:

```shell
make tests
./swarmc_tests
```

Alternatively, you can build and run the test suite with the `test` target:

```shell
make test
```
