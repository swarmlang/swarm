#ifndef SWARMVM_STREAMS
#define SWARMVM_STREAMS

#include "../ISA.h"

namespace swarmc::ISA {

    class StreamInit : public UnaryInstruction<Reference> {
    public:
        explicit StreamInit(Reference* type) :
            UnaryInstruction<Reference>(Tag::STREAMINIT, type) {}
        StreamInit* copy() const override {
            return new StreamInit(_first->copy());
        }
    };

    class StreamPush : public BinaryInstruction<LocationReference, Reference> {
    public:
        StreamPush(LocationReference* stream, Reference* value) :
            BinaryInstruction<LocationReference, Reference>(Tag::STREAMPUSH, stream, value) {}
        StreamPush* copy() const override {
            return new StreamPush(_first->copy(), _second->copy());
        }
    };

    class StreamPop : public UnaryInstruction<LocationReference> {
    public:
        explicit StreamPop(LocationReference* stream) :
            UnaryInstruction<LocationReference>(Tag::STREAMPOP, stream) {}
        StreamPop* copy() const override {
            return new StreamPop(_first->copy());
        }
    };

    class StreamClose : public UnaryInstruction<LocationReference> {
    public:
        explicit StreamClose(LocationReference* stream) :
            UnaryInstruction<LocationReference>(Tag::STREAMCLOSE, stream) {}
        StreamClose* copy() const override {
            return new StreamClose(_first->copy());
        }
    };

    class StreamEmpty : public UnaryInstruction<LocationReference> {
    public:
        explicit StreamEmpty(LocationReference* stream) :
            UnaryInstruction<LocationReference>(Tag::STREAMEMPTY, stream) {}
        StreamEmpty* copy() const override {
            return new StreamEmpty(_first->copy());
        }
    };

    class Out : public StreamPush {
    public:
        explicit Out(Reference* value) :
                StreamPush(new LocationReference(Affinity::SHARED, "STDOUT"), value) {}
    };

    class Err : public StreamPush {
    public:
        explicit Err(Reference* value) :
                StreamPush(new LocationReference(Affinity::SHARED, "STDERR"), value) {}
    };

}

#endif //SWARMVM_STREAMS
