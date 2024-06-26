# Swarm Runtime VM

The swarm VM is a runtime with 2 tiers of storage (local & shared), a shared job queue, and an ISA for operating on them.

**Definitions:**
- SVI - **S**warm **V**M **I**nstructions
- job - a function call, packaged up with the caller's state and context

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
  - `fnparam $lloc $loc` - a parameter to the function of type `$lloc` which will be stored at `$loc`
  - `return $lloc` - ends the function call, returning the value `$lloc`
  - `curry f:NAME $lloc` - bundle a parameter with a function w/o calling it (e.g `curry ((a, b) => a + b) 1  => ((b) => (1, b) => 1+b)`)
  - `call f:NAME` - call `f:NAME`
  - `call $lloc` - call the function at `$lloc`
  - `call f:NAME $lloc2` - call `f:NAME` with the parameter `$lloc2`
  - `call $lloc1 $lloc2` - call the function at `$lloc1` with the parameter `$lloc2`
  - `callif $lloc ...` - behaves like `call ...` if `$lloc` is valued as `true`
  - `callelse $lloc ...` - behaves like `call ...` if `$lloc` is valued as `false`
  - `exit` - terminate execution
- Queue operations
  - `pushcall{,if,else} ...` - behaves like the call derivatives, but pushes the execution of the function onto the work queue. Returns the job ID of the pushed job
  - `drain` - wait for the current context's work queue to finish. Returns a map from job IDs to return values
  - `retmaphas $lloc1 $lloc2` - check if the return value of job ID `$lloc2` is in the return value map `$lloc1`
  - `retmapget $lloc1 $lloc2` - get the return value of job ID `$lloc2` stored in `$lloc1`
  - `entercontext` - enter a new queue context for job batching
  - `resumecontext $lloc1` - return to the queue context with ID `$lloc1` (of `p:CONTEXT_ID`)
  - `popcontext` - exits the current queue context, returning a `p:CONTEXT_ID`
- Stream operations
  - `streaminit $lloc1 $lloc2` - create an empty stream of type `$lloc1` at `$lloc2`
  - `streampush $lloc1 $lloc2` - push the value `$lloc2` onto the stream `$lloc1`
  - `streampop $lloc` - pop the first item of the stream at `$lloc`
  - `streamclose $lloc` - end the stream at `$lloc`
  - `streamempty $lloc` - check if the stream at `$lloc` is empty
- Storage operations
  - `typify $loc $lloc` - specify the type of the given location explicitly
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
  - `mapkeys $lloc` - get an enum of the keys of the map at `$lloc`
- Enumeration operations
  - `enuminit $lloc1` - create an empty enum of type `$lloc1`
  - `enumappend $lloc1 $lloc2` - append the value at `$lloc2` to the enum at `$lloc1`
  - `enumprepend $lloc1 $lloc2` - prepend the value at `$lloc2` to the enum at `$lloc1`
  - `enumlength $lloc` - number of entries in enum at `$lloc`
  - `enumget $lloc1 $lloc2` - get the value of the `$lloc2`-th entry of the enum at `$lloc1`
  - `enumset $lloc1 $lloc2 $lloc3` - set the value of the `$lloc2`-th entry of the enum at `$lloc1` to `$lloc3`
  - `enumconcat $lloc1 $lloc2` - create an enum containing the elements of `$lloc1` followed by those of `$lloc2`
  - `enumerate $lloc1 $lloc2 $lloc3` - syntactic sugar for `f:ENUMERATE`
    - Async enumeration over the elements of the enum `$lloc2`, which are of type `$lloc1`, calling the function `$lloc3`
    - `$lloc3` must take two parameters: first, an element of type `$lloc1`; second, a `p:NUMBER` which is the index of the element
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
  - `while $lloc1 $lloc2` - while `$lloc1`, of type `:: p:BOOLEAN` returns true, call the parameterless function `$lloc2`
- Resource operations
  - `with $lloc1 $lloc2` - execute the function at `$lloc2` in the resource `$lloc1`
    - `$lloc2` must take as a parameter the yield type of the resource `$lloc1`
- Exceptions
  - Exceptions are identified using unique integers. When an exception is raised, the first handler which accepts the
    exception's code on the handler stack is called.
  - The exception handler is responsible for calling into the correct point where execution should be resumed.
  - `pushexhandler $lloc` - register the function `$lloc` as the handler for all exceptions raised in the current context
    - Returns a `p:STRING` with the ID of the exception handler
  - `pushexhandler $lloc1 $lloc2` - register the function `$lloc1` as the handler for all exceptions with the code `$lloc2` in the current context
    - `$lloc2` can be either a static number or a function of type `p:NUMBER -> p:BOOLEAN`
  - `popexhandler $lloc` - pop the exception handler with the ID `$lloc`
  - `raise $lloc` - raise an exception with the code `$lloc`
  - `resume $lloc` - call the function `$lloc` from the scope where the exception handler was registered
