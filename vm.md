# Swarm Runtime VM

The swarm VM is a runtime with 2 tiers of storage (local & shared), a shared job queue, and an ISA for operating on them.

**Definitions:**
- SVI - **S**warm **V**M **I**nstructions
- job - a chunk of SVI executed in its own context
- job context - the set of local variables available to a job

## Storage

The VM defines 2 tiers of storage: local storage (`$l:...`) and shared storage (`$s:...`).

Local storage is cloned for each job context (i.e. if a job modifies a local variable, that variable is NOT changed for other jobs or the parent context.)

Shared storage is global across all jobs that have access to a location.

## Job Queue

## Instructions

`fn` - A function is a region of instructions beginning with `beginfn` and ending with `return`

`lloc` - A location or a literal value.

`loc` - A location.

`val` - A literal value.

- Primitive types
  - `p:TYPE`
  - `p:VOID`
  - `p:NUMBER`
  - `p:STRING`
  - `p:BOOLEAN`
  - `call p:MAP $lloc` - map whose values are of type `$lloc`
  - `call p:ENUM $lloc` - an enum whose values are of type `$lloc`
  - `call p:LAMBDA0 $lloc` - a function taking 0 parameters and returning type `$lloc`
  - `call (call p:LAMBDA $lloc1) $lloc2` - a function taking a parameter of type `$lloc2` and returning type `$lloc1`
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
  - `fnparam $lloc` - a parameter to the function of type `$lloc`
  - `return $lloc` - ends the function call, returning the value `$lloc`
  - `curry f:NAME $lloc` - bundle a parameter with a function w/o calling it (e.g `curry ((a, b) => a + b) 1  => ((b) => (1, b) => 1+b)`)
  - `call f:NAME` - call `f:NAME`
  - `call $lloc` - call the function at `$lloc`
  - `call f:NAME $lloc2` - call `f:NAME` with the parameter `$lloc2`
  - `call $lloc1 $lloc2` - call the function at `$lloc1` with the parameter `$lloc2`
  - `callif $lloc ...` - behaves like `call ...` if `$lloc` is valued as `true`
  - `callelse $lloc ...` - behaves like `call ...` if `$lloc` is valued as `false`
  - `pushcall{,if,else} ...` - behaves like the call derivatives, but pushes the execution of the function onto the work queue
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
  - `scopeof $loc` - creates a new shadow of `$loc` unique to the current scope. Useful for functions
    - NOTE: Variables created with `fnparam` are automatically `scopeof`
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
  - `mapinit $lloc` - create an empty map of type `$lloc`
  - `mapset $lloc1 $lloc2 $lloc3` - set the key `$lloc2` to the value `$lloc3` in the map at `$lloc1`
  - `mapget $lloc1 $lloc2` - get the value of the key `$lloc2` from the map `$lloc1`
  - `maplength $lloc` - get the number of entries in the map at `$lloc`
  - `mapkeys $lloc1 $lloc2` - stores an enum of the keys of the map `$lloc1` at `$lloc2`
- Enumeration operations
  - `enuminit $lloc1` - create an empty enum of type `$lloc1`
  - `enumappend $lloc1 $lloc2` - append the value at `$lloc2` to the enum at `$lloc1`
  - `enumprepend $lloc1 $lloc2` - prepend the value at `$lloc2` to the enum at `$lloc1`
  - `enumlength $lloc` - number of entries in enum at `$lloc`
  - `enumget $lloc1 $lloc2` - get the value of the `$lloc2`-th entry of the enum at `$lloc1`
  - `enumset $lloc1 $lloc2 $lloc3` - set the value of the `$lloc2`-th entry of the enum at `$lloc1` to `$lloc3`
  - `enumerate $lloc1 $lloc2 $lloc3` - syntactic sugar for `f:ENUMERATE`
    - Async enumeration over the elements of the enum `$lloc2`, which are of type `$lloc1`, calling the function `$lloc3`
    - `$lloc3` must take, as its only parameter, an element of type `$lloc1`
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
  - `power $lloc1 $lloc2` - `$lloc1` to the power of `$lloc2`
  - `mod $lloc1 $lloc2` - modulo `$lloc1` by `$lloc2`
  - `neg $lloc1` - negate `$lloc1`
  - `gt $lloc1 $lloc2` - check if `$lloc1` is greater than `$lloc2`
  - `gte $lloc1 $lloc2` - check if `$lloc1` is greater than or equal to `$lloc2`
  - `lt $lloc1 $lloc2` - check if `$lloc1` is less than `$lloc2`
  - `lte $lloc1 $lloc2` - check if `$lloc1` is less than or equal to `$lloc2`
