#ifndef SWARMVM_ISAWALK
#define SWARMVM_ISAWALK

#include "../errors/SwarmError.h"
#include "../shared/nslib.h"
#include "isa_meta.h"

using namespace nslib;

namespace swarmc::ISA {

    /** Base class for operations that are applied to one or more Instructions. */
    template <typename TReturn>
    class ISAWalk : public IStringable, public IUsesConsole {
    public:
        ISAWalk() : IUsesConsole() {}
        ~ISAWalk() override = default;

        virtual std::vector<TReturn> walk(const Instructions& instructions) {
            std::vector<TReturn> returns;

            for ( auto inst : instructions ) {
                returns.push_back(walkOne(inst));
            }

            return returns;
        }

        virtual TReturn walkOne(Instruction* inst) {
            if ( inst->tag() == Tag::POSITION ) return walkPosition((PositionAnnotation*) inst);
            if ( inst->tag() == Tag::PLUS ) return walkPlus((Plus*) inst);
            if ( inst->tag() == Tag::MINUS ) return walkMinus((Minus*) inst);
            if ( inst->tag() == Tag::TIMES ) return walkTimes((Times*) inst);
            if ( inst->tag() == Tag::DIVIDE ) return walkDivide((Divide*) inst);
            if ( inst->tag() == Tag::POWER ) return walkPower((Power*) inst);
            if ( inst->tag() == Tag::MOD ) return walkMod((Mod*) inst);
            if ( inst->tag() == Tag::NEG ) return walkNegative((Negative*) inst);
            if ( inst->tag() == Tag::GT ) return walkGreaterThan((GreaterThan*) inst);
            if ( inst->tag() == Tag::GTE ) return walkGreaterThanOrEqual((GreaterThanOrEqual*) inst);
            if ( inst->tag() == Tag::LT ) return walkLessThan((LessThan*) inst);
            if ( inst->tag() == Tag::LTE ) return walkLessThanOrEqual((LessThanOrEqual*) inst);
            if ( inst->tag() == Tag::WHILE ) return walkWhile((While*) inst);
            if ( inst->tag() == Tag::WITH ) return walkWith((With*) inst);
            if ( inst->tag() == Tag::ENUMINIT ) return walkEnumInit((EnumInit*) inst);
            if ( inst->tag() == Tag::ENUMAPPEND ) return walkEnumAppend((EnumAppend*) inst);
            if ( inst->tag() == Tag::ENUMPREPEND ) return walkEnumPrepend((EnumPrepend*) inst);
            if ( inst->tag() == Tag::ENUMLENGTH ) return walkEnumLength((EnumLength*) inst);
            if ( inst->tag() == Tag::ENUMGET ) return walkEnumGet((EnumGet*) inst);
            if ( inst->tag() == Tag::ENUMSET ) return walkEnumSet((EnumSet*) inst);
            if ( inst->tag() == Tag::ENUMERATE ) return walkEnumerate((Enumerate*) inst);
            if ( inst->tag() == Tag::BEGINFN ) return walkBeginFunction((BeginFunction*) inst);
            if ( inst->tag() == Tag::FNPARAM ) return walkFunctionParam((FunctionParam*) inst);
            if ( inst->tag() == Tag::RETURN1 ) return walkReturn1((Return1*) inst);
            if ( inst->tag() == Tag::RETURN0 ) return walkReturn0((Return0*) inst);
            if ( inst->tag() == Tag::CURRY ) return walkCurry((Curry*) inst);
            if ( inst->tag() == Tag::CALL0 ) return walkCall0((Call0*) inst);
            if ( inst->tag() == Tag::CALL1 ) return walkCall1((Call1*) inst);
            if ( inst->tag() == Tag::CALLIF0 ) return walkCallIf0((CallIf0*) inst);
            if ( inst->tag() == Tag::CALLIF1 ) return walkCallIf1((CallIf1*) inst);
            if ( inst->tag() == Tag::CALLELSE0 ) return walkCallElse0((CallElse0*) inst);
            if ( inst->tag() == Tag::CALLELSE1 ) return walkCallElse1((CallElse1*) inst);
            if ( inst->tag() == Tag::PUSHCALL0 ) return walkPushCall0((PushCall0*) inst);
            if ( inst->tag() == Tag::PUSHCALL1 ) return walkPushCall1((PushCall1*) inst);
            if ( inst->tag() == Tag::PUSHCALLIF0 ) return walkPushCallIf0((PushCallIf0*) inst);
            if ( inst->tag() == Tag::PUSHCALLIF1 ) return walkPushCallIf1((PushCallIf1*) inst);
            if ( inst->tag() == Tag::PUSHCALLELSE0 ) return walkPushCallElse0((PushCallElse0*) inst);
            if ( inst->tag() == Tag::PUSHCALLELSE1 ) return walkPushCallElse1((PushCallElse1*) inst);
            if ( inst->tag() == Tag::DRAIN ) return walkDrain((Drain*) inst);
            if ( inst->tag() == Tag::EXIT ) return walkExit((Exit*) inst);
            if ( inst->tag() == Tag::MAPINIT ) return walkMapInit((MapInit*) inst);
            if ( inst->tag() == Tag::MAPSET ) return walkMapSet((MapSet*) inst);
            if ( inst->tag() == Tag::MAPGET ) return walkMapGet((MapGet*) inst);
            if ( inst->tag() == Tag::MAPLENGTH ) return walkMapLength((MapLength*) inst);
            if ( inst->tag() == Tag::MAPKEYS ) return walkMapKeys((MapKeys*) inst);
            if ( inst->tag() == Tag::TYPIFY ) return walkTypify((Typify*) inst);
            if ( inst->tag() == Tag::ASSIGNVALUE ) return walkAssignValue((AssignValue*) inst);
            if ( inst->tag() == Tag::ASSIGNEVAL ) return walkAssignEval((AssignEval*) inst);
            if ( inst->tag() == Tag::LOCK ) return walkLock((Lock*) inst);
            if ( inst->tag() == Tag::UNLOCK ) return walkUnlock((Unlock*) inst);
            if ( inst->tag() == Tag::EQUAL ) return walkIsEqual((IsEqual*) inst);
            if ( inst->tag() == Tag::SCOPEOF ) return walkScopeOf((ScopeOf*) inst);
            if ( inst->tag() == Tag::STREAMINIT ) return walkStreamInit((StreamInit*) inst);
            if ( inst->tag() == Tag::STREAMPUSH ) return walkStreamPush((StreamPush*) inst);
            if ( inst->tag() == Tag::STREAMPOP ) return walkStreamPop((StreamPop*) inst);
            if ( inst->tag() == Tag::STREAMCLOSE ) return walkStreamClose((StreamClose*) inst);
            if ( inst->tag() == Tag::STREAMEMPTY ) return walkStreamEmpty((StreamEmpty*) inst);
            if ( inst->tag() == Tag::OUT ) return walkOut((Out*) inst);
            if ( inst->tag() == Tag::ERR ) return walkErr((Err*) inst);
            if ( inst->tag() == Tag::STRCONCAT ) return walkStringConcat((StringConcat*) inst);
            if ( inst->tag() == Tag::STRLENGTH ) return walkStringLength((StringLength*) inst);
            if ( inst->tag() == Tag::STRSLICEFROM ) return walkStringSliceFrom((StringSliceFrom*) inst);
            if ( inst->tag() == Tag::STRSLICEFROMTO ) return walkStringSliceFromTo((StringSliceFromTo*) inst);
            if ( inst->tag() == Tag::TYPEOF ) return walkTypeOf((TypeOf*) inst);
            if ( inst->tag() == Tag::COMPATIBLE ) return walkIsCompatible((IsCompatible*) inst);
            if ( inst->tag() == Tag::PUSHEXHANDLER1 ) return walkPushExceptionHandler1((PushExceptionHandler1*) inst);
            if ( inst->tag() == Tag::PUSHEXHANDLER2 ) return walkPushExceptionHandler2((PushExceptionHandler2*) inst);
            if ( inst->tag() == Tag::POPEXHANDLER ) return walkPopExceptionHandler((PopExceptionHandler*) inst);
            if ( inst->tag() == Tag::RAISE ) return walkRaise((Raise*) inst);
            if ( inst->tag() == Tag::RESUME ) return walkResume((Resume*) inst);
            if ( inst->tag() == Tag::AND ) return walkAnd((And*) inst);
            if ( inst->tag() == Tag::OR ) return walkOr((Or*) inst);
            if ( inst->tag() == Tag::XOR ) return walkXor((Xor*) inst);
            if ( inst->tag() == Tag::NAND ) return walkNand((Nand*) inst);
            if ( inst->tag() == Tag::NOR ) return walkNor((Nor*) inst);
            if ( inst->tag() == Tag::NOT ) return walkNot((Not*) inst);
            if ( inst->tag() == Tag::OTYPEINIT ) return walkOTypeInit((OTypeInit*) inst);
            if ( inst->tag() == Tag::OTYPEPROP ) return walkOTypeProp((OTypeProp*) inst);
            if ( inst->tag() == Tag::OTYPEDEL ) return walkOTypeDel((OTypeDel*) inst);
            if ( inst->tag() == Tag::OTYPEGET ) return walkOTypeGet((OTypeGet*) inst);
            if ( inst->tag() == Tag::OTYPEFINALIZE ) return walkOTypeFinalize((OTypeFinalize*) inst);
            if ( inst->tag() == Tag::OTYPESUBSET ) return walkOTypeSubset((OTypeSubset*) inst);
            if ( inst->tag() == Tag::OBJINIT ) return walkObjInit((ObjInit*) inst);
            if ( inst->tag() == Tag::OBJSET ) return walkObjSet((ObjSet*) inst);
            if ( inst->tag() == Tag::OBJGET ) return walkObjGet((ObjGet*) inst);
            if ( inst->tag() == Tag::OBJINSTANCE ) return walkObjInstance((ObjInstance*) inst);
            if ( inst->tag() == Tag::OBJCURRY ) return walkObjCurry((ObjCurry*) inst);

            throw Errors::SwarmError("Invalid instruction tag: " + inst->toString());
        }

