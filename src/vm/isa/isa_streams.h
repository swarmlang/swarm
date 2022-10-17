#ifndef SWARMVM_STREAMS
#define SWARMVM_STREAMS

#include "../ISA.h"

namespace swarmc::ISA {

    class StreamInit : public BinaryInstruction<Reference, LocationReference> {
    public:
        StreamInit(Reference* type, LocationReference* dest) :
            BinaryInstruction<Reference, LocationReference>(Tag::STREAMINIT, type, dest) {}
    };

    class StreamPush : public BinaryInstruction<LocationReference, Reference> {
    public:
        StreamPush(LocationReference* stream, Reference* value) :
            BinaryInstruction<LocationReference, Reference>(Tag::STREAMPUSH, stream, value) {}
    };

    class StreamPop : public UnaryInstruction<LocationReference> {
    public:
        StreamPop(LocationReference* stream) :
            UnaryInstruction<LocationReference>(Tag::STREAMPOP, stream) {}
    };

    class StreamClose : public UnaryInstruction<LocationReference> {
    public:
        StreamClose(LocationReference* stream) :
            UnaryInstruction<LocationReference>(Tag::STREAMCLOSE, stream) {}
    };

    class StreamEmpty : public UnaryInstruction<LocationReference> {
    public:
        StreamEmpty(LocationReference* stream) :
            UnaryInstruction<LocationReference>(Tag::STREAMEMPTY, stream) {}
    };

    class Out : public StreamPush {
    public:
        Out(Reference* value) :
                StreamPush(new LocationReference(Affinity::SHARED, "STDOUT"), value) {}
    };

    class Err : public StreamPush {
    public:
        Err(Reference* value) :
                StreamPush(new LocationReference(Affinity::SHARED, "STDERR"), value) {}
    };

}

#endif //SWARMVM_STREAMS
