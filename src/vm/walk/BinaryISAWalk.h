#ifndef SWARMVM_BINARYISAWALK
#define SWARMVM_BINARYISAWALK

#include <cassert>
#include <iostream>
#include "../../../mod/binn/src/binn.h"
#include "../../shared/nslib.h"
#include "../isa_meta.h"
#include "binary_const.h"
#include "../Wire.h"

using namespace nslib;

namespace swarmc::Runtime {
    class VirtualMachine;
}

namespace swarmc::ISA {

    class BinaryISAWalk : public IStringable {
    public:
        explicit BinaryISAWalk(Runtime::VirtualMachine* vm = nullptr): _vm(vm) {}

        [[nodiscard]] std::string toString() const override {
            return "BinaryISAWalk<>";
        }

        static binn* readInput(std::istream& input) {
            input.seekg(0, std::istream::end);
            auto length = input.tellg();
            input.seekg(0, std::istream::beg);

            void* buf = malloc(sizeof(char) * length);
            input.read(static_cast<char*>(buf), length);

            return binn_open(buf);
        }

        static Instructions fromInput(std::istream& input) {
            ISA::BinaryISAWalk walk;
            auto obj = readInput(input);
            auto list = binn_map_list(obj, BC_BODY);
            auto is = walk.walk((binn*) list);

            binn_free(obj);
            return is;
        }

        Instructions walk(binn* list) {
            Instructions is;
            binn_iter iter;
            binn value;

            binn_list_foreach(list, value) {
                is.push_back(walkOne(&value));
            }

            return is;
        }

