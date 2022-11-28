#ifndef SWARMVM_ISASERIALIZEWALK
#define SWARMVM_ISASERIALIZEWALK

#include "../isa_meta.h"
#include "../ISAWalk.h"
#include "ReferenceSerializeWalk.h"

namespace swarmc::ISA {

    /**
     * ISA walk that writes a set of instruction objects back to SVI code.
     */
    class ISASerializeWalk : public ISAWalk<std::string> {
    public:
        [[nodiscard]] std::string toString() const override {
            return "ISASerializeWalk<>";
        }

    protected:
        ReferenceSerializeWalk _rsw;

        std::string walkPosition(PositionAnnotation* position) override {
            return ".position " + _rsw.walk(position->first()) + " " + _rsw.walk(position->second()) + " " + _rsw.walk(position->third());
        }

        std::string walkPlus(Plus* plus) override {
            return "plus " + _rsw.walk(plus->first()) + " " + _rsw.walk(plus->second());
        }

        std::string walkMinus(Minus* minus) override {
            return "minus " + _rsw.walk(minus->first()) + " " + _rsw.walk(minus->second());
        }

        std::string walkTimes(Times* times) override {
            return "times " + _rsw.walk(times->first()) + " " + _rsw.walk(times->second());
        }

        std::string walkDivide(Divide* divide) override {
            return "divide " + _rsw.walk(divide->first()) + " " + _rsw.walk(divide->second());
        }

        std::string walkPower(Power* power) override {
            return "power " + _rsw.walk(power->first()) + " " + _rsw.walk(power->second());
        }

        std::string walkMod(Mod* mod) override {
            return "mod " + _rsw.walk(mod->first()) + " " + _rsw.walk(mod->second());
        }

        std::string walkNegative(Negative* neg) override {
            return "neg " + _rsw.walk(neg->first());
        }

        std::string walkGreaterThan(GreaterThan* gt) override {
            return "gt " + _rsw.walk(gt->first()) + " " + _rsw.walk(gt->second());
        }

        std::string walkGreaterThanOrEqual(GreaterThanOrEqual* gte) override {
            return "gte " + _rsw.walk(gte->first()) + " " + _rsw.walk(gte->second());
        }

        std::string walkLessThan(LessThan* lt) override {
            return "lt " + _rsw.walk(lt->first()) + " " + _rsw.walk(lt->second());
        }

        std::string walkLessThanOrEqual(LessThanOrEqual* lte) override {
            return "lte " + _rsw.walk(lte->first()) + " " + _rsw.walk(lte->second());
        }

        std::string walkAnd(And* a) override {
            return "and " + _rsw.walk(a->first()) + " " + _rsw.walk(a->second());
        }

        std::string walkOr(Or* o) override {
            return "or " + _rsw.walk(o->first()) + " " + _rsw.walk(o->second());
        }

        std::string walkXor(Xor* x) override {
            return "xor " + _rsw.walk(x->first()) + " " + _rsw.walk(x->second());
        }

        std::string walkNand(Nand* n) override {
            return "nand " + _rsw.walk(n->first()) + " " + _rsw.walk(n->second());
        }

        std::string walkNor(Nor* n) override {
            return "nor " + _rsw.walk(n->first()) + " " + _rsw.walk(n->second());
        }

        std::string walkNot(Not* n) override {
            return "not " + _rsw.walk(n->first());
        }

        std::string walkWhile(While* w) override {
            return "while " + _rsw.walk(w->first()) + " " + _rsw.walk(w->second());
        }

        std::string walkWith(With* w) override {
            return "with " + _rsw.walk(w->first()) + " " + _rsw.walk(w->second());
        }

        std::string walkEnumInit(EnumInit* ei) override {
            return "enuminit " + _rsw.walk(ei->first());
        }

        std::string walkEnumAppend(EnumAppend* ea) override {
            return "enumappend " + _rsw.walk(ea->first()) + " " + _rsw.walk(ea->second());
        }

        std::string walkEnumPrepend(EnumPrepend* ep) override {
            return "enumprepend " + _rsw.walk(ep->first()) + " " + _rsw.walk(ep->second());
        }

