#include "cfg.h"

namespace swarmc {
namespace CFG {

Block::~Block() {
    delete _instructions;
    delete _callOutEdge;
    delete _fallOutEdge;
    delete _retOutEdge;
}

CallEdge* Block::getCallInEdge() const { return _callInEdge; }
CallEdge* Block::getCallOutEdge() const { return _callOutEdge; }
FallEdge* Block::getFallInEdge() const { return _fallInEdge; }
FallEdge* Block::getFallOutEdge() const { return _fallOutEdge; }
ReturnEdge* Block::getRetInEdge() const { return _retInEdge; }
ReturnEdge* Block::getRetOutEdge() const { return _retOutEdge; }

void Block::setCallInEdge(CallEdge* edge) { _callInEdge = edge; }
void Block::setCallOutEdge(CallEdge* edge) { _callOutEdge = edge; }
void Block::setFallInEdge(FallEdge* edge) { _fallInEdge = edge; }
void Block::setFallOutEdge(FallEdge* edge) { _fallOutEdge = edge; }
void Block::setRetInEdge(ReturnEdge* edge) { _retInEdge = edge; }
void Block::setRetOutEdge(ReturnEdge* edge) { _retOutEdge = edge; }

std::string Block::serialize() const {
    std::string c = std::to_string(_copy), idx = std::to_string(_idx);
    std::string out = "\t\"" + id() + ":" + c + ":" + idx 
        + "\" [shape=rectangle,label=\"" + id() + ":" + c + ":" + idx + "\\n";
    for ( auto i : *instructions() ) {
        out += i->toString() + "\\n";
    }
    out += "\"]\n";
    if ( _callOutEdge != nullptr ) out += _callOutEdge->serialize();
    if ( _fallOutEdge != nullptr ) out += _fallOutEdge->serialize();
    if ( _retOutEdge != nullptr ) out += _retOutEdge->serialize();
    return out;
}

std::string AmbiguousFunctionBlock::serialize() const {
    std::string c = std::to_string(_copy), i = std::to_string(_idx);
    std::string out = "\t\"" + id() + ":" + c + ":" + i 
        + "\" [shape=rectangle,label=\"AmbiguousFunction:" + id() + ":" + c + ":" + i + "\"]\n";
    if ( _retOutEdge != nullptr ) out += _retOutEdge->serialize();
    return out;
}

std::pair<Block*,Block*> CFGFunction::makeCopy(size_t i, std::vector<Block*>* blocks, std::stack<CFGFunction*>* callStack) const {
    std::vector<Block*> newblocks;
    std::unordered_map<Block*, Block*> copyOf;
    
    for ( auto b : *_blocks ) {
        Block* copy = b->copy(i);
        newblocks.push_back(copy);
        if (!callStack->empty()) callStack->top()->addBlock(copy);
        // create maps for copying edges
        copyOf.insert({ b, copy });
    }

    for ( auto b : *_blocks ) {
        if ( b->getCallOutEdge() != nullptr && b != _end ) {
            auto edge = new CallEdge(copyOf.at(b), copyOf.at(b->getCallOutEdge()->destination()));
            copyOf.at(b)->setCallOutEdge(edge);
            copyOf.at(b->getCallOutEdge()->destination())->setCallInEdge(edge);
        }
        if ( b->getFallOutEdge() != nullptr ) {
            auto edge = new FallEdge(copyOf.at(b), copyOf.at(b->getFallOutEdge()->destination()));
            copyOf.at(b)->setFallOutEdge(edge);
            copyOf.at(b->getFallOutEdge()->destination())->setFallInEdge(edge);
        }
        if ( b->getRetOutEdge() != nullptr ) {
            auto edge = new ReturnEdge(copyOf.at(b), copyOf.at(b->getRetOutEdge()->destination()));
            copyOf.at(b)->setRetOutEdge(edge);
            copyOf.at(b->getRetOutEdge()->destination())->setRetInEdge(edge);
        }
    }

    blocks->insert(blocks->end(), newblocks.begin(), newblocks.end());
    return std::pair<Block*, Block*>(copyOf.at(_start), copyOf.at(_end));
}

ControlFlowGraph::ControlFlowGraph(ISA::Instructions* instrs) {
    // get mappings of instruction order for ease 
    auto cfg = CFGBuild(instrs);
    _instrs = cfg._instrs;
    _blocks = cfg._blocks;
    _nameMap = cfg._nameMap;
    _first = _blocks->at(0);
    _last = nullptr;
    removeSelfAssigns();
    //literalPropagation();
}

bool ControlFlowGraph::removeSelfAssigns() {
    bool flag = false;

    for ( auto b : *_blocks ) {
        flag = b->removeSelfAssigns() || flag;
    }

    return flag;
}

bool Block::removeSelfAssigns() {
    bool flag = true;

    for ( size_t j = 0; j < _instructions->size(); j++ ) {
        if ( _instructions->at(j)->tag() == ISA::Tag::ASSIGNVALUE ) {
            auto instr = ((ISA::AssignValue*)_instructions->at(j));
            if ( instr->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                if ( instr->first()->name() == ((ISA::LocationReference*)instr->second())->name() ) {
                    // delete _instructions->at(j);
                    _instructions->erase(_instructions->begin() + j);
                    j--;
                    flag = true;
                }
            }
        }
    }

    return flag;
}

bool ControlFlowGraph::literalPropagation() {
    bool flag = false;

    for ( auto b : *_blocks ) {
        std::unordered_map<std::string,ISA::Reference*> valueMap; 
        std::unordered_map<std::string,ISA::Instruction*> instrMap;
        flag = b->literalPropagation(&valueMap, &instrMap) || flag;
    }

    return flag;
}

bool Block::literalPropagation(std::unordered_map<std::string,ISA::Reference*>* valueMap, std::unordered_map<std::string,ISA::Instruction*>* instrMap) {
    bool flag = false;
    for ( size_t j = 0; j < _instructions->size(); j++ ) {
        bool propagate = true;
        auto instr = _instructions->at(j);

        if ( instr->tag() == ISA::Tag::ASSIGNVALUE ) {
            propagate = false;
            auto instrAV = (ISA::AssignValue*)instr;

            // attempt to replace rhs with a value or instruction
            if ( instrAV->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                std::string name = ((ISA::LocationReference*)instrAV->second())->name();
                if ( valueMap->count(name) != 0 ) {
                    // delete instrAV->second();
                    instrAV->setSecond(valueMap->at(name));
                } else if ( instrMap->count(name) != 0) {
                    // propagate instruction
                    auto instrAE = new ISA::AssignEval(instrAV->first(), instrMap->at(name));
                    //delete instrAV;
                    _instructions->at(j) = instrAE;

                    // add instruction to map
                    if ( instrMap->count(instrAE->first()->name()) == 0 ) {
                        instrMap->insert({instrAE->first()->name(), instrAE->second()});
                    } else {
                        instrMap->at(instrAE->first()->name()) = instrAE->second();
                    }
                }
            }

            // add value to map if instruction has not been upgraded to AssignEval
            if ( instr->tag() == ISA::Tag::ASSIGNVALUE ) {
                if ( valueMap->count(instrAV->first()->name()) == 0 ) {
                    valueMap->insert({instrAV->first()->name(), instrAV->second()});
                } else {
                    valueMap->at(instrAV->first()->name()) = instrAV->second();
                }
            }
        } else if ( instr->tag() == ISA::Tag::ASSIGNEVAL ) {
            // add instruction to map
            auto instrAE = (ISA::AssignEval*)instr;
            if ( instrMap->count(instrAE->first()->name()) == 0 ) {
                instrMap->insert({instrAE->first()->name(), instrAE->second()});
            } else {
                instrMap->at(instrAE->first()->name()) = instrAE->second();
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
                    if ( valueMap->count(name) != 0 ) {
                        //delete uinstr->first();
                        uinstr->setFirst(valueMap->at(name));
                        flag = true;
                    }
                }
            } else if ( instr->isBinary() ) {
                auto binstr = (ISA::BinaryInstruction<ISA::Reference,ISA::Reference>*)instr;
                if ( binstr->first()->tag() == ISA::ReferenceTag::LOCATION ) {
                    std::string name = ((ISA::LocationReference*)binstr->first())->name();
                    if ( valueMap->count(name) != 0 ) {
                        //delete binstr->first();
                        binstr->setFirst(valueMap->at(name));
                        flag = true;
                    }
                }
                if ( binstr->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                    std::string name = ((ISA::LocationReference*)binstr->second())->name();
                    if ( valueMap->count(name) != 0 ) {
                        //delete binstr->second();
                        binstr->setSecond(valueMap->at(name));
                        flag = true;
                    }
                }
            } else if ( instr->isTrinary() ) {
                auto tinstr = (ISA::TrinaryInstruction<ISA::Reference,ISA::Reference,ISA::Reference>*)instr;
                if ( tinstr->first()->tag() == ISA::ReferenceTag::LOCATION ) {
                    std::string name = ((ISA::LocationReference*)tinstr->first())->name();
                    if ( valueMap->count(name) != 0 ) {
                        //delete tinstr->first();
                        tinstr->setFirst(valueMap->at(name));
                        flag = true;
                    }
                }
                if ( tinstr->second()->tag() == ISA::ReferenceTag::LOCATION ) {
                    std::string name = ((ISA::LocationReference*)tinstr->second())->name();
                    if ( valueMap->count(name) != 0 ) {
                        //delete tinstr->second();
                        tinstr->setSecond(valueMap->at(name));
                        flag = true;
                    }
                }
                if ( tinstr->third()->tag() == ISA::ReferenceTag::LOCATION ) {
                    std::string name = ((ISA::LocationReference*)tinstr->third())->name();
                    if ( valueMap->count(name) != 0 ) {
                        //delete tinstr->second();
                        tinstr->setThird(valueMap->at(name));
                        flag = true;
                    }
                }
            }
        }
    }

    return flag;
}

void ControlFlowGraph::serialize(std::ostream& out) const {
    out << "digraph cfg {\n";
    for ( auto b : *_blocks ) {
        out << b->serialize();
    }
    out << "}\n";
}

ISA::Instructions* ControlFlowGraph::reconstruct() const {
    return nullptr;
}


}
}