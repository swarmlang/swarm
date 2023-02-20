# Streams in the SVM

Goal: runtime providers should be able to instantiate and return streams which operate over native resources directly,
and their accesses are proxied through the runtime Fabric by the SVM.

Example:

- Node A creates a "FileLinesReaderStream" (`stream_f`) pointing to a file resource on its local node
- Node A pushes calls onto the queue which read from that stream
- Node B executes one of these jobs.
  - Node B loads `stream_f` and is given a `ProxyStream` with the same inner type.
  - Node B calls `pop()` on `stream_f`
  - The runtime asks the Fabric to call `pop()` on Node A's `stream_f` via the Fabric job queue
  - The runtime returns the result of the `pop()` call via Node B's `stream_f`

To accomplish this, `IStreamDriver` will need to be extended:

- Allow registering `IStream` instances created by external sources
- (For distributed backends) create an association between the stream and the node which owns it