- Object types (note: currently proposed, not yet implemented)
  - Primitives:
    - `p:OTYPE_PROTO` - an object type which can be modified
    - `p:OTYPE` - a finalized object type
    - `p:OBJECT_PROTO` - an object which is being constructed
    - `p:OBJECT` - a constructed object instance
    - `o:*` - names of object properties (e.g. `o:MYPROP1`) -- these are referred to as `$oloc`
  - Defining object types
    - `otypeinit` - returns a new, empty object type
    - `otypeprop $lloc1 $oloc $lloc2` - define the `$oloc` property on the object type `$lloc1` to have type `$lloc2`
    - `otypedel $lloc $oloc` - remove the `$oloc` property from the object type `$lloc`
    - `otypeget $lloc $oloc` - get the type of the `$oloc` property on the object type `$lloc`
    - `otypefinalize $lloc` - returns a finalized version of the `$lloc` object type
    - `otypesubset $lloc` - returns a new `p:OTYPE_PROTO` which extends the `p:OTYPE` `$lloc`
      - Note: `$lloc` CANNOT be a `p:OTYPE_PROTO`
    - `otypecurry $lloc` - shorthand for the following:
      - ```text
        $l:TEMP <- curry f:LAMBDA1_T p:THIS
        call $l:TEMP $lloc
        ```
  - Constructing/using objects
    - `objinit $lloc` - returns a prototype object instance of the object type `$lloc`
      - Return value is `p:OBJECT_PROTO`
    - `objset $lloc1 $oloc $lloc2` - sets the `$oloc` property on the `$lloc1` object to the value `$lloc2`
    - `objget $lloc $oloc` - gets the value of the `$oloc` property on the `$lloc` object
      - `$lloc` must be `p:OBJECT`
    - `objinstance $lloc` - instantiates the `p:OBJECT_PROTO` at `$lloc` and returns the `p:OBJECT`
      - Validates the object properties based on the object's type
    - `objcurry $lloc $oloc` - get a pre-curried reference to an object method
      - Most object methods take the object itself as the first parameter.
      - To aid this paradigm, `objcurry` returns an instance of the `$oloc` method on the `$lloc` object pre-curried with `$lloc`
      - The type of `$oloc` must be `p:THIS :: ...`
      - It is equivalent to:
        ```text
        $l:method <- objget $lloc $oloc
        $l:result <- curry $l:method $lloc
        ```

### Grammar

```txt
IDENTIFIER ::= alphanumeric names with underscores, not starting w/ a digit
LITERAL ::= a decimal number | a string literal | true | false
LOCATION ::= $l:IDENTIFIER | $s:IDENTIFIER
PRIMITIVE ::= p:STRING | p:NUMBER | p:ENUM | p:MAP | p:BOOLEAN | p:VOID | p:TYPE | p:LAMBDA0 | p:LAMBDA
FUNCTION ::= f:IDENTIFIER
LLOC ::= PRIMITIVE | FUNCTION | LOCATION | LITERAL
LLOCS ::= LLOC | LLOC LLOCS
EXPRESSION ::= curry | call
         | streaminit | streampop | streamempty
         | equal | typeof | compatible
         | and | or | xor | nand | nor | not
         | mapinit | mapget | maplength | mapkeys
         | enuminit | enumlength | enumget
         | strconcat | strlength | strslice
         | plus | minus | times | divide | power | mod | neg
         | gt | gte | lt | lte
         | otypeinit | otypeget | otypefinalize | otypesubset
         | objinit | objget | objinstance | objcurry
         | pushcall | drain | retmaphas | retmapget
OPERATION ::= out | err | beginfn | fnparam | return
              | callif | callelse | pushcallif | pushcallelse | exit
              | entercontext | resumecontext | popcontext
              | streampush | streamclose
              | typify | lock | unlock | scopeof
              | mapset
              | enumappend | enumprepend | enumerate
              | while | with
              | pushexhandler | popexhandler | raise | resume
              | otypeprop | otypedel
              | objset
OPER ::= OPERATION LLOCS
EXPR ::= EXPRESSION LLOCS
RVAL ::= EXPR | LLOC
ASSIGN ::= LOCATION <- RVAL
INST ::= ASSIGN | OPER | EXPR
SVI ::= INST EOF | INST \n INSTS
```

### Standard Library

- String functions
  - `f:NUMBER_TO_STRING n` - converts a number to a string
  - `f:BOOLEAN_TO_STRING b` - converts a boolean to a string
  - `f:SIN n`/`f:COS n`/`f:TAN n` - the mathematical sine, cosine, and tangent functions
  - `f:RANDOM`/`f:RANDOM_VECTOR n`/`f:RANDOM_MATRIX m n` - get a random number, enum of random numbers of length n, or matrix of random numbers of size m by n
  - `f:RANGE n m s` - get an enum of the range of numbers from n to m with step size s
  - `f:ID v` - the identity function (e.g. `f:ID 3 -> 3`)
  - `f:LAMBDA0_T r` - constructs a lambda type of the form `() -> r`
  - `f:LAMBDA1_T a r` - constructs a lambda type of the form `a -> r`
  - `f:CONTEXT_ID_T` - constructs the opaque `p:CONTEXT_ID` type
  - `f:JOB_ID_T` - constructs the opaque `p:JOB_ID` type
  - `f:RETURN_VALUE_MAP_T` - construct the opaque `p:RETURN_VALUE_MAP` type

### Examples

```txt
fn sum = (a: number, b: number, c: number): number => a + b + c;

number one = 1.1;
number two = 2.2;
number three = 3.3;

shared number onetwothree = sum(one)(two)(three);
```

```txt
call f:MAIN
exit

beginfn f:SUM p:NUMBER
  fnparam p:NUMBER $l:a
  fnparam p:NUMBER $l:b
  fnparam p:NUMBER $l:c

  $l:temp <- plus $l:a $l:b
  $l:temp <- plus $l:temp $l:c
return $l:temp

beginfn f:MAIN p:VOID
  $l:one <- 1.1
  $l:two <- 2.2
  $l:three <- 3.3
  
  $l:fcall <- call f:SUM $l:one
  $l:fcall <- call $l:fcall $l:two
  $s:onetwothree <- call $l:fcall $l:three
return
```

```txt
fn helloer = (name: string): string => {
  if ( name == "world" ) {
    return "Hi, there!";
  }
  
  return "Hello, " . name . "!";
}

log(helloer("Bob"));
```

```txt
call f:MAIN
exit

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

beginfn f:MAIN p:VOID
  $l:helloer <- call f:HELLOER "Bob"
  out $l:helloer
return
```