        std::string walkEnumLength(EnumLength* el) override {
            return "enumlength " + _rsw.walk(el->first());
        }

        std::string walkEnumGet(EnumGet* eg) override {
            return "enumget " + _rsw.walk(eg->first()) + " " + _rsw.walk(eg->second());
        }

        std::string walkEnumSet(EnumSet* es) override {
            return "enumset " + _rsw.walk(es->first()) + " " + _rsw.walk(es->second()) + " " + _rsw.walk(es->third());
        }

        std::string walkEnumerate(Enumerate* e) override {
            return "enumerate " + _rsw.walk(e->first()) + " " + _rsw.walk(e->second()) + " " + _rsw.walk(e->third());
        }

        std::string walkBeginFunction(BeginFunction* bf) override {
            return "beginfn " + _rsw.walk(bf->first()) + " " + _rsw.walk(bf->second());
        }

        std::string walkFunctionParam(FunctionParam* fp) override {
            return "fnparam " + _rsw.walk(fp->first());
        }

        std::string walkReturn1(Return1* r1) override {
            return "return " + _rsw.walk(r1->first());
        }

        std::string walkReturn0(Return0*) override {
            return "return";
        }

        std::string walkCurry(Curry* c) override {
            return "curry " + _rsw.walk(c->first()) + " " + _rsw.walk(c->second());
        }

        std::string walkCall0(Call0* c0) override {
            return "call " + _rsw.walk(c0->first());
        }

        std::string walkCall1(Call1* c1) override {
            return "call " + _rsw.walk(c1->first()) + " " + _rsw.walk(c1->second());
        }

        std::string walkCallIf0(CallIf0* ci0) override {
            return "callif " + _rsw.walk(ci0->first()) + " " + _rsw.walk(ci0->second());
        }

        std::string walkCallIf1(CallIf1* ci1) override {
            return "callif " + _rsw.walk(ci1->first()) + " " + _rsw.walk(ci1->second()) + " " + _rsw.walk(ci1->third());
        }

        std::string walkCallElse0(CallElse0* ce0) override {
            return "callelse " + _rsw.walk(ce0->first()) + " " + _rsw.walk(ce0->second());
        }

        std::string walkCallElse1(CallElse1* ce1) override {
            return "callelse " + _rsw.walk(ce1->first()) + " " + _rsw.walk(ce1->second()) + " " + _rsw.walk(ce1->third());
        }

        std::string walkPushCall0(PushCall0* pc0) override {
            return "pushcall " + _rsw.walk(pc0->first());
        }

        std::string walkPushCall1(PushCall1* pc1) override {
            return "pushcall " + _rsw.walk(pc1->first()) + " " + _rsw.walk(pc1->second());
        }

        std::string walkPushCallIf0(PushCallIf0* pci0) override {
            return "pushcallif " + _rsw.walk(pci0->first()) + " " + _rsw.walk(pci0->second());
        }

        std::string walkPushCallIf1(PushCallIf1* pci1) override {
            return "pushcallif " + _rsw.walk(pci1->first()) + " " + _rsw.walk(pci1->second()) + " " + _rsw.walk(pci1->third());
        }

        std::string walkPushCallElse0(PushCallElse0* pce0) override {
            return "pushcallelse " + _rsw.walk(pce0->first()) + " " + _rsw.walk(pce0->second());
        }

        std::string walkPushCallElse1(PushCallElse1* pce1) override {
            return "pushcallelse " + _rsw.walk(pce1->first()) + " " + _rsw.walk(pce1->second()) + " " + _rsw.walk(pce1->third());
        }

        std::string walkDrain(Drain*) override {
            return "drain";
        }

        std::string walkExit(Exit*) override {
            return "exit";
        }

        std::string walkMapInit(MapInit* mi) override {
            return "mapinit " + _rsw.walk(mi->first());
        }

        std::string walkMapSet(MapSet* ms) override {
            return "mapset " + _rsw.walk(ms->first()) + " " + _rsw.walk(ms->second()) + " " + _rsw.walk(ms->third());
        }

