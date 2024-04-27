#ifndef SWARM_SHAREDLOCATIONSWALK_H
#define SWARM_SHAREDLOCATIONSWALK_H

#include "../isa_meta.h"
#include "../ISAWalk.h"

using namespace swarmc::Runtime;

namespace swarmc::ISA {

    using SharedLocations = std::vector<LocationReference*>;

    class SharedLocationsWalk : public ISAWalk<SharedLocations> {
    public:
        SharedLocationsWalk() : ISAWalk<SharedLocations>() {}

        [[nodiscard]] std::string toString() const override {
            return "ISA::SharedLocationsWalk<>";
        }

    protected:
        SharedLocations walkPosition(PositionAnnotation* i) override {
            return {};
        }

        SharedLocations walkBinaryReferenceInstruction(BinaryReferenceInstruction* i) {
            SharedLocations loc;

            auto first = i->first();
            if ( first->tag() == ReferenceTag::LOCATION ) {
                auto sharedFirst = dynamic_cast<LocationReference*>(first);
                if ( sharedFirst->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedFirst);
                }
            }

            auto second = i->second();
            if ( second->tag() == ReferenceTag::LOCATION ) {
                auto sharedSecond = dynamic_cast<LocationReference*>(second);
                if ( sharedSecond->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedSecond);
                }
            }

