#include <string>
#include <ostream>
#include <assert.h>
#include "AST.h"
#include "../runtime/ISymbolValueStore.h"
#include "SymbolTable.h"
#include "../Reporting.h"
#include "Type.h"
#include "Walk/PrintWalk.h"
#include "../runtime/InterpretWalk.h"

#ifndef SWARMC_SPACE
#define SWARMC_SPACE "  "
#endif

namespace swarmc {
namespace Lang {

    ProgramNode* ProgramNode::copy() const  {
        auto other = new ProgramNode;
        for ( auto stmt : *_body ) {
            other->pushStatement(stmt->copy());
        }

        return other;
    }

    /************* VALUE ACCESSORS ********/
    void IdentifierNode::setValue(Runtime::ISymbolValueStore* store, ExpressionNode* value) {
        assert(value->isValue());
        store->setValue(_symbol, value);
    }

    ExpressionNode* IdentifierNode::getValue(Runtime::ISymbolValueStore* store) {
        auto value = store->getValue(_symbol);
        assert(value == nullptr || value->isValue());
        return value;
    }

    void MapAccessNode::setValue(Runtime::ISymbolValueStore* store, ExpressionNode* value) {
        // Get the lval we are keying into
        auto node = _path->getValue(store);

        // This should be caught by the type-checker, but anyway:
        assert(node->isValue() && node->getName() == "MapNode");
        assert(value->isValue());

        auto mapNode = (MapNode*) node;
        mapNode->setKey(_end, value);
    }

    ExpressionNode* MapAccessNode::getValue(Runtime::ISymbolValueStore* store) {
        // Get the lval we are keying into
        auto node = _path->getValue(store);

        // This should be caught by the type-checker, but anyway
        assert(node->isValue() && node->getName() == "MapNode");

        auto mapNode = (MapNode*) node;
        return mapNode->getKey(_end);
    }

    SemanticSymbol* MapAccessNode::lockable() const {
        return _path->lockable();
    }

    void EnumerableAccessNode::setValue(Runtime::ISymbolValueStore* store, ExpressionNode* value) {
        // Get the lval we are keying into
        auto node = _path->getValue(store);

        // This should be caught by the type-checker, but anyway
        assert(node->isValue() && node->getName() == "EnumerationLiteralExpressionNode");
        assert(value->isValue());

        auto enumNode = (EnumerationLiteralExpressionNode*) node;

        size_t idx = _index->value();  // fixme invalid cast?
        enumNode->setIndex(idx, value);  // todo handle exception here
    }

    ExpressionNode* EnumerableAccessNode::getValue(Runtime::ISymbolValueStore* store) {
        // Get the lval we are keying into
        auto node = _path->getValue(store);

        // This should be caught by the type-checker, but anyway
        assert(node->isValue() && node->getName() == "EnumerationLiteralExpressionNode");

        auto enumNode = (EnumerationLiteralExpressionNode*) node;
        size_t idx = _index->value();  // fixme invalid cast?
        return enumNode->getIndex(idx);
    }

    SemanticSymbol* EnumerableAccessNode::lockable() const {
        return _path->lockable();
    }

    TypeNode* TypeNode::newForType(Type* type) {
        if ( type->isPrimitiveType() ) {
            return new PrimitiveTypeNode(new ProloguePosition("VirtualType"), (PrimitiveType*) type);
        }

        if ( type->isGenericType() ) {
            auto genericType = (GenericType*) type;

            if ( type->valueType() == ValueType::TMAP ) {
                return new MapTypeNode(new ProloguePosition("VirtualType"), newForType(genericType->concrete()));
            } else if ( type->valueType() == ValueType::TENUMERABLE ) {
                return new EnumerableTypeNode(new ProloguePosition("VirtualType"), newForType(genericType->concrete()));
            }
        }

        throw Errors::SwarmError("Unable to reconstruct type node for type: " + type->toString());
    }

    void TagResourceNode::open(Runtime::InterpretWalk* interpreter) {
        auto previousResult = interpreter->_local->_filters.find(_key);
        _hasPrevious = previousResult != interpreter->_local->_filters.end();
        _previous = _hasPrevious ? previousResult->second : "";

        interpreter->_local->_filters[_key] = _value;
        _opened = true;
    }

    void TagResourceNode::close(Runtime::InterpretWalk* interpreter) {
        if ( _hasPrevious ) {
            interpreter->_local->_filters[_key] = _previous;
        } else {
            interpreter->_local->_filters.erase(_key);
        }

        _hasPrevious = false;
        _previous = "";
        _opened = false;
    }

}
}
