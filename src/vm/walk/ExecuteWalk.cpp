#include <cassert>
#include <cmath>
#include "../../shared/uuid.h"
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
        verbose("ensureNumber: " + ref->toString());
        ensureType(ref, Type::Primitive::of(Type::Intrinsic::NUMBER));
        // FIXME: eventually, this should probably generate a runtime exception
        assert(ref->tag() == ReferenceTag::NUMBER);
        return (NumberReference*) ref;
    }

    BooleanReference* ExecuteWalk::ensureBoolean(const Reference* ref) {
        verbose("ensureBoolean: " + ref->toString());
        ensureType(ref, Type::Primitive::of(Type::Intrinsic::BOOLEAN));
        // FIXME: eventually, this should probably generate a runtime exception
        assert(ref->tag() == ReferenceTag::BOOLEAN);
        return (BooleanReference*) ref;
    }

    TypeReference* ExecuteWalk::ensureType(const Reference* ref) {
        verbose("ensureType: " + ref->toString());
        ensureType(ref, Type::Primitive::of(Type::Intrinsic::TYPE));
        // FIXME: eventually, this should probably generate a runtime exception
        assert(ref->tag() == ReferenceTag::TYPE);
        return (TypeReference*) ref;
    }

    StringReference* ExecuteWalk::ensureString(const Reference* ref) {
        verbose("ensureString: " + ref->toString());
        ensureType(ref, Type::Primitive::of(Type::Intrinsic::STRING));
        // FIXME: eventually, this should probably generate a runtime exception
        assert(ref->tag() == ReferenceTag::STRING);
        return (StringReference*) ref;
    }

    FunctionReference* ExecuteWalk::ensureFunction(const Reference* ref) {
        verbose("ensureFunction: " + ref->toString());
        // FIXME: eventually, this should generate a runtime exception
        assert(ref->tag() == ReferenceTag::FUNCTION);
        return (FunctionReference*) ref;
    }

    EnumerationReference* ExecuteWalk::ensureEnumeration(const Reference* ref) {
        verbose("ensureEnumeration: " + ref->toString());
        // FIXME: eventually, this should generate a runtime exception
        assert(ref->tag() == ReferenceTag::ENUMERATION);
        return (EnumerationReference*) ref;
    }

    StreamReference* ExecuteWalk::ensureStream(const Reference* ref) {
        verbose("ensureStream: " + ref->toString());
        // FIXME: eventually, this should generate a runtime exception
        assert(ref->tag() == ReferenceTag::STREAM);
        return (StreamReference*) ref;
    }

    MapReference* ExecuteWalk::ensureMap(const Reference* ref) {
        verbose("ensureMap: " + ref->toString());
        // FIXME: eventually, this should generate a runtime exception
        assert(ref->tag() == ReferenceTag::MAP);
        return (MapReference*) ref;
    }

    ResourceReference* ExecuteWalk::ensureResource(const Reference* ref) {
        verbose("ensureResource: " + ref->toString());
        // FIXME: eventually, this should generate a runtime exception
        assert(ref->tag() == ReferenceTag::RESOURCE);
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
        // FIXME: eventually, this should generate a runtime exception
        assert(rhs->value() != 0.0);
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

        // load callback function & validate the type
        auto callback = ensureFunction(_vm->resolve(i->second()));

        // fixme: eventually, this should raise a runtime exception
        assert(callback->type()->isAssignableTo(callbackType));

        auto cond = ensureBoolean(_vm->resolve(i->first()));
        if ( cond->value() ) {
            // we want the VM to re-evaluate the condition after the call completes,
            // so rewind the program counter by one so the return jump is correct
            _vm->rewind();

            auto call = callback->fn()->call();
            _vm->call(call);
        }

        return nullptr;
    }

    Reference* ExecuteWalk::walkWith(With* i) {
        verbose("with " + i->first()->toString() + " " + i->second()->toString());
        auto resource = ensureResource(_vm->resolve(i->first()));

        auto callbackType = new Type::Lambda1(resource->type()->yields(), Type::Primitive::of(Type::Intrinsic::VOID));

        auto callback = ensureFunction(_vm->resolve(i->second()));

        // fixme: eventually, this should raise a runtime exception
        assert(callback->type()->isAssignableTo(callbackType));

        // open the resource
        resource->resource()->open();

        // perform the call
        auto call = callback->fn()->curry(resource->resource()->innerValue())->call();
        _vm->pushCall(call);
        _vm->drain();

        // close the resource
        resource->resource()->close();

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

        // fixme: eventually this should generate a runtime error
        assert(value->type()->isAssignableTo(enumeration->type()->values()));
        enumeration->append(value);

        return nullptr;
    }

    Reference* ExecuteWalk::walkEnumPrepend(EnumPrepend* i) {
        verbose("enumprepend " + i->first()->toString() + " " + i->second()->toString());
        auto enumeration = ensureEnumeration(_vm->resolve(i->second()));
        auto value = _vm->resolve(i->first());

        // fixme: eventually this should generate a runtime error
        assert(value->type()->isAssignableTo(enumeration->type()->values()));
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

        // fixme: eventually, this should generate a runtime error
        assert(idx->value() < enumeration->length());

        return enumeration->get(static_cast<size_t>(idx->value()));
    }

    Reference* ExecuteWalk::walkEnumSet(EnumSet* i) {
        verbose("enumset " + i->first()->toString() + " " + i->second()->toString() + " " + i->third()->toString());
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
        verbose("enumerate " + i->first()->toString() + " " + i->second()->toString() + " " + i->third()->toString());
        auto elemType = ensureType(_vm->resolve(i->first()));
        auto enumeration = ensureEnumeration(_vm->resolve(i->second()));
        auto callback = ensureFunction(_vm->resolve(i->third()));

        Type::Primitive tVoid(Type::Intrinsic::VOID);
        Type::Primitive tNum(Type::Intrinsic::NUMBER);
        Type::Lambda1 callbackInner(&tNum, &tVoid);
        Type::Lambda1 callbackOuter(elemType->value(), &callbackInner);

        // fixme: eventually, this should generate a runtime error
        verbose("callback: " + callback->toString());
        verbose("callback type: " + callback->type()->toString() + " | callback outer: " + callbackOuter.toString());
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
        _vm->skip(i);
        _vm->rewind();  // skip takes us to the instruction we want to jump to, but we will advance after this
        return nullptr;
    }

    Reference* ExecuteWalk::walkFunctionParam(FunctionParam* i) {
        verbose("fnparam " + i->first()->toString() + " " + i->second()->toString());

        auto call = _vm->getCall();
        // fixme: eventually, this should generate a runtime exception
        assert(call != nullptr);

        auto param = _vm->resolve(call->popParam().second);
        auto loc = i->second();

        // fixme: eventually, this should generate a runtime exception
        assert(param->type()->isAssignableTo(loc->type()));

        debug("fnparam: " + loc->toString() + " <- " + param->toString());
        _vm->store(loc, param);
        return nullptr;
    }

    Reference* ExecuteWalk::walkReturn0(Return0*) {
        verbose("return0");
        _vm->returnToCaller();
        return nullptr;
    }

    ISA::Reference* ExecuteWalk::walkReturn1(ISA::Return1* i) {
        verbose("return1 " + i->first()->toString());
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
        verbose("curry " + i->first()->toString() + " " + i->second()->toString());
        auto fn = ensureFunction(_vm->resolve(i->first()));
        auto param = _vm->resolve(i->second());
        return new FunctionReference(fn->fn()->curry(param));
    }

    Reference* ExecuteWalk::walkCall0(Call0* i) {
        verbose("call0 " + i->first()->toString());
        auto fn = ensureFunction(_vm->resolve(i->first()));
        auto call = fn->fn()->call();
        _vm->call(call);
        return call->getReturn();  // fixme: this doesn't actually return properly, since the "call" is an async jump
    }

    Reference* ExecuteWalk::walkCall1(Call1* i) {
        verbose("call1 " + i->first()->toString() + " " + i->second()->toString());
        auto fn = ensureFunction(_vm->resolve(i->first()));
        auto param = _vm->resolve(i->second());
        auto call = fn->fn()->curry(param)->call();
        _vm->call(call);
        return call->getReturn();
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
        if ( cond->value() ) _vm->call(fn->fn()->curry(param)->call());
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
        if ( !cond->value() ) _vm->call(fn->fn()->curry(param)->call());
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
        auto call = fn->fn()->curry(param)->call();
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
        if ( cond->value() ) _vm->pushCall(fn->fn()->curry(param)->call());
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
        if ( !cond->value() ) _vm->pushCall(fn->fn()->curry(param)->call());
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

        ensureType(value, map->type()->values());
        map->set(key->value(), value);
        return nullptr;
    }

    Reference* ExecuteWalk::walkMapGet(MapGet* i) {
        verbose("mapget " + i->first()->toString() + " " + i->second()->toString());
        auto key = ensureString(_vm->resolve(i->first()));
        auto map = ensureMap(_vm->resolve(i->second()));

        // FIXME: eventually, this should generate a runtime exception
        assert(map->has(key->value()));

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

        if ( loc->type()->isAmbiguous() ) {
            loc->setType(value->type());
        }

        // FIXME: eventually, this needs to generate a runtime type error
        assert(value->type()->isAssignableTo(loc->type()));

        debug(loc->toString() + " <- " + value->toString());
        _vm->store(loc, value);
        return nullptr;
    }

    Reference* ExecuteWalk::walkAssignEval(AssignEval* i) {
        verbose("assigneval " + i->first()->toString() + " " + i->second()->toString());
        auto loc = i->first();
        auto eval = i->second();

        Reference* value = nullptr;

        // if the right-hand side is an call which can yield a value,
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
        auto stream = _vm->getStream(util::uuid4(), type->value());
        return new StreamReference(stream);
    }

    Reference* ExecuteWalk::walkStreamPush(StreamPush* i) {
        verbose("streampush " + i->first()->toString() + " " + i->second()->toString());
        auto stream = ensureStream(_vm->resolve(i->first()));
        auto value = _vm->resolve(i->second());

        // FIXME: eventually, this should generate a runtime exception
        assert(stream->stream()->isOpen() && value->type()->isAssignableTo(stream->type()->inner()));

        stream->stream()->push(value);
        return nullptr;
    }

    Reference* ExecuteWalk::walkStreamPop(StreamPop* i) {
        verbose("streampop " + i->first()->toString());
        auto stream = ensureStream(_vm->resolve(i->first()));

        // FIXME: eventually, this should generate a runtime exception
        assert(stream->stream()->isOpen() && !stream->stream()->isEmpty());

        return stream->stream()->pop();
    }

    Reference* ExecuteWalk::walkStreamClose(StreamClose* i) {
        verbose("streamclose " + i->first()->toString());
        auto stream = ensureStream(_vm->resolve(i->first()));

        // FIXME: eventually, this should generate a runtime exception
        assert(stream->stream()->isOpen());
        stream->stream()->close();

        return nullptr;
    }

    Reference* ExecuteWalk::walkStreamEmpty(StreamEmpty* i) {
        verbose("streamempty " + i->first()->toString());
        auto stream = ensureStream(_vm->resolve(i->first()));

        // FIXME: eventually, this should generate a runtime exception
        assert(stream->stream()->isOpen());

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
        return new NumberReference(opd->value().length());
    }

    Reference* ExecuteWalk::walkStringSliceFrom(StringSliceFrom* i) {
        // FIXME: handle negative indices

        verbose("strslicefrom " + i->first()->toString() + " " + i->second()->toString());
        auto str = ensureString(_vm->resolve(i->first()));
        auto from = ensureNumber(_vm->resolve(i->second()));
        return new StringReference(str->value().substr(from->value()));
    }

    Reference* ExecuteWalk::walkStringSliceFromTo(StringSliceFromTo* i) {
        // FIXME: handle negative indices

        verbose("strslicefromto " + i->first()->toString() + " " + i->second()->toString() + " " + i->third()->toString());
        auto str = ensureString(_vm->resolve(i->first()));
        auto from = ensureNumber(_vm->resolve(i->second()));
        auto to = ensureNumber(_vm->resolve(i->third()));
        return new StringReference(str->value().substr(from->value(), to->value()));
    }

    Reference* ExecuteWalk::walkTypeOf(TypeOf* i) {
        // FIXME: handle ambiguous type narrowing?

        verbose("typeof " + i->first()->toString());
        auto opd = _vm->resolve(i->first());
        return new TypeReference(opd->type());
    }

    Reference* ExecuteWalk::walkIsCompatible(IsCompatible* i) {
        // FIXME: handle ambiguous type narrowing?

        verbose("compatible " + i->first()->toString() + " " + i->second()->toString());
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
