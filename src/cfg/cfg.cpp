#include "cfg_optimize.h"

namespace swarmc::CFG {

CallEdge* Block::getCallInEdge() const { return _callInEdge; }
CallEdge* Block::getCallOutEdge() const { return _callOutEdge; }
FallEdge* Block::getFallInEdge() const { return _fallInEdge; }
FallEdge* Block::getFallOutEdge() const { return _fallOutEdge; }
ReturnEdge* Block::getRetInEdge() const { return _retInEdge; }
ReturnEdge* Block::getRetOutEdge() const { return _retOutEdge; }

void Block::setCallInEdge(CallEdge* edge) { _callInEdge = useref(edge); }
void Block::setCallOutEdge(CallEdge* edge) { _callOutEdge = useref(edge); }
void Block::setFallInEdge(FallEdge* edge) { _fallInEdge = useref(edge); }
void Block::setFallOutEdge(FallEdge* edge) { _fallOutEdge = useref(edge); }
void Block::setRetInEdge(ReturnEdge* edge) { _retInEdge = useref(edge); }
void Block::setRetOutEdge(ReturnEdge* edge) { _retOutEdge = useref(edge); }

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

std::pair<Block*,Block*> CFGFunction::makeCopy(std::size_t i, std::vector<Block*>* blocks, std::stack<CFGFunction*>* callStack) const {
    std::vector<Block*> newblocks;
    std::unordered_map<Block*, Block*> copyOf;

    for ( auto b : *_blocks ) {
        Block* copy = b->copy(i);
        newblocks.push_back(useref(copy));
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

    blocks->insert(
        blocks->end(),
        std::make_move_iterator(newblocks.begin()),
        std::make_move_iterator(newblocks.end())
    );
    return { copyOf.at(_start), copyOf.at(_end) };
}

ControlFlowGraph::ControlFlowGraph(ISA::Instructions* instrs) : IUsesLogger("CFG") {
    // get mappings of instruction order for ease
    auto cfg = CFGBuild(instrs);
    _instrs = cfg._instrs;
    _blocks = cfg._blocks;
    _nameMap = cfg._nameMap;
    _first = _blocks->at(0);
    _last = nullptr;
}

void ControlFlowGraph::serialize(std::ostream& out) const {
    out << "digraph cfg {\n";
    for ( auto b : *_blocks ) {
        out << b->serialize();
    }
    out << "}\n";
}

ISA::Instructions* ControlFlowGraph::reconstruct() const {
    auto instrs = new ISA::Instructions();

    for ( const auto& cfgf : *_nameMap ) {
        auto temp = reconstruct(cfgf.second->start(), 0);
        instrs->insert(
            instrs->end(),
            std::make_move_iterator(temp->begin()),
            std::make_move_iterator(temp->end())
        );
        delete temp;
    }

    auto main = reconstruct(_first, 0);
    instrs->insert(
        instrs->end(),
        std::make_move_iterator(main->begin()),
        std::make_move_iterator(main->end())
    );
    delete main;

    /*
     * Instructions are referenced in 2 possible places:
     * 1. The original instruction list that was used to build the CFG (copies will not have this ref)
     * 2. Blocks
     * 
     * When we reassign the instruction list in the pipeline we have to do 2 things:
     * 1. freeref the original list (some of those instructions might have been removed
     *      by the opt. pass, meaning that list is the last ref to them)
     * 2. freeref all the blocks (happens in CFG and CFGFunction destructors). This needs to
     *      happen because otherwise some instructions (copies) dont get deleted, causing leaks
     * 
     * Thus, this useref loop right here re-userefs the instructions we actually want to keep
     * post-optimization pass, such that when the CFG is free'd, the instructions remain
     */
    for ( auto i : *instrs ) useref(i);

    return instrs;
}

ISA::Instructions* ControlFlowGraph::reconstruct(Block* block, std::size_t fDepth) const {
    auto instrs = new ISA::Instructions();

    if ( fDepth == 0 ) {
        instrs->insert(
            instrs->end(),
            std::make_move_iterator(block->instructions()->begin()),
            std::make_move_iterator(block->instructions()->end())
        );
    }

    ISA::Instructions* tempInstrs = nullptr;
    if ( block->getFallOutEdge() != nullptr ) {
        tempInstrs = reconstruct(block->getFallOutEdge()->destination(), fDepth);
    } else if ( block->getRetOutEdge() != nullptr ) {
        tempInstrs = reconstruct(block->getRetOutEdge()->destination(), fDepth - 1);
    } else if ( block->getCallOutEdge() != nullptr ) {
        tempInstrs = reconstruct(block->getCallOutEdge()->destination(), fDepth + 1);
    }
    if ( tempInstrs != nullptr ) {
        instrs->insert(
            instrs->end(),
            std::make_move_iterator(tempInstrs->begin()),
            std::make_move_iterator(tempInstrs->end())
        );
        delete tempInstrs;
    }

    return instrs;
}

ISA::Instructions* ControlFlowGraph::optimize(ISA::Instructions* instrs, bool rSelfAssign, bool litProp, std::ostream* out) {
       
    bool flag;
    auto iterations = 1;
    auto iOpt = instrs;

    do {
        ControlFlowGraph cfg(iOpt);

        if ( iterations == 1 && !rSelfAssign ) 
            cfg.logger->warn("Disabling removal of self-assignments can result in the loss of atomicity in swarm statements.");
        if ( iterations == 1 && out != nullptr )
            cfg.serialize(*out);

        cfg.logger->debug("Starting CFG optimization pass " + s(iterations++));
        flag = false;
        if (rSelfAssign) flag = RemoveSelfAssign::optimize(&cfg) || flag;
        if (litProp) flag = ConstantPropagation::optimize(&cfg) || flag;
        iOpt = cfg.reconstruct();
    } while (flag);

    return iOpt;
}


}
