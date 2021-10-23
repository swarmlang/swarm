# swarm

Swarm is the distributed-computing-native DSL for our project.

For now, this repo contains `swarmc`, the compiler for the Swarm language.

## Building

To build a normal version, run `make`. This outputs a `swarmc` executable.

To build a version with debug output enabled, run `make debug`. This outputs a `swarmc_debug` executable.

## Testing

Run `make test` to run the test suite.

To add a new test:

1. Create a new folder in test, e.g. `003_my_test`
2. In that folder, create an executable `run` that performs the test
    - The test should write to standard output and standard error
    - The following environment variables are available
        - `$SWARMC` - the absolute path to the compiled `swarmc` to test
        - `$TESTSWARM` - the absolute path to the `test.swarm` file in the test dir
3. Create `err.expected` and `out.expected` files in that folder with the expected standard error and output, respectively
4. If you need Swarm code to test, put it in `test.swarm` and the file path will be available to the test code


## Lexer Notes

To add a new lexer entry:

1. Modify `src/bison/cshanty.yy` and add a new token **BELOW** the `%token  END     0` line.
2. Modify `src/lang/Debugging.h` and add your new token kind to the `tokenKindToString` method.
3. Modify `src/swarm.l` to add the lexer rule. Try to name re-usable chunks of regex to keep it maintainable.
    - If your token requires storing additional information with it (e.g. string value), add a new class in `src/lang/Token.h`.

To test that your tokens are being recognized correctly, you can parse an input file and output the tokens:

```sh
./swarmc_debug --dbg-output-tokens-to -- test.swarm
```
