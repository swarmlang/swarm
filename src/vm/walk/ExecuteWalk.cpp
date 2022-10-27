#include <cassert>
#include <cmath>
#include "../VirtualMachine.h"
#include "ExecuteWalk.h"

// FIXME: type checking for functions/function calls/&c. can be much better

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

    TypeReference* ExecuteWalk::ensureType(const Reference* ref) {
        ensureType(ref, Type::Primitive::of(Type::Intrinsic::TYPE));
        // FIXME: eventually, this should probably generate a runtime exception
        assert(ref->tag() == ReferenceTag::TYPE);
        return (TypeReference*) ref;
    }

    StringReference* ExecuteWalk::ensureString(const Reference* ref) {
        ensureType(ref, Type::Primitive::of(Type::Intrinsic::STRING));
        // FIXME: eventually, this should probably generate a runtime exception
        assert(ref->tag() == ReferenceTag::STRING);
        return (StringReference*) ref;
    }

    FunctionReference* ExecuteWalk::ensureFunction(const ISA::Reference* ref) {
        // FIXME: eventually, this should generate a runtime exception
        assert(ref->tag() == ReferenceTag::FUNCTION);
        return (FunctionReference*) ref;
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
    // TODO: walkEnum*

    Reference* ExecuteWalk::walkBeginFunction(BeginFunction* i) {
        // Function definitions are read statically when the SVI is loaded into
        // the virtual machine. Calls jump to the instruction _after_ a beginfn.
        // So, we should never encounter this.
        console->warn("Detected virtual execution across boundary of function body: " + i->toString());
        return nullptr;
    }

    // TODO: walkFunctionParam
    // TODO: walkReturn1
    // TODO: walkReturn0

    Reference* ExecuteWalk::walkCurry(Curry* i) {
        auto fn = ensureFunction(_vm->resolve(i->first()));
        auto param = _vm->resolve(i->second());
        return new FunctionReference(fn->fn()->curry(param));
    }

    Reference* ExecuteWalk::walkCall0(ISA::Call0* i) {
        auto fn = ensureFunction(_vm->resolve(i->first()));

        // FIXME: eventually, this needs to generate a runtime type error
        assert(fn->fn()->paramTypes().empty());

        return nullptr;
    }

    // TODO: walkCall*
    // TODO: walkPushCall*
    // TODO: walkMap*

    Reference* ExecuteWalk::walkTypify(Typify* i) {
        auto loc = i->first();
        auto type = ensureType(i->second());
        _vm->typify(loc, type->value());
        return nullptr;
    }

    Reference* ExecuteWalk::walkAssignValue(AssignValue* i) {
        auto loc = i->first();
        auto value = i->second();

        if ( loc->type()->isAmbiguous() ) {
            loc->setType(value->type());
        }

        // FIXME: eventually, this needs to generate a runtime type error
        assert(value->type()->isAssignableTo(loc->type()));

        _vm->store(loc, value);
        return nullptr;
    }

    Reference* ExecuteWalk::walkAssignEval(AssignEval* i) {
        auto loc = i->first();
        auto eval = i->second();

        auto value = walkOne(eval);
        if ( value == nullptr ) {
            // FIXME: eventually, this should probably generate a runtime error
            throw Errors::SwarmError("Attempted to assign result of an instruction which does not yield a value: " + eval->toString());
        }

        if ( loc->type()->isAmbiguous() ) {
            loc->setType(value->type());
        }

        // FIXME: eventually, this needs to generate a runtime type error
        assert(value->type()->isAssignableTo(loc->type()));

        _vm->store(loc, value);
        return nullptr;
    }

    Reference* ExecuteWalk::walkLock(Lock* i) {
        _vm->lock(i->first());
        return nullptr;
    }

    Reference* ExecuteWalk::walkUnlock(Unlock* i) {
        _vm->unlock(i->first());
        return nullptr;
    }

    Reference* ExecuteWalk::walkScopeOf(ScopeOf* i) {
        _vm->shadow(i->first());
        return nullptr;
    }

    // TODO: walkStream*
    // TODO: walkOut
    // TODO: walkErr

    Reference* ExecuteWalk::walkStringConcat(StringConcat* i) {
        auto lhs = ensureString(_vm->resolve(i->first()));
        auto rhs = ensureString(_vm->resolve(i->second()));
        return new StringReference(lhs->value() + rhs->value());
    }

    Reference* ExecuteWalk::walkStringLength(StringLength* i) {
        auto opd = ensureString(_vm->resolve(i->first()));
        return new NumberReference(opd->value().length());
    }

    Reference* ExecuteWalk::walkStringSliceFrom(StringSliceFrom* i) {
        // FIXME: handle negative indices

        auto str = ensureString(_vm->resolve(i->first()));
        auto from = ensureNumber(_vm->resolve(i->second()));
        return new StringReference(str->value().substr(from->value()));
    }

    Reference* ExecuteWalk::walkStringSliceFromTo(StringSliceFromTo* i) {
        // FIXME: handle negative indices

        auto str = ensureString(_vm->resolve(i->first()));
        auto from = ensureNumber(_vm->resolve(i->second()));
        auto to = ensureNumber(_vm->resolve(i->third()));
        return new StringReference(str->value().substr(from->value(), to->value()));
    }

    Reference* ExecuteWalk::walkTypeOf(TypeOf* i) {
        // FIXME: handle ambiguous type narrowing?

        auto opd = _vm->resolve(i->first());
        return new TypeReference(opd->type());
    }

    Reference* ExecuteWalk::walkIsCompatible(IsCompatible* i) {
        // FIXME: handle ambiguous type narrowing?

        auto lhs = _vm->resolve(i->first());
        auto rhs = _vm->resolve(i->second());
        return new BooleanReference(rhs->type()->isAssignableTo(lhs->type()));
    }

    // TODO: walkPushExceptionHandler1
    // TODO: walkPushExceptionHandler2
    // TODO: walkPopExceptionHandler
    // TODO: walkRaise
    // TODO: walkResume
}
