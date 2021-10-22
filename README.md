# swarm

Swarm is the distributed-computing-native DSL for our project.

For now, this repo contains `swarmc`, the compiler for the Swarm language.

## Building

To build a normal version, run `make`. This outputs a `swarmc` executable.

To build a version with debug output enabled, run `make debug`. This outputs a `swarmc_debug` executable.


## Lexer Notes

To add a new lexer entry:

1. Modify `src/bison/cshanty.yy` and add a new token **BELOW** the `%token  END     0` line.
2. Modify `src/swarm.l` to add the lexer rule. Try to name re-usable chunks of regex to keep it maintainable.
    - If your token requires storing additional information with it (e.g. string value), add a new class in `src/lang/Token.h`.

To test that your tokens are being recognized correctly, you can parse an input file and output the tokens:

```sh
./swarmc_debug --dbg-output-tokens-to -- test.swarm
```