        std::string walkMapGet(MapGet* mg) override {
            return "mapget " + _rsw.walk(mg->first()) + " " + _rsw.walk(mg->second());
        }

        std::string walkMapLength(MapLength* ml) override {
            return "maplength " + _rsw.walk(ml->first());
        }

        std::string walkMapKeys(MapKeys* mk) override {
            return "mapkeys " + _rsw.walk(mk->first());
        }

        std::string walkTypify(Typify* t) override {
            return "typify " + _rsw.walk(t->first()) + " " + _rsw.walk(t->second());
        }

        std::string walkAssignValue(AssignValue* av) override {
            return _rsw.walk(av->first()) + " <- " + _rsw.walk(av->second());
        }

        std::string walkAssignEval(AssignEval* ae) override {
            return _rsw.walk(ae->first()) + " <- " + walkOne(ae->second());
        }

        std::string walkLock(Lock* l) override {
            return "lock " + _rsw.walk(l->first());
        }

        std::string walkUnlock(Unlock* ul) override {
            return "unlock " + _rsw.walk(ul->first());
        }

        std::string walkIsEqual(IsEqual* ie) override {
            return "equal " + _rsw.walk(ie->first()) + " " + _rsw.walk(ie->second());
        }

        std::string walkScopeOf(ScopeOf* so) override {
            return "scopeof " + _rsw.walk(so->first());
        }

        std::string walkStreamInit(StreamInit* si) override {
            return "streaminit " + _rsw.walk(si->first());
        }

        std::string walkStreamPush(StreamPush* sp) override {
            return "streampush " + _rsw.walk(sp->first()) + " " + _rsw.walk(sp->second());
        }

        std::string walkStreamPop(StreamPop* sp) override {
            return "streampop " + _rsw.walk(sp->first());
        }

        std::string walkStreamClose(StreamClose* sc) override {
            return "streamclose " + _rsw.walk(sc->first());
        }

        std::string walkStreamEmpty(StreamEmpty* se) override {
            return "streamempty " + _rsw.walk(se->first());
        }

        std::string walkOut(Out* o) override {
            return "out " + _rsw.walk(o->second());
        }

        std::string walkErr(Err* e) override {
            return "err " + _rsw.walk(e->second());
        }

        std::string walkStringConcat(StringConcat* sc) override {
            return "strconcat " + _rsw.walk(sc->first()) + " " + _rsw.walk(sc->second());
        }

        std::string walkStringLength(StringLength* sl) override {
            return "strlength " + _rsw.walk(sl->first());
        }

        std::string walkStringSliceFrom(StringSliceFrom* ssf) override {
            return "strslice " + _rsw.walk(ssf->first()) + " " + _rsw.walk(ssf->second());
        }

        std::string walkStringSliceFromTo(StringSliceFromTo* ssft) override {
            return "strslice " + _rsw.walk(ssft->first()) + " " + _rsw.walk(ssft->second()) + " " + _rsw.walk(ssft->third());
        }

        std::string walkTypeOf(TypeOf* to) override {
            return "typeof " + _rsw.walk(to->first());
        }

        std::string walkIsCompatible(IsCompatible* ic) override {
            return "compatible " + _rsw.walk(ic->first()) + " " + _rsw.walk(ic->second());
        }

        std::string walkPushExceptionHandler1(PushExceptionHandler1* peh) override {
            return "pushexhandler " + _rsw.walk(peh->first());
        }

        std::string walkPushExceptionHandler2(PushExceptionHandler2* peh) override {
            return "pushexhandler " + _rsw.walk(peh->first()) + " " + _rsw.walk(peh->second());
        }

        std::string walkPopExceptionHandler(PopExceptionHandler* peh) override {
            return "popexhandler " + _rsw.walk(peh->first());
        }

        std::string walkRaise(Raise* peh) override {
            return "raise " + _rsw.walk(peh->first());
        }

        std::string walkResume(Resume* peh) override {
            return "resume " + _rsw.walk(peh->first());
        }
    };

}

#endif //SWARMVM_ISASERIALIZEWALK
