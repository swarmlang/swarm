#ifndef SWARMC_CFG_H
#define SWARMC_CFG_H

#include <vector>
#include <unordered_map>
#include <stack>
#include <cassert>
#include "../vm/isa_meta.h"

namespace swarmc {
namespace CFG {

class CFGBuild;
class CallEdge;
class FallEdge;
class ReturnEdge;

class Block : public IStringable {
public:
    enum class BlockType {
        BLOCK, AMBIGUOUSFUNCTION, FUNCTION, ERROR
    };
    
    Block(std::string id, BlockType type, size_t i) : _id(id), _copy(0), _idx(i), _type(type), 
        _instructions(new ISA::Instructions()), _callInEdge(nullptr), _callOutEdge(nullptr), 
        _fallInEdge(nullptr), _fallOutEdge(nullptr), _retInEdge(nullptr), _retOutEdge(nullptr) {}

    ~Block();

    void addInstruction(ISA::Instruction* instr) {
        _instructions->push_back(instr);
    }

    std::string id() const { return _id; }

    size_t idx() const { return _idx; }

    size_t numCopy() const { return _copy; }
    
    BlockType blockType() const { return _type; }
    
    std::string blockTypeString() const {
        if (_type == BlockType::BLOCK) return "BLOCK";
        if (_type == BlockType::AMBIGUOUSFUNCTION) return "AMBIGUOUSFUNCTION";
        if (_type == BlockType::FUNCTION) return "FUNCTION";
        return "ERROR";
    }
    
    ISA::Instructions* instructions() const { return _instructions; }
    
    std::string toString() const { return "Block<" + _id + ">"; }

    CallEdge* getCallInEdge() const;
    CallEdge* getCallOutEdge() const;
    FallEdge* getFallInEdge() const ;
    FallEdge* getFallOutEdge() const;
    ReturnEdge* getRetInEdge() const;
    ReturnEdge* getRetOutEdge() const;

    void setCallInEdge(CallEdge* edge);
    void setCallOutEdge(CallEdge* edge);
    void setFallInEdge(FallEdge* edge);
    void setFallOutEdge(FallEdge* edge);
    void setRetInEdge(ReturnEdge* edge);
    void setRetOutEdge(ReturnEdge* edge);

    /* DOES NOT COPY EDGES */
    virtual Block* copy(size_t idx) const {
        Block* copy = new Block(_id, _type, idx);
        copy->_copy = _copy + 1;
        for ( auto i : *_instructions ) {
            copy->addInstruction(i->copy());
        }
        return copy;
    }
    
    virtual std::string serialize() const;

    virtual bool removeSelfAssigns();

    virtual bool literalPropagation(std::unordered_map<std::string,ISA::Reference*>*, std::unordered_map<std::string,ISA::Instruction*>*);
protected:
    std::string _id;
    size_t _copy;
    size_t _idx;
    BlockType _type;
    ISA::Instructions* _instructions;
    CallEdge* _callInEdge, * _callOutEdge;
    FallEdge* _fallInEdge, * _fallOutEdge;
    ReturnEdge* _retInEdge, * _retOutEdge;
};

class AmbiguousFunctionBlock : public Block {
public:
    AmbiguousFunctionBlock(std::string id, size_t i) : Block(id, BlockType::AMBIGUOUSFUNCTION, i) {}

    std::string toString() const { return "AmbiguousFunction" + Block::toString(); }
    
    std::string serialize() const;
    
    virtual AmbiguousFunctionBlock* copy(size_t idx) const {
        auto copy = new AmbiguousFunctionBlock(_id, idx);
        copy->_copy = true;
        return copy;
    }
};

class CFGFunction : public IStringable {
public:
    CFGFunction(std::string id, Block* start) : _id(id), _start(start), _end(nullptr), _blocks(new std::vector<Block*>()) {
        _blocks->push_back(_start);
    }

    std::string id() const { return _id; }
    Block* start() const { return _start; }
    Block* end() const { return _end; }
    void setEnd(Block* end) { 
        assert(_end == nullptr);
        _end = end;
    }
    void addBlock(Block* block) { _blocks->push_back(block); }

    std::string toString() const override { return "CFGFunction<" + _id + ">"; }

    std::pair<Block*,Block*> makeCopy(size_t i, std::vector<Block*>* blocks, std::stack<CFGFunction*>* callStack) const;

private:
    std::string _id;

    Block* _start;
    Block* _end;

    std::vector<Block*>* _blocks;
};

class Edge : public IStringable {
public:
    enum class EdgeType {
        CALL, FALL, RETURN, ERROR
    };

    Edge(Block* source, Block* dest, EdgeType label) : _source(source), _destination(dest), _label(label) {}
    
    Block* source() const { return _source; }
    
    Block* destination() const { return _destination; }
    
    EdgeType label() const { return _label; }
    
    std::string labelString() const {
        if (_label == EdgeType::CALL) return "call";
        if (_label == EdgeType::FALL) return "fall";
        if (_label == EdgeType::RETURN) return "return";
        return "ERROR";
    }
    
    std::string serialize() const {
        return "\t\"" + source()->id() + ":" + std::to_string(source()->numCopy()) 
            + ":" + std::to_string(source()->idx()) + "\"->\"" 
            + destination()->id() + ":" + std::to_string(destination()->numCopy()) 
            + ":" + std::to_string(destination()->idx())
            + "\" [label=\"" + labelString() + "\"]\n";
    }
protected:
    Block* _source;
    Block* _destination;
    EdgeType _label;
};

class CallEdge : public Edge {
public:
    CallEdge(Block* source, Block* dest) : Edge(source, dest, Edge::EdgeType::CALL) {}
    std::string toString() const { 
        return "CallEdge<" + _source->toString() + "," + _destination->toString()+ ">";
    }
};

class FallEdge : public Edge {
public:
    FallEdge(Block* source, Block* dest) : Edge(source, dest, Edge::EdgeType::FALL) {}
    std::string toString() const { 
        return "FallEdge<" + _source->toString() + "," + _destination->toString()+ ">";
    }
};

class ReturnEdge : public Edge {
public:
    ReturnEdge(Block* source, Block* dest) : Edge(source, dest, Edge::EdgeType::RETURN) {}
    std::string toString() const { 
        return "ReturnEdge<" + _source->toString() + "," + _destination->toString()+ ">";
    }
};

class ControlFlowGraph {
public:
    ControlFlowGraph(ISA::Instructions*);
    
    ~ControlFlowGraph() {
        for ( auto b : *_blocks ) delete b;
        delete _blocks;
    }
    
    // make a dot file :)
    void serialize(std::ostream&) const;

    ISA::Instructions* reconstruct() const;
private:
    Block* _first = nullptr;
    Block* _last = nullptr;

    ISA::Instructions* _instrs;
    std::vector<Block*>* _blocks;
    std::unordered_map<std::string, CFGFunction*>* _nameMap;

    bool removeSelfAssigns();
    bool literalPropagation();
};

class CFGBuild : public IUsesConsole {
public:
    CFGBuild(ISA::Instructions* instrs) : _instrs(instrs), _blocks(new std::vector<Block*>()), 
        _nameMap(new std::unordered_map<std::string, CFGFunction*>()) {
        //sequenceISA();
        buildBlocks();
    }
protected:
    //void sequenceISA();
    void buildBlocks();

    ISA::Instructions* _instrs;
    std::vector<Block*>* _blocks;
    std::unordered_map<std::string, CFGFunction*>* _nameMap;

    friend ControlFlowGraph;
};

}
}

#endif