# SWARM

## Types

- **Primitive Types:**  
The primitive types in swarm are `number`, `string`, `bool`, and `type`. `void` also exists, but can only be used as the return type of a function
- **Lambda Types:**  
Lambda types are the types of functions, constructed out of other types using the `->` operator.  
Example: `number->number sqr = (a: number): number => { return a^2; };`  
In the case of declarations of function variables, the type of the variable can be replaced with `fn` for brevity:  
`fn sqr = (a: number): number => { return a^2; };`  
- **Enumerables**  
An enumerable is effectively just a vector. The type of the contents of the enumerable must be specified, but can be any other type.  
Example:
`enumerable<number> nums = [1,2,3];`  
*Note: To have an empty enumerable literal, you must ascribe a type to its values to prevent ambiguity.*  
*Example:* `enumerable<number> nums = [] of number;`  
To access the values of an enumerable, use the `[]` operator.  
Example: `nums[1]`  
- **Maps**  
A map is an unordered set of mappings from identifiers to values. The types of the values must be declared similar to an enumerable.  
Example: `map<number> m = { a: 3, b: 4 };`  
*Note: To have an empty map literal, you must ascribe a type to its values to prevent ambiguity. Example:* `map<number> nums = {} of number;`  
To access the values of a map, use the `{}` operator.  
Example: `m{a}`  
- **User-Defined Types:**  
See [User-Defined Types](#user-defined-types)

## Basic Operations

Swarm supports most of the basic operations on primitive values. The operators, from lowest to highest precedence, are as follows:
| **Operation** | **Syntax** |
|---------------|------------|
| Boolean OR | `x \|\| y` |
| Boolean AND | `x && y` |
| Equality and Inequality | `x <op> y` |
| Addition and Subtraction | `x <op> y` |
| String Concatenation | `x + y` |
| Multiplication, Division, and Modulus | `x <op> y` |
| Exponentiation | `x ^ y` |

## Functions

In swarm, functions are values, just like numbers and strings. Thus, they can be assigned to variables and passed as arguments.  
The following is an example of a function literal:

```swarm
(a: number, b: number): number => {
    number c = a + b;
    return c;
}
```

A function consists of 3 parts, the arguments, the return type, and the body. In this example, `a` and `b` are the arguments, which are both of type `number`. The return type is also `number`, and the body adds the two arguments and returns that sum.  
To call a function, simply call them as you would in Java or C. The thing being called does not have to be an identifier, but rather can be anything that evaluates to a function. Swarm curries functions and thus supports partial application of functions (except constructors).  

## Prologue functions

| **Name**          | **Description**                                             | **Type**                                                                            | **Arguments**                                                                                                                                  |
|-------------------|-------------------------------------------------------------|-------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------|
| `numberToString`  | Converts a number to a string.                              | `number -> string`                                                                  | The number to convert.                                                                                                                         |
| `booleanToString` | Converts a boolean to a string.                             | `bool -> string`                                                                    | The boolean to convert.                                                                                                                        |
| `vectorToString`  | Converts a numeric enumeration to a string.                 | `enumerable<number> -> string`                                                      | The vector to convert.                                                                                                                         |
| `matrixToString`  | Converts a nested numeric enumeration to a string.          | `enumerable<enumerable<number>> -> string`                                          | The matrix to convert.                                                                                                                         |
| `sin`             | Returns the sine of the angle.                              | `number -> number`                                                                  | The angle in radians.                                                                                                                          |
| `cos`             | Returns the cosine of the angle.                            | `number -> number`                                                                  | The angle in radians.                                                                                                                          |
| `tan`             | Returns the tangent of the angle.                           | `number -> number`                                                                  | The angle in radians.                                                                                                                          |
| `random`          | Returns a random number in the range [0,1).                 | `-> number`                                                                         | N/A                                                                                                                                            |
| `randomVector`    | Returns an enumerable of random numbers in the range [0,1). | `number -> enumerable<number>`                                                      | The length of the enumerable to be generated.                                                                                                  |
| `randomMatrix`    | Returns a matrix of random numbers in the range [0,1).      | `number -> number -> enumerable<enumerable<number>>`                                | The number of rows and the number of columns.                                                                                                  |
| `zeroVector`      | Returns an enumerable of zeros.                             | `number -> enumerable<number>`                                                      | The length of the enumerable to be generated.                                                                                                  |
| `zeroMatrix`      | Returns a matrix of zeros.                                  | `number -> number -> enumerable<enumerable<number>>`                                | The number of rows and the number of columns.                                                                                                  |
| `range`           | Returns an enumerable containing a range of numbers.        | `number -> number -> number -> enumerable<number>`                                  | The start number, the end number, and the step size.                                                                                           |
| `lLog`            | Logs a message to the local worker's console.               | `string -> void`                                                                    | The string to be logged.                                                                                                                       |
| `sLog`            | Logs a message to the master worker's console.              | `string -> void`                                                                    | The string to be logged.                                                                                                                       |
| `lError`          | Logs an error to the local worker's console.                | `string -> void`                                                                    | The string to be logged.                                                                                                                       |
| `sError`          | Logs an error to the master worker's console.               | `string -> void`                                                                    | The string to be logged.                                                                                                                       |
| `floor`           | Round a number DOWN to the nearest integer.                 | `number -> number`                                                                  | The number to take the floor of.                                                                                                               |
| `ceiling`         | Round a number UP to the nearest integer.                   | `number -> number`                                                                  | The number to take the ceiling of.                                                                                                             |
| `nthRoot`         | Take the nth-root of the specified number.                  | `number -> number -> number`                                                        | The root to take (e.g. 2 is square root) and the number to root.                                                                               |
| `count`           | Count the number of elements in an enumerable.              | `enumerable<_> -> number`                                                           | The enumerable to find the size of.                                                                                                            |
| `time`            | Get the current UNIX timestamp in fractional seconds        | `-> number`                                                                         | N/A                                                                                                                                            |
| `subVector`       | Get a slice of a numeric enumeration.                       | `number -> number -> enumerable<number> -> enumerable<number>`                      | The element index to start the slice at, the length of the slice, and the vector to slice.                                                     |
| `subMatrix`       | Get a slice of a nested numeric enumeration.                | `number -> number -> number -> number -> enumerable<number> -> enumerable<number>`  | Row index to start at, row index to end at (non-inclusive), column index to start at, column index to end at (non-inclusive), matrix to slice. |

## Variable Declarations

Variable declarations follow a similar set of rules to many languages. They must have a specified type, a name, and a value to assign to it (uninitialized variables can only exist in [User-Defined Types](#user-defined-types)).  
Example: `number num = 4;`  
*Note: All variables are mutable except for variables of type `type`, which cannot change in value after they are assigned to prevent compile-time ambiguity.*  
For function and [user-defined type](#user-defined-types) declarations, the variable is added to the scope before the value, meaning that functions and types can be recursive.

## Shared Variables

By default, variables are stored locally within each worker. Updating these variables will only affect the value as seen by the worker in which it was updated. Shared variables are stored in a global store that can be accessed and written to by all workers. To declare a shared variable, simply add the keyword `shared` in front of the variable declaration.  
Example: `shared number num = 4;`

## Enumeration Blocks

An enumeration block in swarm is akin to a for loop in any other language, except the iterations of the loop are distributed to worker nodes to be executed in parallel instead of sequentially. The syntax for an enumeration block is as follows:  

```swarm
enumerable<number> nums = [1,2,3];

enumerate nums as num {
    lLog(num);
    ...
};
```

`num` is a local variable created that will take on a different value from `nums` in each distributed iteration.

## User-Defined Types

A user-defined type (also called an object type) is a collection of unordered declarations. The following is an example of a user-defined type declaration:  

```swarm
type Person = {
    constructor(n: string, a: number) => {
        name = n;
        age = a;
    };

    string name;

    number age = 0;

    fn isAlive(): bool {
        return age <= 116;
    }
};
```

There are three types of declarations within a type body: initialized variable declarations, uninitialized variable declarations, and constructors.

- **Initialized variable declarations:**  
The values of initialized members of a type are computed when the type is created.  
*Note: initialized members of a class cannot directly refer to members of the type. For instance, you cannot add a new member* `bool isalive = isAlive();` *. However, indirectly referring to other members of the type (such as using them in a function) is fine.*  
- **Uninitialized variable declarations:**  
These are not assigned any value by default. However, all members of a type must be instantiated in an object, meaning that you *must* initialize these members in the constructor.
- **Constructors:**  
Constructors can be thought of as functions that return instances of the type. Since they always return an instance of the type, the return type is omitted in the syntax. Constructors are not technically members of the type and thus cannot be access as members of the type.  
Types can have multiple constructors. If two or more constructors have the same function signature, the compiler will always choose the first one, making the second dead code.  
All uninitialized member variables must be initialized in *all* constructors.

## Objects

Objects are instances of user-defined types.  
To create an object, use the popular Java-esque syntax: `Person p = Person("George Washington", 291);`  
Constructors cannot be partially applied, so all arguments to the constructor must be given.

## Other

Swarm supports a few other generic programming-language-isms, such as `while` loops and `if` statements, `break` and `continue`. It currently does not support else statements or for loops.
