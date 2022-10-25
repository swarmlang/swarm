#include <cassert>
#include <cmath>
#include "../VirtualMachine.h"
#include "ExecuteWalk.h"

namespace swarmc::Runtime {

    using namespace swarmc::ISA;

    void ExecuteWalk::ensureType(const Reference* ref, const Type::Type* type) {
        // FIXME: eventually, this needs to generate a runtime type exception
        assert(ref->type()->isAssignableTo(type));
    }

    NumberReference* ExecuteWalk::ensureNumber(const Reference* ref) {
        ensureType(ref, Type::Primitive::of(Type::Intrinsic::NUMBER));
        // FIXME: eventually, this should probably generate a runtime exception
        assert(ref->tag() == ReferenceTag::NUMBER);
        return (NumberReference*) ref;
    }

    BooleanReference* ExecuteWalk::ensureBoolean(const Reference* ref) {
        ensureType(ref, Type::Primitive::of(Type::Intrinsic::BOOLEAN));
        // FIXME: eventually, this should probably generate a runtime exception
        assert(ref->tag() == ReferenceTag::BOOLEAN);
        return (BooleanReference*) ref;
    }

    Reference* ExecuteWalk::walkPlus(Plus* i) {
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new NumberReference(lhs->value() + rhs->value());
    }

    Reference* ExecuteWalk::walkMinus(Minus* i) {
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new NumberReference(lhs->value() - rhs->value());
    }

    Reference* ExecuteWalk::walkTimes(Times* i) {
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new NumberReference(lhs->value() * rhs->value());
    }

    Reference* ExecuteWalk::walkDivide(Divide* i) {
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        // FIXME: eventually, this should generate a runtime exception
        assert(rhs->value() != 0.0);
        return new NumberReference(lhs->value() / rhs->value());
    }

    Reference* ExecuteWalk::walkPower(Power* i) {
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new NumberReference(pow(lhs->value(), rhs->value()));
    }

    Reference* ExecuteWalk::walkMod(Mod* i) {
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new NumberReference(std::fmod(lhs->value(), rhs->value()));
    }

    Reference* ExecuteWalk::walkNegative(Negative* i) {
        auto opd = ensureNumber(i->first());
        return new NumberReference(- opd->value());
    }

    Reference* ExecuteWalk::walkGreaterThan(GreaterThan* i) {
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new BooleanReference(lhs->value() > rhs->value());
    }

    Reference* ExecuteWalk::walkGreaterThanOrEqual(GreaterThanOrEqual* i) {
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new BooleanReference(lhs->value() >= rhs->value());
    }

    Reference* ExecuteWalk::walkLessThan(LessThan* i) {
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new BooleanReference(lhs->value() < rhs->value());
    }

    Reference* ExecuteWalk::walkLessThanOrEqual(LessThanOrEqual* i) {
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new BooleanReference(lhs->value() <= rhs->value());
    }

    Reference* ExecuteWalk::walkAnd(And* i) {
        auto lhs = ensureBoolean(_vm->resolve(i->first()));
        auto rhs = ensureBoolean(_vm->resolve(i->second()));
        return new BooleanReference(lhs->value() && rhs->value());
    }

    Reference* ExecuteWalk::walkOr(Or* i) {
        auto lhs = ensureBoolean(_vm->resolve(i->first()));
        auto rhs = ensureBoolean(_vm->resolve(i->second()));
        return new BooleanReference(lhs->value() || rhs->value());
    }

    Reference* ExecuteWalk::walkXor(Xor* i) {
        auto lhs = ensureBoolean(_vm->resolve(i->first()));
        auto rhs = ensureBoolean(_vm->resolve(i->second()));
        return new BooleanReference(!lhs->value() != !rhs->value());
    }

    Reference* ExecuteWalk::walkNand(Nand* i) {
        auto lhs = ensureBoolean(_vm->resolve(i->first()));
        auto rhs = ensureBoolean(_vm->resolve(i->second()));
        return new BooleanReference(!(lhs->value() && rhs->value()));
    }

    Reference* ExecuteWalk::walkNor(Nor* i) {
        auto lhs = ensureBoolean(_vm->resolve(i->first()));
        auto rhs = ensureBoolean(_vm->resolve(i->second()));
        return new BooleanReference(!(lhs->value() || rhs->value()));
    }

    Reference* ExecuteWalk::walkNot(Not* i) {
        auto opd = ensureBoolean(_vm->resolve(i->first()));
        return new BooleanReference(!opd->value());
    }

    // TODO: walkWhile
    // TODO: walkWith
}
