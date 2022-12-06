#ifndef SWARMC_CFG_OPTIMIZE
#define SWARMC_CFG_OPTIMIZE

#include "cfg.h"

namespace swarmc::CFG {

template <typename T>
class TieredMap : public IUsesConsole {
public:
    TieredMap() : _map(new std::vector<std::unordered_map<std::string, T*>*>()) {}
    ~TieredMap() override {
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

class ConstantPropagation : public IUsesConsole {
public:
    static bool optimize(ControlFlowGraph* graph) {
        ConstantPropagation cp;
        Console::get()->debug("Starting Constant Propagation");
        return cp.execute(graph);
    }
private:
    ConstantPropagation() : _valueMap(new TieredMap<ISA::Reference>()), _instrMap(new TieredMap<ISA::Instruction>()) {
        console->debug("CP: Added scope");
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
        std::string s = instr->toString();
        if ( instr->isUnary() ) {
            auto uinstr = (ISA::UnaryInstruction<ISA::Reference>*)instr;
            if ( uinstr->first()->tag() == ISA::ReferenceTag::LOCATION ) {
                std::string name = ((ISA::LocationReference*)uinstr->first())->fqName();
                auto v = _valueMap->get(name);
                if ( v != nullptr ) {
                    //delete uinstr->first();
                    uinstr->setFirst(v->copy());
                    flag = true;
                    console->debug("CP: " + name + " found (" + v->toString() + ")!");
                } else {
                    console->debug("CP: " + name + " not found!");
                }
            }
        } else if ( instr->isBinary() ) {
            auto binstr = (ISA::BinaryInstruction<ISA::Reference,ISA::Reference>*)instr;
            if ( binstr->first()->tag() == ISA::ReferenceTag::LOCATION && doFirstBinary ) {
                std::string name = ((ISA::LocationReference*)binstr->first())->fqName();
                auto v = _valueMap->get(name);
                if ( v != nullptr ) {
                    //delete binstr->first();
                    binstr->setFirst(v->copy());
                    flag = true;
                    console->debug("CP: " + name + " found (" + v->toString() + ")!");
                } else {
                    console->debug("CP: " + name + " not found!");
                }
            }
            if ( binstr->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                std::string name = ((ISA::LocationReference*)binstr->second())->fqName();
                auto v = _valueMap->get(name);
                if ( v != nullptr ) {
                    //delete binstr->second();
                    binstr->setSecond(v->copy());
                    flag = true;
                    console->debug("CP: " + name + " found (" + v->toString() + ")!");
                } else {
                    console->debug("CP: " + name + " not found!");
                }
            }
        } else if ( instr->isTrinary() ) {
            auto tinstr = (ISA::TrinaryInstruction<ISA::Reference,ISA::Reference,ISA::Reference>*)instr;
            if ( tinstr->first()->tag() == ISA::ReferenceTag::LOCATION ) {
                std::string name = ((ISA::LocationReference*)tinstr->first())->fqName();
                auto v = _valueMap->get(name);
                if ( v != nullptr ) {
                    //delete tinstr->first();
                    tinstr->setFirst(v->copy());
                    flag = true;
                    console->debug("CP: " + name + " found (" + v->toString() + ")!");
                } else {
                    console->debug("CP: " + name + " not found!");
                }
            }
            if ( tinstr->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                std::string name = ((ISA::LocationReference*)tinstr->second())->fqName();
                auto v = _valueMap->get(name);
                if ( v != nullptr ) {
                    //delete tinstr->second();
                    tinstr->setSecond(v->copy());
                    flag = true;
                    console->debug("CP: " + name + " found (" + v->toString() + ")!");
                } else {
                    console->debug("CP: " + name + " not found!");
                }
            }
            if ( tinstr->third()->tag() == ISA::ReferenceTag::LOCATION ) {
                std::string name = ((ISA::LocationReference*)tinstr->third())->fqName();
                auto v = _valueMap->get(name);
                if ( v != nullptr ) {
                    //delete tinstr->third();
                    tinstr->setThird(v->copy());
                    flag = true;
                    console->debug("CP: " + name + " found (" + v->toString() + ")!");
                } else {
                    console->debug("CP: " + name + " not found!");
                }
            }
        }
        
        if (flag) console->debug(s + "\n\t->\n\t\t" + instr->toString());
        return flag;
    }

    void reset() {
        console->debug("CP: Reset");
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

        console->debug("CP: " + block->toString());

        if ( fDepth == 0 ) {
            for (auto instr : *block->instructions()) {
                bool p = true;
                if ( instr->tag() == ISA::Tag::ASSIGNVALUE ) {
                    std::string name = ((ISA::AssignValue*)instr)->first()->fqName();
                    // check for self-assigns because they cause an infinite loop
                    if ( ((ISA::AssignValue*)instr)->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                        // FIXME: Add propagation of instructions a la upgrading AssignValue to AssignEval
                        // note: whichever one was assigned most recent is the one that gets propagated
                        if (name != ((ISA::LocationReference*)((ISA::AssignValue*)instr)->second())->fqName()) {
                            flag = propagate(instr, false) || flag;
                            _valueMap->set(name, ((ISA::AssignValue*)instr)->second());
                            console->debug("CP: Added " + name + " = " + ((ISA::AssignValue*)instr)->second()->toString() + " to ValueMap");
                        }
                    }
                    p = false;
                } else if ( instr->tag() == ISA::Tag::ASSIGNEVAL ) {
                    std::string name = ((ISA::AssignEval*)instr)->first()->fqName();
                    _instrMap->set(name, ((ISA::AssignEval*)instr)->second());
                    console->debug("CP: Added " + name + " = " + ((ISA::AssignEval*)instr)->second()->toString() + " to InstrMap");
                    instr = ((ISA::AssignEval*)instr)->second();
                } else if ( instr->tag() == ISA::Tag::FNPARAM || instr->tag() == ISA::Tag::SCOPEOF || instr->tag() == ISA::Tag::TYPIFY ) {
                    p = false;
                }

                if ( p ) {
                    flag = propagate(instr, true) || flag;
                }
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

class RemoveSelfAssign : public IUsesConsole {
public:
    static bool optimize(ControlFlowGraph* graph) {
        RemoveSelfAssign rsa;
        Console::get()->debug("Starting Removal of Self-Assigns");
        return rsa.execute(graph->first());
    }
private:
    bool execute(Block* block) {
        bool flag = false;
        console->debug("RSA: block " + block->toString());

        for ( size_t j = 0; j < block->instructions()->size(); j++ ) {
            if ( block->instructions()->at(j)->tag() == ISA::Tag::ASSIGNVALUE ) {
                auto instr = ((ISA::AssignValue*)block->instructions()->at(j));
                if ( instr->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                    if ( instr->first()->name() == ((ISA::LocationReference*)instr->second())->name() ) {
                        console->debug("RSA: removed " + instr->toString());
                        delete block->instructions()->at(j);
                        block->instructions()->erase(block->instructions()->begin() + (long)j);
                        j--;
                        flag = true;
                    }
                }
            }
        }

        if ( block->getCallOutEdge() != nullptr ) {
            flag = execute(block->getCallOutEdge()->destination()) || flag;
        } else if ( block->getRetOutEdge() != nullptr ) {
            flag = execute(block->getRetOutEdge()->destination()) || flag;
        }

        return flag;
    }
};
}

#endif