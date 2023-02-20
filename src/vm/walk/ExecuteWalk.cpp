#include <cassert>
#include <cmath>
#include "../../shared/nslib.h"
#include "../VirtualMachine.h"
#include "ExecuteWalk.h"

// FIXME: type checking for functions/function calls/&c. can be much better

namespace swarmc::Runtime {

    using namespace swarmc::ISA;

    std::string ExecuteWalk::toString() const {
        return "ExecuteWalk<vm: " + _vm->toString() + ">";
    }

    void ExecuteWalk::ensureType(const Reference* ref, const Type::Type* type) {
        if ( !ref->typei()->isAssignableTo(type) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::TypeError,
                "Value " + s(ref) + " of type " + s(ref->typei()) + " is not assignable to required type " + s(type)
            );
        }
    }

    void ExecuteWalk::ensureType(const Reference* ref, const InlineRefHandle<Type::Type>& type) {
        ensureType(ref, type.get());
    }

    Reference* ExecuteWalk::walkOne(Instruction* inst) {
        try {
            return ISAWalk<Reference*>::walkOne(inst);
        } catch (Errors::RuntimeError& e) {
            std::string msg = e.what();
            logger->error(msg);
            _vm->raise((std::size_t) e.code());
        }

        return nullptr;
    }

    Reference* ExecuteWalk::walkOnePropagatingExceptions(Instruction* inst) {
        return ISAWalk<Reference*>::walkOne(inst);
    }

    NumberReference* ExecuteWalk::ensureNumber(const Reference* ref) {
        verbose("ensureNumber: " + ref->toString());
        ensureType(ref, Type::Primitive::of(Type::Intrinsic::NUMBER));
        if ( ref->tag() != ReferenceTag::NUMBER ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidReferenceImplementation,
                "Reference " + s(ref) + " has type " + s(ref->typei()) + " but invalid tag " + s(ref->tag())
            );
        }
        return (NumberReference*) ref;
    }

    BooleanReference* ExecuteWalk::ensureBoolean(const Reference* ref) {
        verbose("ensureBoolean: " + ref->toString());
        ensureType(ref, Type::Primitive::of(Type::Intrinsic::BOOLEAN));
        if ( ref->tag() != ReferenceTag::BOOLEAN ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidReferenceImplementation,
                "Reference " + s(ref) + " has type " + s(ref->typei()) + " but invalid tag " + s(ref->tag())
            );
        }
        return (BooleanReference*) ref;
    }

    TypeReference* ExecuteWalk::ensureType(const Reference* ref) {
        verbose("ensureType: " + ref->toString());
        ensureType(ref, Type::Primitive::of(Type::Intrinsic::TYPE));
        if ( ref->tag() != ReferenceTag::TYPE ) {
            throw Errors::RuntimeError(
                    Errors::RuntimeExCode::InvalidReferenceImplementation,
                    "Reference " + s(ref) + " has type " + s(ref->typei()) + " but invalid tag " + s(ref->tag())
            );
        }
        return (TypeReference*) ref;
    }

    ObjectTypeReference* ExecuteWalk::ensureObjectType(const Reference* ref) {
        verbose("ensureType: " + ref->toString());
        ensureType(ref, Type::Primitive::of(Type::Intrinsic::TYPE));
        if ( ref->tag() != ReferenceTag::OTYPE ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidReferenceImplementation,
                "Reference " + s(ref) + " has type " + s(ref->typei()) + " but invalid tag " + s(ref->tag())
            );
        }
        return (ObjectTypeReference*) ref;
    }

    StringReference* ExecuteWalk::ensureString(const Reference* ref) {
        verbose("ensureString: " + ref->toString());
        ensureType(ref, Type::Primitive::of(Type::Intrinsic::STRING));
        if ( ref->tag() != ReferenceTag::STRING ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidReferenceImplementation,
                "Reference " + s(ref) + " has type " + s(ref->typei()) + " but invalid tag " + s(ref->tag())
            );
        }
        return (StringReference*) ref;
    }

    FunctionReference* ExecuteWalk::ensureFunction(const Reference* ref) {
        verbose("ensureFunction: " + ref->toString());
        if ( ref->tag() != ReferenceTag::FUNCTION ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidReferenceImplementation,
                "Reference " + s(ref) + " has type " + s(ref->typei()) + " but invalid tag " + s(ref->tag())
            );
        }
        return (FunctionReference*) ref;
    }

    InlineRefHandle<FunctionReference> ExecuteWalk::ensureFunction(const InlineRefHandle<Reference>& ref) {
        return inlineref<FunctionReference>(ensureFunction(ref.get()));
    }

    EnumerationReference* ExecuteWalk::ensureEnumeration(const Reference* ref) {
        verbose("ensureEnumeration: " + ref->toString());
        if ( ref->tag() != ReferenceTag::ENUMERATION ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidReferenceImplementation,
                "Reference " + s(ref) + " has type " + s(ref->typei()) + " but invalid tag " + s(ref->tag())
            );
        }
        return (EnumerationReference*) ref;
    }

    StreamReference* ExecuteWalk::ensureStream(const Reference* ref) {
        verbose("ensureStream: " + ref->toString());
        if ( ref->tag() != ReferenceTag::STREAM ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidReferenceImplementation,
                "Reference " + s(ref) + " has type " + s(ref->typei()) + " but invalid tag " + s(ref->tag())
            );
        }
        return (StreamReference*) ref;
    }

    MapReference* ExecuteWalk::ensureMap(const Reference* ref) {
        verbose("ensureMap: " + ref->toString());
        if ( ref->tag() != ReferenceTag::MAP ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidReferenceImplementation,
                "Reference " + s(ref) + " has type " + s(ref->typei()) + " but invalid tag " + s(ref->tag())
            );
        }
        return (MapReference*) ref;
    }

    ResourceReference* ExecuteWalk::ensureResource(const Reference* ref) {
        verbose("ensureResource: " + ref->toString());
        if ( ref->tag() != ReferenceTag::RESOURCE ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidReferenceImplementation,
                "Reference " + s(ref) + " has type " + s(ref->typei()) + " but invalid tag " + s(ref->tag())
            );
        }
        return (ResourceReference*) ref;
    }

    Reference* ExecuteWalk::walkPosition(PositionAnnotation* i) {
        throw Errors::SwarmError("Attempted to interpret debugging annotation: " + i->toString());
    }

    Reference* ExecuteWalk::walkPlus(Plus* i) {
        verbose("plus " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new NumberReference(lhs->value() + rhs->value());
    }

    Reference* ExecuteWalk::walkMinus(Minus* i) {
        verbose("minus " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new NumberReference(lhs->value() - rhs->value());
    }

    Reference* ExecuteWalk::walkTimes(Times* i) {
        verbose("times " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new NumberReference(lhs->value() * rhs->value());
    }

    Reference* ExecuteWalk::walkDivide(Divide* i) {
        verbose("divide " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));

        if ( rhs->value() == 0.0 ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::DivisionByZero,
                "Attempted to divide by zero"
            );
        }

        return new NumberReference(lhs->value() / rhs->value());
    }

    Reference* ExecuteWalk::walkPower(Power* i) {
        verbose("power " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new NumberReference(pow(lhs->value(), rhs->value()));
    }

    Reference* ExecuteWalk::walkMod(Mod* i) {
        verbose("mod " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new NumberReference(std::fmod(lhs->value(), rhs->value()));
    }

    Reference* ExecuteWalk::walkNegative(Negative* i) {
        verbose("neg " + i->first()->toString());
        auto opd = ensureNumber(_vm->resolve(i->first()));
        return new NumberReference(- opd->value());
    }

    Reference* ExecuteWalk::walkGreaterThan(GreaterThan* i) {
        verbose("gt " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new BooleanReference(lhs->value() > rhs->value());
    }

    Reference* ExecuteWalk::walkGreaterThanOrEqual(GreaterThanOrEqual* i) {
        verbose("gte " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new BooleanReference(lhs->value() >= rhs->value());
    }

    Reference* ExecuteWalk::walkLessThan(LessThan* i) {
        verbose("lt " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new BooleanReference(lhs->value() < rhs->value());
    }

    Reference* ExecuteWalk::walkLessThanOrEqual(LessThanOrEqual* i) {
        verbose("lte " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureNumber(_vm->resolve(i->first()));
        auto rhs = ensureNumber(_vm->resolve(i->second()));
        return new BooleanReference(lhs->value() <= rhs->value());
    }

    Reference* ExecuteWalk::walkAnd(And* i) {
        verbose("and " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureBoolean(_vm->resolve(i->first()));
        auto rhs = ensureBoolean(_vm->resolve(i->second()));
        return new BooleanReference(lhs->value() && rhs->value());
    }

    Reference* ExecuteWalk::walkOr(Or* i) {
        verbose("or " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureBoolean(_vm->resolve(i->first()));
        auto rhs = ensureBoolean(_vm->resolve(i->second()));
        return new BooleanReference(lhs->value() || rhs->value());
    }

    Reference* ExecuteWalk::walkXor(Xor* i) {
        verbose("xor " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureBoolean(_vm->resolve(i->first()));
        auto rhs = ensureBoolean(_vm->resolve(i->second()));
        return new BooleanReference(!lhs->value() != !rhs->value());
    }

    Reference* ExecuteWalk::walkNand(Nand* i) {
        verbose("nand " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureBoolean(_vm->resolve(i->first()));
        auto rhs = ensureBoolean(_vm->resolve(i->second()));
        return new BooleanReference(!(lhs->value() && rhs->value()));
    }

    Reference* ExecuteWalk::walkNor(Nor* i) {
        verbose("nor " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureBoolean(_vm->resolve(i->first()));
        auto rhs = ensureBoolean(_vm->resolve(i->second()));
        return new BooleanReference(!(lhs->value() || rhs->value()));
    }

    Reference* ExecuteWalk::walkNot(Not* i) {
        verbose("not " + i->first()->toString());
        auto opd = ensureBoolean(_vm->resolve(i->first()));
        return new BooleanReference(!opd->value());
    }

    Reference* ExecuteWalk::walkWhile(While* i) {
        verbose("while " + i->first()->toString() + " " + i->second()->toString());

        // create expected type for callback
        auto callbackType = new Type::Lambda0(Type::Primitive::of(Type::Intrinsic::VOID));
        GC_LOCAL_REF(callbackType)

        // load callback function & validate the type
        auto callback = ensureFunction(_vm->resolve(i->second()));
        GC_LOCAL_REF(callback)

        if ( !callback->typei()->isAssignableTo(callbackType) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::WhileCallbackTypeInvalid,
                "Invalid while callback " + s(callback) + " (expected: " + s(callbackType) + ", got: " + s(callback->typei()) + ")"
            );
        }

        auto cond = ensureBoolean(_vm->resolve(i->first()));
        if ( cond->value() ) {
            // we want the VM to re-evaluate the condition after the call completes,
            // so rewind the program counter by one so the return jump is correct
            _vm->rewind();

            auto call = callback->fn()->call();
            GC_LOCAL_REF(call);
            _vm->call(call);
        }

        return nullptr;
    }

    Reference* ExecuteWalk::walkWith(With* i) {
        verbose("with " + i->first()->toString() + " " + i->second()->toString());
        auto resource = ensureResource(_vm->resolve(i->first()));

        auto callbackType = new Type::Lambda1(resource->type(), Type::Primitive::of(Type::Intrinsic::VOID));
        GC_LOCAL_REF(callbackType)

        auto callback = ensureFunction(_vm->resolve(i->second()));

        if ( !callback->typei()->isAssignableTo(callbackType) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::WithCallbackTypeInvalid,
                "Invalid with callback " + s(callback) + " (expected: " + s(callbackType) + ", got: " + s(callback->typei()) + ")"
            );
        }

        // Open the resource
        resource->resource()->acquire(_vm);

        // Perform the call
        auto call = callback->fn()->curryi(resource)->call();
        GC_LOCAL_REF(call);

        _vm->pushCall(call);
        _vm->drain();

        // Close the resource
        resource->resource()->release(_vm);
        return nullptr;
    }

    Reference* ExecuteWalk::walkEnumInit(EnumInit* i) {
        verbose("enuminit " + i->first()->toString());
        auto type = ensureType(i->first());
        return new EnumerationReference(type->value());
    }

    Reference* ExecuteWalk::walkEnumAppend(EnumAppend* i) {
        verbose("enumappend " + i->first()->toString() + " " + i->second()->toString());
        auto enumeration = ensureEnumeration(_vm->resolve(i->second()));
        auto value = _vm->resolve(i->first());

        auto enumType = enumeration->type();
        GC_LOCAL_REF(enumType)
        if ( !value->typei()->isAssignableTo(enumType->valuesi()) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidValueTypeForEnum,
                "Cannot append value to enum: invalid type (expected: " + s(enumType->valuesi()) + ", got: " + s(value->typei()) + ")"
            );
        }

        enumeration->append(value);

        return nullptr;
    }

    Reference* ExecuteWalk::walkEnumPrepend(EnumPrepend* i) {
        verbose("enumprepend " + i->first()->toString() + " " + i->second()->toString());
        auto enumeration = ensureEnumeration(_vm->resolve(i->second()));
        auto value = _vm->resolve(i->first());

        auto enumType = enumeration->type();
        GC_LOCAL_REF(enumType)
        if ( !value->typei()->isAssignableTo(enumType->valuesi()) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidValueTypeForEnum,
                "Cannot prepend value to enum: invalid type (expected: " + s(enumType->valuesi()) + ", got: " + s(value->typei()) + ")"
            );
        }

        enumeration->prepend(value);

        return nullptr;
    }

    Reference* ExecuteWalk::walkEnumLength(EnumLength* i) {
        verbose("enumlength " + i->first()->toString());
        auto enumeration = ensureEnumeration(_vm->resolve(i->first()));
        return new NumberReference(static_cast<double>(enumeration->length()));
    }

    Reference* ExecuteWalk::walkEnumGet(EnumGet* i) {
        verbose("enumget " + i->first()->toString() + " " + i->second()->toString());
        auto enumeration = ensureEnumeration(_vm->resolve(i->first()));
        auto idx = ensureNumber(_vm->resolve(i->second()));

        if ( static_cast<std::size_t>(idx->value()) >= enumeration->length() ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::EnumIndexOutOfBounds,
                "Index " + s(idx->value()) + " out of bounds for enumeration " + s(enumeration)
            );
        }

        return enumeration->get(static_cast<std::size_t>(idx->value()));
    }

    Reference* ExecuteWalk::walkEnumSet(EnumSet* i) {
        verbose("enumset " + i->first()->toString() + " " + i->second()->toString() + " " + i->third()->toString());
        auto enumeration = ensureEnumeration(_vm->resolve(i->first()));
        auto idx = ensureNumber(_vm->resolve(i->second()));
        auto value = _vm->resolve(i->third());

        if ( static_cast<std::size_t>(idx->value()) > enumeration->length() ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::EnumIndexOutOfBounds,
                "Index " + s(idx->value()) + " out of bounds for enumeration " + s(enumeration)
            );
        }

        auto enumType = enumeration->type();
        GC_LOCAL_REF(enumType)
        if ( !value->typei()->isAssignableTo(enumType->valuesi()) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::TypeError,
                "Element " + s(value) + " is incompatible with the type of the enumeration " + s(enumeration) + " (expected: " + s(enumType->valuesi()) + ", got: " + s(value->typei()) + ")"
            );
        }

        enumeration->set(static_cast<std::size_t>(idx->value()), value);
        return nullptr;
    }

    Reference* ExecuteWalk::walkEnumerate(Enumerate* i) {
        verbose("enumerate " + i->first()->toString() + " " + i->second()->toString() + " " + i->third()->toString());
        auto elemType = ensureType(_vm->resolve(i->first()));
        auto enumeration = ensureEnumeration(_vm->resolve(i->second()));
        auto callback = ensureFunction(_vm->resolve(i->third()));
        GC_LOCAL_REF(callback)

        Type::Lambda1 callbackInner(Type::Primitive::of(Type::Intrinsic::NUMBER), Type::Primitive::of(Type::Intrinsic::VOID));
        GC_NO_REF(callbackInner)

        Type::Lambda1 callbackOuter(elemType->value(), &callbackInner);
        GC_NO_REF(callbackOuter)

        if ( !callback->typei()->isAssignableTo(&callbackOuter) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::EnumerateCallbackTypeInvalid,
                "Invalid enumerate callback " + s(callback) + " (expected: " + s(callbackOuter) + ", got: " + s(callback->typei()) + ")"
            );
        }

        _vm->enterQueueContext();

        for ( std::size_t idx = 0; idx < enumeration->length(); idx += 1 ) {
            auto elem = enumeration->get(idx);
            auto call = callback->fn()
                ->curryi(elem)
                ->curryi(new NumberReference(static_cast<double>(idx)))
                ->call();

            GC_LOCAL_REF(call);

            auto job = _vm->pushCall(call);
            GC_LOCAL_REF(job)
        }

        _vm->drain();
        _vm->exitQueueContext();
        return nullptr;
    }

    Reference* ExecuteWalk::walkBeginFunction(BeginFunction* i) {
        _vm->skip(i);
        _vm->rewind();  // skip takes us to the instruction we want to jump to, but we will advance after this
        return nullptr;
    }

    Reference* ExecuteWalk::walkFunctionParam(FunctionParam* i) {
        verbose("fnparam " + i->first()->toString() + " " + i->second()->toString());

        auto call = _vm->getCall();
        if ( call == nullptr ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::FnParamOutsideCall,
                "Attempted to load fnparam outside call context"
            );
        }

        auto loc = i->second();
        auto param = _vm->resolve(call->popParam().second);

        if ( !param->typei()->isAssignableTo(loc->typei()) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::TypeError,
                "Value " + s(param) + " has incompatible type for parameter " + s(loc) + " (expected: " + s(loc->typei()) + ", got: " + s(param->typei()) + ")"
            );
        }

        debug("fnparam: " + loc->toString() + " <- " + param->toString());
        _vm->store(loc, param);
        return nullptr;
    }

    Reference* ExecuteWalk::walkReturn0(Return0*) {
        verbose("return0");

        if ( _vm->getCall() == nullptr ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::ReturnOutsideCall,
                "Cannot return: no call in progress"
            );
        }

        _vm->returnToCaller(true);
        return nullptr;
    }

    ISA::Reference* ExecuteWalk::walkReturn1(ISA::Return1* i) {
        verbose("return1 " + i->first()->toString());
        auto call = _vm->getCall();
        GC_LOCAL_REF(call)
        if ( call == nullptr ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::ReturnOutsideCall,
                "Cannot return: no call in progress"
            );
        }

        // Resolve the return value
        auto ref = _vm->resolve(i->first());
        GC_LOCAL_REF(ref)

        // Validate the return type
        if ( !ref->typei()->isAssignableTo(call->returnTypei()) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::TypeError,
                "Value " + s(ref) + " is incompatible with the return type of " + s(call) + " (expected: " + s(call->returnTypei()) + ", got: " + s(ref->typei()) + ")"
            );
        }

        // Set the return value on the function call
        call->setReturn(ref);

        // Jump back to the caller
        _vm->returnToCaller(true);

        return nullptr;
    }

    Reference* ExecuteWalk::walkCurry(Curry* i) {
        verbose("curry " + i->first()->toString() + " " + i->second()->toString());
        auto fn = ensureFunction(_vm->resolvei(i->first()));
        auto param = _vm->resolve(i->second());
        return new FunctionReference(fn->fn()->curry(param));
    }

    Reference* ExecuteWalk::walkCall0(Call0* i) {
        verbose("call0 " + i->first()->toString());
        auto fn = ensureFunction(_vm->resolve(i->first()));
        auto call = fn->fn()->call();
        _vm->call(call);
        return nullptr;
    }

    Reference* ExecuteWalk::walkCall1(Call1* i) {
        verbose("call1 " + i->first()->toString() + " " + i->second()->toString());
        auto fn = ensureFunction(_vm->resolve(i->first()));
        auto param = _vm->resolve(i->second());
        auto call = fn->fn()->curryi(param)->call();
        _vm->call(call);
        return nullptr;
    }

    Reference* ExecuteWalk::walkCallIf0(CallIf0* i) {
        verbose("callif0 " + i->first()->toString() + " " + i->second()->toString());
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        if ( cond->value() ) _vm->call(fn->fn()->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkCallIf1(CallIf1* i) {
        verbose("callif1 " + i->first()->toString() + " " + i->second()->toString() + " " + i->third()->toString());
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        auto param = _vm->resolve(i->third());
        if ( cond->value() ) _vm->call(fn->fn()->curryi(param)->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkCallElse0(CallElse0* i) {
        verbose("callelse0 " + i->first()->toString() + " " + i->second()->toString());
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        if ( !cond->value() ) _vm->call(fn->fn()->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkCallElse1(CallElse1* i) {
        verbose("callelse1 " + i->first()->toString() + " " + i->second()->toString() + " " + i->third()->toString());
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        auto param = _vm->resolve(i->third());
        if ( !cond->value() ) _vm->call(fn->fn()->curryi(param)->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkPushCall0(PushCall0* i) {
        verbose("pushcall0 " + i->first()->toString());
        auto fn = ensureFunction(_vm->resolve(i->first()));
        auto call = fn->fn()->call();
        _vm->pushCall(call);
        return call->getReturn();
    }

    Reference* ExecuteWalk::walkPushCall1(PushCall1* i) {
        verbose("pushcall1 " + i->first()->toString() + " " + i->second()->toString());
        auto fn = ensureFunction(_vm->resolve(i->first()));
        auto param = _vm->resolve(i->second());
        auto call = fn->fn()->curryi(param)->call();
        _vm->pushCall(call);
        return call->getReturn();
    }

    Reference* ExecuteWalk::walkPushCallIf0(PushCallIf0* i) {
        verbose("pushcallif0 " + i->first()->toString() + " " + i->second()->toString());
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        if ( cond->value() ) _vm->pushCall(fn->fn()->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkPushCallIf1(PushCallIf1* i) {
        verbose("pushcallif1 " + i->first()->toString() + " " + i->second()->toString() + " " + i->third()->toString());
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        auto param = _vm->resolve(i->third());
        if ( cond->value() ) _vm->pushCall(fn->fn()->curryi(param)->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkPushCallElse0(PushCallElse0* i) {
        verbose("pushcallelse0 " + i->first()->toString() + " " + i->second()->toString());
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        if ( !cond->value() ) _vm->pushCall(fn->fn()->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkPushCallElse1(PushCallElse1* i) {
        verbose("pushcallelse1 " + i->first()->toString() + " " + i->second()->toString() + " " + i->third()->toString());
        auto cond = ensureBoolean(_vm->resolve(i->first()));
        auto fn = ensureFunction(_vm->resolve(i->second()));
        auto param = _vm->resolve(i->third());
        if ( !cond->value() ) _vm->pushCall(fn->fn()->curryi(param)->call());
        return nullptr;
    }

    Reference* ExecuteWalk::walkDrain(Drain*) {
        verbose("drain");
        _vm->drain();
        return nullptr;
    }

    Reference* ExecuteWalk::walkExit(Exit*) {
        verbose("exit");
        _vm->exit();
        return nullptr;
    }

    Reference* ExecuteWalk::walkMapInit(MapInit* i) {
        verbose("mapinit " + i->first()->toString());
        auto innerType = ensureType(_vm->resolve(i->first()));
        return new MapReference(innerType->value());
    }

    Reference* ExecuteWalk::walkMapSet(MapSet* i) {
        verbose("mapset " + i->first()->toString() + " " + i->second()->toString() + " " + i->third()->toString());
        auto key = ensureString(_vm->resolve(i->first()));
        auto map = ensureMap(_vm->resolve(i->third()));
        auto value = _vm->resolve(i->second());

        auto mapType = map->type();
        GC_LOCAL_REF(mapType)
        ensureType(value, mapType->valuesi());
        map->set(key->value(), value);
        return nullptr;
    }

    Reference* ExecuteWalk::walkMapGet(MapGet* i) {
        verbose("mapget " + i->first()->toString() + " " + i->second()->toString());
        auto key = ensureString(_vm->resolve(i->first()));
        auto map = ensureMap(_vm->resolve(i->second()));

        if ( !map->has(key->value()) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidMapKey,
                "Invalid key " + s(key) + " for map " + s(map)
            );
        }

        return map->get(key->value());
    }

    Reference* ExecuteWalk::walkMapLength(MapLength* i) {
        verbose("maplength " + i->first()->toString());
        auto map = ensureMap(_vm->resolve(i->first()));
        return new NumberReference(static_cast<double>(map->length()));
    }

    Reference* ExecuteWalk::walkMapKeys(MapKeys* i) {
        verbose("mapkeys " + i->first()->toString());
        auto map = ensureMap(_vm->resolve(i->first()));
        return map->keys();
    }

    Reference* ExecuteWalk::walkTypify(Typify* i) {
        verbose("typify " + i->first()->toString() + " " + i->second()->toString());
        auto loc = i->first();
        auto type = ensureType(i->second());
        _vm->typify(loc, type->value());
        return nullptr;
    }

    Reference* ExecuteWalk::walkAssignValue(AssignValue* i) {
        verbose("assignvalue " + i->first()->toString() + " " + i->second()->toString());
        auto loc = i->first();
        auto value = _vm->resolve(i->second());

        if ( loc->typei()->isAmbiguous() ) {
            loc->setType(value->type());
        }

        if ( !value->typei()->isAssignableTo(loc->typei()) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::TypeError,
                "Value " + s(value) + " has type which is incompatible with location " + s(loc) + " (expected: " + s(loc->typei()) + ", got: " + s(value->typei()) + ")"
            );
        }

        debug(loc->toString() + " <- " + value->toString());
        _vm->store(loc, value);
        return nullptr;
    }

    Reference* ExecuteWalk::walkAssignEval(AssignEval* i) {
        verbose("assigneval " + i->first()->toString() + " " + i->second()->toString());
        auto loc = i->first();
        auto eval = i->second();

        Reference* value = nullptr;

        // if the right-hand side is a call which can yield a value,
        // then we will need to make the call and wait for the return
        // to jump back here
        if ( eval->tag() == Tag::CALL0 || eval->tag() == Tag::CALL1 ) {
            verbose("assignEval: got call0 or call1");

            // Check if we got here because of the return
            auto returnCall = _vm->getReturn();
            if ( returnCall != nullptr ) {
                verbose("assignEval: jumped from return");

                // Get the return value and store that
                value = returnCall->getReturn();
            } else {
                verbose("assignEval: jumping to call");

                // Otherwise, we need to make the call. Step back so we
                // return-jump to the correct instruction.
                _vm->setCaptureReturn();
                _vm->rewind();
                return walkOnePropagatingExceptions(eval);
            }
        } else {
            value = walkOnePropagatingExceptions(eval);
        }

        if ( value == nullptr ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidAssignEval,
                "Attempted to assign result of instruction " + s(eval) + " to location " + s(loc) + ", but the instruction does not yield a value"
            );
        }

        GC_LOCAL_REF(value)

        if ( loc->typei()->isAmbiguous() ) {
            loc->setType(value->type());
        }

        if ( !value->typei()->isAssignableTo(loc->typei()) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::TypeError,
                "Value " + s(value) + " has type which is incompatible with location " + s(loc) + " (expected: " + s(loc->typei()) + ", got: " + s(value->typei()) + ")"
            );
        }

        debug(loc->toString() + " <- " + value->toString());
        _vm->store(loc, value);
        return nullptr;
    }

    Reference* ExecuteWalk::walkLock(Lock* i) {
        verbose("lock " + i->first()->toString());
        _vm->lock(i->first());
        return nullptr;
    }

    Reference* ExecuteWalk::walkUnlock(Unlock* i) {
        verbose("unlock " + i->first()->toString());
        _vm->unlock(i->first());
        return nullptr;
    }

    Reference* ExecuteWalk::walkIsEqual(IsEqual* i) {
        verbose("equal " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = _vm->resolve(i->first());
        auto rhs = _vm->resolve(i->second());
        return new BooleanReference(lhs->isEqualTo(rhs));
    }

    Reference* ExecuteWalk::walkScopeOf(ScopeOf* i) {
        verbose("scopeof " + i->first()->toString());
        _vm->shadow(i->first());
        return nullptr;
    }

    Reference* ExecuteWalk::walkStreamInit(StreamInit* i) {
        verbose("streaminit " + i->first()->toString());
        auto type = ensureType(_vm->resolve(i->first()));
        auto stream = _vm->getStream(nslib::uuid(), type->value());
        return new StreamReference(stream);
    }

    Reference* ExecuteWalk::walkStreamPush(StreamPush* i) {
        verbose("streampush " + i->first()->toString() + " " + i->second()->toString());
        auto stream = ensureStream(_vm->resolve(i->first()));
        auto value = _vm->resolve(i->second());

        if ( !stream->stream()->isOpen() ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::StreamNotOpen,
                "Attempted to push to stream " + s(stream->stream()) + " which is not yet open"
            );
        }

        auto streamType = stream->type();
        GC_LOCAL_REF(streamType)
        if ( !value->typei()->isAssignableTo(streamType->inneri()) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::TypeError,
                "Value " + s(value) + " has incompatible type for stream " + s(stream->stream()) + " (expected: " + s(streamType->inneri()) + ", got: " + s(value->typei()) + ")"
            );
        }

        stream->stream()->push(value);
        return nullptr;
    }

    Reference* ExecuteWalk::walkStreamPop(StreamPop* i) {
        verbose("streampop " + i->first()->toString());
        auto stream = ensureStream(_vm->resolve(i->first()));

        if ( !stream->stream()->isOpen() ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::StreamNotOpen,
                "Attempted to pop from stream " + s(stream->stream()) + " which is not yet open"
            );
        }

        if ( stream->stream()->isEmpty() ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::StreamEmpty,
                "Attempted to pop from empty stream " + s(stream->stream())
            );
        }

        return stream->stream()->pop();
    }

    Reference* ExecuteWalk::walkStreamClose(StreamClose* i) {
        verbose("streamclose " + i->first()->toString());
        auto stream = ensureStream(_vm->resolve(i->first()));

        if ( !stream->stream()->isOpen() ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::StreamNotOpen,
                "Attempted to close stream " + s(stream->stream()) + " which is not yet open"
            );
        }

        stream->stream()->close();
        return nullptr;
    }

    Reference* ExecuteWalk::walkStreamEmpty(StreamEmpty* i) {
        verbose("streamempty " + i->first()->toString());
        auto stream = ensureStream(_vm->resolve(i->first()));

        if ( !stream->stream()->isOpen() ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::StreamNotOpen,
                "Attempted to check stream " + s(stream->stream()) + " which is not yet open"
            );
        }

        return new BooleanReference(stream->stream()->isEmpty());
    }

    Reference* ExecuteWalk::walkOut(Out* i) {
        return walkStreamPush(i);
    }

    Reference* ExecuteWalk::walkErr(Err* i) {
        return walkStreamPush(i);
    }

    Reference* ExecuteWalk::walkStringConcat(StringConcat* i) {
        verbose("strconcat " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = ensureString(_vm->resolve(i->first()));
        auto rhs = ensureString(_vm->resolve(i->second()));
        return new StringReference(lhs->value() + rhs->value());
    }

    Reference* ExecuteWalk::walkStringLength(StringLength* i) {
        verbose("strlength " + i->first()->toString());
        auto opd = ensureString(_vm->resolve(i->first()));
        return new NumberReference(static_cast<double>(opd->value().length()));
    }

    Reference* ExecuteWalk::walkStringSliceFrom(StringSliceFrom* i) {
        // FIXME: handle negative indices

        verbose("strslicefrom " + i->first()->toString() + " " + i->second()->toString());
        auto str = ensureString(_vm->resolve(i->first()));
        auto from = ensureNumber(_vm->resolve(i->second()));
        return new StringReference(str->value().substr(static_cast<std::size_t>(from->value())));
    }

    Reference* ExecuteWalk::walkStringSliceFromTo(StringSliceFromTo* i) {
        // FIXME: handle negative indices

        verbose("strslicefromto " + i->first()->toString() + " " + i->second()->toString() + " " + i->third()->toString());
        auto str = ensureString(_vm->resolve(i->first()));
        auto from = ensureNumber(_vm->resolve(i->second()));
        auto to = ensureNumber(_vm->resolve(i->third()));
        return new StringReference(str->value().substr(static_cast<std::size_t>(from->value()), static_cast<std::size_t>(to->value())));
    }

    Reference* ExecuteWalk::walkTypeOf(TypeOf* i) {
        // FIXME: handle ambiguous type narrowing?

        verbose("typeof " + i->first()->toString());
        auto opd = _vm->resolvei(i->first());
        return new TypeReference(opd->type());
    }

    Reference* ExecuteWalk::walkIsCompatible(IsCompatible* i) {
        // FIXME: handle ambiguous type narrowing?

        verbose("compatible " + i->first()->toString() + " " + i->second()->toString());
        auto lhs = _vm->resolve(i->first());
        auto rhs = _vm->resolve(i->second());
        return new BooleanReference(rhs->typei()->isAssignableTo(lhs->typei()));
    }

    Reference* ExecuteWalk::walkPushExceptionHandler1(PushExceptionHandler1* i) {
        verbose("pushexhandler " + i->first()->toString());
        auto handler = ensureFunction(_vm->resolve(i->first()));

        if ( !handler->typei()->isAssignableTo(_typeOfExceptionHandler) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidExceptionHandlerType,
                "Exception handler callback has invalid type (expected: " + s(_typeOfExceptionHandler) + ", got: " + s(handler->typei()) + ")"
            );
        }

        auto id = _vm->pushExceptionHandler(handler->fn());
        return new StringReference(id);
    }

    Reference* ExecuteWalk::walkPushExceptionHandler2(PushExceptionHandler2* i) {
        verbose("pushexhandler " + i->first()->toString() + " " + i->second()->toString());
        auto handler = ensureFunction(_vm->resolve(i->first()));
        auto discriminator = _vm->resolve(i->second());

        if ( !handler->typei()->isAssignableTo(_typeOfExceptionHandler) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::InvalidExceptionHandlerType,
                "Exception handler callback has invalid type (expected: " + s(_typeOfExceptionHandler) + ", got: " + s(handler->typei()) + ")"
            );
        }

        ExceptionHandlerId id;
        if ( discriminator->tag() == ReferenceTag::NUMBER ) {
            id = _vm->pushExceptionHandler(static_cast<std::size_t>(ensureNumber(discriminator)->value()), handler->fn());
        } else {
            auto discriminatorFn = ensureFunction(discriminator);

            if ( !discriminatorFn->typei()->isAssignableTo(_typeOfExceptionDiscriminator) ) {
                throw Errors::RuntimeError(
                    Errors::RuntimeExCode::InvalidExceptionHandlerType,
                    "Exception handler discriminator has invalid type (expected: " + s(_typeOfExceptionDiscriminator) + ", got: " + s(discriminatorFn->typei()) + ")"
                );
            }

            id = _vm->pushExceptionHandler(discriminatorFn->fn(), handler->fn());
        }

        return new StringReference(id);
    }

    Reference* ExecuteWalk::walkPopExceptionHandler(PopExceptionHandler* i) {
        verbose("popexhandler " + i->first()->toString());
        auto id = ensureString(_vm->resolve(i->first()));
        _vm->popExceptionHandler(id->value());
        return nullptr;
    }

    Reference* ExecuteWalk::walkRaise(Raise* i) {
        verbose("raise " + i->first()->toString());
        auto id = ensureNumber(_vm->resolve(i->first()));
        _vm->raise(static_cast<std::size_t>(id->value()));
        return nullptr;
    }

    Reference* ExecuteWalk::walkResume(Resume* i) {
        verbose("resume " + i->first()->toString());
        auto fn = ensureFunction(_vm->resolve(i->first()));

        // Get the scope where the exception handler was registered
        auto scope = _vm->getExceptionFrame();
        GC_LOCAL_REF(scope)
        if ( scope == nullptr ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::ResumeOutsideExHandler,
                "Attempted to call `resume` from outside context of exception handler"
            );
        }

        // The "resumed" code inherits the scope where the exception handler was
        // registered. This means that it is responsible for satisfying the return type
        // of that scope's IFunctionCall, if one exists.
        auto inheritedCall = scope->call();
        GC_LOCAL_REF(inheritedCall)

        auto returnType = inheritedCall == nullptr ? Type::Primitive::of(Type::Intrinsic::VOID) : inheritedCall->returnType();
        GC_LOCAL_REF(returnType)

        auto fnType = new Type::Lambda0(returnType);
        GC_LOCAL_REF(fnType)

        if ( !fn->typei()->isAssignableTo(fnType) ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::TypeError,
                "Resumed function " + s(fn) + " has type which is incompatible with the parent scope it subsumes " + s(inheritedCall) + " (expected: " + s(fnType) + ", got: " + s(fn->typei()) + ")"
            );
        }

        // Rewind to the scope we are resuming to
        _vm->restore(scope->clearExceptionFrame());

        // Call the resumed function w/in the scope fo the exception handler
        _vm->callWithInheritedScope(fn->fn()->call());

        return nullptr;
    }

    Reference* ExecuteWalk::walkOTypeInit(OTypeInit*) {
        return new ObjectTypeReference(new Type::Object);
    }

    Reference* ExecuteWalk::walkOTypeProp(OTypeProp* i) {
        auto otype = ensureObjectType(_vm->resolve(i->first()));
        auto oprop = i->second();
        auto propType = ensureType(_vm->resolve(i->third()));

        otype->otypei()->defineProperty(oprop->name(), propType->value());
        return nullptr;
    }

    Reference* ExecuteWalk::walkOTypeDel(OTypeDel* i) {
        auto otype = ensureObjectType(_vm->resolve(i->first()));
        auto oprop = i->second();

        otype->otypei()->deleteProperty(oprop->name());
        return nullptr;
    }

    Reference* ExecuteWalk::walkOTypeGet(OTypeGet* i) {
        auto otype = ensureObjectType(_vm->resolve(i->first()));
        auto oprop = i->second();

        auto propType = otype->otypei()->getProperty(oprop->name());
        return new TypeReference(propType);
    }

    Reference* ExecuteWalk::walkOTypeFinalize(OTypeFinalize* i) {
        auto otype = ensureObjectType(_vm->resolve(i->first()));
        auto finalized = otype->otypei()->finalize();
        return new ObjectTypeReference(finalized);
    }

    Reference* ExecuteWalk::walkOTypeSubset(OTypeSubset* i) {
        auto otype = ensureObjectType(_vm->resolve(i->first()));
        if ( !otype->isFinal() ) {
            throw Errors::RuntimeError(
                Errors::RuntimeExCode::SubsetNonFinalObjectType,
                "Cannot create subset from incomplete object type " + s(otype->value())
            );
        }

        return new ObjectTypeReference(new Type::Object(otype->value()));
    }

}
