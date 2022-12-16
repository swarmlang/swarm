Need to be able to tunnel resources + operations.

## Motivating Examples

```text
enumerable<number> ranks = range(0, 5, 1);
enumerable<string> contents = ['', '', '', '', '', ''] of string;
with file('/home/user/fubar.txt') as f {
    enumerate ranks as rank {
        contents[rank] = read(f);
    }
}
```

Each job in the `enumerate` block will need the ability to `read` the file `f`.

Alternative, with a theoretical database connection:

```text
enumerable<string> queries = [
    'update table1 set name = "foo" where id = 1',
    'update table1 set name = "bar" where id = 2',
    'delete from table2 where id in (3, 4)'
]
with dbconn('127.0.0.1', 5432) as db {
    enumerate queries as query {
        dbexec(db, query);
    }
}
```

Here, each job in the `enumerate` block will need the ability to perform queries on the connection.

This can either be accomplished via tunneling the operations back to the control which originally opened the connection
or by replicating the connection on every control which uses it.

This might also be used in cases like:

```text
with dbconn('127.0.0.1', 5432) as db {
    ~dbexec(db, 'insert into analytics ("foo", "bar")');
    
    -- ... other work ...
}
```

The result of the `dbexec` call isn't used by the later program, so it is safely farmed off to be executed in the
background. Whatever node executes the deferred call will either need to tunnel operations back to the original control
that holds the `db`, or will need to open a new connection.


## Resource Fabric Design

To accomplish this, the SVM runtime will incorporate a distributed cluster layer called the Resource Fabric. The RF
will provide an interface for defining distributed resources and performing operations on those resources.

Broadly, we will allow for three categories of resources, based on how they are accessed by distributed jobs:

- Tunneled Resources
    - Operations performed on these resources are sent over the fabric to the node which owns the resource.
    - That node performs the operation and sends the result back over the fabric.
    - Potential issue: Tunneling operations may be inefficient, especially if a large number of jobs access a single resource.
- Replicated Resources
    - The resource implementation provides a way to "clone" the resource onto any nodes which use it.
    - e.g. a database connection can be replicated by having new nodes open a connection to the same server & credentials
    - e.g. a file which is copied to a scratch filesystem of the new nodes (only works for read-only)
    - Potential issue: Copying the resource may be slower than tunneling it or not deferring calls at all
- Exclusive Resources
    - Cannot be tunneled or replicated. Forces any deferred calls to execute exclusively on the node which owns the resource.
    - Potential issue: we could have a scheduling conflict for jobs which use 2+ exclusive resources held by different nodes.

To implement the RF, each node will spin up a dedicated RF thread which the main SVM interacts with via a queue. The
local SVM pushes operations onto the queue which are executed by the thread. The thread then pushes the results into
a separate queue to be retrieved by the local node. The thread/queue will also be responsible for performing tunneled
operations.

References to a resource will encode some unique identifier for that resource, the resource's provider, the resource's
category, and the ID of the node that owns it.

Functionally, resources are implemented by providers which will:

- Need the ability to introduce their own opaque types (e.g. a "p:FILE" type) for use by resources.
- Provide functions which operate on these resources. Such functions are native provider functions.
  - e.g. Prologue might register a function `f:READ_FILE` of type `p:FILE -> p:STRING`. The file resource will encode
    the path of the file to be read. The provider implementation will extract that path, read the file using native code,
    then return the result as a string reference.

The benefit of implementing resource operations as provider functions is that we can use the existing deferred call
infrastructure to execute the operations. Maybe: use a "meta" queue context called "resource_fabric". When an operation
on a resource needs to be performed, the call to the provider function is pushed onto this special queue with a context
filter limiting it to the node which owns the resource (or some similar filter). The owner's RF thread just waits for
and executes jobs pushed to this context.

A call w/ resources goes like this:

- Ask the provider function call for any resources that need to be acquired
- Obtain resources in this order
  - If there are any exclusive resources or tunneled resources marked as call-atomic:
    - Check the list of scheduling filters for conflicts
    - If none, push the call to the queue w/ those scheduling filters and wait for it to complete
  - If there are any replicable resources, clone them
- Once the resources have been acquired for the current context, perform the logic and return
- Release any replicated resources

