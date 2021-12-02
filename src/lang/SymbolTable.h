#ifndef SWARMC_SYMBOLTABLE_H
#define SWARMC_SYMBOLTABLE_H

#include <string>
#include <iostream>
#include <list>
#include <stdexcept>
#include "../shared/IStringable.h"
#include "Position.h"
#include "Type.h"

namespace swarmc {
namespace Lang {

    /** Enum of the various possible things that a name may reference. */
    enum SemanticSymbolKind {
        VARIABLE,
        FUNCTION,
    };


    /** Base class for names identified in code. */
    class SemanticSymbol : public IStringable {
    public:
        SemanticSymbol(std::string name, const Type* type, const Position* declaredAt) : _name(name), _type(type), _declaredAt(declaredAt) {}

        virtual std::string toString() const {
            return "SemanticSymbol<name: " + _name + ", type: " + _type->toString() + ", declaredAt: " + _declaredAt->start() + ">";
        }

        /** The user-given name of the symbol. */
        virtual std::string name() const {
            return _name;
        }

        /** The kind of the symbol. */
        virtual SemanticSymbolKind kind() const = 0;

        /** The type of the symbol. */
        virtual const Type* type() const {
            return _type;
        }

        /** Get the position where this symbol was declared. */
        virtual const Position* declaredAt() const {
            return _declaredAt;
        }

    protected:
        std::string _name;
        const Type* _type;
        const Position* _declaredAt;
    };


    /** Semantic symbol implementation for names referencing variables. */
    class VariableSymbol : public SemanticSymbol {
    public:
        VariableSymbol(std::string name, const Type* type, const Position* declaredAt) : SemanticSymbol(name, type, declaredAt) {}

        virtual SemanticSymbolKind kind() const override {
            return VARIABLE;
        }
    };


    /** Semantic symbol implementation for names referencing variables. */
    class FunctionSymbol : public SemanticSymbol {
    public:
        FunctionSymbol(std::string name, const FunctionType* type, const Position* declaredAt) : SemanticSymbol(name, type, declaredAt) {}

        virtual SemanticSymbolKind kind() const override {
            return FUNCTION;
        }
    };


    /** Partial symbol table for a specific scope. */
    class ScopeTable : public IStringable {
    public:
        ScopeTable() {
            _symbols = new std::map<std::string, SemanticSymbol*>();
        }

        /** Try to find a symbol in scope by name. Returns nullptr if none exists. */
        SemanticSymbol* lookup(std::string name) {
            auto found = _symbols->find(name);
            if ( found == _symbols->end() ) {
                return nullptr;
            }

            return found->second;
        }

        /** Add a new symbol to this scope. */
        bool insert(SemanticSymbol* symbol) {
            std::string name = symbol->name();
            if ( isClashing(name) ) {
                return false;  // FIXME: generate exception
            }

            _symbols->insert(std::make_pair(name, symbol));
            return true;
        }

        /** Returns true if this scope already has the given name. */
        bool isClashing(std::string name) {
            SemanticSymbol* found = lookup(name);
            return found != nullptr;
        }

        /** Add a new variable to this scope. */
        void addVariable(std::string name, Type* type, const Position* declaredAt) {
            insert(new VariableSymbol(name, type, declaredAt));
        }

        virtual std::string toString() const {
            return "ScopeTable<#symbols: " + std::to_string(_symbols->size()) + " >";
        }

        /** Print all names in the current scope. */
        void print(std::ostream& out) const {
            for ( auto symbol : *_symbols ) {
                out << symbol.second->toString();
                out << std::endl;
            }
        }

    protected:
        std::map<std::string, SemanticSymbol*>* _symbols;
    };


    /** Linked-list of scopes for tracking symbols in the program. */
    class SymbolTable : public IStringable {
    public:
        SymbolTable() {
            _scopes = new std::list<ScopeTable*>();
        }

        /** Create a new scope. */
        ScopeTable* enter() {
            ScopeTable* scope = new ScopeTable();
            _scopes->push_front(scope);
            return scope;
        }

        /** Leave the current scope. */
        void leave() {
            if ( _scopes->empty() ) {
                throw new std::runtime_error("Attempted to pop non-existent scope.");
            }

            _scopes->pop_front();
        }

        /** Get the current scope. */
        ScopeTable* current() {
            return _scopes->front();
        }

        /** Add a new symbol to the current scope. */
        bool insert(SemanticSymbol* symbol) {
            return current()->insert(symbol);
        }

        /** Find a symbol by name, recursing up the scope list. */
        SemanticSymbol* lookup(std::string name) {
            for ( ScopeTable* scope : *_scopes ) {
                SemanticSymbol* symbol = scope->lookup(name);
                if ( symbol != nullptr ) {
                    return symbol;
                }
            }

            return nullptr;
        }

        /** Returns true if the current scope already has a symbol with the given name. */
        bool isClashing(std::string name) {
            return current()->isClashing(name);
        }

        /** Add a new variable to the current scope. */
        void addVariable(std::string name, Type* type, const Position* declaredAt) {
            return current()->addVariable(name, type, declaredAt);
        }

        virtual std::string toString() const {
            return "SymbolTable<#scopes: " + std::to_string(_scopes->size()) + ">";
        }

        /** Print out the current scope stack and its contents. */
        virtual void print(std::ostream& out) {
            for ( auto scope : *_scopes ) {
                out << "========== SCOPE ==========" << std::endl;
                scope->print(out);
            }
        }

    protected:
        std::list<ScopeTable*>* _scopes;
    };

}
}

#endif