    protected:
        virtual TReturn walkPosition(PositionAnnotation*) = 0;
        virtual TReturn walkPlus(Plus*) = 0;
        virtual TReturn walkMinus(Minus*) = 0;
        virtual TReturn walkTimes(Times*) = 0;
        virtual TReturn walkDivide(Divide*) = 0;
        virtual TReturn walkPower(Power*) = 0;
        virtual TReturn walkMod(Mod*) = 0;
        virtual TReturn walkNegative(Negative*) = 0;
        virtual TReturn walkGreaterThan(GreaterThan*) = 0;
        virtual TReturn walkGreaterThanOrEqual(GreaterThanOrEqual*) = 0;
        virtual TReturn walkLessThan(LessThan*) = 0;
        virtual TReturn walkLessThanOrEqual(LessThanOrEqual*) = 0;
        virtual TReturn walkAnd(And*) = 0;
        virtual TReturn walkOr(Or*) = 0;
        virtual TReturn walkXor(Xor*) = 0;
        virtual TReturn walkNand(Nand*) = 0;
        virtual TReturn walkNor(Nor*) = 0;
        virtual TReturn walkNot(Not*) = 0;
        virtual TReturn walkWhile(While*) = 0;
        virtual TReturn walkWith(With*) = 0;
        virtual TReturn walkEnumInit(EnumInit*) = 0;
        virtual TReturn walkEnumAppend(EnumAppend*) = 0;
        virtual TReturn walkEnumPrepend(EnumPrepend*) = 0;
        virtual TReturn walkEnumLength(EnumLength*) = 0;
        virtual TReturn walkEnumGet(EnumGet*) = 0;
        virtual TReturn walkEnumSet(EnumSet*) = 0;
        virtual TReturn walkEnumerate(Enumerate*) = 0;
        virtual TReturn walkBeginFunction(BeginFunction*) = 0;
        virtual TReturn walkFunctionParam(FunctionParam*) = 0;
        virtual TReturn walkReturn1(Return1*) = 0;
        virtual TReturn walkReturn0(Return0*) = 0;
        virtual TReturn walkCurry(Curry*) = 0;
        virtual TReturn walkCall0(Call0*) = 0;
        virtual TReturn walkCall1(Call1*) = 0;
        virtual TReturn walkCallIf0(CallIf0*) = 0;
        virtual TReturn walkCallIf1(CallIf1*) = 0;
        virtual TReturn walkCallElse0(CallElse0*) = 0;
        virtual TReturn walkCallElse1(CallElse1*) = 0;
        virtual TReturn walkPushCall0(PushCall0*) = 0;
        virtual TReturn walkPushCall1(PushCall1*) = 0;
        virtual TReturn walkPushCallIf0(PushCallIf0*) = 0;
        virtual TReturn walkPushCallIf1(PushCallIf1*) = 0;
        virtual TReturn walkPushCallElse0(PushCallElse0*) = 0;
        virtual TReturn walkPushCallElse1(PushCallElse1*) = 0;
        virtual TReturn walkDrain(Drain*) = 0;
        virtual TReturn walkExit(Exit*) = 0;
        virtual TReturn walkMapInit(MapInit*) = 0;
        virtual TReturn walkMapSet(MapSet*) = 0;
        virtual TReturn walkMapGet(MapGet*) = 0;
        virtual TReturn walkMapLength(MapLength*) = 0;
        virtual TReturn walkMapKeys(MapKeys*) = 0;
        virtual TReturn walkTypify(Typify*) = 0;
        virtual TReturn walkAssignValue(AssignValue*) = 0;
        virtual TReturn walkAssignEval(AssignEval*) = 0;
        virtual TReturn walkLock(Lock*) = 0;
        virtual TReturn walkUnlock(Unlock*) = 0;
        virtual TReturn walkIsEqual(IsEqual*) = 0;
        virtual TReturn walkScopeOf(ScopeOf*) = 0;
        virtual TReturn walkStreamInit(StreamInit*) = 0;
        virtual TReturn walkStreamPush(StreamPush*) = 0;
        virtual TReturn walkStreamPop(StreamPop*) = 0;
        virtual TReturn walkStreamClose(StreamClose*) = 0;
        virtual TReturn walkStreamEmpty(StreamEmpty*) = 0;
        virtual TReturn walkOut(Out*) = 0;
        virtual TReturn walkErr(Err*) = 0;
        virtual TReturn walkStringConcat(StringConcat*) = 0;
        virtual TReturn walkStringLength(StringLength*) = 0;
        virtual TReturn walkStringSliceFrom(StringSliceFrom*) = 0;
        virtual TReturn walkStringSliceFromTo(StringSliceFromTo*) = 0;
        virtual TReturn walkTypeOf(TypeOf*) = 0;
        virtual TReturn walkIsCompatible(IsCompatible*) = 0;
        virtual TReturn walkPushExceptionHandler1(PushExceptionHandler1*) = 0;
        virtual TReturn walkPushExceptionHandler2(PushExceptionHandler2*) = 0;
        virtual TReturn walkPopExceptionHandler(PopExceptionHandler*) = 0;
        virtual TReturn walkRaise(Raise*) = 0;
        virtual TReturn walkResume(Resume*) = 0;
        virtual TReturn walkOTypeInit(OTypeInit*) = 0;
        virtual TReturn walkOTypeProp(OTypeProp*) = 0;
        virtual TReturn walkOTypeDel(OTypeDel*) = 0;
        virtual TReturn walkOTypeGet(OTypeGet*) = 0;
        virtual TReturn walkOTypeFinalize(OTypeFinalize*) = 0;
        virtual TReturn walkOTypeSubset(OTypeSubset*) = 0;
        virtual TReturn walkObjInit(ObjInit*) = 0;
        virtual TReturn walkObjSet(ObjSet*) = 0;
        virtual TReturn walkObjGet(ObjGet*) = 0;
        virtual TReturn walkObjInstance(ObjInstance*) = 0;
        virtual TReturn walkObjCurry(ObjCurry*) = 0;
    };

}

#endif //SWARMVM_ISAWALK
