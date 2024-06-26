#ifndef SWARMVM_EXECUTEWALK
#define SWARMVM_EXECUTEWALK

#include <cassert>
#include "../../shared/nslib.h"
#include "../../lang/Type.h"
#include "../ISAWalk.h"
#include "./SharedLocationsWalk.h"

using namespace nslib;

namespace swarmc::Runtime {

    class VirtualMachine;

    /**
     * ISA walk which uses the virtual machine to execute SVI instructions.
     */
    class ExecuteWalk : public ISA::ISAWalk<ISA::Reference*>, public IUsesLogger, public IRefCountable {
    public:
        explicit ExecuteWalk(VirtualMachine* vm) : ISA::ISAWalk<ISA::Reference*>(), IUsesLogger("vm"), _vm(vm) {
            _typeOfExceptionHandler = new Type::Lambda1(
                Type::Primitive::of(Type::Intrinsic::NUMBER),
                Type::Primitive::of(Type::Intrinsic::VOID)
            );
            _typeOfExceptionDiscriminator = new Type::Lambda1(
                Type::Primitive::of(Type::Intrinsic::NUMBER),
                Type::Primitive::of(Type::Intrinsic::BOOLEAN)
            );
            _sharedLocations = new ISA::SharedLocationsWalk;
        }

        ~ExecuteWalk() override {
            delete _typeOfExceptionHandler;
            delete _typeOfExceptionDiscriminator;
            delete _sharedLocations;
        }

        ISA::Reference* walkOne(ISA::Instruction* inst) override;

        ISA::Reference* walkOnePropagatingExceptions(ISA::Instruction* inst);

        /** Cast the reference as a number, or raise an exception. */
        virtual ISA::NumberReference* ensureNumber(const ISA::Reference*);

        /** Cast the reference as a boolean, or raise an exception. */
        virtual ISA::BooleanReference* ensureBoolean(const ISA::Reference*);

        /** Cast the reference as a type value, or raise an exception. */
        virtual ISA::TypeReference* ensureType(const ISA::Reference*);

        /** Cast the reference as an object type value, or raise an exception. */
        virtual ISA::ObjectTypeReference* ensureObjectType(const ISA::Reference*);

        /** Cast the reference as an object value, or raise an exception. */
        virtual ISA::ObjectReference* ensureObject(const ISA::Reference*);

        /** Cast the reference as a string, or raise an exception. */
        virtual ISA::StringReference* ensureString(const ISA::Reference*);

        /** Cast the reference as a function value, or raise an exception. */
        virtual ISA::FunctionReference* ensureFunction(const ISA::Reference*);

        /** Cast the reference as a context ID value, or raise an exception. */
        virtual ISA::ContextIdReference* ensureContextId(const ISA::Reference*);

        /** Cast the reference as a Job ID value, or raise an exception. */
        virtual ISA::JobIdReference* ensureJobId(const ISA::Reference*);

        /** Cast the reference as a ReturnValueMap value, or raise an exception */
        virtual ISA::ReturnValueMapReference* ensureReturnValueMap(const ISA::Reference*);

        /** Cast the reference as a function value, or raise an exception. */
        virtual InlineRefHandle<ISA::FunctionReference> ensureFunction(const InlineRefHandle<ISA::Reference>& ref);

        /** Cast the reference as an enumeration, or raise an exception. */
        virtual ISA::EnumerationReference* ensureEnumeration(const ISA::Reference*);

        virtual ISA::StreamReference* ensureStream(const ISA::Reference*);

        /** Cast the reference as a map, or raise an exception. */
        virtual ISA::MapReference* ensureMap(const ISA::Reference*);

        virtual ISA::ResourceReference* ensureResource(const ISA::Reference*);

    protected:
        VirtualMachine* _vm;
        ISA::SharedLocationsWalk* _sharedLocations;
        Type::Type* _typeOfExceptionHandler;
        Type::Type* _typeOfExceptionDiscriminator;

