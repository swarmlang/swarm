# External Providers

The Swarm VM supports the addition of native functions & other resources via a set of runtime provider interfaces.

These "providers" can be implemented using shared modules which are loaded by the SVM when it boots.


## Using a Provider

Providers are registered on the CLI:

```shell
swarmc --external-provider ../path/to/external_provider.so [...]
```

## Structure of a Provider

Your provider should contain a top-level symbol called `factory` which creates an instance of `ProviderModule` from
`src/vm/runtime/external.h`. For example:

```c++
#include "external.h"
using namespace swarmc::Runtime;

extern "C" ProviderModule* factory() { /* ... */ }
```

This function will be called by Swarm to install your Provider into the virtual machine.

Swarm's `external.h` includes interfaces needed to implement `IProvider` and the accompanying classes.


## Building a Provider

Currently, the `IProvider` interface provides support for loading and executing custom functions in the VM.

This is done by implementing `IFunction` and `IFunctionCall` and returning them from your `IProvider`'s `loadFunction`
method.

The `swarmlang` GitHub includes an example provider implementation for reference, available [here](https://github.com/swarmlang/example_provider).
