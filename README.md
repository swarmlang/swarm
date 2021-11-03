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

## Parser Notes

The general workflow for adding an item to the parser is as follows:

1. Modify `src/bison/swarm.yy` to add your grammar productions
    - Easiest to leave the productions empty for now, just get the grammar structure right

    - Shouldn't need to add any terminals, but you can add non-termals below the comment c. line 94.
        - Use the `transPlaceholder` type to start. We'll fill it in later.

2. Modify `src/lang/AST.h` to create the AST nodes necessary for your productions.

    1. Declare the class at the bottom of the namespace
        - Make sure to extend the appropriate parent class.
        - In most cases, want to mark the class as `final`, unless it's a generic parent class designed to be reused.
        - You'll need to implement `IStringable::toString()`

    2. See below note on error handling

3. Now that you've declared the new AST node class, add a mapping for it to the `%union` at the top of `swarm.yy`.
    - Generally, this should start with the `trans` prefix, for translation.

4. If you added a non-termal in step (1), change its type to the new alias you just added.

5. Fill out the productions in the file to instantiate your new AST node clases.


### Note on Error Handling
If you notice any edge cases with your AST node classes that should cause an error, declare a new error class in its own file in `src/errors`.

Make the error class extend `swarmc::Errors::SwarmError` and provide a descriptive error message.
