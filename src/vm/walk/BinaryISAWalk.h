#ifndef SWARMVM_BINARYISAWALK
#define SWARMVM_BINARYISAWALK

#include <cassert>
#include <iostream>
#include "../../../mod/binn/src/binn.h"
#include "../../shared/nslib.h"
#include "../isa_meta.h"
#include "./BinaryReferenceWalk.h"
#include "binary_const.h"

using namespace nslib;

namespace swarmc::Runtime {
    class VirtualMachine;
}

namespace swarmc::ISA {

    class BinaryISAWalk : public IStringable {
    public:
        explicit BinaryISAWalk(Runtime::VirtualMachine* vm = nullptr) {
            _ref = BinaryReferenceWalk(vm);
        }

        std::string toString() const override {
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

            throw Errors::SwarmError("Unable to deserialize instruction with invalid or unknown tag.");
        }

    protected:
        BinaryReferenceWalk _ref;

        NumberReference* walkNumberReference(binn* obj) {
            auto ref = _ref.walk(obj);
            assert(ref->tag() == ReferenceTag::NUMBER);
            return (NumberReference*) ref;
        }

        StringReference* walkStringReference(binn* obj) {
            auto ref = _ref.walk(obj);
            assert(ref->tag() == ReferenceTag::STRING);
            return (StringReference*) ref;
        }

        LocationReference* walkLocationReference(binn* obj) {
            auto ref = _ref.walk(obj);
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
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        Minus* walkMinus(binn* obj) {
            return new Minus(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        Times* walkTimes(binn* obj) {
            return new Times(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        Divide* walkDivide(binn* obj) {
            return new Divide(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        Power* walkPower(binn* obj) {
            return new Power(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        Mod* walkMod(binn* obj) {
            return new Mod(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }
        
        Negative* walkNegative(binn* obj) {
            return new Negative(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        GreaterThan* walkGreaterThan(binn* obj) {
            return new GreaterThan(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        GreaterThanOrEqual* walkGreaterThanOrEqual(binn* obj) {
            return new GreaterThanOrEqual(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        LessThan* walkLessThan(binn* obj) {
            return new LessThan(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        LessThanOrEqual* walkLessThanOrEqual(binn* obj) {
            return new LessThanOrEqual(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        And* walkAnd(binn* obj) {
            return new And(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        Or* walkOr(binn* obj) {
            return new Or(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        Xor* walkXor(binn* obj) {
            return new Xor(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        Nand* walkNand(binn* obj) {
            return new Nand(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        Nor* walkNor(binn* obj) {
            return new Nor(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        Not* walkNot(binn* obj) {
            return new Not(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        While* walkWhile(binn* obj) {
            return new While(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        With* walkWith(binn* obj) {
            return new With(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        EnumInit* walkEnumInit(binn* obj) {
            return new EnumInit(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        EnumAppend* walkEnumAppend(binn* obj) {
            return new EnumAppend(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        EnumPrepend* walkEnumPrepend(binn* obj) {
            return new EnumPrepend(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
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
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        EnumSet* walkEnumSet(binn* obj) {
            return new EnumSet(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND)),
                _ref.walk((binn*) binn_map_map(obj, BC_THIRD))
            );
        }

        Enumerate* walkEnumerate(binn* obj) {
            return new Enumerate(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND)),
                walkLocationReference((binn*) binn_map_map(obj, BC_THIRD))
            );
        }

        BeginFunction* walkBeginFunction(binn* obj) {
            auto bf = new BeginFunction(
                binn_map_str(obj, BC_NAME),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );

            if ( binn_map_bool(obj, BC_ISPURE) ) {
                bf->markAsPure();
            }

            return bf;
        }

        FunctionParam* walkFunctionParam(binn* obj) {
            return new FunctionParam(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                walkLocationReference((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        Return0* walkReturn0(binn* obj) {
            return new Return0;
        }

        Return1* walkReturn1(binn* obj) {
            return new Return1(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        Curry* walkCurry(binn* obj) {
            return new Curry(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        Call0* walkCall0(binn* obj) {
            return new Call0(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        Call1* walkCall1(binn* obj) {
            return new Call1(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        CallIf0* walkCallIf0(binn* obj) {
            return new CallIf0(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        CallIf1* walkCallIf1(binn* obj) {
            return new CallIf1(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND)),
                _ref.walk((binn*) binn_map_map(obj, BC_THIRD))
            );
        }

        CallElse0* walkCallElse0(binn* obj) {
            return new CallElse0(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        CallElse1* walkCallElse1(binn* obj) {
            return new CallElse1(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND)),
                _ref.walk((binn*) binn_map_map(obj, BC_THIRD))
            );
        }

        PushCall0* walkPushCall0(binn* obj) {
            return new PushCall0(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        PushCall1* walkPushCall1(binn* obj) {
            return new PushCall1(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        PushCallIf0* walkPushCallIf0(binn* obj) {
            return new PushCallIf0(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        PushCallIf1* walkPushCallIf1(binn* obj) {
            return new PushCallIf1(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND)),
                _ref.walk((binn*) binn_map_map(obj, BC_THIRD))
            );
        }

        PushCallElse0* walkPushCallElse0(binn* obj) {
            return new PushCallElse0(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        PushCallElse1* walkPushCallElse1(binn* obj) {
            return new PushCallElse1(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND)),
                _ref.walk((binn*) binn_map_map(obj, BC_THIRD))
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
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        MapSet* walkMapSet(binn* obj) {
            return new MapSet(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND)),
                walkLocationReference((binn*) binn_map_map(obj, BC_THIRD))
            );
        }

        MapGet* walkMapGet(binn* obj) {
            return new MapGet(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
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
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        AssignValue* walkAssignValue(binn* obj) {
            return new AssignValue(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
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
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        ScopeOf* walkScopeOf(binn* obj) {
            return new ScopeOf(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        StreamInit* walkStreamInit(binn* obj) {
            return new StreamInit(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        StreamPush* walkStreamPush(binn* obj) {
            return new StreamPush(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
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
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        StringLength* walkStringLength(binn* obj) {
            return new StringLength(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        StringSliceFrom* walkStringSliceFrom(binn* obj) {
            return new StringSliceFrom(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
            );
        }

        StringSliceFromTo* walkStringSliceFromTo(binn* obj) {
            return new StringSliceFromTo(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND)),
                _ref.walk((binn*) binn_map_map(obj, BC_THIRD))
            );
        }

        TypeOf* walkTypeOf(binn* obj) {
            return new TypeOf(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        IsCompatible* walkIsCompatible(binn* obj) {
            return new IsCompatible(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST)),
                _ref.walk((binn*) binn_map_map(obj, BC_SECOND))
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
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        Raise* walkRaise(binn* obj) {
            return new Raise(
                _ref.walk((binn*) binn_map_map(obj, BC_FIRST))
            );
        }

        Resume* walkResume(binn* obj) {
            return new Resume(
                walkLocationReference((binn*) binn_map_map(obj, BC_FIRST))
            );
        }
    };

}

#endif //SWARMVM_BINARYISAWALK
