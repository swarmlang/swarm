# Swarm Runtime VM

The swarm VM is a runtime with 2 tiers of storage (local & shared), a shared job queue, and an ISA for operating on them.

## Storage

## Job Queue

## Instructions

`fn` - A function is a region of instructions beginning with `beginfn` and ending with `return`

`lloc` - A location or a literal value.

`loc` - A location.

`val` - A literal value.

- Special locations
  - `$l:STDOUT` - standard output on the local executor
  - `$l:STDERR` - standard error on the local executor
  - `$s:STDOUT` - standard output on the original interface
  - `$s:STDERR` - standard error on the original interface
  - `out $lloc` - writes the string `$lloc` to the shared standard output
    - Equivalent to `streampush $s:STDOUT $lloc`
  - `err $lloc` - writes the string `$lloc` to the shared standard error
    - Equivalent to `streampush $s:STDERR $lloc`
- Function operations
  - `beginfn f:NAME $lloc` - starts a function body with return type `$lloc`
  - `fnparam $lloc $loc` - a parameter to the function of type `$lloc` to be stored in `$loc`
  - `return $lloc` - ends the function call, returning the value `$lloc`
  - `call f:NAME` - call `f:NAME`
  - `call $lloc` - call the function at `$lloc`
  - `call f:NAME $lloc2` - call `f:NAME` with the parameter `$lloc2`
  - `call $lloc1 $lloc2` - call the function at `$lloc1` with the parameter `$lloc2`
  - `callif $lloc ...` - behaves like `call ...` if `$lloc` is valued as `true`
  - `callelse $lloc ...` - behaves like `call ...` if `$lloc` is valued as `false`
- Stream operations
  - `streaminit $lloc1 $lloc2` - create an empty stream of type `$lloc1` at `$lloc2`
  - `streampush $lloc1 $lloc2` - push the value `$lloc2` onto the stream `$lloc1`
  - `streampop $lloc` - pop the first item of the stream at `$lloc`
  - `streamclose $lloc` - end the stream at `$lloc`
  - `streamempty $lloc` - check if the stream at `$lloc` is empty
- Storage operations
  - `typify $loc` - specify the type of the given location explicitly
  - `$l:NAME <- $lloc` - assigns the value of `$lloc` to the local storage variable with name `NAME`
  - `$s:NAME <- $lloc` - assigns the value of `$lloc` to the shared storage variable with name `NAME`
  - `lock $s:NAME` - acquires an exclusive lock on the given shared storage variable
  - `unlock $s:NAME` - releases a held lock on the given shared storage variable
  - `equal $lloc1 $lloc2` - determine whether two values are the same
- Type operations
  - `typeof $lloc`
  - `compatible $lloc $lloc`
- Boolean operations
  - `and $lloc1 $lloc2` - boolean AND of `$lloc1` and `$lloc2`
  - `or $lloc1 $lloc2` - boolean OR of `$lloc1` and `$lloc2`
  - `xor $lloc1 $lloc2` - boolean XOR of `$lloc1` and `$lloc2`
  - `nand $lloc1 $lloc2` - boolean NAND of `$lloc1` and `$lloc2`
  - `nor $lloc1 $lloc2` - boolean NOR of `$lloc1` and `$lloc2`
  - `not $lloc` - boolean negation of `$lloc`
- Map operations
  - `mapinit $lloc1 $lloc2` - create an empty map of type `$lloc1` at `$lloc2`
  - `mapset $lloc1 $lloc2 $lloc3` - set the key `$lloc2` to the value `$lloc3` in the map at `$lloc1`
  - `mapget $lloc1 $lloc2` - get the value of the key `$lloc2` from the map `$lloc1`
  - `maplength $lloc` - get the number of entries in the map at `$lloc`
  - `mapkeys $lloc1 $lloc2` - stores an enum of the keys of the map `$lloc1` at `$lloc2`
- Enumeration operations
  - `enuminit $lloc1 $lloc2` - create an empty enum of type `$lloc1` at `$lloc2`
  - `enumappend $lloc1 $lloc2` - append the value at `$lloc2` to the enum at `$lloc1`
  - `enumprepend $lloc1 $lloc2` - prepend the value at `$lloc2` to the enum at `$lloc1`
  - `enumlength $lloc` - number of entries in enum at `$lloc`
  - `enumget $lloc1 $lloc2` - get the value of the `$lloc2`-th entry of the enum at `$lloc1`
  - `enumset $lloc1 $lloc2 $lloc3` - set the value of the `$lloc2`-th entry of the enum at `$lloc1` to `$lloc3`
- String operations
  - `strconcat $lloc1 $lloc2` - concat string `$lloc2` onto the end of `$lloc1`
  - `strlength $lloc` - get the length of the string `$lloc`
  - `strslice $lloc1 $lloc2` - the substring of the string `$lloc1` starting at the index `$lloc2`
  - `strslice $lloc1 $lloc2 $lloc3` - the substring of the string `$lloc1` starting at index `$lloc2` and ending at `$lloc3`
- Arithmetic operations
  - `plus $lloc1 $lloc2` - add `$lloc2` to `$lloc1`
  - `minus $lloc1 $lloc2` - subtract `$lloc2` from `$lloc1`
  - `times $lloc1 $lloc2` - multiply `$lloc1` by `$lloc2`
  - `divide $lloc1 $lloc2` - divide `$lloc1` by `$lloc2`
  - `mod $lloc1 $lloc2` - modulo `$lloc1` by `$lloc2`
  - `neg $lloc1` - negate `$lloc1`

### Examples

```txt
fn sum = (a: number, b: number, c: number): number -> a + b + c;

local number one = 1.1;
local number two = 2.2;
local number three = 3.3;

shared number onetwothree = sum(one)(two)(three);
```

```txt
beginfn f:SUM p:NUMBER
  fnparam p:NUMBER $l:a
  fnparam p:NUMBER $l:b
  fnparam p:NUMBER $l:c

  $l:temp <- plus $l:a $l:b
  $l:temp <- plus $l:temp $l:c
return $l:temp

$l:one <- 1.1
$l:two <- 2.2
$l:three <- 3.3

$l:fcall <- call f:SUM $l:one
$l:fcall <- call $l:fcall $l:two
$s:onetwothree <- call $l:fcall $l:three
```

```txt
fn helloer = (name: string): string -> {
  if ( name === "world" ) {
    return "Hi, there!";
  }
  
  return "Hello, " . name . "!";
}

log(helloer("Bob"));
```

```txt
beginfn f:HELLOER p:STRING
  fnparam p:STRING $l:name

  beginfn f:HELLOER_IF
    $l:retstring <- "Hi, there!"
  return

  beginfn f:HELLOER_ELSE
    $l:retstring <- strconcat "Hello, " $l:name
    $l:retstring <- strconcat $l:retstring "!"
  return

  $l:name_world <- equal $l:name "world"
  callif $l:name_world f:HELLOER_IF
  callelse $l:name_world f:HELLOER_ELSE
return $l:retstring

$l:helloer <- call f:HELLOER "Bob"
log $l:helloer
```