        Instruction* walkOne(binn* obj) {
            Tag tag = (Tag) binn_map_uint64(obj, BC_TAG);
            if ( tag == Tag::POSITION ) return walkPosition(obj);
            if ( tag == Tag::BEGINFN ) return walkBeginFunction(obj);
            if ( tag == Tag::FNPARAM ) return walkFunctionParam(obj);
            if ( tag == Tag::RETURN0 ) return walkReturn0(obj);
            if ( tag == Tag::RETURN1 ) return walkReturn1(obj);
            if ( tag == Tag::CURRY ) return walkCurry(obj);
            if ( tag == Tag::CALL0 ) return walkCall0(obj);
            if ( tag == Tag::CALL1 ) return walkCall1(obj);
            if ( tag == Tag::CALLIF0 ) return walkCallIf0(obj);
            if ( tag == Tag::CALLIF1 ) return walkCallIf1(obj);
            if ( tag == Tag::CALLELSE0 ) return walkCallElse0(obj);
            if ( tag == Tag::CALLELSE1 ) return walkCallElse1(obj);
            if ( tag == Tag::PUSHCALL0 ) return walkPushCall0(obj);
            if ( tag == Tag::PUSHCALL1 ) return walkPushCall1(obj);
            if ( tag == Tag::PUSHCALLIF0 ) return walkPushCallIf0(obj);
            if ( tag == Tag::PUSHCALLIF1 ) return walkPushCallIf1(obj);
            if ( tag == Tag::PUSHCALLELSE0 ) return walkPushCallElse0(obj);
            if ( tag == Tag::PUSHCALLELSE1 ) return walkPushCallElse1(obj);
            if ( tag == Tag::DRAIN ) return walkDrain(obj);
            if ( tag == Tag::EXIT ) return walkExit(obj);
            if ( tag == Tag::OUT ) return walkOut(obj);
            if ( tag == Tag::ERR ) return walkErr(obj);
            if ( tag == Tag::STREAMINIT ) return walkStreamInit(obj);
            if ( tag == Tag::STREAMPUSH ) return walkStreamPush(obj);
            if ( tag == Tag::STREAMPOP ) return walkStreamPop(obj);
            if ( tag == Tag::STREAMCLOSE ) return walkStreamClose(obj);
            if ( tag == Tag::STREAMEMPTY ) return walkStreamEmpty(obj);
            if ( tag == Tag::TYPIFY ) return walkTypify(obj);
            if ( tag == Tag::ASSIGNVALUE ) return walkAssignValue(obj);
            if ( tag == Tag::ASSIGNEVAL ) return walkAssignEval(obj);
            if ( tag == Tag::LOCK ) return walkLock(obj);
            if ( tag == Tag::UNLOCK ) return walkUnlock(obj);
            if ( tag == Tag::EQUAL ) return walkIsEqual(obj);
            if ( tag == Tag::SCOPEOF ) return walkScopeOf(obj);
            if ( tag == Tag::TYPEOF ) return walkTypeOf(obj);
            if ( tag == Tag::COMPATIBLE ) return walkIsCompatible(obj);
            if ( tag == Tag::AND ) return walkAnd(obj);
            if ( tag == Tag::OR ) return walkOr(obj);
            if ( tag == Tag::XOR ) return walkXor(obj);
            if ( tag == Tag::NAND ) return walkNand(obj);
            if ( tag == Tag::NOR ) return walkNor(obj);
            if ( tag == Tag::NOT ) return walkNot(obj);
            if ( tag == Tag::MAPINIT ) return walkMapInit(obj);
            if ( tag == Tag::MAPSET ) return walkMapSet(obj);
            if ( tag == Tag::MAPGET ) return walkMapGet(obj);
            if ( tag == Tag::MAPLENGTH ) return walkMapLength(obj);
            if ( tag == Tag::MAPKEYS ) return walkMapKeys(obj);
            if ( tag == Tag::ENUMINIT ) return walkEnumInit(obj);
            if ( tag == Tag::ENUMAPPEND ) return walkEnumAppend(obj);
            if ( tag == Tag::ENUMPREPEND ) return walkEnumPrepend(obj);
            if ( tag == Tag::ENUMLENGTH ) return walkEnumLength(obj);
            if ( tag == Tag::ENUMGET ) return walkEnumGet(obj);
            if ( tag == Tag::ENUMSET ) return walkEnumSet(obj);
            if ( tag == Tag::ENUMERATE ) return walkEnumerate(obj);
            if ( tag == Tag::STRCONCAT ) return walkStringConcat(obj);
            if ( tag == Tag::STRLENGTH ) return walkStringLength(obj);
            if ( tag == Tag::STRSLICEFROM ) return walkStringSliceFrom(obj);
            if ( tag == Tag::STRSLICEFROMTO ) return walkStringSliceFromTo(obj);
            if ( tag == Tag::PLUS ) return walkPlus(obj);
            if ( tag == Tag::MINUS ) return walkMinus(obj);
            if ( tag == Tag::TIMES ) return walkTimes(obj);
            if ( tag == Tag::DIVIDE ) return walkDivide(obj);
            if ( tag == Tag::POWER ) return walkPower(obj);
            if ( tag == Tag::MOD ) return walkMod(obj);
            if ( tag == Tag::NEG ) return walkNegative(obj);
            if ( tag == Tag::GT ) return walkGreaterThan(obj);
            if ( tag == Tag::GTE ) return walkGreaterThanOrEqual(obj);
            if ( tag == Tag::LT ) return walkLessThan(obj);
            if ( tag == Tag::LTE ) return walkLessThanOrEqual(obj);
            if ( tag == Tag::WHILE ) return walkWhile(obj);
            if ( tag == Tag::WITH ) return walkWith(obj);
            if ( tag == Tag::PUSHEXHANDLER1 ) return walkPushExceptionHandler1(obj);
            if ( tag == Tag::PUSHEXHANDLER2 ) return walkPushExceptionHandler2(obj);
            if ( tag == Tag::POPEXHANDLER ) return walkPopExceptionHandler(obj);
            if ( tag == Tag::RAISE ) return walkRaise(obj);
            if ( tag == Tag::RESUME ) return walkResume(obj);
            if ( tag == Tag::OTYPEINIT ) return walkOTypeInit(obj);
            if ( tag == Tag::OTYPEPROP ) return walkOTypeProp(obj);
            if ( tag == Tag::OTYPEDEL ) return walkOTypeDel(obj);
            if ( tag == Tag::OTYPEGET ) return walkOTypeGet(obj);
            if ( tag == Tag::OTYPEFINALIZE ) return walkOTypeFinalize(obj);
            if ( tag == Tag::OTYPESUBSET ) return walkOTypeSubset(obj);

            throw Errors::SwarmError("Unable to deserialize instruction with invalid or unknown tag.");
        }

    protected:
        VirtualMachine* _vm;

        NumberReference* walkNumberReference(binn* obj) {
            auto ref = Wire::references()->produce(obj, _vm);
            assert(ref->tag() == ReferenceTag::NUMBER);
            return (NumberReference*) ref;
        }

        StringReference* walkStringReference(binn* obj) {
            auto ref = Wire::references()->produce(obj, _vm);
            assert(ref->tag() == ReferenceTag::STRING);
            return (StringReference*) ref;
        }

