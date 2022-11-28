#ifndef SWARMVM_ISABINARYWALK
#define SWARMVM_ISABINARYWALK

#include "../../../mod/binn/src/binn.h"
#include "../isa_meta.h"
#include "../ISAWalk.h"
#include "./ReferenceBinaryWalk.h"
#include "binary_const.h"

// FIXME: I'm sure this leaks memory out the ass.

namespace swarmc::ISA {

    class ISABinaryWalk : public ISAWalk<binn*> {
    public:
        [[nodiscard]] std::string toString() const override {
            return "ISABinaryWalk<>";
        }

        static binn* serialize(const Instructions& is) {
            ISABinaryWalk walk;
            auto list = binn_list();
            for ( auto i : walk.walk(is) ) {
                binn_list_add_map(list, i);
            }

            auto obj = binn_map();
            binn_map_set_list(obj, BC_BODY, list);
            return obj;
        }

    protected:
        ReferenceBinaryWalk _ref;

        binn* walkPosition(PositionAnnotation* position) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) position->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(position->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(position->second()));
            binn_map_set_map(obj, BC_THIRD, _ref.walk(position->third()));
            return obj;
        }

        binn* walkPlus(Plus* plus) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) plus->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(plus->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(plus->second()));
            return obj;
        }

        binn* walkMinus(Minus* minus) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) minus->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(minus->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(minus->second()));
            return obj;
        }

        binn* walkTimes(Times* times) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) times->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(times->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(times->second()));
            return obj;
        }

        binn* walkDivide(Divide* divide) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) divide->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(divide->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(divide->second()));
            return obj;
        }

        binn* walkPower(Power* power) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) power->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(power->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(power->second()));
            return obj;
        }

        binn* walkMod(Mod* mod) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) mod->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(mod->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(mod->second()));
            return obj;
        }

        binn* walkNegative(Negative* neg) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) neg->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(neg->first()));
            return obj;
        }

        binn* walkGreaterThan(GreaterThan* gt) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) gt->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(gt->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(gt->second()));
            return obj;
        }

        binn* walkGreaterThanOrEqual(GreaterThanOrEqual* gte) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) gte->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(gte->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(gte->second()));
            return obj;
        }

        binn* walkLessThan(LessThan* lt) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) lt->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(lt->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(lt->second()));
            return obj;
        }

        binn* walkLessThanOrEqual(LessThanOrEqual* lte) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) lte->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(lte->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(lte->second()));
            return obj;
        }

        binn* walkAnd(And* a) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) a->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(a->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(a->second()));
            return obj;
        }

        binn* walkOr(Or* o) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) o->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(o->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(o->second()));
            return obj;
        }

        binn* walkXor(Xor* x) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) x->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(x->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(x->second()));
            return obj;
        }

        binn* walkNand(Nand* n) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) n->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(n->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(n->second()));
            return obj;
        }

        binn* walkNor(Nor* n) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) n->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(n->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(n->second()));
            return obj;
        }

        binn* walkNot(Not* n) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) n->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(n->first()));
            return obj;
        }

        binn* walkWhile(While* w) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) w->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(w->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(w->second()));
            return obj;
        }

        binn* walkWith(With* w) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) w->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(w->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(w->second()));
            return obj;
        }

        binn* walkEnumInit(EnumInit* ei) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ei->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ei->first()));
            return obj;
        }

        binn* walkEnumAppend(EnumAppend* ea) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ea->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ea->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(ea->second()));
            return obj;
        }

        binn* walkEnumPrepend(EnumPrepend* ep) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ep->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ep->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(ep->second()));
            return obj;
        }

        binn* walkEnumLength(EnumLength* el) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) el->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(el->first()));
            return obj;
        }

        binn* walkEnumGet(EnumGet* eg) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) eg->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(eg->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(eg->second()));
            return obj;
        }

        binn* walkEnumSet(EnumSet* es) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) es->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(es->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(es->second()));
            binn_map_set_map(obj, BC_THIRD, _ref.walk(es->third()));
            return obj;
        }

        binn* walkEnumerate(Enumerate* e) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) e->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(e->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(e->second()));
            binn_map_set_map(obj, BC_THIRD, _ref.walk(e->third()));
            return obj;
        }

        binn* walkBeginFunction(BeginFunction* bf) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) bf->tag());
            binn_map_set_str(obj, BC_NAME, strdup(bf->first()->name().c_str()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(bf->second()));
            binn_map_set_bool(obj, BC_ISPURE, bf->isPure());
            return obj;
        }

        binn* walkFunctionParam(FunctionParam* fp) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) fp->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(fp->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(fp->second()));
            return obj;
        }

        binn* walkReturn0(Return0* r0) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) r0->tag());
            return obj;
        }

        binn* walkReturn1(Return1* r1) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) r1->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(r1->first()));
            return obj;
        }

        binn* walkCurry(Curry* c) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) c->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(c->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(c->second()));
            return obj;
        }

        binn* walkCall0(Call0* c0) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) c0->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(c0->first()));
            return obj;
        }

        binn* walkCall1(Call1* c1) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) c1->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(c1->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(c1->second()));
            return obj;
        }

        binn* walkCallIf0(CallIf0* ci0) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ci0->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ci0->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(ci0->second()));
            return obj;
        }

        binn* walkCallIf1(CallIf1* ci1) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ci1->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ci1->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(ci1->second()));
            binn_map_set_map(obj, BC_THIRD, _ref.walk(ci1->third()));
            return obj;
        }

        binn* walkCallElse0(CallElse0* ce0) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ce0->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ce0->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(ce0->second()));
            return obj;
        }

        binn* walkCallElse1(CallElse1* ce1) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ce1->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ce1->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(ce1->second()));
            binn_map_set_map(obj, BC_THIRD, _ref.walk(ce1->third()));
            return obj;
        }

        binn* walkPushCall0(PushCall0* pc0) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) pc0->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(pc0->first()));
            return obj;
        }

        binn* walkPushCall1(PushCall1* pc1) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) pc1->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(pc1->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(pc1->second()));
            return obj;
        }

        binn* walkPushCallIf0(PushCallIf0* pci0) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) pci0->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(pci0->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(pci0->second()));
            return obj;
        }

        binn* walkPushCallIf1(PushCallIf1* pci1) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) pci1->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(pci1->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(pci1->second()));
            binn_map_set_map(obj, BC_THIRD, _ref.walk(pci1->third()));
            return obj;
        }

        binn* walkPushCallElse0(PushCallElse0* pce0) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) pce0->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(pce0->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(pce0->second()));
            return obj;
        }

        binn* walkPushCallElse1(PushCallElse1* pce1) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) pce1->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(pce1->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(pce1->second()));
            binn_map_set_map(obj, BC_THIRD, _ref.walk(pce1->third()));
            return obj;
        }

        binn* walkDrain(Drain* drain) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG,  (size_t) drain->tag());
            return obj;
        }

        binn* walkExit(Exit* exit) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG,  (size_t) exit->tag());
            return obj;
        }

        binn* walkMapInit(MapInit* mi) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) mi->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(mi->first()));
            return obj;
        }

        binn* walkMapSet(MapSet* ms) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ms->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ms->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(ms->second()));
            binn_map_set_map(obj, BC_THIRD, _ref.walk(ms->third()));
            return obj;
        }

        binn* walkMapGet(MapGet* mg) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) mg->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(mg->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(mg->second()));
            return obj;
        }

        binn* walkMapLength(MapLength* ml) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ml->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ml->first()));
            return obj;
        }

        binn* walkMapKeys(MapKeys* mk) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) mk->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(mk->first()));
            return obj;
        }

        binn* walkTypify(Typify* t) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) t->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(t->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(t->second()));
            return obj;
        }

        binn* walkAssignValue(AssignValue* av) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) av->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(av->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(av->second()));
            return obj;
        }

        binn* walkAssignEval(AssignEval* ae) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ae->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ae->first()));
            binn_map_set_map(obj, BC_SECOND, walkOne(ae->second()));
            return obj;
        }

        binn* walkLock(Lock* l) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) l->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(l->first()));
            return obj;
        }

        binn* walkUnlock(Unlock* ul) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ul->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ul->first()));
            return obj;
        }

        binn* walkIsEqual(IsEqual* ie) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ie->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ie->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(ie->second()));
            return obj;
        }

        binn* walkScopeOf(ScopeOf* so) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) so->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(so->first()));
            return obj;
        }

        binn* walkStreamInit(StreamInit* si) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) si->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(si->first()));
            return obj;
        }

        binn* walkStreamPush(StreamPush* sp) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) sp->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(sp->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(sp->second()));
            return obj;
        }

        binn* walkStreamPop(StreamPop* sp) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) sp->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(sp->first()));
            return obj;
        }

        binn* walkStreamClose(StreamClose* sc) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) sc->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(sc->first()));
            return obj;
        }

        binn* walkStreamEmpty(StreamEmpty* se) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) se->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(se->first()));
            return obj;
        }

        binn* walkOut(Out* o) override {
            return walkStreamPush(o);
        }

        binn* walkErr(Err* e) override {
            return walkStreamPush(e);
        }

        binn* walkStringConcat(StringConcat* sc) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) sc->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(sc->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(sc->second()));
            return obj;
        }

        binn* walkStringLength(StringLength* sl) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) sl->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(sl->first()));
            return obj;
        }

        binn* walkStringSliceFrom(StringSliceFrom* ssf) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ssf->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ssf->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(ssf->second()));
            return obj;
        }

        binn* walkStringSliceFromTo(StringSliceFromTo* ssft) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ssft->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ssft->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(ssft->second()));
            binn_map_set_map(obj, BC_THIRD, _ref.walk(ssft->third()));
            return obj;
        }

        binn* walkTypeOf(TypeOf* to) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) to->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(to->first()));
            return obj;
        }

        binn* walkIsCompatible(IsCompatible* ic) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) ic->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(ic->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(ic->second()));
            return obj;
        }

        binn* walkPushExceptionHandler1(PushExceptionHandler1* peh) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) peh->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(peh->first()));
            return obj;
        }

        binn* walkPushExceptionHandler2(PushExceptionHandler2* peh) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) peh->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(peh->first()));
            binn_map_set_map(obj, BC_SECOND, _ref.walk(peh->second()));
            return obj;
        }

        binn* walkPopExceptionHandler(PopExceptionHandler* peh) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) peh->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(peh->first()));
            return obj;
        }

        binn* walkRaise(Raise* peh) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) peh->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(peh->first()));
            return obj;
        }

        binn* walkResume(Resume* peh) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (size_t) peh->tag());
            binn_map_set_map(obj, BC_FIRST, _ref.walk(peh->first()));
            return obj;
        }
    };

}

#endif //SWARMVM_ISABINARYWALK
