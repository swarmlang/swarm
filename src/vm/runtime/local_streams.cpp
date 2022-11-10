#include <cassert>
#include "local_streams.h"
#include "../../lang/Type.h"
#include "../isa_meta.h"

namespace swarmc::Runtime {

    const Type::Type* LocalStream::innerType() {
        return Type::Primitive::of(Type::Intrinsic::STRING);
    }


    void LocalOutputStream::push(ISA::Reference* value) {
        assert(value->tag() == ISA::ReferenceTag::STRING);
        auto string = (ISA::StringReference*) value;
        console->info("[l] " + string->value());
    }


    void LocalErrorStream::push(ISA::Reference* value) {
        assert(value->tag() == ISA::ReferenceTag::STRING);
        auto string = (ISA::StringReference*) value;
        console->error("[l] " + string->value());
    }

}
