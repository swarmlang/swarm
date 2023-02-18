# SVM Object Polymorphism

> This is a proposal and is not yet implemented.

**Goal:** Provide a basic, native mechanism for objects with well-defined, but heterogeneous properties and basic parent-child polymorphism.

## Overview

SVI will provide instructions for incrementally constructing object types by defining a mapping of property names to their corresponding types.

Once constructed, this object type is "finalized" which prevents the modification of its type post-facto. Finalized object types can be used to construct instances which satisfy those types.

Polymorphism is achieved using the `otypesubset` instruction which returns a new `p:OTYPE_PROTO` which allows properties on the underlying type to be refined & added to while respecting the parent type.

## Instructions

See the "Object types" section of [the SVI instruction docs](../../vm.md).

## Examples

### `$l:ANIMAL_T`

Here we're going to declare a basic `ANIMAL` object type with a `o:NAME` property and a method, `o:SPEAK`.

```text
-- Construct the type of the o:SPEAK property:
$l:SPEAK_T <- call p:LAMBDA0 p:STRING

-- Construct the object type:
$l:ANIMAL_T_PROTO <- otypeinit
otypeprop $l:ANIMAL_T_PROTO o:NAME p:STRING
otypeprop $l:ANIMAL_T_PROTO o:SPEAK $l:SPEAK_T

-- Finalize the object type:
$l:ANIMAL_T <- otypefinalize $l:ANIMAL_T_PROTO
```

We define a generic `o:SPEAK` method:

```text
beginfn f:ANIMAL_SPEAK p:STRING
    fnparam $l:ANIMAL_T $l:this
    scopeof $l:greeting
    
    $l:greeting <- objget $l:this o:NAME
    $l:greeting <- strconcat $l:greeting " says hello!"
return $l:greeting
```

Now, we can create an instance of `ANIMAL`:

```text
-- Construct the object instance:
$l:SNEK_PROTO <- objinit $l:ANIMAL_T
objset $l:SNEK_PROTO o:NAME "Carl"
objset $l:SNEK_PROTO o:SPEAK f:ANIMAL_SPEAK

-- Finalize the instance:
$l:SNEK <- objinstance $l:SNEK_PROTO
```

Once we have the `$l:SNEK` instance, we can perform operations with it:

```text
-- Get the type:
typeof $l:SNEK  -- returns $l:ANIMAL_T

-- Access properties:
$l:name <- objget $l:SNEK o:NAME
out $l:name

-- Call methods:
$l:call <- objcurry $l:SNEK o:SPEAK
$l:greeting <- call $l:call
out $l:greeting
```

### `$l:DOG`

Now, we can create a `DOG` object type which is a subset of the `ANIMAL` type.

This `DOG` type will have an additional property, `o:WAG_TAIL`.

First, we construct the `DOG_T` type:

```text
-- Construct the type of o:WAG_TAIL
$l:WAG_TAIL_T <- call p:LAMBDA0 p:VOID

-- Construct the DOG_T object type
$l:DOG_T_PROTO <- otypesubset $l:ANIMAL_T
otypeprop $l:DOG_T_PROTO o:WAG_TAIL $l:WAG_TAIL_T

-- Finalize the DOT_T type
$l:DOG_T <- otypefinalize $l:DOG_T_PROTO
```

Next, we'll define the tail wagging function and a custom speak method for the dog. Note that, for `o:SPEAK`, we could have
used the base `f:ANIMAL_SPEAK`. This is an example of method inheritance/compatibility.

```text
beginfn f:DOG_WAG_TAIL p:VOID
    fnparam $l:DOG_T $l:this
    scopeof $l:str
    $l:str <- objget $l:this o:NAME
    $l:str <- strconcat $l:str " wags its tail"
    out $l:str
return

beginfn f:DOG_SPEAK p:STRING
    fnparam $l:DOG_T $l:this
    scopeof $l:str
    $l:str <- objget $l:this o:NAME
    $l:str <- strconcat $l:str " says woof!"
return $l:str
```

Now we can construct instances of `DOG_T`:

```text
-- Construct the DOG_T instance
$l:DOG_PROTO <- objinit $l:DOG_T
objset $l:DOG_PROTO o:NAME "Rufus"
objset $l:DOG_PROTO o:SPEAK f:DOG_SPEAK
objset $l:DOG_PROTO o:WAG_TAIL f:DOG_WAG_TAIL

-- Finalize the DOG instance
$l:DOG <- objinstance $l:DOG_PROTO
```

Finally, we can use our dog instance. Notice that it satisfies the base type `ANIMAL_T`:

```text
-- Get the type of the instance
typeof $l:DOG  -- returns $l:DOG_T

-- Notice that the child type is a subset of the parent type
compatible $l:ANIMAL_T $l:DOG_T  -- returns true

-- Access properties:
$l:name <- objget $l:DOG o:NAME
out $l:name

-- Call methods:
$l:call <- objcurry $l:DOG o:SPEAK
$l:greeting <- call $l:call
out $l:greeting

$l:call2 <- objcurry $l:DOC o:WAG_TAIL
call $l:call2
```
