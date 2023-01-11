#ifndef SWARMVM_ISA_ANNOTATION
#define SWARMVM_ISA_ANNOTATION

#include "../ISA.h"

namespace swarmc::ISA {

    class PositionAnnotation : public TrinaryInstruction<StringReference, NumberReference, NumberReference> {
    public:
        PositionAnnotation(StringReference* file, NumberReference* line, NumberReference* col):
            TrinaryInstruction<StringReference, NumberReference, NumberReference>(Tag::POSITION, useref(file), useref(line), useref(col)) {}

        ~PositionAnnotation() override {
            freeref(_first);
            freeref(_second);
            freeref(_third);
        }

        [[nodiscard]] PositionAnnotation* copy() const override {
            return new PositionAnnotation(first(), second(), third());
        }
    };

}

#endif //SWARMVM_ISA_ANNOTATION
