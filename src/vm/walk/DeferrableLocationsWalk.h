#ifndef SWARM_DeferrableLocationsWALK_H
#define SWARM_DeferrableLocationsWALK_H

#include "../isa_meta.h"
#include "../ISAWalk.h"

using namespace swarmc::Runtime;

namespace swarmc::ISA {

    using DeferrableLocations = std::vector<LocationReference*>;

    class DeferrableLocationsWalk : public ISAWalk<DeferrableLocations> {
    public:
        DeferrableLocationsWalk() : ISAWalk<DeferrableLocations>() {}

        [[nodiscard]] std::string toString() const override {
            return "ISA::DeferrableLocationsWalk<>";
        }

    protected:
        DeferrableLocations walkPosition(PositionAnnotation* i) override {
            return {};
        }

        DeferrableLocations walkBinaryReferenceInstruction(BinaryReferenceInstruction* i) {
            DeferrableLocations loc;

            auto first = i->first();
            if ( first->tag() == ReferenceTag::LOCATION ) {
                auto deferredFirst = dynamic_cast<LocationReference*>(first);
                loc.push_back(deferredFirst);
            }

            auto second = i->second();
            if ( second->tag() == ReferenceTag::LOCATION ) {
                auto deferredSecond = dynamic_cast<LocationReference*>(second);
                loc.push_back(deferredSecond);
            }

            return loc;
        }

        DeferrableLocations walkBinaryLocationOperatorInstruction(BinaryInstruction<Reference, LocationReference>* i) {
            DeferrableLocations loc;

            auto first = i->first();
            if ( first->tag() == ReferenceTag::LOCATION ) {
                auto deferredFirst = dynamic_cast<LocationReference*>(first);
                loc.push_back(deferredFirst);
            }

            auto second = i->second();
            loc.push_back(second);

            return loc;
        }

        DeferrableLocations walkBinaryLocationConsumerInstruction(BinaryInstruction<LocationReference, Reference>* i) {
            DeferrableLocations loc;

            auto first = i->first();
            loc.push_back(first);

            auto second = i->second();
            if ( second->tag() == ReferenceTag::LOCATION ) {
                auto deferredSecond = dynamic_cast<LocationReference*>(second);
                loc.push_back(deferredSecond);
            }

            return loc;
        }

        DeferrableLocations walkUnaryReferenceInstruction(UnaryInstruction<Reference>* i) {
            DeferrableLocations loc;

            auto first = i->first();
            if ( first->tag() == ReferenceTag::LOCATION ) {
                auto deferredFirst = dynamic_cast<LocationReference*>(first);
                loc.push_back(deferredFirst);
            }

            return loc;
        }

        DeferrableLocations walkUnaryLocationReferenceInstruction(UnaryInstruction<LocationReference>* i) {
            return {i->first()};
        }

        DeferrableLocations walkTrinaryReferenceInstruction(TrinaryReferenceInstruction* i) {
            DeferrableLocations loc;

            auto first = i->first();
            if ( first->tag() == ReferenceTag::LOCATION ) {
                auto deferredFirst = dynamic_cast<LocationReference*>(first);
                loc.push_back(deferredFirst);
            }

            auto second = i->second();
            if ( second->tag() == ReferenceTag::LOCATION ) {
                auto deferredSecond = dynamic_cast<LocationReference*>(second);
                loc.push_back(deferredSecond);
            }

            auto third = i->third();
            if ( third->tag() == ReferenceTag::LOCATION ) {
                auto deferredThird = dynamic_cast<LocationReference*>(third);
                loc.push_back(deferredThird);
            }

            return loc;
        }

