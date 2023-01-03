#include <stack>
#include <cassert>
#include "cfg.h"

namespace swarmc::CFG {

void CFGBuild::buildBlocks() {
    std::stack<Block*> bstack;
    std::stack<CFGFunction*> callStack;
    _blocks->push_back(new Block("Top", Block::BlockType::BLOCK, 0));
    bstack.push(_blocks->back());
    if ( _instrs->empty() ) return;
    for ( std::size_t i = 0; i < _instrs->size(); i++ ) {
        if ( _instrs->at(i)->tag() == ISA::Tag::BEGINFN ) {
            std::string name = ((ISA::BeginFunction*)_instrs->at(i))->first()->fqName();
            auto fstart = new Block(name, Block::BlockType::FUNCTION, i);
            auto cfgf = new CFGFunction(name, fstart);
            console->debug("Created function " + name);
            _blocks->push_back(fstart);
            bstack.push(fstart);
            _nameMap->insert({ name, cfgf });
            callStack.push(cfgf);
            bstack.top()->addInstruction(_instrs->at(i));
        } else if ( _instrs->at(i)->tag() == ISA::Tag::RETURN0
                || _instrs->at(i)->tag() == ISA::Tag::RETURN1 )
        {
            bstack.top()->addInstruction(_instrs->at(i));
            callStack.top()->setEnd(bstack.top());
            while ( bstack.top()->blockType() != Block::BlockType::FUNCTION ) bstack.pop();
            bstack.pop();
            callStack.pop();
        } else {
            bstack.top()->addInstruction(_instrs->at(i));
            auto instr = _instrs->at(i)->tag() == ISA::Tag::ASSIGNEVAL
                ? ((ISA::AssignEval*)_instrs->at(i))->second()
                : _instrs->at(i);

            if ( instr->tag() == ISA::Tag::CALL0
                || instr->tag() == ISA::Tag::CALL1
                || instr->tag() == ISA::Tag::CALLIF0
                || instr->tag() == ISA::Tag::CALLIF1
                || instr->tag() == ISA::Tag::CALLELSE0
                || instr->tag() == ISA::Tag::CALLELSE1
                || instr->tag() == ISA::Tag::WITH
                || instr->tag() == ISA::Tag::ENUMERATE
                || instr->tag() == ISA::Tag::WHILE )
            {
                std::string name;
                bool cond = true;
                switch ( instr->tag() ) {
                case ISA::Tag::CALL0:
                    assert(((ISA::Call0*)instr)->first()->tag() == ISA::ReferenceTag::LOCATION);
                    name = ((ISA::LocationReference*)((ISA::Call0*)instr)->first())->fqName();
                    cond = false;
                    break;
                case ISA::Tag::CALL1:
                    assert(((ISA::Call1*)instr)->first()->tag() == ISA::ReferenceTag::LOCATION);
                    name = ((ISA::LocationReference*)((ISA::Call1*)instr)->first())->fqName();
                    cond = false;
                    break;
                case ISA::Tag::CALLIF0:
                    assert(((ISA::CallIf0*)instr)->second()->tag() == ISA::ReferenceTag::LOCATION);
                    name = ((ISA::LocationReference*)((ISA::CallIf0*)instr)->second())->fqName();
                    break;
                case ISA::Tag::CALLIF1:
                    assert(((ISA::CallIf1*)instr)->second()->tag() == ISA::ReferenceTag::LOCATION);
                    name = ((ISA::LocationReference*)((ISA::CallIf1*)instr)->second())->fqName();
                    break;
                case ISA::Tag::CALLELSE0:
                    assert(((ISA::CallElse0*)instr)->second()->tag() == ISA::ReferenceTag::LOCATION);
                    name = ((ISA::LocationReference*)((ISA::CallElse0*)instr)->second())->fqName();
                    break;
                case ISA::Tag::CALLELSE1:
                    assert(((ISA::CallElse1*)instr)->second()->tag() == ISA::ReferenceTag::LOCATION);
                    name = ((ISA::LocationReference*)((ISA::CallElse1*)instr)->second())->fqName();
                    break;
                case ISA::Tag::WITH:
                    name = ((ISA::With*)instr)->second()->fqName();
                    cond = false;
                    break;
                case ISA::Tag::WHILE:
                    name = ((ISA::While*)instr)->second()->fqName();
                    cond = false;
                    break;
                case ISA::Tag::ENUMERATE:
                    name = ((ISA::Enumerate*)instr)->third()->fqName();
                    cond = false;
                    break;
                default:
                    break;
                }

                assert(!bstack.empty());
                Block* previous = bstack.top();
                // postcall block
                auto postcall = new Block("POSTCALL:" + name, Block::BlockType::BLOCK, i);
                _blocks->push_back(postcall);
                bstack.push(postcall);
                if (!callStack.empty()) callStack.top()->addBlock(postcall);

                // create call edge
                if ( _nameMap->count(name) == 0 ) {
                    Block* am = new AmbiguousFunctionBlock(name, i);
                    console->debug("Created ambiguous function " + am->id());
                    _blocks->push_back(am);
                    auto cedge = new CallEdge(previous, am);
                    previous->setCallOutEdge(cedge);
                    am->setCallInEdge(cedge);
                    auto redge = new ReturnEdge(am, postcall);
                    am->setRetOutEdge(redge);
                    postcall->setRetInEdge(redge);

                    if (!callStack.empty()) callStack.top()->addBlock(am);
                } else {
                    auto funcCopy = _nameMap->at(name)->makeCopy(i, _blocks, &callStack);
                    console->debug("Copied function " + _nameMap->at(name)->id());
                    auto cedge = new CallEdge(previous, funcCopy.first);
                    previous->setCallOutEdge(cedge);
                    funcCopy.first->setCallInEdge(cedge);
                    auto redge = new ReturnEdge(funcCopy.second, postcall);
                    funcCopy.second->setRetOutEdge(redge);
                    postcall->setRetInEdge(redge);
                }

                // create fallthrough edge
                if ( cond ) {
                    auto edge = new FallEdge(previous, bstack.top());
                    previous->setFallOutEdge(edge);
                    bstack.top()->setFallInEdge(edge);
                }
            }
        }
    }
}

// void CFGBuild::sequenceISA() {
//     std::stack<ISA::Instruction*> prefunc;
//     if ( _instrs->size() == 0 ) return;
//     _nameMap->insert({_instrs->at(0), "Top"});
//     _prevInstr->insert({_instrs->at(0), nullptr});
//     for ( std::size_t i = 0; i < _instrs->size(); i++ ) {
//         if ( _instrs->at(i)->tag() == ISA::Tag::BEGINFN ) {
//             /* BEGINFN
//              * Next instr = the instruction after the corresponding return
//              * Previous instruction = i - 1
//              */
//             if ( i != 0 ) {
//                 _prevInstr->insert({_instrs->at(i), _instrs->at(i - 1)});
//             } else {
//                 _prevInstr->insert({_instrs->at(i), nullptr});
//             }
//             prefunc.push(_instrs->at(i));
//             if ( i != _instrs->size() - 1 ) {
//                 _nameMap->insert({_instrs->at(i + 1), ((ISA::BeginFunction*)_instrs->at(i))->first()->fqName()});
//             }
//         } else if ( _instrs->at(i)->tag() == ISA::Tag::RETURN0
//                 || _instrs->at(i)->tag() == ISA::Tag::RETURN1 )
//         {
//             /* RETURN
//              * Next instr = nullptr
//              * Previous instruction = i - 1
//              * set top of prefunc's next instr to i + 1
//              * set i + 1's prev instr to prefunc.top and pop
//              */
//             _nextInstr->insert({_instrs->at(i), nullptr});
//             if ( i != 0 ) {
//                 _prevInstr->insert({_instrs->at(i), _instrs->at(i - 1)});
//             }  else {
//                 _prevInstr->insert({_instrs->at(i), nullptr});
//             }
//             if ( i != _instrs->size() - 1 ) {
//                 _nextInstr->insert({prefunc.top(), _instrs->at(i + 1)});
//                 _prevInstr->insert({_instrs->at(i + 1), prefunc.top()});
//             } else {
//                 _prevInstr->insert({prefunc.top(), nullptr});
//             }

//             prefunc.pop();
//         } else if ( _instrs->at(i)->tag() == ISA::Tag::CALL0
//             || _instrs->at(i)->tag() == ISA::Tag::CALL1
//             || _instrs->at(i)->tag() == ISA::Tag::CALLIF0
//             || _instrs->at(i)->tag() == ISA::Tag::CALLIF1
//             || _instrs->at(i)->tag() == ISA::Tag::CALLELSE0
//             || _instrs->at(i)->tag() == ISA::Tag::CALLELSE1
//             || _instrs->at(i)->tag() == ISA::Tag::WITH
//             || _instrs->at(i)->tag() == ISA::Tag::ENUMERATE
//             || _instrs->at(i)->tag() == ISA::Tag::WHILE )
//         {
//             /* CALL/WITH/WHILE/ENUMERATE
//              * Next instr = nullptr
//              * Previous instruction = i - 1
//              * give name to i + 1
//              */
//             _nextInstr->insert({_instrs->at(i), nullptr});
//             if ( i != 0 ) {
//                 _prevInstr->insert({_instrs->at(i), _instrs->at(i - 1)});
//             } else {
//                 _prevInstr->insert({_instrs->at(i), nullptr});
//             }
//             if ( i != _instrs->size() - 1 ) {
//                 _nameMap->insert({_instrs->at(i + 1), "POSTCALL" + std::to_string(i)});
//             }
//         } else if ( _instrs->at(i)->tag() == ISA::Tag::ASSIGNEVAL
//                 && (((ISA::AssignEval*)_instrs->at(i))->second()->tag() == ISA::Tag::CALL0
//                 || ((ISA::AssignEval*)_instrs->at(i))->second()->tag() == ISA::Tag::CALL1
//                 || ((ISA::AssignEval*)_instrs->at(i))->second()->tag() == ISA::Tag::CALLIF0
//                 || ((ISA::AssignEval*)_instrs->at(i))->second()->tag() == ISA::Tag::CALLIF1
//                 || ((ISA::AssignEval*)_instrs->at(i))->second()->tag() == ISA::Tag::CALLELSE0
//                 || ((ISA::AssignEval*)_instrs->at(i))->second()->tag() == ISA::Tag::CALLELSE1)) {
//             /* ASSIGNING A CALL
//              * Next instr = nullptr
//              * Previous instruction = i - 1
//              * give name to i + 1
//              */
//             _nextInstr->insert({_instrs->at(i), nullptr});
//             if ( i != 0 ) {
//                 _prevInstr->insert({_instrs->at(i), _instrs->at(i - 1)});
//             } else {
//                 _prevInstr->insert({_instrs->at(i), nullptr});
//             }
//             if ( i != _instrs->size() - 1 ) {
//                 _nameMap->insert({_instrs->at(i + 1), "POSTCALL" + std::to_string(i)});
//             }
//         } else {
//             /* Other
//              * Next instr = i + 1
//              * Previous instr = i - 1 (if not already set)
//              */
//             if ( i != _instrs->size() - 1 ) {
//                 _nextInstr->insert({_instrs->at(i), _instrs->at(i + 1)});
//             } else {
//                 _nextInstr->insert({_instrs->at(i), nullptr});
//             }
//             if ( _prevInstr->count(_instrs->at(i)) == 0 ) {
//                 if ( i != 0 ) {
//                     _prevInstr->insert({_instrs->at(i), _instrs->at(i - 1)});
//                 } else {
//                     _prevInstr->insert({_instrs->at(i), nullptr});
//                 }
//             }
//         }
//     }
// }

}