        [[nodiscard]] std::string toString() const override;

        /** Prints a message shown by default in the debug binary. */
        virtual void debug(const std::string& output) const {
            logger->debug(output);
        }

        /** Prints a message shown only when the `--verbose` flag is used. */
        virtual void verbose(const std::string& output) const {
            if ( Configuration::VERBOSE ) debug(output);
        }

        /** Verify that the reference is assignable to the given type, or raise an exception. */
        virtual void ensureType(const ISA::Reference*, const Type::Type*);

        virtual void ensureType(const ISA::Reference*, const InlineRefHandle<Type::Type>&);

        ISA::Reference* walkPosition(ISA::PositionAnnotation*) override;
        ISA::Reference* walkPlus(ISA::Plus*) override;
        ISA::Reference* walkMinus(ISA::Minus*) override;
        ISA::Reference* walkTimes(ISA::Times*) override;
        ISA::Reference* walkDivide(ISA::Divide*) override;
        ISA::Reference* walkPower(ISA::Power*) override;
        ISA::Reference* walkMod(ISA::Mod*) override;
        ISA::Reference* walkNegative(ISA::Negative*) override;
        ISA::Reference* walkGreaterThan(ISA::GreaterThan*) override;
        ISA::Reference* walkGreaterThanOrEqual(ISA::GreaterThanOrEqual*) override;
        ISA::Reference* walkLessThan(ISA::LessThan*) override;
        ISA::Reference* walkLessThanOrEqual(ISA::LessThanOrEqual*) override;
        ISA::Reference* walkAnd(ISA::And*) override;
        ISA::Reference* walkOr(ISA::Or*) override;
        ISA::Reference* walkXor(ISA::Xor*) override;
        ISA::Reference* walkNand(ISA::Nand*) override;
        ISA::Reference* walkNor(ISA::Nor*) override;
        ISA::Reference* walkNot(ISA::Not*) override;
        ISA::Reference* walkWhile(ISA::While*) override;
        ISA::Reference* walkWith(ISA::With*) override;
        ISA::Reference* walkEnumInit(ISA::EnumInit*) override;
        ISA::Reference* walkEnumAppend(ISA::EnumAppend*) override;
        ISA::Reference* walkEnumPrepend(ISA::EnumPrepend*) override;
        ISA::Reference* walkEnumLength(ISA::EnumLength*) override;
        ISA::Reference* walkEnumGet(ISA::EnumGet*) override;
        ISA::Reference* walkEnumSet(ISA::EnumSet*) override;
        ISA::Reference* walkEnumConcat(ISA::EnumConcat*) override;
        ISA::Reference* walkEnumerate(ISA::Enumerate*) override;
        ISA::Reference* walkBeginFunction(ISA::BeginFunction*) override;
        ISA::Reference* walkFunctionParam(ISA::FunctionParam*) override;
        ISA::Reference* walkReturn1(ISA::Return1*) override;
        ISA::Reference* walkReturn0(ISA::Return0*) override;
        ISA::Reference* walkCurry(ISA::Curry*) override;
        ISA::Reference* walkCall0(ISA::Call0*) override;
        ISA::Reference* walkCall1(ISA::Call1*) override;
        ISA::Reference* walkCallIf0(ISA::CallIf0*) override;
        ISA::Reference* walkCallIf1(ISA::CallIf1*) override;
        ISA::Reference* walkCallElse0(ISA::CallElse0*) override;
        ISA::Reference* walkCallElse1(ISA::CallElse1*) override;
        ISA::Reference* walkPushCall0(ISA::PushCall0*) override;
        ISA::Reference* walkPushCall1(ISA::PushCall1*) override;
        ISA::Reference* walkPushCallIf0(ISA::PushCallIf0*) override;
        ISA::Reference* walkPushCallIf1(ISA::PushCallIf1*) override;
        ISA::Reference* walkPushCallElse0(ISA::PushCallElse0*) override;
        ISA::Reference* walkPushCallElse1(ISA::PushCallElse1*) override;
        ISA::Reference* walkDrain(ISA::Drain*) override;
        ISA::Reference* walkRetMapHas(ISA::RetMapHas*) override;
        ISA::Reference* walkRetMapGet(ISA::RetMapGet*) override;
        ISA::Reference* walkEnterContext(ISA::EnterContext*) override;
        ISA::Reference* walkResumeContext(ISA::ResumeContext*) override;
        ISA::Reference* walkPopContext(ISA::PopContext*) override;
        ISA::Reference* walkExit(ISA::Exit*) override;
        ISA::Reference* walkMapInit(ISA::MapInit*) override;
        ISA::Reference* walkMapSet(ISA::MapSet*) override;
        ISA::Reference* walkMapGet(ISA::MapGet*) override;
        ISA::Reference* walkMapLength(ISA::MapLength*) override;
        ISA::Reference* walkMapKeys(ISA::MapKeys*) override;
        ISA::Reference* walkTypify(ISA::Typify*) override;
        ISA::Reference* walkAssignValue(ISA::AssignValue*) override;
        ISA::Reference* walkAssignEval(ISA::AssignEval*) override;
        ISA::Reference* walkLock(ISA::Lock*) override;
        ISA::Reference* walkUnlock(ISA::Unlock*) override;
        ISA::Reference* walkIsEqual(ISA::IsEqual*) override;
        ISA::Reference* walkScopeOf(ISA::ScopeOf*) override;
        ISA::Reference* walkStreamInit(ISA::StreamInit*) override;
        ISA::Reference* walkStreamPush(ISA::StreamPush*) override;
        ISA::Reference* walkStreamPop(ISA::StreamPop*) override;
        ISA::Reference* walkStreamClose(ISA::StreamClose*) override;
        ISA::Reference* walkStreamEmpty(ISA::StreamEmpty*) override;
        ISA::Reference* walkOut(ISA::Out*) override;
        ISA::Reference* walkErr(ISA::Err*) override;
        ISA::Reference* walkStringConcat(ISA::StringConcat*) override;
        ISA::Reference* walkStringLength(ISA::StringLength*) override;
        ISA::Reference* walkStringSliceFrom(ISA::StringSliceFrom*) override;
        ISA::Reference* walkStringSliceFromTo(ISA::StringSliceFromTo*) override;
        ISA::Reference* walkTypeOf(ISA::TypeOf*) override;
        ISA::Reference* walkIsCompatible(ISA::IsCompatible*) override;
        ISA::Reference* walkPushExceptionHandler1(ISA::PushExceptionHandler1*) override;
        ISA::Reference* walkPushExceptionHandler2(ISA::PushExceptionHandler2*) override;
        ISA::Reference* walkPopExceptionHandler(ISA::PopExceptionHandler*) override;
        ISA::Reference* walkRaise(ISA::Raise*) override;
        ISA::Reference* walkResume(ISA::Resume*) override;
        ISA::Reference* walkOTypeInit(ISA::OTypeInit*) override;
        ISA::Reference* walkOTypeProp(ISA::OTypeProp*) override;
        ISA::Reference* walkOTypeDel(ISA::OTypeDel*) override;
        ISA::Reference* walkOTypeGet(ISA::OTypeGet*) override;
        ISA::Reference* walkOTypeFinalize(ISA::OTypeFinalize*) override;
        ISA::Reference* walkOTypeSubset(ISA::OTypeSubset*) override;
        ISA::Reference* walkObjInit(ISA::ObjInit*) override;
        ISA::Reference* walkObjSet(ISA::ObjSet*) override;
        ISA::Reference* walkObjGet(ISA::ObjGet*) override;
        ISA::Reference* walkObjInstance(ISA::ObjInstance*) override;
        ISA::Reference* walkObjCurry(ISA::ObjCurry*) override;
    };

}

#endif //SWARMVM_EXECUTEWALK