        DeferrableLocations walkPlus(Plus* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkMinus(Minus* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkTimes(Times* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkDivide(Divide* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkPower(Power* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkMod(Mod* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkNegative(Negative* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkGreaterThan(GreaterThan* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkGreaterThanOrEqual(GreaterThanOrEqual* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkLessThan(LessThan* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkLessThanOrEqual(LessThanOrEqual* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkAnd(And* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkOr(Or* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkXor(Xor* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkNand(Nand* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkNor(Nor* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkNot(Not* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkWhile(While* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        DeferrableLocations walkWith(With* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        DeferrableLocations walkEnumInit(EnumInit* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkEnumAppend(EnumAppend* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        DeferrableLocations walkEnumPrepend(EnumPrepend* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        DeferrableLocations walkEnumLength(EnumLength* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        DeferrableLocations walkEnumGet(EnumGet* i) override {
            return walkBinaryLocationConsumerInstruction(i);
        }

        DeferrableLocations walkEnumSet(EnumSet* i) override {
            DeferrableLocations loc;

            loc.push_back(i->first());

            if ( i->second()->tag() == ReferenceTag::LOCATION ) {
                auto deferredSecond = dynamic_cast<LocationReference*>(i->second());
                loc.push_back(deferredSecond);
            }

            if ( i->third()->tag() == ReferenceTag::LOCATION ) {
                auto deferredThird = dynamic_cast<LocationReference*>(i->third());
                loc.push_back(deferredThird);
            }

            return loc;
        }

        DeferrableLocations walkEnumConcat(EnumConcat* i) override {
            DeferrableLocations loc;
            loc.push_back(i->first());
            loc.push_back(i->second());
            return loc;
        }

        DeferrableLocations walkEnumerate(Enumerate* i) override {
            DeferrableLocations loc;

            if ( i->first()->tag() == ReferenceTag::LOCATION ) {
                auto deferredFirst = dynamic_cast<LocationReference*>(i->first());
                loc.push_back(deferredFirst);
            }

            loc.push_back(i->second());

            loc.push_back(i->third());

            return loc;
        }

        DeferrableLocations walkBeginFunction(BeginFunction* i) override {
            return walkBinaryLocationConsumerInstruction(i);
        }

        DeferrableLocations walkFunctionParam(FunctionParam* i) override {
            // FIXME: type can technically be a deferrable location
            // currently not getting it because that can cause a drain operation
            // to occur between beginfn and fnparam
            return {};
        }

        DeferrableLocations walkReturn1(Return1* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkReturn0(Return0* i) override {
            return {};
        }

        DeferrableLocations walkCurry(Curry* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkCall0(Call0* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkCall1(Call1* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkCallIf0(CallIf0* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkCallIf1(CallIf1* i) override {
            return walkTrinaryReferenceInstruction(i);
        }

        DeferrableLocations walkCallElse0(CallElse0* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkCallElse1(CallElse1* i) override {
            return walkTrinaryReferenceInstruction(i);
        }

        DeferrableLocations walkPushCall0(PushCall0* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkPushCall1(PushCall1* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkPushCallIf0(PushCallIf0* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkPushCallIf1(PushCallIf1* i) override {
            return walkTrinaryReferenceInstruction(i);
        }

        DeferrableLocations walkPushCallElse0(PushCallElse0* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkPushCallElse1(PushCallElse1* i) override {
            return walkTrinaryReferenceInstruction(i);
        }

        DeferrableLocations walkDrain(Drain* i) override {
            return {};
        }

        DeferrableLocations walkRetMapHas(RetMapHas* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkRetMapGet(RetMapGet* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkEnterContext(EnterContext* i) override {
            return {};
        }

        DeferrableLocations walkResumeContext(ResumeContext* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkPopContext(PopContext* i) override {
            return {};
        }

        DeferrableLocations walkExit(Exit* i) override {
            return {};
        }

        DeferrableLocations walkMapInit(MapInit* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkMapSet(MapSet* i) override {
            DeferrableLocations loc;

            if ( i->first()->tag() == ReferenceTag::LOCATION ) {
                auto deferredFirst = dynamic_cast<LocationReference*>(i->first());
                loc.push_back(deferredFirst);
            }

            if ( i->second()->tag() == ReferenceTag::LOCATION ) {
                auto deferredSecond = dynamic_cast<LocationReference*>(i->second());
                loc.push_back(deferredSecond);
            }

            loc.push_back(i->third());

            return loc;
        }

        DeferrableLocations walkMapGet(MapGet* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        DeferrableLocations walkMapLength(MapLength* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        DeferrableLocations walkMapKeys(MapKeys* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        DeferrableLocations walkTypify(Typify* i) override {
            return walkBinaryLocationConsumerInstruction(i);
        }

        DeferrableLocations walkAssignValue(AssignValue* i) override {
            DeferrableLocations loc;

            if ( i->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                auto deferredSecond = dynamic_cast<LocationReference*>(i->second());
                loc.push_back(deferredSecond);
            }

            return loc;
        }

        DeferrableLocations walkAssignEval(AssignEval* i) override {
            return walkOne(i->second());
        }

        DeferrableLocations walkLock(Lock* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        DeferrableLocations walkUnlock(Unlock* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        DeferrableLocations walkIsEqual(IsEqual* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkScopeOf(ScopeOf* i) override {
            return {};
        }

        DeferrableLocations walkStreamInit(StreamInit* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkStreamPush(StreamPush* i) override {
            return walkBinaryLocationConsumerInstruction(i);
        }

        DeferrableLocations walkStreamPop(StreamPop* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        DeferrableLocations walkStreamClose(StreamClose* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        DeferrableLocations walkStreamEmpty(StreamEmpty* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        DeferrableLocations walkOut(Out* i) override {
            return walkStreamPush(i);
        }

        DeferrableLocations walkErr(Err* i) override {
            return walkStreamPush(i);
        }

        DeferrableLocations walkStringConcat(StringConcat* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkStringLength(StringLength* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkStringSliceFrom(StringSliceFrom* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkStringSliceFromTo(StringSliceFromTo* i) override {
            return walkTrinaryReferenceInstruction(i);
        }

        DeferrableLocations walkTypeOf(TypeOf* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkIsCompatible(IsCompatible* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        DeferrableLocations walkPushExceptionHandler1(PushExceptionHandler1* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        DeferrableLocations walkPushExceptionHandler2(PushExceptionHandler2* i) override {
            DeferrableLocations loc;

            loc.push_back(i->first());

            loc.push_back(i->second());

            return loc;
        }

        DeferrableLocations walkPopExceptionHandler(PopExceptionHandler* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkRaise(Raise* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkResume(Resume* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        DeferrableLocations walkOTypeInit(OTypeInit* i) override {
            return {};
        }

        DeferrableLocations walkOTypeProp(OTypeProp* i) override {
            DeferrableLocations loc;

            if ( i->first()->tag() == ReferenceTag::LOCATION ) {
                auto deferredFirst = dynamic_cast<LocationReference*>(i->first());
                loc.push_back(deferredFirst);
            }

            loc.push_back(i->second());

            if ( i->third()->tag() == ReferenceTag::LOCATION ) {
                auto deferredThird = dynamic_cast<LocationReference*>(i->third());
                loc.push_back(deferredThird);
            }

            return loc;
        }

        DeferrableLocations walkOTypeDel(OTypeDel* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        DeferrableLocations walkOTypeGet(OTypeGet* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        DeferrableLocations walkOTypeFinalize(OTypeFinalize* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkOTypeSubset(OTypeSubset* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkObjInit(ObjInit* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkObjSet(ObjSet* i) override {
            DeferrableLocations loc;

            if ( i->first()->tag() == ReferenceTag::LOCATION ) {
                auto deferredFirst = dynamic_cast<LocationReference*>(i->first());
                loc.push_back(deferredFirst);
            }

            loc.push_back(i->second());

            if ( i->third()->tag() == ReferenceTag::LOCATION ) {
                auto deferredThird = dynamic_cast<LocationReference*>(i->third());
                loc.push_back(deferredThird);
            }

            return loc;
        }

        DeferrableLocations walkObjGet(ObjGet* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        DeferrableLocations walkObjInstance(ObjInstance* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        DeferrableLocations walkObjCurry(ObjCurry* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }
    };

}

#endif //SWARM_DEFERRABLELOCATIONSWALK_H
