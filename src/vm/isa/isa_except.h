#ifndef SWARMVM_EXCEPT
#define SWARMVM_EXCEPT

#include "../ISA.h"

namespace swarmc::ISA {

    class PushExceptionHandler1 : public UnaryInstruction<LocationReference> {
    public:
        PushExceptionHandler1(LocationReference* handlerFn) :
            UnaryInstruction<LocationReference>(Tag::PUSHEXHANDLER1, handlerFn) {}
    };

    class PushExceptionHandler2 : public BinaryInstruction<LocationReference, LocationReference> {
    public:
        PushExceptionHandler2(LocationReference* handlerFn, LocationReference* discriminatorFn) :
            BinaryInstruction<LocationReference, LocationReference>(Tag::PUSHEXHANDLER2, handlerFn, discriminatorFn) {}
    };

    class PopExceptionHandler : public UnaryInstruction<Reference> {
    public:
        PopExceptionHandler(Reference* handlerId) :
            UnaryInstruction<Reference>(Tag::POPEXHANDLER, handlerId) {}
    };

    class Raise : public UnaryInstruction<Reference> {
    public:
        Raise(Reference* exceptionId) :
            UnaryInstruction<Reference>(Tag::RAISE, exceptionId) {}
    };

    class Resume : public UnaryInstruction<LocationReference> {
    public:
        Resume(LocationReference* fn) :
            UnaryInstruction<LocationReference>(Tag::RESUME, fn) {}
    };

}

#endif //SWARMVM_EXCEPT
