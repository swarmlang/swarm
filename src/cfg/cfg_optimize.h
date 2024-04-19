#ifndef SWARMC_CFG_OPTIMIZE
#define SWARMC_CFG_OPTIMIZE

#include "cfg.h"
#include "../errors/SwarmError.h"

namespace swarmc::CFG {

template <typename T>
class TieredMap {
public:
    TieredMap() : _map(new std::vector<std::unordered_map<std::string, T*>*>()) {}
    ~TieredMap() {
        for ( auto v : *_map ) delete v;
        delete _map; 
    }

    /* Gets the value from any tier, nullptr if it doesn't exist. */
    [[nodiscard]] T* get(std::string name) const {
        for ( auto i = _map->rbegin(); i != _map->rend(); i++ ) {
            if ( (*i)->count(name) == 0 ) continue;
            else return (*i)->at(name);
        }
        return nullptr;
    }

    /* Sets value in the lowest layer */
    void set(std::string name, T* value) {
        if ( _map->back()->count(name) == 0 ) {
            _map->back()->insert({ name, value });
        } else {
            _map->back()->at(name) = value;
        }
    }

    void clear() {
        for ( auto v : *_map ) delete v;
        _map->clear();
    }

    void popTier() {
        if ( !_map->empty() ) {
            delete _map->back();
            _map->pop_back();
        }
    }

    void pushTier(std::unordered_map<std::string, T*>* tier) {
        _map->push_back(tier);
    }
private:
    std::vector<std::unordered_map<std::string, T*>*>* _map;
};

class ConstantPropagation : public IUsesLogger {
public:
    static bool optimize(ControlFlowGraph* graph) {
        ConstantPropagation cp;
        return cp.execute(graph);
    }
private:
    ConstantPropagation() : IUsesLogger("Const. Prop."), _valueMap(new TieredMap<ISA::Reference>()), _instrMap(new TieredMap<ISA::Instruction>()) {
        logger->debug("Added scope");
        _valueMap->pushTier(new std::unordered_map<std::string, ISA::Reference*>());
        _instrMap->pushTier(new std::unordered_map<std::string, ISA::Instruction*>());
    }

    ~ConstantPropagation() override {
        delete _valueMap;
        delete _instrMap;
    }

    TieredMap<ISA::Reference>* _valueMap;
    TieredMap<ISA::Instruction>* _instrMap;

    bool propagate(ISA::Instruction* instr, bool doFirstBinary) {
        bool flag = false;
        // FIXME: upgrade assignvalues to assignevals using the instr map
        if ( instr->isUnary() ) {
            auto uinstr = (ISA::UnaryInstruction<ISA::Reference>*)instr;
            if ( uinstr->first()->tag() == ISA::ReferenceTag::LOCATION ) {
                std::string name = ((ISA::LocationReference*)uinstr->first())->fqName();
                auto v = _valueMap->get(name);
                if ( v != nullptr ) {
                    logger->debug("Replaced " + s(uinstr->first()) + " with " + s(v) + " in " + s(uinstr));
                    uinstr->setFirst(v);
                    flag = true;
                }
            }
        } else if ( instr->isBinary() ) {
            auto binstr = (ISA::BinaryInstruction<ISA::Reference,ISA::Reference>*)instr;
            if ( binstr->first()->tag() == ISA::ReferenceTag::LOCATION && doFirstBinary ) {
                std::string name = ((ISA::LocationReference*)binstr->first())->fqName();
                auto v = _valueMap->get(name);
                if ( v != nullptr ) {
                    logger->debug("Replaced " + s(binstr->first()) + " with " + s(v) + " in " + s(binstr));
                    binstr->setFirst(v);
                    flag = true;
                }
            }
            if ( binstr->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                std::string name = ((ISA::LocationReference*)binstr->second())->fqName();
                auto v = _valueMap->get(name);
                if ( v != nullptr ) {
                    logger->debug("Replaced " + s(binstr->second()) + " with " + s(v) + " in " + s(binstr));
                    binstr->setSecond(v);
                    flag = true;
                }
            }
        } else if ( instr->isTrinary() ) {
            auto tinstr = (ISA::TrinaryInstruction<ISA::Reference,ISA::Reference,ISA::Reference>*)instr;
            if ( tinstr->first()->tag() == ISA::ReferenceTag::LOCATION ) {
                std::string name = ((ISA::LocationReference*)tinstr->first())->fqName();
                auto v = _valueMap->get(name);
                if ( v != nullptr ) {
                    logger->debug("Replaced " + s(tinstr->first()) + " with " + s(v) + " in " + s(tinstr));
                    tinstr->setFirst(v);
                    flag = true;
                }
            }
            if ( tinstr->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                std::string name = ((ISA::LocationReference*)tinstr->second())->fqName();
                auto v = _valueMap->get(name);
                if ( v != nullptr ) {
                    logger->debug("Replaced " + s(tinstr->second()) + " with " + s(v) + " in " + s(tinstr));
                    tinstr->setSecond(v);
                    flag = true;
                }
            }
            if ( tinstr->third()->tag() == ISA::ReferenceTag::LOCATION ) {
                std::string name = ((ISA::LocationReference*)tinstr->third())->fqName();
                auto v = _valueMap->get(name);
                if ( v != nullptr ) {
                    logger->debug("Replaced " + s(tinstr->third()) + " with " + s(v) + " in " + s(tinstr));
                    tinstr->setThird(v);
                    flag = true;
                }
            }
        }

        return flag;
    }

