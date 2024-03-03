#ifndef SWARMVM_ISABINARYWALK
#define SWARMVM_ISABINARYWALK

#include "../../../mod/binn/src/binn.h"
#include "../isa_meta.h"
#include "../ISAWalk.h"
#include "../Wire.h"
#include "binary_const.h"

// FIXME: I'm sure this leaks memory out the ass.

namespace swarmc::Runtime {
    class VirtualMachine;
}

using namespace swarmc::Runtime;

namespace swarmc::ISA {

    class ISABinaryWalk : public ISAWalk<binn*> {
    public:
        explicit ISABinaryWalk(VirtualMachine* vm) : _vm(vm) {}

        [[nodiscard]] std::string toString() const override {
            return "ISABinaryWalk<>";
        }

        static binn* serialize(const Instructions& is, Runtime::VirtualMachine* vm) {
            ISABinaryWalk walk(vm);
            auto list = binn_list();
            for ( auto i : walk.walk(is) ) {
                binn_list_add_map(list, i);
            }

            auto obj = binn_map();
            binn_map_set_list(obj, BC_BODY, list);
            return obj;
        }

    protected:
        VirtualMachine* _vm;

        binn* walkPosition(PositionAnnotation* position) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) position->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(position->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(position->second(), _vm));
            binn_map_set_map(obj, BC_THIRD, Wire::references()->reduce(position->third(), _vm));
            return obj;
        }

        binn* walkPlus(Plus* plus) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) plus->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(plus->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(plus->second(), _vm));
            return obj;
        }

        binn* walkMinus(Minus* minus) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) minus->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(minus->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(minus->second(), _vm));
            return obj;
        }

        binn* walkTimes(Times* times) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) times->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(times->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(times->second(), _vm));
            return obj;
        }

        binn* walkDivide(Divide* divide) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) divide->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(divide->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(divide->second(), _vm));
            return obj;
        }

        binn* walkPower(Power* power) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) power->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(power->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(power->second(), _vm));
            return obj;
        }

        binn* walkMod(Mod* mod) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) mod->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(mod->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(mod->second(), _vm));
            return obj;
        }

        binn* walkNegative(Negative* neg) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) neg->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(neg->first(), _vm));
            return obj;
        }

        binn* walkGreaterThan(GreaterThan* gt) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) gt->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(gt->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(gt->second(), _vm));
            return obj;
        }

        binn* walkGreaterThanOrEqual(GreaterThanOrEqual* gte) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) gte->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(gte->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(gte->second(), _vm));
            return obj;
        }

        binn* walkLessThan(LessThan* lt) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) lt->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(lt->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(lt->second(), _vm));
            return obj;
        }

        binn* walkLessThanOrEqual(LessThanOrEqual* lte) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) lte->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(lte->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(lte->second(), _vm));
            return obj;
        }

        binn* walkAnd(And* a) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) a->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(a->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(a->second(), _vm));
            return obj;
        }

        binn* walkOr(Or* o) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) o->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(o->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(o->second(), _vm));
            return obj;
        }

        binn* walkXor(Xor* x) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) x->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(x->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(x->second(), _vm));
            return obj;
        }

        binn* walkNand(Nand* n) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) n->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(n->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(n->second(), _vm));
            return obj;
        }

        binn* walkNor(Nor* n) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) n->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(n->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(n->second(), _vm));
            return obj;
        }

        binn* walkNot(Not* n) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) n->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(n->first(), _vm));
            return obj;
        }

        binn* walkWhile(While* w) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) w->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(w->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(w->second(), _vm));
            return obj;
        }

        binn* walkWith(With* w) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) w->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(w->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(w->second(), _vm));
            return obj;
        }

        binn* walkEnumInit(EnumInit* ei) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ei->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ei->first(), _vm));
            return obj;
        }

        binn* walkEnumAppend(EnumAppend* ea) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ea->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ea->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(ea->second(), _vm));
            return obj;
        }

        binn* walkEnumPrepend(EnumPrepend* ep) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ep->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ep->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(ep->second(), _vm));
            return obj;
        }

        binn* walkEnumLength(EnumLength* el) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) el->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(el->first(), _vm));
            return obj;
        }

        binn* walkEnumGet(EnumGet* eg) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) eg->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(eg->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(eg->second(), _vm));
            return obj;
        }

        binn* walkEnumSet(EnumSet* es) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) es->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(es->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(es->second(), _vm));
            binn_map_set_map(obj, BC_THIRD, Wire::references()->reduce(es->third(), _vm));
            return obj;
        }

        binn* walkEnumerate(Enumerate* e) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) e->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(e->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(e->second(), _vm));
            binn_map_set_map(obj, BC_THIRD, Wire::references()->reduce(e->third(), _vm));
            return obj;
        }

        binn* walkBeginFunction(BeginFunction* bf) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) bf->tag());
            binn_map_set_str(obj, BC_NAME, strdup(bf->first()->name().c_str()));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(bf->second(), _vm));
            binn_map_set_bool(obj, BC_ISPURE, bf->isPure());
            return obj;
        }

        binn* walkFunctionParam(FunctionParam* fp) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) fp->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(fp->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(fp->second(), _vm));
            return obj;
        }

        binn* walkReturn0(Return0* r0) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) r0->tag());
            return obj;
        }

        binn* walkReturn1(Return1* r1) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) r1->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(r1->first(), _vm));
            return obj;
        }

        binn* walkCurry(Curry* c) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) c->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(c->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(c->second(), _vm));
            return obj;
        }

        binn* walkCall0(Call0* c0) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) c0->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(c0->first(), _vm));
            return obj;
        }

        binn* walkCall1(Call1* c1) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) c1->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(c1->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(c1->second(), _vm));
            return obj;
        }

        binn* walkCallIf0(CallIf0* ci0) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ci0->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ci0->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(ci0->second(), _vm));
            return obj;
        }

        binn* walkCallIf1(CallIf1* ci1) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ci1->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ci1->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(ci1->second(), _vm));
            binn_map_set_map(obj, BC_THIRD, Wire::references()->reduce(ci1->third(), _vm));
            return obj;
        }

        binn* walkCallElse0(CallElse0* ce0) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ce0->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ce0->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(ce0->second(), _vm));
            return obj;
        }

        binn* walkCallElse1(CallElse1* ce1) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ce1->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ce1->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(ce1->second(), _vm));
            binn_map_set_map(obj, BC_THIRD, Wire::references()->reduce(ce1->third(), _vm));
            return obj;
        }

        binn* walkPushCall0(PushCall0* pc0) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) pc0->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(pc0->first(), _vm));
            return obj;
        }

        binn* walkPushCall1(PushCall1* pc1) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) pc1->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(pc1->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(pc1->second(), _vm));
            return obj;
        }

        binn* walkPushCallIf0(PushCallIf0* pci0) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) pci0->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(pci0->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(pci0->second(), _vm));
            return obj;
        }

        binn* walkPushCallIf1(PushCallIf1* pci1) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) pci1->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(pci1->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(pci1->second(), _vm));
            binn_map_set_map(obj, BC_THIRD, Wire::references()->reduce(pci1->third(), _vm));
            return obj;
        }

        binn* walkPushCallElse0(PushCallElse0* pce0) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) pce0->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(pce0->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(pce0->second(), _vm));
            return obj;
        }

        binn* walkPushCallElse1(PushCallElse1* pce1) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) pce1->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(pce1->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(pce1->second(), _vm));
            binn_map_set_map(obj, BC_THIRD, Wire::references()->reduce(pce1->third(), _vm));
            return obj;
        }

        binn* walkDrain(Drain* drain) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG,  (std::size_t) drain->tag());
            return obj;
        }

        binn* walkRetMapHas(RetMapHas* retMapHas) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) retMapHas->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(retMapHas->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(retMapHas->second(), _vm));
            return obj;
        }

        binn* walkRetMapGet(RetMapGet* retMapGet) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) retMapGet->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(retMapGet->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(retMapGet->second(), _vm));
            return obj;
        }

        binn* walkEnterContext(EnterContext* ec) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ec->tag());
            return obj;
        }

        binn* walkResumeContext(ResumeContext* rc) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) rc->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(rc->first(), _vm));
            return obj;
        }

        binn* walkPopContext(PopContext* pc) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) pc->tag());
            return obj;
        }

        binn* walkExit(Exit* exit) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG,  (std::size_t) exit->tag());
            return obj;
        }

        binn* walkMapInit(MapInit* mi) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) mi->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(mi->first(), _vm));
            return obj;
        }

        binn* walkMapSet(MapSet* ms) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ms->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ms->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(ms->second(), _vm));
            binn_map_set_map(obj, BC_THIRD, Wire::references()->reduce(ms->third(), _vm));
            return obj;
        }

        binn* walkMapGet(MapGet* mg) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) mg->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(mg->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(mg->second(), _vm));
            return obj;
        }

        binn* walkMapLength(MapLength* ml) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ml->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ml->first(), _vm));
            return obj;
        }

        binn* walkMapKeys(MapKeys* mk) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) mk->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(mk->first(), _vm));
            return obj;
        }

        binn* walkTypify(Typify* t) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) t->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(t->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(t->second(), _vm));
            return obj;
        }

        binn* walkAssignValue(AssignValue* av) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) av->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(av->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(av->second(), _vm));
            return obj;
        }

        binn* walkAssignEval(AssignEval* ae) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ae->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ae->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, walkOne(ae->second()));
            return obj;
        }

        binn* walkLock(Lock* l) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) l->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(l->first(), _vm));
            return obj;
        }

        binn* walkUnlock(Unlock* ul) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ul->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ul->first(), _vm));
            return obj;
        }

        binn* walkIsEqual(IsEqual* ie) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ie->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ie->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(ie->second(), _vm));
            return obj;
        }

        binn* walkScopeOf(ScopeOf* so) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) so->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(so->first(), _vm));
            return obj;
        }

        binn* walkStreamInit(StreamInit* si) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) si->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(si->first(), _vm));
            return obj;
        }

        binn* walkStreamPush(StreamPush* sp) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) sp->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(sp->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(sp->second(), _vm));
            return obj;
        }

        binn* walkStreamPop(StreamPop* sp) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) sp->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(sp->first(), _vm));
            return obj;
        }

        binn* walkStreamClose(StreamClose* sc) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) sc->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(sc->first(), _vm));
            return obj;
        }

        binn* walkStreamEmpty(StreamEmpty* se) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) se->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(se->first(), _vm));
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
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) sc->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(sc->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(sc->second(), _vm));
            return obj;
        }

        binn* walkStringLength(StringLength* sl) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) sl->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(sl->first(), _vm));
            return obj;
        }

        binn* walkStringSliceFrom(StringSliceFrom* ssf) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ssf->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ssf->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(ssf->second(), _vm));
            return obj;
        }

        binn* walkStringSliceFromTo(StringSliceFromTo* ssft) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ssft->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ssft->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(ssft->second(), _vm));
            binn_map_set_map(obj, BC_THIRD, Wire::references()->reduce(ssft->third(), _vm));
            return obj;
        }

        binn* walkTypeOf(TypeOf* to) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) to->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(to->first(), _vm));
            return obj;
        }

        binn* walkIsCompatible(IsCompatible* ic) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ic->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ic->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(ic->second(), _vm));
            return obj;
        }

        binn* walkPushExceptionHandler1(PushExceptionHandler1* peh) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) peh->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(peh->first(), _vm));
            return obj;
        }

        binn* walkPushExceptionHandler2(PushExceptionHandler2* peh) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) peh->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(peh->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(peh->second(), _vm));
            return obj;
        }

        binn* walkPopExceptionHandler(PopExceptionHandler* peh) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) peh->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(peh->first(), _vm));
            return obj;
        }

        binn* walkRaise(Raise* peh) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) peh->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(peh->first(), _vm));
            return obj;
        }

        binn* walkResume(Resume* peh) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) peh->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(peh->first(), _vm));
            return obj;
        }

        binn* walkOTypeInit(OTypeInit* oti) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) oti->tag());
            return obj;
        }

        binn* walkOTypeProp(OTypeProp* otp) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) otp->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(otp->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(otp->second(), _vm));
            binn_map_set_map(obj, BC_THIRD, Wire::references()->reduce(otp->third(), _vm));
            return obj;
        }

        binn* walkOTypeDel(OTypeDel* otd) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) otd->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(otd->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(otd->second(), _vm));
            return obj;
        }

        binn* walkOTypeGet(OTypeGet* otg) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) otg->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(otg->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(otg->second(), _vm));
            return obj;
        }

        binn* walkOTypeFinalize(OTypeFinalize* otf) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) otf->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(otf->first(), _vm));
            return obj;
        }

        binn* walkOTypeSubset(OTypeSubset* ots) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) ots->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(ots->first(), _vm));
            return obj;
        }

        binn* walkObjInit(ObjInit* oi) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) oi->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(oi->first(), _vm));
            return obj;
        }

        binn* walkObjSet(ObjSet* os) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) os->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(os->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(os->second(), _vm));
            binn_map_set_map(obj, BC_THIRD, Wire::references()->reduce(os->third(), _vm));
            return obj;
        }

        binn* walkObjGet(ObjGet* og) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) og->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(og->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(og->second(), _vm));
            return obj;
        }

        binn* walkObjInstance(ObjInstance* oi) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) oi->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(oi->first(), _vm));
            return obj;
        }

        binn* walkObjCurry(ObjCurry* oc) override {
            auto obj = binn_map();
            binn_map_set_uint64(obj, BC_TAG, (std::size_t) oc->tag());
            binn_map_set_map(obj, BC_FIRST, Wire::references()->reduce(oc->first(), _vm));
            binn_map_set_map(obj, BC_SECOND, Wire::references()->reduce(oc->second(), _vm));
            return obj;
        }
    };

}

#endif //SWARMVM_ISABINARYWALK
