#ifndef SWARMC_CFG_H
#define SWARMC_CFG_H

#include <vector>
#include <unordered_map>
#include "../vm/isa_meta.h"

namespace swarmc {
namespace CFG {

class CFGBuild;

class Block {
public:
    Block(std::string id, bool isFunction) : _id(id), _isFunction(isFunction), _copy(false), 
        _returnFrom(nullptr), _instructions(new ISA::Instructions()) {}

    ~Block() {
        delete _instructions;
    }

    void addInstruction(ISA::Instruction* instr) {
        _instructions->push_back(instr);
    }

    Block* copy() const {
        auto b = new Block(_id, _isFunction);
        for ( auto i : *_instructions ) b->addInstruction(i);
        b->_copy = true;
        return b;
    }

    std::string id() const { return _id; }
    bool isFunction() const { return _isFunction; }
    Block* returnFrom() const { return _returnFrom; }
    void setReturnsFrom(Block* block) { _returnFrom = block; } 
    ISA::Instructions* instructions() const { return _instructions; }
private:
    std::string _id;
    bool _isFunction, _copy;
    Block* _returnFrom;
    ISA::Instructions* _instructions;
};

class Edge {
public:
    Edge(Block* source, Block* dest, std::string label) : _source(source), _destination(dest), _label(label) {}
    Block* source() const { return _source; }
    Block* destination() const { return _destination; }
    std::string label() const { return _label; }
private:
    Block* _source;
    Block* _destination;
    std::string _label;
};

class ControlFlowGraph {
public:
    ControlFlowGraph(ISA::Instructions*);
    ~ControlFlowGraph() {
        delete _blocks;
        delete _edges;
    }
    // make a dot file :)
    void serialize(std::ostream&) const;
private:
    Block* _first = nullptr;
    Block* _last = nullptr;

    ISA::Instructions* _instrs;
    std::vector<Block*>* _blocks;
    std::vector<Edge*>* _edges;

    bool removeSelfAssigns();
    bool intraBlockLiteralPropagation();
};

class CFGBuild {
public:
    CFGBuild(ISA::Instructions* instrs) : _instrs(instrs), _blocks(new std::vector<Block*>()), _edges(new std::vector<Edge*>()) {
        //sequenceISA();
        buildBlocks();
    }
    ~CFGBuild() = default;
protected:
    //void sequenceISA();
    void buildBlocks();
    void addCallEdge();

    ISA::Instructions* _instrs;
    std::vector<Block*>* _blocks;
    std::vector<Edge*>* _edges;

    friend ControlFlowGraph;
};

}
}

#endif