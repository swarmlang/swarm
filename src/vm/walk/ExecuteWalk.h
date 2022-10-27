#ifndef SWARMVM_EXECUTEWALK
#define SWARMVM_EXECUTEWALK

#include "../../shared/util/Console.h"
#include "../../lang/Type.h"
#include "../ISAWalk.h"

namespace swarmc::Runtime {

    class VirtualMachine;

    class ExecuteWalk : public ISA::ISAWalk<ISA::Reference*> {
    public:
        ExecuteWalk(VirtualMachine* vm) : ISA::ISAWalk<ISA::Reference*>(), _vm(vm) {}
        virtual ~ExecuteWalk() = default;

    protected:
        VirtualMachine* _vm;

        virtual void ensureType(const ISA::Reference*, const Type::Type*);
        virtual ISA::NumberReference* ensureNumber(const ISA::Reference*);
        virtual ISA::BooleanReference* ensureBoolean(const ISA::Reference*);
        virtual ISA::TypeReference* ensureType(const ISA::Reference*);
        virtual ISA::StringReference* ensureString(const ISA::Reference*);
        virtual ISA::FunctionReference* ensureFunction(const ISA::Reference*);

        virtual ISA::Reference* walkPlus(ISA::Plus*) = 0;
        virtual ISA::Reference* walkMinus(ISA::Minus*) = 0;
        virtual ISA::Reference* walkTimes(ISA::Times*) = 0;
        virtual ISA::Reference* walkDivide(ISA::Divide*) = 0;
        virtual ISA::Reference* walkPower(ISA::Power*) = 0;
        virtual ISA::Reference* walkMod(ISA::Mod*) = 0;
        virtual ISA::Reference* walkNegative(ISA::Negative*) = 0;
        virtual ISA::Reference* walkGreaterThan(ISA::GreaterThan*) = 0;
        virtual ISA::Reference* walkGreaterThanOrEqual(ISA::GreaterThanOrEqual*) = 0;
        virtual ISA::Reference* walkLessThan(ISA::LessThan*) = 0;
        virtual ISA::Reference* walkLessThanOrEqual(ISA::LessThanOrEqual*) = 0;
        virtual ISA::Reference* walkAnd(ISA::And*) = 0;
        virtual ISA::Reference* walkOr(ISA::Or*) = 0;
        virtual ISA::Reference* walkXor(ISA::Xor*) = 0;
        virtual ISA::Reference* walkNand(ISA::Nand*) = 0;
        virtual ISA::Reference* walkNor(ISA::Nor*) = 0;
        virtual ISA::Reference* walkNot(ISA::Not*) = 0;
        virtual ISA::Reference* walkWhile(ISA::While*) = 0;
        virtual ISA::Reference* walkWith(ISA::With*) = 0;
        virtual ISA::Reference* walkEnumInit(ISA::EnumInit*) = 0;
        virtual ISA::Reference* walkEnumAppend(ISA::EnumAppend*) = 0;
        virtual ISA::Reference* walkEnumPrepend(ISA::EnumPrepend*) = 0;
        virtual ISA::Reference* walkEnumLength(ISA::EnumLength*) = 0;
        virtual ISA::Reference* walkEnumGet(ISA::EnumGet*) = 0;
        virtual ISA::Reference* walkEnumSet(ISA::EnumSet*) = 0;
        virtual ISA::Reference* walkEnumerate(ISA::Enumerate*) = 0;
        virtual ISA::Reference* walkBeginFunction(ISA::BeginFunction*) = 0;
        virtual ISA::Reference* walkFunctionParam(ISA::FunctionParam*) = 0;
        virtual ISA::Reference* walkReturn1(ISA::Return1*) = 0;
        virtual ISA::Reference* walkReturn0(ISA::Return0*) = 0;
        virtual ISA::Reference* walkCurry(ISA::Curry*) = 0;
        virtual ISA::Reference* walkCall0(ISA::Call0*) = 0;
        virtual ISA::Reference* walkCall1(ISA::Call1*) = 0;
        virtual ISA::Reference* walkCallIf0(ISA::CallIf0*) = 0;
        virtual ISA::Reference* walkCallIf1(ISA::CallIf1*) = 0;
        virtual ISA::Reference* walkCallElse0(ISA::CallElse0*) = 0;
        virtual ISA::Reference* walkCallElse1(ISA::CallElse1*) = 0;
        virtual ISA::Reference* walkPushCall0(ISA::PushCall0*) = 0;
        virtual ISA::Reference* walkPushCall1(ISA::PushCall1*) = 0;
        virtual ISA::Reference* walkPushCallIf0(ISA::PushCallIf0*) = 0;
        virtual ISA::Reference* walkPushCallIf1(ISA::PushCallIf1*) = 0;
        virtual ISA::Reference* walkPushCallElse0(ISA::PushCallElse0*) = 0;
        virtual ISA::Reference* walkPushCallElse1(ISA::PushCallElse1*) = 0;
        virtual ISA::Reference* walkMapInit(ISA::MapInit*) = 0;
        virtual ISA::Reference* walkMapSet(ISA::MapSet*) = 0;
        virtual ISA::Reference* walkMapGet(ISA::MapGet*) = 0;
        virtual ISA::Reference* walkMapLength(ISA::MapLength*) = 0;
        virtual ISA::Reference* walkMapKeys(ISA::MapKeys*) = 0;
        virtual ISA::Reference* walkTypify(ISA::Typify*) = 0;
        virtual ISA::Reference* walkAssignValue(ISA::AssignValue*) = 0;
        virtual ISA::Reference* walkAssignEval(ISA::AssignEval*) = 0;
        virtual ISA::Reference* walkLock(ISA::Lock*) = 0;
        virtual ISA::Reference* walkUnlock(ISA::Unlock*) = 0;
        virtual ISA::Reference* walkIsEqual(ISA::IsEqual*) = 0;
        virtual ISA::Reference* walkScopeOf(ISA::ScopeOf*) = 0;
        virtual ISA::Reference* walkStreamInit(ISA::StreamInit*) = 0;
        virtual ISA::Reference* walkStreamPush(ISA::StreamPush*) = 0;
        virtual ISA::Reference* walkStreamPop(ISA::StreamPop*) = 0;
        virtual ISA::Reference* walkStreamClose(ISA::StreamClose*) = 0;
        virtual ISA::Reference* walkStreamEmpty(ISA::StreamEmpty*) = 0;
        virtual ISA::Reference* walkOut(ISA::Out*) = 0;
        virtual ISA::Reference* walkErr(ISA::Err*) = 0;
        virtual ISA::Reference* walkStringConcat(ISA::StringConcat*) = 0;
        virtual ISA::Reference* walkStringLength(ISA::StringLength*) = 0;
        virtual ISA::Reference* walkStringSliceFrom(ISA::StringSliceFrom*) = 0;
        virtual ISA::Reference* walkStringSliceFromTo(ISA::StringSliceFromTo*) = 0;
        virtual ISA::Reference* walkTypeOf(ISA::TypeOf*) = 0;
        virtual ISA::Reference* walkIsCompatible(ISA::IsCompatible*) = 0;
        virtual ISA::Reference* walkPushExceptionHandler1(ISA::PushExceptionHandler1*) = 0;
        virtual ISA::Reference* walkPushExceptionHandler2(ISA::PushExceptionHandler2*) = 0;
        virtual ISA::Reference* walkPopExceptionHandler(ISA::PopExceptionHandler*) = 0;
        virtual ISA::Reference* walkRaise(ISA::Raise*) = 0;
        virtual ISA::Reference* walkResume(ISA::Resume*) = 0;
    };

}

#endif //SWARMVM_EXECUTEWALK
