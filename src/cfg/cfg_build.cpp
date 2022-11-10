#include <stack>
#include <assert.h>
#include "cfg.h"
#include <iostream>

namespace swarmc {
namespace CFG {

void CFGBuild::buildBlocks() {
    std::unordered_map<std::string, Block*> nameMap;
    std::stack<Block*> bstack;
    std::stack<Block*> callStack;
    _blocks->push_back(new Block("Top", false));
    bstack.push(_blocks->back());
    if ( _instrs->size() == 0 ) return;
    for ( size_t i = 0; i < _instrs->size(); i++ ) {
        bstack.top()->addInstruction(_instrs->at(i));
        if ( _instrs->at(i)->tag() == ISA::Tag::BEGINFN ) {
            std::string name = ((ISA::BeginFunction*)_instrs->at(i))->first()->fqName();
            _blocks->push_back(new Block(name, true));
            bstack.push(_blocks->back());
            nameMap.insert({ name, bstack.top() });
            callStack.push(_blocks->back());
        } else if ( _instrs->at(i)->tag() == ISA::Tag::RETURN0 
                || _instrs->at(i)->tag() == ISA::Tag::RETURN1 ) 
        {
            callStack.top()->setReturnsFrom(bstack.top());
            while ( !bstack.top()->isFunction() ) bstack.pop();
            bstack.pop();
            callStack.pop();
        } else if ( _instrs->at(i)->tag() == ISA::Tag::CALL0
            || _instrs->at(i)->tag() == ISA::Tag::CALL1
            || _instrs->at(i)->tag() == ISA::Tag::CALLIF0
            || _instrs->at(i)->tag() == ISA::Tag::CALLIF1
            || _instrs->at(i)->tag() == ISA::Tag::CALLELSE0
            || _instrs->at(i)->tag() == ISA::Tag::CALLELSE1
            || _instrs->at(i)->tag() == ISA::Tag::WITH
            || _instrs->at(i)->tag() == ISA::Tag::ENUMERATE
            || _instrs->at(i)->tag() == ISA::Tag::WHILE ) 
        {
            std::string name = "";
            bool cond = true;
            switch ( _instrs->at(i)->tag() ) {
            case ISA::Tag::CALL0:
                assert(((ISA::Call0*)_instrs->at(i))->first()->tag() == ISA::ReferenceTag::LOCATION);
                name = ((ISA::LocationReference*)((ISA::Call0*)_instrs->at(i))->first())->fqName();
                cond = false;
                break; 
            case ISA::Tag::CALL1:
                assert(((ISA::Call1*)_instrs->at(i))->first()->tag() == ISA::ReferenceTag::LOCATION);
                name = ((ISA::LocationReference*)((ISA::Call1*)_instrs->at(i))->first())->fqName();
                cond = false;
                break; 
            case ISA::Tag::CALLIF0:
                assert(((ISA::CallIf0*)_instrs->at(i))->second()->tag() == ISA::ReferenceTag::LOCATION);
                name = ((ISA::LocationReference*)((ISA::CallIf0*)_instrs->at(i))->second())->fqName();
                break; 
            case ISA::Tag::CALLIF1:
                assert(((ISA::CallIf1*)_instrs->at(i))->second()->tag() == ISA::ReferenceTag::LOCATION);
                name = ((ISA::LocationReference*)((ISA::CallIf1*)_instrs->at(i))->second())->fqName();
                break; 
            case ISA::Tag::CALLELSE0:
                assert(((ISA::CallElse0*)_instrs->at(i))->second()->tag() == ISA::ReferenceTag::LOCATION);
                name = ((ISA::LocationReference*)((ISA::CallElse0*)_instrs->at(i))->second())->fqName();
                break; 
            case ISA::Tag::CALLELSE1:
                assert(((ISA::CallElse1*)_instrs->at(i))->second()->tag() == ISA::ReferenceTag::LOCATION);
                name = ((ISA::LocationReference*)((ISA::CallElse1*)_instrs->at(i))->second())->fqName();
                break;  
            case ISA::Tag::WITH:
                name = ((ISA::With*)_instrs->at(i))->second()->fqName();
                cond = false;
                break;
            case ISA::Tag::WHILE:
                name = ((ISA::While*)_instrs->at(i))->second()->fqName();
                cond = false;
                break;
            case ISA::Tag::ENUMERATE:
                name = ((ISA::Enumerate*)_instrs->at(i))->third()->fqName();
                cond = false;
                break;
            default:
                break; 
            }
            
            Block* p = bstack.top();
            // postcall block
            _blocks->push_back(new Block("POSTCALL:" + name + ":" + std::to_string(i), false));
            bstack.push(_blocks->back());

            // create call edge
            if ( nameMap.count(name) == 0 ) {
                _blocks->push_back(new Block("AmbiguousFunction:" + name + ":" + std::to_string(i), true));
                _edges->push_back(new Edge(p, _blocks->back(), "call"));
                _edges->push_back(new Edge(_blocks->back(), bstack.top(), "return"));
            } else {
                _edges->push_back(new Edge(p, nameMap.at(name), "call"));
                _edges->push_back(new Edge(nameMap.at(name)->returnFrom(), bstack.top(), "return"));
            }

            // create fallthrough edge
            if ( cond ) {
                _edges->push_back(new Edge(p, bstack.top(), "fall"));
            }
        } else if (_instrs->at(i)->tag() == ISA::Tag::ASSIGNEVAL
                && (((ISA::AssignEval*)_instrs->at(i))->second()->tag() == ISA::Tag::CALL0
                || ((ISA::AssignEval*)_instrs->at(i))->second()->tag() == ISA::Tag::CALL1
                || ((ISA::AssignEval*)_instrs->at(i))->second()->tag() == ISA::Tag::CALLIF0
                || ((ISA::AssignEval*)_instrs->at(i))->second()->tag() == ISA::Tag::CALLIF1
                || ((ISA::AssignEval*)_instrs->at(i))->second()->tag() == ISA::Tag::CALLELSE0
                || ((ISA::AssignEval*)_instrs->at(i))->second()->tag() == ISA::Tag::CALLELSE1)) 
        {
            auto call = ((ISA::AssignEval*)_instrs->at(i))->second();
            std::string name = "";
            bool cond = true;
            switch ( call->tag() ) {
            case ISA::Tag::CALL0:
                assert(((ISA::Call0*)call)->first()->tag() == ISA::ReferenceTag::LOCATION);
                name = ((ISA::LocationReference*)((ISA::Call0*)call)->first())->fqName();
                cond = false;
                break; 
            case ISA::Tag::CALL1:
                assert(((ISA::Call1*)call)->first()->tag() == ISA::ReferenceTag::LOCATION);
                name = ((ISA::LocationReference*)((ISA::Call1*)call)->first())->fqName();
                cond = false;
                break; 
            case ISA::Tag::CALLIF0:
                assert(((ISA::CallIf0*)call)->second()->tag() == ISA::ReferenceTag::LOCATION);
                name = ((ISA::LocationReference*)((ISA::CallIf0*)call)->second())->fqName();
                break; 
            case ISA::Tag::CALLIF1:
                assert(((ISA::CallIf1*)call)->second()->tag() == ISA::ReferenceTag::LOCATION);
                name = ((ISA::LocationReference*)((ISA::CallIf1*)call)->second())->fqName();
                break; 
            case ISA::Tag::CALLELSE0:
                assert(((ISA::CallElse0*)call)->second()->tag() == ISA::ReferenceTag::LOCATION);
                name = ((ISA::LocationReference*)((ISA::CallElse0*)call)->second())->fqName();
                break; 
            case ISA::Tag::CALLELSE1:
                assert(((ISA::CallElse1*)call)->second()->tag() == ISA::ReferenceTag::LOCATION);
                name = ((ISA::LocationReference*)((ISA::CallElse1*)call)->second())->fqName();
                break;
            default:
                break; 
            }
            
            Block* p = bstack.top();
            // postcall block
            _blocks->push_back(new Block("POSTCALL:" + name + ":" + std::to_string(i), false));
            bstack.push(_blocks->back());

            // create call edge
            if ( nameMap.count(name) == 0 ) {
                _blocks->push_back(new Block("AmbiguousFunction:" + name + ":" + std::to_string(i), true));
                _edges->push_back(new Edge(p, _blocks->back(), "call"));
                _edges->push_back(new Edge(_blocks->back(), bstack.top(), "return"));
            } else {
                _edges->push_back(new Edge(p, nameMap.at(name), "call"));
                _edges->push_back(new Edge(nameMap.at(name)->returnFrom(), bstack.top(), "return"));
            }

            // create fallthrough edge
            if ( cond ) {
                _edges->push_back(new Edge(p, bstack.top(), "fall"));
            }
        }
    }
}

// void CFGBuild::sequenceISA() {
//     std::stack<ISA::Instruction*> prefunc;
//     if ( _instrs->size() == 0 ) return;
//     _nameMap->insert({_instrs->at(0), "Top"});
//     _prevInstr->insert({_instrs->at(0), nullptr});
//     for ( size_t i = 0; i < _instrs->size(); i++ ) {
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

// std::vector<Block*>* CFGBuild::buildBlocks() {
//     auto blocks = new std::vector<Block*>();
//     for ( auto i : *_instrs ) {
//         if ( _nameMap->count(i) != 0 ) {
//             blocks->push_back(new Block(_nameMap->at(i)));
//             auto temp = i;
//             while ( temp != nullptr ) {
//                 blocks->back()->addInstruction(temp);
//                 temp = _nextInstr->at(temp);
//             }
//         }
//     }

//     return blocks;
// }

}
}