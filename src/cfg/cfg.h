#ifndef SWARMC_CFG_H
#define SWARMC_CFG_H

#include <utility>
#include <vector>
#include <unordered_map>
#include <stack>
#include <cassert>
#include "../shared/nslib.h"
#include "../vm/isa_meta.h"

using namespace nslib;

namespace swarmc::CFG {

class CFGBuild;
class CallEdge;
class FallEdge;
class ReturnEdge;

class Block : public IStringable, public IRefCountable {
public:
    enum class BlockType {
        BLOCK, AMBIGUOUSFUNCTION, FUNCTION
    };
    
    Block(std::string id, BlockType type, size_t i) : _id(std::move(id)), _copy(0), _idx(std::move(i)), _type(std::move(type)),
        _instructions(new ISA::Instructions()) {}

    ~Block() override {
        for (auto i : *_instructions) freeref(i);
        delete _instructions;
        freeref(_callOutEdge);
        freeref(_callInEdge);
        freeref(_fallOutEdge);
        freeref(_fallInEdge);
        freeref(_retOutEdge);
        freeref(_retInEdge);
    }

    void addInstruction(ISA::Instruction* instr) {
        _instructions->push_back(useref(instr));
    }

    void removeInstruction(ISA::Instruction* instr) {
        for (auto i = _instructions->begin(); i != _instructions->end(); i++) {
            if (*i == instr) {
                freeref(*i);
                _instructions->erase(i);
                break;
            }
        }
    }

    /* Returns name of node */
    [[nodiscard]] std::string id() const { return _id; }

    [[nodiscard]] size_t idx() const { return _idx; }

    /* Returns which copy of the set of blocks this block is a part of this is */
    [[nodiscard]] size_t numCopy() const { return _copy; }

    [[nodiscard]] BlockType blockType() const { return _type; }

    [[nodiscard]] std::string blockTypeString() const {
        if (_type == BlockType::BLOCK) return "BLOCK";
        if (_type == BlockType::AMBIGUOUSFUNCTION) return "AMBIGUOUSFUNCTION";
        if (_type == BlockType::FUNCTION) return "FUNCTION";
        return "ERROR";
    }

    [[nodiscard]] ISA::Instructions* instructions() const { return _instructions; }

    [[nodiscard]] std::string toString() const override {
        return "Block<id:" + _id + ", cpy:" + std::to_string(_copy) 
                + ", idx:" + std::to_string(_idx) + ">"; 
    }

    [[nodiscard]] CallEdge* getCallInEdge() const;
    [[nodiscard]] CallEdge* getCallOutEdge() const;
    [[nodiscard]] FallEdge* getFallInEdge() const ;
    [[nodiscard]] FallEdge* getFallOutEdge() const;
    [[nodiscard]] ReturnEdge* getRetInEdge() const;
    [[nodiscard]] ReturnEdge* getRetOutEdge() const;

    void setCallInEdge(CallEdge* edge);
    void setCallOutEdge(CallEdge* edge);
    void setFallInEdge(FallEdge* edge);
    void setFallOutEdge(FallEdge* edge);
    void setRetInEdge(ReturnEdge* edge);
    void setRetOutEdge(ReturnEdge* edge);

    /* DOES NOT COPY EDGES */
    [[nodiscard]] virtual Block* copy(size_t idx) const {
        auto copy = new Block(_id, _type, idx);
        copy->_copy = _copy + 1;
        for ( auto i : *_instructions ) {
            copy->addInstruction(i->copy());
        }
        return copy;
    }
    
    /* Returns the dot file lines for the block and its outgoing edges */
    [[nodiscard]] virtual std::string serialize() const;
protected:
    std::string _id;
    size_t _copy;
    size_t _idx;
    BlockType _type;
    ISA::Instructions* _instructions;
    CallEdge* _callInEdge = nullptr, * _callOutEdge = nullptr;
    FallEdge* _fallInEdge = nullptr, * _fallOutEdge = nullptr;
    ReturnEdge* _retInEdge = nullptr, * _retOutEdge = nullptr;

};

class AmbiguousFunctionBlock : public Block {
public:
    AmbiguousFunctionBlock(std::string id, size_t i) : Block(id, BlockType::AMBIGUOUSFUNCTION, i) {}

    [[nodiscard]] std::string toString() const override { return "AmbiguousFunction" + Block::toString(); }

    [[nodiscard]] std::string serialize() const override;

