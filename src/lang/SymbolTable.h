#ifndef SWARMC_SYMBOLTABLE_H
#define SWARMC_SYMBOLTABLE_H

#include <string>
#include <iostream>
#include <list>
#include <stdexcept>
#include "../shared/util/Console.h"
#include "../shared/IStringable.h"
#include "../shared/uuid.h"
#include "Position.h"
#include "Type.h"

namespace swarmc {
namespace Lang {
namespace Walk {
    class TypeAnalysisWalk;
    class DeSerializeWalk;
}

    class WithStatement;

    /** Enum of the various possible things that a name may reference. */
    enum class SemanticSymbolKind {
        VARIABLE,
        FUNCTION,
    };


    /** Base class for names identified in code. */
    class SemanticSymbol : public IStringable {
    public:
        SemanticSymbol(std::string name, const Type* type, const Position* declaredAt) : _name(name), _type(type), _declaredAt(declaredAt) {
            _uuid = util::uuid4();
        }

        virtual std::string toString() const {
            return "SemanticSymbol<name: " + _name + ", type: " + _type->toString() + ", declaredAt: " + _declaredAt->start() + ", uuid: " + _uuid + ">";
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

        /** Get a universally-unique ID for this symbol. */
        virtual std::string uuid() const {
            return _uuid;
        }

        /** Get the shared flag of this symbol */
        virtual bool shared() const {
            return _type->shared();
        }

        virtual bool isPrologue() const {
            return false;
        }

    protected:
        std::string _uuid;
        std::string _name;
        const Type* _type;
        const Position* _declaredAt;

        friend class Walk::TypeAnalysisWalk;
        friend class Walk::DeSerializeWalk;
    };


    /** Semantic symbol implementation for names referencing variables. */
    class VariableSymbol : public SemanticSymbol {
    public:
        VariableSymbol(std::string name, const Type* type, const Position* declaredAt) : SemanticSymbol(name, type, declaredAt) {}

        virtual SemanticSymbolKind kind() const override {
            return SemanticSymbolKind::VARIABLE;
        }
    };


    /** Semantic symbol implementation for names referencing variables. */
    class FunctionSymbol : public SemanticSymbol {
    public:
        FunctionSymbol(std::string name, const FunctionType* type, const Position* declaredAt) : SemanticSymbol(name, type, declaredAt) {}

        virtual SemanticSymbolKind kind() const override {
            return SemanticSymbolKind::FUNCTION;
        }
    };


    /** Semantic symbol implementation for names referencing variables. */
    class PrologueFunctionSymbol : public FunctionSymbol {
    public:
        PrologueFunctionSymbol(std::string name, const FunctionType* type, const Position* declaredAt) : FunctionSymbol(name, type, declaredAt) {}

        virtual bool isPrologue() const override { return true; }
    };


    /** Partial symbol table for a specific scope. */
    class ScopeTable : public IStringable {
    public:
        static ScopeTable* prologue();

        ScopeTable() {
            _symbols = new std::map<std::string, SemanticSymbol*>();
        }

        /** Try to find a symbol in scope by name. Returns nullptr if none exists. */
        SemanticSymbol* lookup(const std::string& name) {
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

        /** Add a new function to this scope. */
        void addFunction(std::string name, FunctionType* type, const Position* declaredAt) {
            insert(new FunctionSymbol(name, type, declaredAt));
        }

        /** Add a new function to this scope as if it were from the Prologue. */
        void addPrologueFunction(std::string name, FunctionType* type, const Position* declaredAt) {
            insert(new PrologueFunctionSymbol(name, type, declaredAt));
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
            _scopes->push_back(ScopeTable::prologue());
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

            delete _scopes->front();
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

            Console::get()->debug("Unable to find symbol in table for identifier: " + name);
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

        /** Add a new function to the current scope. */
        void addFunction(std::string name, FunctionType* type, const Position* declaredAt) {
            return current()->addFunction(name, type, declaredAt);
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
