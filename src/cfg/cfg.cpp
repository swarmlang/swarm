#include "cfg.h"

namespace swarmc {
namespace CFG {

ControlFlowGraph::ControlFlowGraph(ISA::Instructions* instrs) {
    // get mappings of instruction order for ease 
    auto cfg = CFGBuild(instrs);
    _instrs = cfg._instrs;
    _blocks = cfg._blocks;
    removeSelfAssigns();
    intraBlockLiteralPropagation();
    _edges = cfg._edges;
}

bool ControlFlowGraph::removeSelfAssigns() {
    bool flag = false;

    for ( size_t i = 0; i < _blocks->size(); i++ ) {
        auto instructions = _blocks->at(i)->instructions();
        for ( size_t j = 0; j < instructions->size(); j++ ) {
            if ( instructions->at(j)->tag() == ISA::Tag::ASSIGNVALUE ) {
                auto instr = ((ISA::AssignValue*)instructions->at(j));
                if ( instr->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                    if ( instr->first()->name() == ((ISA::LocationReference*)instr->second())->name() ) {
                        // delete instructions->at(j);
                        instructions->erase(instructions->begin() + j);
                        j--;
                        flag = true;
                    }
                }
            }
        }
    }

    return flag;
}

bool ControlFlowGraph::intraBlockLiteralPropagation() {
    bool flag = false;

    // for literal propagation
    std::unordered_map<std::string,ISA::Reference*> valueMap;
    // for upgrading AssignValue to AssignEval
    std::unordered_map<std::string,ISA::Instruction*> instrMap;

    for ( size_t i = 0; i < _blocks->size(); i++ ) {
        auto instructions = _blocks->at(i)->instructions();
        for ( size_t j = 0; j < instructions->size(); j++ ) {
            bool propagate = true;
            auto instr = instructions->at(j);

            if ( instr->tag() == ISA::Tag::ASSIGNVALUE ) {
                propagate = false;
                auto instrAV = (ISA::AssignValue*)instr;

                // attempt to replace rhs with a value or instruction
                if ( instrAV->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                    std::string name = ((ISA::LocationReference*)instrAV->second())->name();
                    if ( valueMap.count(name) != 0 ) {
                        // delete instrAV->second();
                        instrAV->setSecond(valueMap.at(name));
                    } else if ( instrMap.count(name) != 0) {
                        // propagate instruction
                        auto instrAE = new ISA::AssignEval(instrAV->first(), instrMap.at(name));
                        //delete instrAV;
                        instructions->at(j) = instrAE;

                        // add instruction to map
                        if ( instrMap.count(instrAE->first()->name()) == 0 ) {
                            instrMap.insert({instrAE->first()->name(), instrAE->second()});
                        } else {
                            instrMap.at(instrAE->first()->name()) = instrAE->second();
                        }
                    }
                }

                // add value to map if instruction has not been upgraded to AssignEval
                if ( instr->tag() == ISA::Tag::ASSIGNVALUE ) {
                    if ( valueMap.count(instrAV->first()->name()) == 0 ) {
                        valueMap.insert({instrAV->first()->name(), instrAV->second()});
                    } else {
                        valueMap.at(instrAV->first()->name()) = instrAV->second();
                    }
                }
            } else if ( instr->tag() == ISA::Tag::ASSIGNEVAL ) {
                // add instruction to map
                auto instrAE = (ISA::AssignEval*)instr;
                if ( instrMap.count(instrAE->first()->name()) == 0 ) {
                    instrMap.insert({instrAE->first()->name(), instrAE->second()});
                } else {
                    instrMap.at(instrAE->first()->name()) = instrAE->second();
                }
                // propagate rhs of assign
                instr = instrAE->second();
            }

            // propagate to all instructions except assignvalue
            if ( propagate ) {
                if ( instr->isUnary() ) {
                    auto uinstr = (ISA::UnaryInstruction<ISA::Reference>*)instr;
                    if ( uinstr->first()->tag() == ISA::ReferenceTag::LOCATION ) {
                        std::string name = ((ISA::LocationReference*)uinstr->first())->name();
                        if ( valueMap.count(name) != 0 ) {
                            //delete uinstr->first();
                            uinstr->setFirst(valueMap.at(name));
                            flag = true;
                        }
                    }
                } else if ( instr->isBinary() ) {
                    auto binstr = (ISA::BinaryInstruction<ISA::Reference,ISA::Reference>*)instr;
                    if ( binstr->first()->tag() == ISA::ReferenceTag::LOCATION ) {
                        std::string name = ((ISA::LocationReference*)binstr->first())->name();
                        if ( valueMap.count(name) != 0 ) {
                            //delete binstr->first();
                            binstr->setFirst(valueMap.at(name));
                            flag = true;
                        }
                    }
                    if ( binstr->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                        std::string name = ((ISA::LocationReference*)binstr->second())->name();
                        if ( valueMap.count(name) != 0 ) {
                            //delete binstr->second();
                            binstr->setSecond(valueMap.at(name));
                            flag = true;
                        }
                    }
                } else if ( instr->isTrinary() ) {
                    auto tinstr = (ISA::TrinaryInstruction<ISA::Reference,ISA::Reference,ISA::Reference>*)instr;
                    if ( tinstr->first()->tag() == ISA::ReferenceTag::LOCATION ) {
                        std::string name = ((ISA::LocationReference*)tinstr->first())->name();
                        if ( valueMap.count(name) != 0 ) {
                            //delete tinstr->first();
                            tinstr->setFirst(valueMap.at(name));
                            flag = true;
                        }
                    }
                    if ( tinstr->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                        std::string name = ((ISA::LocationReference*)tinstr->second())->name();
                        if ( valueMap.count(name) != 0 ) {
                            //delete tinstr->second();
                            tinstr->setSecond(valueMap.at(name));
                            flag = true;
                        }
                    }
                    if ( tinstr->third()->tag() == ISA::ReferenceTag::LOCATION ) {
                        std::string name = ((ISA::LocationReference*)tinstr->third())->name();
                        if ( valueMap.count(name) != 0 ) {
                            //delete tinstr->second();
                            tinstr->setThird(valueMap.at(name));
                            flag = true;
                        }
                    }
                }
            }
        }
        valueMap.clear();
    }

    return flag;
}

void ControlFlowGraph::serialize(std::ostream& out) const {
    out << "digraph cfg {\n";
    for ( auto b : *_blocks ) {
        std::string label = b->id() + "\\n";
        for ( auto i : *b->instructions() ) {
            label += i->toString() + "\\n";
        }
        out << "\t\"" << b->id() << "\" [shape=rectangle,label=\"" << label << "\"]\n";
    }
    for ( auto e : *_edges ) {
        out << "\t\"" << e->source()->id() << "\"->\"" << e->destination()->id() 
            << "\" [label=\"" << e->label() << "\"]\n";
    }
    out << "}\n";
}


}
}