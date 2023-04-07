#include "Type.h"
#include "AST.h"

namespace nslib {
    [[nodiscard]] std::string s(swarmc::Type::Intrinsic v) {
        if ( v == swarmc::Type::Intrinsic::STRING ) return "Type::Intrinsic(STRING)";
        if ( v == swarmc::Type::Intrinsic::NUMBER ) return "Type::Intrinsic(NUMBER)";
        if ( v == swarmc::Type::Intrinsic::BOOLEAN ) return "Type::Intrinsic(BOOLEAN)";
        if ( v == swarmc::Type::Intrinsic::ERROR ) return "Type::Intrinsic(ERROR)";
        if ( v == swarmc::Type::Intrinsic::VOID ) return "Type::Intrinsic(VOID)";
        if ( v == swarmc::Type::Intrinsic::UNIT ) return "Type::Intrinsic(UNIT)";
        if ( v == swarmc::Type::Intrinsic::TYPE ) return "Type::Intrinsic(TYPE)";
        if ( v == swarmc::Type::Intrinsic::MAP ) return "Type::Intrinsic(MAP)";
        if ( v == swarmc::Type::Intrinsic::ENUMERABLE ) return "Type::Intrinsic(ENUMERABLE)";
        if ( v == swarmc::Type::Intrinsic::STREAM ) return "Type::Intrinsic(STREAM)";
        if ( v == swarmc::Type::Intrinsic::LAMBDA0 ) return "Type::Intrinsic(LAMBDA0)";
        if ( v == swarmc::Type::Intrinsic::LAMBDA1 ) return "Type::Intrinsic(LAMBDA1)";
        if ( v == swarmc::Type::Intrinsic::RESOURCE ) return "Type::Intrinsic(RESOURCE)";
        if ( v == swarmc::Type::Intrinsic::AMBIGUOUS ) return "Type::Intrinsic(AMBIGUOUS)";
        if ( v == swarmc::Type::Intrinsic::CONTRADICTION ) return "Type::Intrinsic(CONTRADICTION)";
        if ( v == swarmc::Type::Intrinsic::OPAQUE ) return "Type::Intrinsic(OPAQUE)";
        if ( v == swarmc::Type::Intrinsic::OBJECT_PROTO ) return "Type::Intrinsic(OBJECT_PROTO)";
        if ( v == swarmc::Type::Intrinsic::OBJECT ) return "Type::Intrinsic(OBJECT)";
        return "Type::Intrinsic(UNKNOWN)";
    }
}

namespace swarmc::Type {

    std::size_t Type::_nextId = 0;

    std::map<std::size_t, std::vector<std::size_t>> Type::_assignableCache;

    std::map<Intrinsic, Primitive*> Primitive::_primitives;

    std::map<std::string, Opaque*> Opaque::_opaques;

    Ambiguous* Ambiguous::_inst = nullptr;

    Ambiguous::Ambiguous(Lang::IdentifierNode* id) : Type(), _typeid(useref(id)) {}
    Ambiguous::~Ambiguous() { freeref(_typeid); }

    Type* Ambiguous::disambiguateStatically() {
        auto sym = _typeid->symbol();
        if (sym == nullptr) {
            throw Errors::SwarmError("Failed to disambiguate type at " + _typeid->position()->toString());
        }
        assert(sym->kind() == Lang::SemanticSymbolKind::VARIABLE);
        if (((Lang::VariableSymbol*)sym)->getObjectType() == nullptr) {
            throw Errors::SwarmError(_typeid->name() + " is not a type!");
        }
        auto value = useref(((Lang::VariableSymbol*)sym)->getObjectType()->value());
        freeref(this);
        return value->disambiguateStatically();
    } 

}
