#ifndef SWARMVM_ISABINARYWALK
#define SWARMVM_ISABINARYWALK

#include "../../../mod/binn/src/binn.h"
#include "../isa_meta.h"
#include "../ISAWalk.h"
#include "./ReferenceBinaryWalk.h"

// FIXME: I'm sure this leaks memory out the ass.

namespace swarmc::ISA {

    class ISABinaryWalk : public ISAWalk<binn*> {
    public:
        std::string toString() const override {
            return "ISABinaryWalk<>";
        }

        static binn* serialize(const Instructions& is) {
            ISABinaryWalk walk;
            auto list = binn_list();
            for ( auto i : walk.walk(is) ) {
                binn_list_add_object(list, i);
            }

            auto obj = binn_object();
            binn_object_set_list(obj, "body", list);
            return obj;
        }

    protected:
        ReferenceBinaryWalk _ref;

        binn* walkPosition(PositionAnnotation* position) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) position->tag());
            binn_object_set_object(obj, "first", _ref.walk(position->first()));
            binn_object_set_object(obj, "second", _ref.walk(position->second()));
            binn_object_set_object(obj, "third", _ref.walk(position->third()));
            return obj;
        }

        binn* walkPlus(Plus* plus) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) plus->tag());
            binn_object_set_object(obj, "first", _ref.walk(plus->first()));
            binn_object_set_object(obj, "second", _ref.walk(plus->second()));
            return obj;
        }

        binn* walkMinus(Minus* minus) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) minus->tag());
            binn_object_set_object(obj, "first", _ref.walk(minus->first()));
            binn_object_set_object(obj, "second", _ref.walk(minus->second()));
            return obj;
        }

        binn* walkTimes(Times* times) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) times->tag());
            binn_object_set_object(obj, "first", _ref.walk(times->first()));
            binn_object_set_object(obj, "second", _ref.walk(times->second()));
            return obj;
        }

        binn* walkDivide(Divide* divide) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) divide->tag());
            binn_object_set_object(obj, "first", _ref.walk(divide->first()));
            binn_object_set_object(obj, "second", _ref.walk(divide->second()));
            return obj;
        }

        binn* walkPower(Power* power) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) power->tag());
            binn_object_set_object(obj, "first", _ref.walk(power->first()));
            binn_object_set_object(obj, "second", _ref.walk(power->second()));
            return obj;
        }

        binn* walkMod(Mod* mod) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) mod->tag());
            binn_object_set_object(obj, "first", _ref.walk(mod->first()));
            binn_object_set_object(obj, "second", _ref.walk(mod->second()));
            return obj;
        }

        binn* walkNegative(Negative* neg) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) neg->tag());
            binn_object_set_object(obj, "first", _ref.walk(neg->first()));
            return obj;
        }

        binn* walkGreaterThan(GreaterThan* gt) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) gt->tag());
            binn_object_set_object(obj, "first", _ref.walk(gt->first()));
            binn_object_set_object(obj, "second", _ref.walk(gt->second()));
            return obj;
        }

        binn* walkGreaterThanOrEqual(GreaterThanOrEqual* gte) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) gte->tag());
            binn_object_set_object(obj, "first", _ref.walk(gte->first()));
            binn_object_set_object(obj, "second", _ref.walk(gte->second()));
            return obj;
        }

        binn* walkLessThan(LessThan* lt) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) lt->tag());
            binn_object_set_object(obj, "first", _ref.walk(lt->first()));
            binn_object_set_object(obj, "second", _ref.walk(lt->second()));
            return obj;
        }

        binn* walkLessThanOrEqual(LessThanOrEqual* lte) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) lte->tag());
            binn_object_set_object(obj, "first", _ref.walk(lte->first()));
            binn_object_set_object(obj, "second", _ref.walk(lte->second()));
            return obj;
        }

        binn* walkAnd(And* a) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) a->tag());
            binn_object_set_object(obj, "first", _ref.walk(a->first()));
            binn_object_set_object(obj, "second", _ref.walk(a->second()));
            return obj;
        }

        binn* walkOr(Or* o) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) o->tag());
            binn_object_set_object(obj, "first", _ref.walk(o->first()));
            binn_object_set_object(obj, "second", _ref.walk(o->second()));
            return obj;
        }

        binn* walkXor(Xor* x) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) x->tag());
            binn_object_set_object(obj, "first", _ref.walk(x->first()));
            binn_object_set_object(obj, "second", _ref.walk(x->second()));
            return obj;
        }

        binn* walkNand(Nand* n) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) n->tag());
            binn_object_set_object(obj, "first", _ref.walk(n->first()));
            binn_object_set_object(obj, "second", _ref.walk(n->second()));
            return obj;
        }

        binn* walkNor(Nor* n) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) n->tag());
            binn_object_set_object(obj, "first", _ref.walk(n->first()));
            binn_object_set_object(obj, "second", _ref.walk(n->second()));
            return obj;
        }

        binn* walkNot(Not* n) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) n->tag());
            binn_object_set_object(obj, "first", _ref.walk(n->first()));
            return obj;
        }

        binn* walkWhile(While* w) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) w->tag());
            binn_object_set_object(obj, "first", _ref.walk(w->first()));
            binn_object_set_object(obj, "second", _ref.walk(w->second()));
            return obj;
        }

        binn* walkWith(With* w) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) w->tag());
            binn_object_set_object(obj, "first", _ref.walk(w->first()));
            binn_object_set_object(obj, "second", _ref.walk(w->second()));
            return obj;
        }

        binn* walkEnumInit(EnumInit* ei) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ei->tag());
            binn_object_set_object(obj, "first", _ref.walk(ei->first()));
            return obj;
        }

        binn* walkEnumAppend(EnumAppend* ea) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ea->tag());
            binn_object_set_object(obj, "first", _ref.walk(ea->first()));
            binn_object_set_object(obj, "second", _ref.walk(ea->second()));
            return obj;
        }

        binn* walkEnumPrepend(EnumPrepend* ep) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ep->tag());
            binn_object_set_object(obj, "first", _ref.walk(ep->first()));
            binn_object_set_object(obj, "second", _ref.walk(ep->second()));
            return obj;
        }

        binn* walkEnumLength(EnumLength* el) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) el->tag());
            binn_object_set_object(obj, "first", _ref.walk(el->first()));
            return obj;
        }

        binn* walkEnumGet(EnumGet* eg) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) eg->tag());
            binn_object_set_object(obj, "first", _ref.walk(eg->first()));
            binn_object_set_object(obj, "second", _ref.walk(eg->second()));
            return obj;
        }

        binn* walkEnumSet(EnumSet* es) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) es->tag());
            binn_object_set_object(obj, "first", _ref.walk(es->first()));
            binn_object_set_object(obj, "second", _ref.walk(es->second()));
            binn_object_set_object(obj, "third", _ref.walk(es->third()));
            return obj;
        }

        binn* walkEnumerate(Enumerate* e) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) e->tag());
            binn_object_set_object(obj, "first", _ref.walk(e->first()));
            binn_object_set_object(obj, "second", _ref.walk(e->second()));
            binn_object_set_object(obj, "third", _ref.walk(e->third()));
            return obj;
        }

        binn* walkBeginFunction(BeginFunction* bf) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) bf->tag());
            binn_object_set_object(obj, "first", _ref.walk(bf->first()));
            binn_object_set_object(obj, "second", _ref.walk(bf->second()));
            binn_object_set_bool(obj, "isPure", bf->isPure());
            return obj;
        }

        binn* walkFunctionParam(FunctionParam* fp) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) fp->tag());
            binn_object_set_object(obj, "first", _ref.walk(fp->first()));
            binn_object_set_object(obj, "second", _ref.walk(fp->second()));
            return obj;
        }

        binn* walkReturn0(Return0* r0) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) r0->tag());
            return obj;
        }

        binn* walkReturn1(Return1* r1) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) r1->tag());
            binn_object_set_object(obj, "first", _ref.walk(r1->first()));
            return obj;
        }

        binn* walkCurry(Curry* c) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) c->tag());
            binn_object_set_object(obj, "first", _ref.walk(c->first()));
            binn_object_set_object(obj, "second", _ref.walk(c->second()));
            return obj;
        }

        binn* walkCall0(Call0* c0) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) c0->tag());
            binn_object_set_object(obj, "first", _ref.walk(c0->first()));
            return obj;
        }

        binn* walkCall1(Call1* c1) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) c1->tag());
            binn_object_set_object(obj, "first", _ref.walk(c1->first()));
            binn_object_set_object(obj, "second", _ref.walk(c1->second()));
            return obj;
        }

        binn* walkCallIf0(CallIf0* ci0) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ci0->tag());
            binn_object_set_object(obj, "first", _ref.walk(ci0->first()));
            binn_object_set_object(obj, "second", _ref.walk(ci0->second()));
            return obj;
        }

        binn* walkCallIf1(CallIf1* ci1) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ci1->tag());
            binn_object_set_object(obj, "first", _ref.walk(ci1->first()));
            binn_object_set_object(obj, "second", _ref.walk(ci1->second()));
            binn_object_set_object(obj, "third", _ref.walk(ci1->third()));
            return obj;
        }

        binn* walkCallElse0(CallElse0* ce0) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ce0->tag());
            binn_object_set_object(obj, "first", _ref.walk(ce0->first()));
            binn_object_set_object(obj, "second", _ref.walk(ce0->second()));
            return obj;
        }

        binn* walkCallElse1(CallElse1* ce1) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ce1->tag());
            binn_object_set_object(obj, "first", _ref.walk(ce1->first()));
            binn_object_set_object(obj, "second", _ref.walk(ce1->second()));
            binn_object_set_object(obj, "third", _ref.walk(ce1->third()));
            return obj;
        }

        binn* walkPushCall0(PushCall0* pc0) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) pc0->tag());
            binn_object_set_object(obj, "first", _ref.walk(pc0->first()));
            return obj;
        }

        binn* walkPushCall1(PushCall1* pc1) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) pc1->tag());
            binn_object_set_object(obj, "first", _ref.walk(pc1->first()));
            binn_object_set_object(obj, "second", _ref.walk(pc1->second()));
            return obj;
        }

        binn* walkPushCallIf0(PushCallIf0* pci0) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) pci0->tag());
            binn_object_set_object(obj, "first", _ref.walk(pci0->first()));
            binn_object_set_object(obj, "second", _ref.walk(pci0->second()));
            return obj;
        }

        binn* walkPushCallIf1(PushCallIf1* pci1) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) pci1->tag());
            binn_object_set_object(obj, "first", _ref.walk(pci1->first()));
            binn_object_set_object(obj, "second", _ref.walk(pci1->second()));
            binn_object_set_object(obj, "third", _ref.walk(pci1->third()));
            return obj;
        }

        binn* walkPushCallElse0(PushCallElse0* pce0) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) pce0->tag());
            binn_object_set_object(obj, "first", _ref.walk(pce0->first()));
            binn_object_set_object(obj, "second", _ref.walk(pce0->second()));
            return obj;
        }

        binn* walkPushCallElse1(PushCallElse1* pce1) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) pce1->tag());
            binn_object_set_object(obj, "first", _ref.walk(pce1->first()));
            binn_object_set_object(obj, "second", _ref.walk(pce1->second()));
            binn_object_set_object(obj, "third", _ref.walk(pce1->third()));
            return obj;
        }

        binn* walkDrain(Drain* drain) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag",  (size_t) drain->tag());
            return obj;
        }

        binn* walkExit(Exit* exit) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag",  (size_t) exit->tag());
            return obj;
        }

        binn* walkMapInit(MapInit* mi) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) mi->tag());
            binn_object_set_object(obj, "first", _ref.walk(mi->first()));
            return obj;
        }

        binn* walkMapSet(MapSet* ms) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ms->tag());
            binn_object_set_object(obj, "first", _ref.walk(ms->first()));
            binn_object_set_object(obj, "second", _ref.walk(ms->second()));
            binn_object_set_object(obj, "third", _ref.walk(ms->third()));
            return obj;
        }

        binn* walkMapGet(MapGet* mg) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) mg->tag());
            binn_object_set_object(obj, "first", _ref.walk(mg->first()));
            binn_object_set_object(obj, "second", _ref.walk(mg->second()));
            return obj;
        }

        binn* walkMapLength(MapLength* ml) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ml->tag());
            binn_object_set_object(obj, "first", _ref.walk(ml->first()));
            return obj;
        }

        binn* walkMapKeys(MapKeys* mk) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) mk->tag());
            binn_object_set_object(obj, "first", _ref.walk(mk->first()));
            return obj;
        }

        binn* walkTypify(Typify* t) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) t->tag());
            binn_object_set_object(obj, "first", _ref.walk(t->first()));
            binn_object_set_object(obj, "second", _ref.walk(t->second()));
            return obj;
        }

        binn* walkAssignValue(AssignValue* av) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) av->tag());
            binn_object_set_object(obj, "first", _ref.walk(av->first()));
            binn_object_set_object(obj, "second", _ref.walk(av->second()));
            return obj;
        }

        binn* walkAssignEval(AssignEval* ae) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ae->tag());
            binn_object_set_object(obj, "first", _ref.walk(ae->first()));
            binn_object_set_object(obj, "second", walkOne(ae->second()));
            return obj;
        }

        binn* walkLock(Lock* l) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) l->tag());
            binn_object_set_object(obj, "first", _ref.walk(l->first()));
            return obj;
        }

        binn* walkUnlock(Unlock* ul) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ul->tag());
            binn_object_set_object(obj, "first", _ref.walk(ul->first()));
            return obj;
        }

        binn* walkIsEqual(IsEqual* ie) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ie->tag());
            binn_object_set_object(obj, "first", _ref.walk(ie->first()));
            binn_object_set_object(obj, "second", _ref.walk(ie->second()));
            return obj;
        }

        binn* walkScopeOf(ScopeOf* so) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) so->tag());
            binn_object_set_object(obj, "first", _ref.walk(so->first()));
            return obj;
        }

        binn* walkStreamInit(StreamInit* si) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) si->tag());
            binn_object_set_object(obj, "first", _ref.walk(si->first()));
            return obj;
        }

        binn* walkStreamPush(StreamPush* sp) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) sp->tag());
            binn_object_set_object(obj, "first", _ref.walk(sp->first()));
            binn_object_set_object(obj, "second", _ref.walk(sp->second()));
            return obj;
        }

        binn* walkStreamPop(StreamPop* sp) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) sp->tag());
            binn_object_set_object(obj, "first", _ref.walk(sp->first()));
            return obj;
        }

        binn* walkStreamClose(StreamClose* sc) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) sc->tag());
            binn_object_set_object(obj, "first", _ref.walk(sc->first()));
            return obj;
        }

        binn* walkStreamEmpty(StreamEmpty* se) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) se->tag());
            binn_object_set_object(obj, "first", _ref.walk(se->first()));
            return obj;
        }

        binn* walkOut(Out* o) override {
            return walkStreamPush(o);
        }

        binn* walkErr(Err* e) override {
            return walkStreamPush(e);
        }

        binn* walkStringConcat(StringConcat* sc) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) sc->tag());
            binn_object_set_object(obj, "first", _ref.walk(sc->first()));
            binn_object_set_object(obj, "second", _ref.walk(sc->second()));
            return obj;
        }

        binn* walkStringLength(StringLength* sl) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) sl->tag());
            binn_object_set_object(obj, "first", _ref.walk(sl->first()));
            return obj;
        }

        binn* walkStringSliceFrom(StringSliceFrom* ssf) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ssf->tag());
            binn_object_set_object(obj, "first", _ref.walk(ssf->first()));
            binn_object_set_object(obj, "second", _ref.walk(ssf->second()));
            return obj;
        }

        binn* walkStringSliceFromTo(StringSliceFromTo* ssft) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ssft->tag());
            binn_object_set_object(obj, "first", _ref.walk(ssft->first()));
            binn_object_set_object(obj, "second", _ref.walk(ssft->second()));
            binn_object_set_object(obj, "third", _ref.walk(ssft->third()));
            return obj;
        }

        binn* walkTypeOf(TypeOf* to) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) to->tag());
            binn_object_set_object(obj, "first", _ref.walk(to->first()));
            return obj;
        }

        binn* walkIsCompatible(IsCompatible* ic) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) ic->tag());
            binn_object_set_object(obj, "first", _ref.walk(ic->first()));
            binn_object_set_object(obj, "second", _ref.walk(ic->second()));
            return obj;
        }

        binn* walkPushExceptionHandler1(PushExceptionHandler1* peh) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) peh->tag());
            binn_object_set_object(obj, "first", _ref.walk(peh->first()));
            return obj;
        }

        binn* walkPushExceptionHandler2(PushExceptionHandler2* peh) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) peh->tag());
            binn_object_set_object(obj, "first", _ref.walk(peh->first()));
            binn_object_set_object(obj, "second", _ref.walk(peh->second()));
            return obj;
        }

        binn* walkPopExceptionHandler(PopExceptionHandler* peh) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) peh->tag());
            binn_object_set_object(obj, "first", _ref.walk(peh->first()));
            return obj;
        }

        binn* walkRaise(Raise* peh) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) peh->tag());
            binn_object_set_object(obj, "first", _ref.walk(peh->first()));
            return obj;
        }

        binn* walkResume(Resume* peh) override {
            auto obj = binn_object();
            binn_object_set_uint64(obj, "tag", (size_t) peh->tag());
            binn_object_set_object(obj, "first", _ref.walk(peh->first()));
            return obj;
        }
    };

}

#endif //SWARMVM_ISABINARYWALK