    [[nodiscard]] AmbiguousFunctionBlock* copy(size_t idx) const override {
        auto copy = new AmbiguousFunctionBlock(_id, idx);
        copy->_copy = true;
        return copy;
    }
};

class CFGFunction : public IStringable {
public:
    CFGFunction(std::string id, Block* start) : _id(std::move(id)), _start(useref(start)), _end(nullptr), _blocks(new std::vector<Block*>()) {
        _blocks->push_back(_start);
    }

    ~CFGFunction() {
        for (auto b : *_blocks) freeref(b);
        delete _blocks;
        freeref(_end);
    }

    [[nodiscard]] std::string id() const { return _id; }
    [[nodiscard]] Block* start() const { return _start; }
    [[nodiscard]] Block* end() const { return _end; }
    void setEnd(Block* end) { 
        assert(_end == nullptr);
        _end = useref(end);
    }
    void addBlock(Block* block) { _blocks->push_back(useref(block)); }

    [[nodiscard]] std::string toString() const override { return "CFGFunction<" + _id + ">"; }

    /* Returns a deep copy of the set of blocks and edges that make up this function */
    std::pair<Block*,Block*> makeCopy(size_t i, std::vector<Block*>* blocks, std::stack<CFGFunction*>* callStack) const;

private:
    std::string _id;

    Block* _start;
    Block* _end;

    std::vector<Block*>* _blocks;
};

class Edge : public IStringable, public IRefCountable {
public:
    enum class EdgeType {
        CALL, FALL, RETURN
    };

    Edge(Block* source, Block* dest, EdgeType label) : _source(useref(source)), _destination(useref(dest)), _label(std::move(label)) {}
    ~Edge() { freeref(_source); freeref(_destination); }

    [[nodiscard]] Block* source() const { return _source; }

    [[nodiscard]] Block* destination() const { return _destination; }

    [[nodiscard]] EdgeType label() const { return _label; }

    [[nodiscard]] std::string labelString() const {
        if (_label == EdgeType::CALL) return "call";
        if (_label == EdgeType::FALL) return "fall";
        if (_label == EdgeType::RETURN) return "return";
        return "ERROR";
    }

    [[nodiscard]] std::string serialize() const {
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
    [[nodiscard]] std::string toString() const override {
        return "CallEdge<" + _source->toString() + "," + _destination->toString()+ ">";
    }
};

class FallEdge : public Edge {
public:
    FallEdge(Block* source, Block* dest) : Edge(source, dest, Edge::EdgeType::FALL) {}
    [[nodiscard]] std::string toString() const override {
        return "FallEdge<" + _source->toString() + "," + _destination->toString()+ ">";
    }
};

class ReturnEdge : public Edge {
public:
    ReturnEdge(Block* source, Block* dest) : Edge(source, dest, Edge::EdgeType::RETURN) {}
    [[nodiscard]] std::string toString() const override {
        return "ReturnEdge<" + _source->toString() + "," + _destination->toString()+ ">";
    }
};

class ControlFlowGraph {
public:
    explicit ControlFlowGraph(ISA::Instructions*);
    
    ~ControlFlowGraph() {
        for ( auto b : *_blocks ) freeref(b);
        delete _blocks;
        for ( const auto& p : *_nameMap ) delete p.second;
        delete _nameMap;
    }
    
    // make a dot file :)
    void serialize(std::ostream&) const;

    /* Rebuilds a linear ISA with optimizations */
    [[nodiscard]] ISA::Instructions* reconstruct() const;

    /* Calls different optimization walks until a fixpoint is reached */
    bool optimize(bool rSelfAssign, bool litProp);

    [[nodiscard]] Block* first() const { return _first; }
    [[nodiscard]] Block* last() const { return _last; }
    [[nodiscard]] std::vector<Block*>* blocks() const { return _blocks; }
    [[nodiscard]] std::unordered_map<std::string, CFGFunction*>* getNameMap() const { return _nameMap; }
private:
    Block* _first = nullptr;
    Block* _last = nullptr;

    ISA::Instructions* _instrs;
    std::vector<Block*>* _blocks;
    std::unordered_map<std::string, CFGFunction*>* _nameMap;

    ISA::Instructions* reconstruct(Block*, size_t) const;
};

class CFGBuild : public IUsesConsole {
public:
    explicit CFGBuild(ISA::Instructions* instrs) : _instrs(instrs), _blocks(new std::vector<Block*>()),
        _nameMap(new std::unordered_map<std::string, CFGFunction*>()) {
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

#endif