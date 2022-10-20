#ifndef SWARMVM_REFERENCESERIALIZEWALK
#define SWARMVM_REFERENCESERIALIZEWALK

#include "../../shared/util_string_helpers.h"
#include "../ReferenceWalk.h"

namespace swarmc::ISA {

    class ReferenceSerializeWalk : public ReferenceWalk<std::string> {
    public:
        std::string toString() const {
            return "ReferenceSerializeWalk<>";
        }

    protected:
        std::string walkLocationReference(LocationReference* ref) override {
            return "$" + ref->affinityString(ref->affinity()) + ":" + ref->name();
        }

        std::string walkTypeReference(TypeReference*) override {
            return "FIXME:TYPE";  // FIXME
        }

        std::string walkStringReference(StringReference* ref) override {
            std::string escaped = string_replace(string_replace(ref->value(), "\"", "\\\""), "\n", "\\\n");
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