        LocationReference* walkLocationReference(binn* obj) {
            auto ref = Wire::references()->produce(obj, _vm);
            assert(ref->tag() == ReferenceTag::LOCATION);
            return (LocationReference*) ref;
        }

        PositionAnnotation* walkPosition(binn* obj) {
            return new PositionAnnotation(
                walkStringReference((binn*) binn_map_map(obj, BC_FIRST)),
                walkNumberReference((binn*) binn_map_map(obj, BC_SECOND)),
                walkNumberReference((binn*) binn_map_map(obj, BC_THIRD))
            );
        }

        Plus* walkPlus(binn* obj) {
            return new Plus(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        Minus* walkMinus(binn* obj) {
            return new Minus(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        Times* walkTimes(binn* obj) {
            return new Times(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        Divide* walkDivide(binn* obj) {
            return new Divide(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        Power* walkPower(binn* obj) {
            return new Power(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        Mod* walkMod(binn* obj) {
            return new Mod(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        Negative* walkNegative(binn* obj) {
            return new Negative(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        GreaterThan* walkGreaterThan(binn* obj) {
            return new GreaterThan(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        GreaterThanOrEqual* walkGreaterThanOrEqual(binn* obj) {
            return new GreaterThanOrEqual(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        LessThan* walkLessThan(binn* obj) {
            return new LessThan(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        LessThanOrEqual* walkLessThanOrEqual(binn* obj) {
            return new LessThanOrEqual(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        And* walkAnd(binn* obj) {
            return new And(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        Or* walkOr(binn* obj) {
            return new Or(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        Xor* walkXor(binn* obj) {
            return new Xor(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        Nand* walkNand(binn* obj) {
            return new Nand(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        Nor* walkNor(binn* obj) {
            return new Nor(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        Not* walkNot(binn* obj) {
            return new Not(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        While* walkWhile(binn* obj) {
            return new While(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        With* walkWith(binn* obj) {
            return new With(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        EnumInit* walkEnumInit(binn* obj) {
            return new EnumInit(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        EnumAppend* walkEnumAppend(binn* obj) {
            return new EnumAppend(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        EnumPrepend* walkEnumPrepend(binn* obj) {
            return new EnumPrepend(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        EnumLength* walkEnumLength(binn* obj) {
            return new EnumLength(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        EnumGet* walkEnumGet(binn* obj) {
            return new EnumGet(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST)),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        EnumSet* walkEnumSet(binn* obj) {
            return new EnumSet(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST)),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_THIRD), _vm)
            );
        }

        Enumerate* walkEnumerate(binn* obj) {
            return new Enumerate(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND)),
                walkLocationReference((binn*) binn_map_map(obj, BC_THIRD))
            );
        }

        BeginFunction* walkBeginFunction(binn* obj) {
            auto bf = new BeginFunction(
                binn_map_str(obj, BC_NAME),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );

            if ( binn_map_bool(obj, BC_ISPURE) ) {
                bf->markAsPure();
            }

            return bf;
        }

        FunctionParam* walkFunctionParam(binn* obj) {
            return new FunctionParam(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        Return0* walkReturn0(binn* obj) {
            return new Return0;
        }

        Return1* walkReturn1(binn* obj) {
            return new Return1(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        Curry* walkCurry(binn* obj) {
            return new Curry(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        Call0* walkCall0(binn* obj) {
            return new Call0(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        Call1* walkCall1(binn* obj) {
            return new Call1(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        CallIf0* walkCallIf0(binn* obj) {
            return new CallIf0(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        CallIf1* walkCallIf1(binn* obj) {
            return new CallIf1(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_THIRD), _vm)
            );
        }

        CallElse0* walkCallElse0(binn* obj) {
            return new CallElse0(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        CallElse1* walkCallElse1(binn* obj) {
            return new CallElse1(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_THIRD), _vm)
            );
        }

        PushCall0* walkPushCall0(binn* obj) {
            return new PushCall0(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        PushCall1* walkPushCall1(binn* obj) {
            return new PushCall1(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        PushCallIf0* walkPushCallIf0(binn* obj) {
            return new PushCallIf0(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        PushCallIf1* walkPushCallIf1(binn* obj) {
            return new PushCallIf1(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_THIRD), _vm)
            );
        }

        PushCallElse0* walkPushCallElse0(binn* obj) {
            return new PushCallElse0(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        PushCallElse1* walkPushCallElse1(binn* obj) {
            return new PushCallElse1(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_THIRD), _vm)
            );
        }

        Drain* walkDrain(binn* obj) {
            return new Drain;
        }

        Exit* walkExit(binn* obj) {
            return new Exit;
        }

        MapInit* walkMapInit(binn* obj) {
            return new MapInit(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        MapSet* walkMapSet(binn* obj) {
            return new MapSet(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm),
                walkLocationReference((binn*) binn_map_map(obj, BC_THIRD))
            );
        }

        MapGet* walkMapGet(binn* obj) {
            return new MapGet(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        MapLength* walkMapLength(binn* obj) {
            return new MapLength(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        MapKeys* walkMapKeys(binn* obj) {
            return new MapKeys(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        Typify* walkTypify(binn* obj) {
            return new Typify(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST)),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        AssignValue* walkAssignValue(binn* obj) {
            return new AssignValue(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST)),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        AssignEval* walkAssignEval(binn* obj) {
            return new AssignEval(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST)),
                walkOne((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        Lock* walkLock(binn* obj) {
            return new Lock(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        Unlock* walkUnlock(binn* obj) {
            return new Unlock(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        IsEqual* walkIsEqual(binn* obj) {
            return new IsEqual(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        ScopeOf* walkScopeOf(binn* obj) {
            return new ScopeOf(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        StreamInit* walkStreamInit(binn* obj) {
            return new StreamInit(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        StreamPush* walkStreamPush(binn* obj) {
            return new StreamPush(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST)),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        StreamPop* walkStreamPop(binn* obj) {
            return new StreamPop(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        StreamClose* walkStreamClose(binn* obj) {
            return new StreamClose(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        StreamEmpty* walkStreamEmpty(binn* obj) {
            return new StreamEmpty(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        StreamPush* walkOut(binn* obj) {
            return walkStreamPush(obj);
        }

        StreamPush* walkErr(binn* obj) {
            return walkStreamPush(obj);
        }

        StringConcat* walkStringConcat(binn* obj) {
            return new StringConcat(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        StringLength* walkStringLength(binn* obj) {
            return new StringLength(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        StringSliceFrom* walkStringSliceFrom(binn* obj) {
            return new StringSliceFrom(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        StringSliceFromTo* walkStringSliceFromTo(binn* obj) {
            return new StringSliceFromTo(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_THIRD), _vm)
            );
        }

        TypeOf* walkTypeOf(binn* obj) {
            return new TypeOf(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        IsCompatible* walkIsCompatible(binn* obj) {
            return new IsCompatible(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_SECOND), _vm)
            );
        }

        PushExceptionHandler1* walkPushExceptionHandler1(binn* obj) {
            return new PushExceptionHandler1(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        PushExceptionHandler2* walkPushExceptionHandler2(binn* obj) {
            return new PushExceptionHandler2(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST)),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        PopExceptionHandler* walkPopExceptionHandler(binn* obj) {
            return new PopExceptionHandler(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        Raise* walkRaise(binn* obj) {
            return new Raise(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        Resume* walkResume(binn* obj) {
            return new Resume(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        OTypeInit* walkOTypeInit(binn*) {
            return new OTypeInit;
        }

        OTypeProp* walkOTypeProp(binn* obj) {
            return new OTypeProp(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND)),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_THIRD), _vm)
            );
        }

        OTypeDel* walkOTypeDel(binn* obj) {
            return new OTypeDel(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        OTypeGet* walkOTypeGet(binn* obj) {
            return new OTypeGet(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        OTypeFinalize* walkOTypeFinalize(binn* obj) {
            return new OTypeFinalize(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        OTypeSubset* walkOTypeSubset(binn* obj) {
            return new OTypeSubset(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        ObjInit* walkObjInit(binn* obj) {
            return new ObjInit(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        ObjSet* walkObjSet(binn* obj) {
            return new ObjSet(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND)),
                Wire::references()->produce((binn*) binn_map_map(obj, BC_THIRD), _vm)
            );
        }

        ObjGet* walkObjGet(binn* obj) {
            return new ObjGet(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        ObjInstance* walkObjInstance(binn* obj) {
            return new ObjInstance(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm)
            );
        }

        ObjCurry* walkObjCurry(binn* obj) {
            return new ObjCurry(
                Wire::references()->produce((binn*) binn_map_map(obj, BC_FIRST), _vm),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }
    };

}

#endif //SWARMVM_BINARYISAWALK