    void reset() {
        logger->debug("Reset");
        _valueMap->clear();
        _instrMap->clear();
        _valueMap->pushTier(new std::unordered_map<std::string, ISA::Reference*>());
        _instrMap->pushTier(new std::unordered_map<std::string, ISA::Instruction*>());
    }

    bool execute(ControlFlowGraph* graph) {
        bool flag = false;

        for ( const auto& cfgf : *graph->getNameMap() ) {
            flag = blockPropagate(cfgf.second->start(), 0) || flag;
            reset();
        }

        flag = blockPropagate(graph->first(), 0) || flag;
        reset();
        return flag;
    }

    bool blockPropagate(Block* block, size_t fDepth) {
        bool flag = false;

        logger->debug("Starting " + s(block));

        if ( fDepth == 0 ) {
            for (auto instr : *block->instructions()) {
                if ( instr->tag() == ISA::Tag::ASSIGNVALUE ) {
                    auto a = (ISA::AssignValue*)instr;
                    std::string name = a->first()->fqName();
                    // check for self-assigns because they cause an infinite loop
                    if ( a->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                        if (a->first()->is((ISA::LocationReference*)a->second())) {
                            continue;
                        }
                    }
                    flag = propagate(instr, false) || flag;
                    if ( a->first()->affinity() != ISA::Affinity::SHARED ) {
                        _valueMap->set(name, ((ISA::AssignValue*)instr)->second());
                        logger->debug("Added " + name + " <- " + ((ISA::AssignValue*)instr)->second()->toString() + " to ValueMap");
                    }
                    continue;
                } else if ( instr->tag() == ISA::Tag::ASSIGNEVAL ) {
                    // FIXME: dont add impure calls
                    auto a = (ISA::AssignEval*)instr;
                    std::string name = a->first()->fqName();
                    if ( a->first()->affinity() != ISA::Affinity::SHARED ) {
                        _instrMap->set(name, ((ISA::AssignEval*)instr)->second());
                        logger->debug("Added " + name + " <- " + a->second()->toString() + " to InstrMap");
                    }
                    instr = a->second();
                } else if ( instr->tag() == ISA::Tag::FNPARAM 
                        || instr->tag() == ISA::Tag::SCOPEOF 
                        || instr->tag() == ISA::Tag::TYPIFY
                        || instr->tag() == ISA::Tag::LOCK 
                        || instr->tag() == ISA::Tag::UNLOCK )
                {
                    continue;
                }

                flag = propagate(instr, true) || flag;
                // TODO: if we modify an enum/map/obj, we can no longer propagate `enuminit`-esque
                // instructions, so we should remove them from the instruction map
            }
        }
    
        if ( block->getFallOutEdge() != nullptr ) {
            // FIXME: change for existence of pure functions, reset not needed
            reset();
            flag = blockPropagate(block->getFallOutEdge()->destination(), fDepth) || flag;
        } else if ( block->getCallOutEdge() != nullptr ) {
            _valueMap->pushTier(new std::unordered_map<std::string, ISA::Reference*>());
            _instrMap->pushTier(new std::unordered_map<std::string, ISA::Instruction*>());
            flag = blockPropagate(block->getCallOutEdge()->destination(), fDepth + 1) || flag;
        } else if ( block->getRetOutEdge() != nullptr ) {
            reset();
            flag = blockPropagate(block->getRetOutEdge()->destination(), fDepth - 1) || flag;
        }

        return flag;
    }
};

class RemoveSelfAssign : public IUsesLogger {
public:
    static bool optimize(ControlFlowGraph* graph) {
        RemoveSelfAssign rsa;
        bool flag = false;

        for (auto b : *graph->blocks()) rsa.execute(b);

        return flag;
    }
private:
    RemoveSelfAssign() : IUsesLogger("Remove Self-Assigns") {}

    bool execute(Block* block) {
        bool flag = false;
        logger->debug("Starting " + block->toString());

        for ( size_t j = 0; j < block->instructions()->size(); j++ ) {
            if ( block->instructions()->at(j)->tag() == ISA::Tag::ASSIGNVALUE ) {
                auto instr = ((ISA::AssignValue*)block->instructions()->at(j));
                if ( instr->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                    if ( instr->first()->name() == ((ISA::LocationReference*)instr->second())->name() ) {
                        logger->debug("Removed " + instr->toString());
                        freeref(block->instructions()->at(j));
                        block->instructions()->erase(block->instructions()->begin() + (long)j);
                        j--;
                        flag = true;
                    }
                }
            }
        }

        return flag;
    }
};

}

#endif