- Loop operations
  - `while $lloc1 $lloc2` - while `$lloc1` is true, call the parameterless function `$lloc2`
- Resource operations
  - `with $lloc1 $lloc2` - execute the function at `$lloc2` in the resource `$lloc1`
    - `$lloc2` must take as a parameter the yield type of the resource `$lloc1`

### Grammar

```txt
IDENTIFIER ::= alphanumeric names with underscores, not starting w/ a digit
LITERAL ::= a decimal number | a string literal | true | false
LOCATION ::= $l:IDENTIFIER | $s:IDENTIFIER
PRIMITIVE ::= p:STRING | p:NUMBER | p:ENUM | p:MAP | p:BOOLEAN | p:VOID | p:TYPE | p:LAMBDA0 | p:LAMBDA
FUNCTION ::= f:IDENTIFIER
LLOC ::= PRIMITIVE | FUNCTION | LOCATION | LITERAL
LLOCS ::= LLOC | LLOC LLOCS
OPERATION ::= out | err | beginfn | fnparam | return | curry
              | call | callif | callelse | pushcall | pushcallif | pushcallelse
              | streaminit | streampush | streampop | streamclose | streamempty
              | typify | lock | unlock | equal | scopeof | typeof | compatible
              | and | or | xor | nand | nor | not
              | mapinit | mapset | mapget | maplength | mapkeys
              | enuminit | enumappend | enumprepend | enumlength | enumget | enumerate
              | strconcat | strlength | strslice
              | plus | minus | times | divide | power | mod | neg
              | gt | gte | lt | lte
              | while | with
OPER ::= OPERATION LLOCS
RVAL ::= OPER | LLOC
ASSIGN ::= LOCATION <- RVAL
INST ::= ASSIGN | OPER
SVI ::= INST EOF | INST \n INSTS
```

### Standard Library

- String functions
  - `f:NUMBER_TO_STRING n` - converts a number to a string
  - `f:BOOLEAN_TO_STRING b` - converts a boolean to a string
  - `f:SIN n`/`f:COS n`/`f:TAN n` - the mathematical sine, cosine, and tangent functions
  - `f:RANDOM`/`f:RANDOM_VECTOR n`/`f:RANDOM_MATRIX m n` - get a random number, enum of random numbers of length n, or matrix of random numbers of size m by n
  - `f:RANGE n m s` - get an enum of the range of numbers from n to m with step size s
  - `f:ENUMERATE t e c` - performs `pushcall`s of `c` on the elements of `e` which are of type `t`

```txt
beginfn f:ENUMERATE_INNER p:VOID
	scopeof $l:entry
	$l:entry <- enumget $l:enum $l:entryi

	pushcall $l:callee $l:entry

	$l:entryi <- add $l:entryi 1
	$l:entryi_lt_entrylen <- lt $l:entryi $l:entrylen
return

beginfn f:ENUMERATE p:VOID
	fnparam p:TYPE $l:tenumof

	scopeof $l:tenum
	$l:tenum <- call p:ENUM $l:tenumof

	fnparam $l:tenum $l:enum

	scopeof $l:tcallee
	$l:tcallee <- call p:LAMBDA p:VOID
	$l:tcallee <- call $l:tcallee $l:tenumof

	fnparam $l:tcallee $l:callee

	scopeof $l:entryi
	scopeof $l:entrylen
	scopeof $l:entryi_lt_entrylen

	$l:entryi <- 0
	$l:entrylen <- enumlength $l:enum
	$l:entryi_lt_entrylen <- lt $l:entryi $l:entrylen

	while $l:entryi_lt_entrylen f:ENUMERATE_INNER
return
```

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