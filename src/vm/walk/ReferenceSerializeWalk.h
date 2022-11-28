#ifndef SWARMVM_REFERENCESERIALIZEWALK
#define SWARMVM_REFERENCESERIALIZEWALK

#include "../../shared/nslib.h"
#include "../ReferenceWalk.h"

namespace swarmc::ISA {

    /**
     * Runtime reference walk which writes reference values as SVI code.
     */
    class ReferenceSerializeWalk : public ReferenceWalk<std::string> {
    public:
        [[nodiscard]] std::string toString() const override {
            return "ReferenceSerializeWalk<>";
        }

    protected:
        std::string walkLocationReference(LocationReference* ref) override {
            return "$" + LocationReference::affinityString(ref->affinity()) + ":" + ref->name();
        }

        std::string walkTypeReference(TypeReference*) override {
            return "FIXME:TYPE";  // FIXME
        }

        std::string walkFunctionReference(FunctionReference* f) override {
            return "FIXME:FUNCTION";  // FIXME
        }

        std::string walkStreamReference(StreamReference*) override {
            return "FIXME:STREAM";
        }

        std::string walkStringReference(StringReference* ref) override {
            std::string escaped = nslib::str::replace(nslib::str::replace(ref->value(), "\"", "\\\""), "\n", "\\\n");
            return "\"" + escaped + "\"";
        }

        std::string walkNumberReference(NumberReference* ref) override {
            return std::to_string(ref->value());
        }

        std::string walkBooleanReference(BooleanReference* ref) override {
            if ( ref->value() ) return "true";
            return "false";
        }
    };

}

#endif //SWARMVM_REFERENCESERIALIZEWALK
