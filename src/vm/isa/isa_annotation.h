#ifndef SWARMVM_ISA_ANNOTATION
#define SWARMVM_ISA_ANNOTATION

#include "../ISA.h"

namespace swarmc::ISA {

    class PositionAnnotation : public TrinaryInstruction<StringReference, NumberReference, NumberReference> {
    public:
        PositionAnnotation(StringReference* file, NumberReference* line, NumberReference* col):
            TrinaryInstruction<StringReference, NumberReference, NumberReference>(Tag::POSITION, file, line, col) {}

        PositionAnnotation* copy() const override {
            return new PositionAnnotation(first(), second(), third());
        }
    };

}

#endif //SWARMVM_ISA_ANNOTATION