            return loc;
        }

        SharedLocations walkBinaryLocationOperatorInstruction(BinaryInstruction<Reference, LocationReference>* i) {
            SharedLocations loc;

            auto first = i->first();
            if ( first->tag() == ReferenceTag::LOCATION ) {
                auto sharedFirst = dynamic_cast<LocationReference*>(first);
                if ( sharedFirst->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedFirst);
                }
            }

            auto second = i->second();
            if ( second->affinity() == Affinity::SHARED ) {
                loc.push_back(second);
            }

            return loc;
        }

        SharedLocations walkBinaryLocationConsumerInstruction(BinaryInstruction<LocationReference, Reference>* i) {
            SharedLocations loc;

            auto first = i->first();
            if ( first->affinity() == Affinity::SHARED ) {
                loc.push_back(first);
            }

            auto second = i->second();
            if ( second->tag() == ReferenceTag::LOCATION ) {
                auto sharedSecond = dynamic_cast<LocationReference*>(second);
                if ( sharedSecond->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedSecond);
                }
            }

            return loc;
        }

        SharedLocations walkUnaryReferenceInstruction(UnaryInstruction<Reference>* i) {
            SharedLocations loc;

            auto first = i->first();
            if ( first->tag() == ReferenceTag::LOCATION ) {
                auto sharedFirst = dynamic_cast<LocationReference*>(first);
                if ( sharedFirst->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedFirst);
                }
            }

            return loc;
        }

        SharedLocations walkUnaryLocationReferenceInstruction(UnaryInstruction<LocationReference>* i) {
            if ( i->first()->affinity() == Affinity::SHARED ) {
                return {i->first()};
            }

            return {};
        }

        SharedLocations walkTrinaryReferenceInstruction(TrinaryReferenceInstruction* i) {
            SharedLocations loc;

            auto first = i->first();
            if ( first->tag() == ReferenceTag::LOCATION ) {
                auto sharedFirst = dynamic_cast<LocationReference*>(first);
                if ( sharedFirst->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedFirst);
                }
            }

            auto second = i->second();
            if ( second->tag() == ReferenceTag::LOCATION ) {
                auto sharedSecond = dynamic_cast<LocationReference*>(second);
                if ( sharedSecond->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedSecond);
                }
            }

            auto third = i->third();
            if ( third->tag() == ReferenceTag::LOCATION ) {
                auto sharedThird = dynamic_cast<LocationReference*>(third);
                if ( sharedThird->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedThird);
                }
            }

            return loc;
        }

        SharedLocations walkPlus(Plus* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkMinus(Minus* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkTimes(Times* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkDivide(Divide* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkPower(Power* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkMod(Mod* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkNegative(Negative* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkGreaterThan(GreaterThan* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkGreaterThanOrEqual(GreaterThanOrEqual* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkLessThan(LessThan* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkLessThanOrEqual(LessThanOrEqual* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkAnd(And* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkOr(Or* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkXor(Xor* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkNand(Nand* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkNor(Nor* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkNot(Not* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkWhile(While* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        SharedLocations walkWith(With* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        SharedLocations walkEnumInit(EnumInit* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkEnumAppend(EnumAppend* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        SharedLocations walkEnumPrepend(EnumPrepend* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        SharedLocations walkEnumLength(EnumLength* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        SharedLocations walkEnumGet(EnumGet* i) override {
            return walkBinaryLocationConsumerInstruction(i);
        }

        SharedLocations walkEnumSet(EnumSet* i) override {
            SharedLocations loc;

            if ( i->first()->affinity() == Affinity::SHARED ) {
                loc.push_back(i->first());
            }

            if ( i->second()->tag() == ReferenceTag::LOCATION ) {
                auto sharedSecond = dynamic_cast<LocationReference*>(i->second());
                if ( sharedSecond->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedSecond);
                }
            }

            if ( i->third()->tag() == ReferenceTag::LOCATION ) {
                auto sharedThird = dynamic_cast<LocationReference*>(i->third());
                if ( sharedThird->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedThird);
                }
            }

            return loc;
        }

        SharedLocations walkEnumConcat(EnumConcat* i) override {
            SharedLocations loc;

            if ( i->first()->affinity() == Affinity::SHARED ) {
                loc.push_back(i->first());
            }

            if ( i->second()->affinity() == Affinity::SHARED ) {
                loc.push_back(i->second());
            }

            return loc;
        }

        SharedLocations walkEnumerate(Enumerate* i) override {
            SharedLocations loc;

            if ( i->first()->tag() == ReferenceTag::LOCATION ) {
                auto sharedFirst = dynamic_cast<LocationReference*>(i->first());
                if ( sharedFirst->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedFirst);
                }
            }

            if ( i->second()->affinity() == Affinity::SHARED ) {
                loc.push_back(i->second());
            }

            if ( i->third()->affinity() == Affinity::SHARED ) {
                loc.push_back(i->third());
            }

            return loc;
        }

        SharedLocations walkBeginFunction(BeginFunction* i) override {
            return walkBinaryLocationConsumerInstruction(i);
        }

        SharedLocations walkFunctionParam(FunctionParam* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        SharedLocations walkReturn1(Return1* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkReturn0(Return0* i) override {
            return {};
        }

        SharedLocations walkCurry(Curry* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkCall0(Call0* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkCall1(Call1* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkCallIf0(CallIf0* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkCallIf1(CallIf1* i) override {
            return walkTrinaryReferenceInstruction(i);
        }

        SharedLocations walkCallElse0(CallElse0* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkCallElse1(CallElse1* i) override {
            return walkTrinaryReferenceInstruction(i);
        }

        SharedLocations walkPushCall0(PushCall0* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkPushCall1(PushCall1* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkPushCallIf0(PushCallIf0* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkPushCallIf1(PushCallIf1* i) override {
            return walkTrinaryReferenceInstruction(i);
        }

        SharedLocations walkPushCallElse0(PushCallElse0* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkPushCallElse1(PushCallElse1* i) override {
            return walkTrinaryReferenceInstruction(i);
        }

        SharedLocations walkDrain(Drain* i) override {
            return {};
        }

        SharedLocations walkRetMapHas(RetMapHas* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkRetMapGet(RetMapGet* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkEnterContext(EnterContext* i) override {
            return {};
        }

        SharedLocations walkResumeContext(ResumeContext* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkPopContext(PopContext* i) override {
            return {};
        }

        SharedLocations walkExit(Exit* i) override {
            return {};
        }

        SharedLocations walkMapInit(MapInit* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkMapSet(MapSet* i) override {
            SharedLocations loc;

            if ( i->first()->tag() == ReferenceTag::LOCATION ) {
                auto sharedFirst = dynamic_cast<LocationReference*>(i->first());
                if ( sharedFirst->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedFirst);
                }
            }

            if ( i->second()->tag() == ReferenceTag::LOCATION ) {
                auto sharedSecond = dynamic_cast<LocationReference*>(i->second());
                if ( sharedSecond->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedSecond);
                }
            }

            if ( i->third()->affinity() == Affinity::SHARED ) {
                loc.push_back(i->third());
            }

            return loc;
        }

        SharedLocations walkMapGet(MapGet* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        SharedLocations walkMapLength(MapLength* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        SharedLocations walkMapKeys(MapKeys* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        SharedLocations walkTypify(Typify* i) override {
            return walkBinaryLocationConsumerInstruction(i);
        }

        SharedLocations walkAssignValue(AssignValue* i) override {
            return walkBinaryLocationConsumerInstruction(i);
        }

        SharedLocations walkAssignEval(AssignEval* i) override {
            SharedLocations loc;

            if ( i->first()->affinity() == Affinity::SHARED ) {
                loc.push_back(i->first());
            }

            auto subloc = walkOne(i->second());

            loc.insert(
                loc.end(),
                std::make_move_iterator(subloc.begin()),
                std::make_move_iterator(subloc.end())
            );

            return loc;
        }

        SharedLocations walkLock(Lock* i) override {
            return {};
        }

        SharedLocations walkUnlock(Unlock* i) override {
            return {};
        }

        SharedLocations walkIsEqual(IsEqual* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkScopeOf(ScopeOf* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        SharedLocations walkStreamInit(StreamInit* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkStreamPush(StreamPush* i) override {
            return walkBinaryLocationConsumerInstruction(i);
        }

        SharedLocations walkStreamPop(StreamPop* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        SharedLocations walkStreamClose(StreamClose* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        SharedLocations walkStreamEmpty(StreamEmpty* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        SharedLocations walkOut(Out* i) override {
            return walkStreamPush(i);
        }

        SharedLocations walkErr(Err* i) override {
            return walkStreamPush(i);
        }

        SharedLocations walkStringConcat(StringConcat* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkStringLength(StringLength* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkStringSliceFrom(StringSliceFrom* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkStringSliceFromTo(StringSliceFromTo* i) override {
            return walkTrinaryReferenceInstruction(i);
        }

        SharedLocations walkTypeOf(TypeOf* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkIsCompatible(IsCompatible* i) override {
            return walkBinaryReferenceInstruction(i);
        }

        SharedLocations walkPushExceptionHandler1(PushExceptionHandler1* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        SharedLocations walkPushExceptionHandler2(PushExceptionHandler2* i) override {
            SharedLocations loc;

            if ( i->first()->affinity() == Affinity::SHARED ) {
                loc.push_back(i->first());
            }

            if ( i->second()->affinity() == Affinity::SHARED ) {
                loc.push_back(i->second());
            }

            return loc;
        }

        SharedLocations walkPopExceptionHandler(PopExceptionHandler* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkRaise(Raise* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkResume(Resume* i) override {
            return walkUnaryLocationReferenceInstruction(i);
        }

        SharedLocations walkOTypeInit(OTypeInit* i) override {
            return {};
        }

        SharedLocations walkOTypeProp(OTypeProp* i) override {
            SharedLocations loc;

            if ( i->first()->tag() == ReferenceTag::LOCATION ) {
                auto sharedFirst = dynamic_cast<LocationReference*>(i->first());
                if ( sharedFirst->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedFirst);
                }
            }

            if ( i->second()->affinity() == Affinity::SHARED ) {
                loc.push_back(i->second());
            }

            if ( i->third()->tag() == ReferenceTag::LOCATION ) {
                auto sharedThird = dynamic_cast<LocationReference*>(i->third());
                if ( sharedThird->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedThird);
                }
            }

            return loc;
        }

        SharedLocations walkOTypeDel(OTypeDel* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        SharedLocations walkOTypeGet(OTypeGet* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        SharedLocations walkOTypeFinalize(OTypeFinalize* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkOTypeSubset(OTypeSubset* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkObjInit(ObjInit* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkObjSet(ObjSet* i) override {
            SharedLocations loc;

            if ( i->first()->tag() == ReferenceTag::LOCATION ) {
                auto sharedFirst = dynamic_cast<LocationReference*>(i->first());
                if ( sharedFirst->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedFirst);
                }
            }

            if ( i->second()->affinity() == Affinity::SHARED ) {
                loc.push_back(i->second());
            }

            if ( i->third()->tag() == ReferenceTag::LOCATION ) {
                auto sharedThird = dynamic_cast<LocationReference*>(i->third());
                if ( sharedThird->affinity() == Affinity::SHARED ) {
                    loc.push_back(sharedThird);
                }
            }

            return loc;
        }

        SharedLocations walkObjGet(ObjGet* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

        SharedLocations walkObjInstance(ObjInstance* i) override {
            return walkUnaryReferenceInstruction(i);
        }

        SharedLocations walkObjCurry(ObjCurry* i) override {
            return walkBinaryLocationOperatorInstruction(i);
        }

    };

}

#endif //SWARM_SHAREDLOCATIONSWALK_H
