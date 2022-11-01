#include <cassert>
#include <cmath>
#include "../VirtualMachine.h"
#include "ExecuteWalk.h"

// FIXME: type checking for functions/function calls/&c. can be much better

namespace swarmc::Runtime {

    using namespace swarmc::ISA;

    std::string ExecuteWalk::toString() const {
        return "ExecuteWalk<vm: " + _vm->toString() + ">";
    }

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

    EnumerationReference* ExecuteWalk::ensureEnumeration(const ISA::Reference* ref) {
        // FIXME: eventually, this should generate a runtime exception
        assert(ref->tag() == ReferenceTag::ENUMERATION);
        return (EnumerationReference*) ref;
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

    Reference* ExecuteWalk::walkWhile(While* i) {
        // create expected type for callback
        auto callbackType = new Type::Lambda0(Type::Primitive::of(Type::Intrinsic::VOID));

        // load callback function & validate the type
        auto callback = ensureFunction(_vm->resolve(i->second()));

        // fixme: eventually, this should raise a runtime exception
        assert(callback->type()->isAssignableTo(callbackType));

        bool cond = _vm->resolve(i->first());
        if ( cond ) {
            // we want the VM to re-evaluate the condition after the call completes,
            // so rewind the program counter by one so the return jump is correct
            _vm->rewind();

            auto call = callback->fn()->call();
            _vm->call(call);
        }

        return nullptr;
    }

    // TODO: walkWith

    Reference* ExecuteWalk::walkEnumInit(EnumInit* i) {
        auto type = ensureType(i->first());
        return new EnumerationReference(type->value());
    }

    Reference* ExecuteWalk::walkEnumAppend(EnumAppend* i) {
        auto enumeration = ensureEnumeration(_vm->resolve(i->second()));
        auto value = _vm->resolve(i->first());

        // fixme: eventually this should generate a runtime error
        assert(value->type()->isAssignableTo(enumeration->type()->values()));
        enumeration->append(value);

        return nullptr;
    }

    Reference* ExecuteWalk::walkEnumPrepend(EnumPrepend* i) {
        auto enumeration = ensureEnumeration(_vm->resolve(i->second()));
        auto value = _vm->resolve(i->first());

        // fixme: eventually this should generate a runtime error
        assert(value->type()->isAssignableTo(enumeration->type()->values()));
        enumeration->prepend(value);

        return nullptr;
    }

    Reference* ExecuteWalk::walkEnumLength(EnumLength* i) {
        auto enumeration = ensureEnumeration(_vm->resolve(i->first()));
        return new NumberReference(static_cast<double>(enumeration->length()));
    }

    Reference* ExecuteWalk::walkEnumGet(EnumGet* i) {
        auto enumeration = ensureEnumeration(_vm->resolve(i->first()));
        auto idx = ensureNumber(_vm->resolve(i->second()));

        // fixme: eventually, this should generate a runtime error
        assert(idx->value() < enumeration->length());

        return enumeration->get(static_cast<size_t>(idx->value()));
    }

    Reference* ExecuteWalk::walkEnumSet(EnumSet* i) {
        auto enumeration = ensureEnumeration(_vm->resolve(i->first()));
        auto idx = ensureNumber(_vm->resolve(i->second()));
        auto value = _vm->resolve(i->third());

        // fixme: eventually, these should generate runtime errors
        assert(idx->value() <= enumeration->length());
        assert(value->type()->isAssignableTo(enumeration->type()->values()));

        enumeration->set(static_cast<size_t>(idx->value()), value);
        return nullptr;
    }

    Reference* ExecuteWalk::walkEnumerate(Enumerate* i) {
        auto elemType = ensureType(_vm->resolve(i->first()));
        auto enumeration = ensureEnumeration(_vm->resolve(i->second()));
        auto callback = ensureFunction(_vm->resolve(i->third()));

        Type::Primitive tVoid(Type::Intrinsic::VOID);
        Type::Primitive tNum(Type::Intrinsic::NUMBER);
        Type::Lambda1 callbackInner(&tNum, &tVoid);
        Type::Lambda1 callbackOuter(elemType->value(), &callbackInner);

        // fixme: eventually, this should generate a runtime error
        assert(callback->type()->isAssignableTo(&callbackOuter));

        _vm->enterQueueContext();

        for ( size_t idx = 0; idx < enumeration->length(); idx += 1 ) {
            auto elem = enumeration->get(idx);
            auto call = callback->fn()
                ->curry(elem)
                ->curry(new NumberReference(static_cast<double>(idx)))
                ->call();

            _vm->pushCall(call);
        }

        _vm->drain();
        _vm->exitQueueContext();
        return nullptr;
    }

    Reference* ExecuteWalk::walkBeginFunction(BeginFunction* i) {
        // Function definitions are read statically when the SVI is loaded into
        // the virtual machine. Calls jump to the instruction _after_ a beginfn.
        // So, we should never encounter this.
        console->warn("Detected virtual execution across boundary of function body: " + i->toString());
        return nullptr;
    }

    Reference* ExecuteWalk::walkFunctionParam(FunctionParam* i) {
        auto call = _vm->getCall();
        // fixme: eventually, this should generate a runtime exception
        assert(call != nullptr);

        auto param = call->popParam();
        auto loc = i->second();

        // fixme: eventually, this should generate a runtime exception
        assert(param.second->type()->isAssignableTo(loc->type()));

        _vm->store(loc, param.second);
        return nullptr;
    }

    Reference* ExecuteWalk::walkReturn0(Return0*) {
        _vm->returnToCaller();
        return nullptr;
    }

    ISA::Reference* ExecuteWalk::walkReturn1(ISA::Return1* i) {
        auto call = _vm->getCall();
        // fixme: eventually, this should generate a runtime exception
        assert(call != nullptr);

        // Resolve the return value
        auto ref = _vm->resolve(i->first());

        // Validate the return type
        // fixme: eventually, this should generate a runtime exception
        assert(ref->type()->isAssignableTo(call->returnType()));

        // Set the return value on the function call
        call->setReturn(ref);

        // Jump back to the caller
        _vm->returnToCaller();

        return nullptr;
    }

    Reference* ExecuteWalk::walkCurry(Curry* i) {
        auto fn = ensureFunction(_vm->resolve(i->first()));
        auto param = _vm->resolve(i->second());
        return new FunctionReference(fn->fn()->curry(param));
    }

    Reference* ExecuteWalk::walkCall0(Call0* i) {
        auto fn = ensureFunction(_vm->resolve(i->first()));
        auto call = fn->fn()->call();
        _vm->call(call);
        return call->getReturn();  // fixme: this doesn't actually return properly, since the "call" is an async jump
    }

    Reference* ExecuteWalk::walkCall1(Call1* i) {
        auto fn = ensureFunction(_vm->resolve(i->first()));
        auto param = _vm->resolve(i->second());
        auto call = fn->fn()->curry(param)->call();
        _vm->call(call);
        return call->getReturn();
    }

    Reference* ExecuteWalk::walkCallIf0(CallIf0* i) {
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        if ( cond->value() ) _vm->call(fn->fn()->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkCallIf1(CallIf1* i) {
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        auto param = _vm->resolve(i->third());
        if ( cond->value() ) _vm->call(fn->fn()->curry(param)->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkCallElse0(CallElse0* i) {
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        if ( !cond->value() ) _vm->call(fn->fn()->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkCallElse1(CallElse1* i) {
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        auto param = _vm->resolve(i->third());
        if ( !cond->value() ) _vm->call(fn->fn()->curry(param)->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkPushCall0(PushCall0* i) {
        auto fn = ensureFunction(_vm->resolve(i->first()));
        auto call = fn->fn()->call();
        _vm->pushCall(call);
        return call->getReturn();
    }

    Reference* ExecuteWalk::walkPushCall1(PushCall1* i) {
        auto fn = ensureFunction(_vm->resolve(i->first()));
        auto param = _vm->resolve(i->second());
        auto call = fn->fn()->curry(param)->call();
        _vm->pushCall(call);
        return call->getReturn();
    }

    Reference* ExecuteWalk::walkPushCallIf0(PushCallIf0* i) {
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        if ( cond->value() ) _vm->pushCall(fn->fn()->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkPushCallIf1(PushCallIf1* i) {
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        auto param = _vm->resolve(i->third());
        if ( cond->value() ) _vm->pushCall(fn->fn()->curry(param)->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkPushCallElse0(PushCallElse0* i) {
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        if ( !cond->value() ) _vm->pushCall(fn->fn()->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkPushCallElse1(PushCallElse1* i) {
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        auto param = _vm->resolve(i->third());
        if ( !cond->value() ) _vm->pushCall(fn->fn()->curry(param)->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkDrain(Drain*) {
        _vm->drain();
        return nullptr;
    }

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

        Reference* value = nullptr;

        // if the right-hand side is an call which can yield a value,
        // then we will need to make the call and wait for the return
        // to jump back here
        if ( eval->tag() == Tag::CALL0 || eval->tag() == Tag::CALL1 ) {
            // Check if we got here because of the return
            if ( _vm->hasFlag(StateFlag::JUMPED_FROM_RETURN) ) {
                // Get the return value and store that
                auto call = _vm->getCall();
                // FIXME: raise a runtime error?
                assert(call != nullptr);

                value = call->getReturn();
            } else {
                // Otherwise, we need to make the call. Step back so we
                // return-jump to the correct instruction.
                _vm->rewind();
                return walkOne(eval);
            }
        } else {
            value = walkOne(eval);
        }

        if ( value == nullptr ) {
            // FIXME: eventually, this should generate a runtime error
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
