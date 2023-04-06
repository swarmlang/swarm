#ifndef SWARMC_SYMBOLTABLE_H
#define SWARMC_SYMBOLTABLE_H

#include <string>
#include <iostream>
#include <list>
#include <utility>
#include "../shared/nslib.h"
#include "Position.h"
#include "Type.h"
#include "../Reporting.h"

using namespace nslib;

namespace swarmc::Lang {
namespace Walk {
    class TypeAnalysisWalk;
    class DeSerializeWalk;
}

    class WithStatement;
    class ExpressionNode;

    /** Enum of the various possible things that a name may reference. */
    enum class SemanticSymbolKind {
        VARIABLE,
        FUNCTION,
    };


    /** Base class for names identified in code. */
    class SemanticSymbol : public IStringable, public IRefCountable {
    public:
        SemanticSymbol(std::string name, Type::Type* type, Position* declaredAt, bool shared) : _name(std::move(name)), _type(useref(type)), _declaredAt(useref(declaredAt)), _shared(shared) {
            _uuid = nslib::uuid();
        }

        ~SemanticSymbol() {
            freeref(_declaredAt);
            freeref(_type);
        }

        [[nodiscard]] std::string toString() const override {
            return "SemanticSymbol<name: " + _name + ", type: " + ((_type)?_type->toString():"(nullptr)") + ", declaredAt: " + _declaredAt->start() + ", uuid: " + _uuid + ">";
        }

        /** The user-given name of the symbol. */
        [[nodiscard]] virtual std::string name() const {
            return _name;
        }

        /** The kind of the symbol. */
        [[nodiscard]] virtual SemanticSymbolKind kind() const = 0;

        /** The type of the symbol. */
        [[nodiscard]] virtual Type::Type* type() const {
            return _type;
        }

        /** Get the position where this symbol was declared. */
        [[nodiscard]] virtual Position* declaredAt() const {
            return _declaredAt;
        }

        /** Get a universally-unique ID for this symbol. */
        [[nodiscard]] virtual std::string uuid() const {
            return _uuid;
        }

        /** Get the shared flag of this symbol */
        [[nodiscard]] virtual bool shared() const {
            return _shared;
        }

        [[nodiscard]] virtual bool isPrologue() const {
            return false;
        }

        [[nodiscard]] virtual bool isProperty() const { 
            return false; 
        }

    protected:
        std::string _uuid;
        std::string _name;
        Type::Type* _type;
        Position* _declaredAt;
        bool _shared;

        friend class Walk::TypeAnalysisWalk;
        friend class Walk::DeSerializeWalk;
    };


    /** Semantic symbol implementation for names referencing variables. */
    class VariableSymbol : public SemanticSymbol {
    public:
        VariableSymbol(std::string name, Type::Type* type, Position* declaredAt, bool shared) : SemanticSymbol(std::move(name), type, declaredAt, std::move(shared)), _value(nullptr) {}

        ~VariableSymbol() {
            freeref(_value);
        }

        [[nodiscard]] SemanticSymbolKind kind() const override {
            return SemanticSymbolKind::VARIABLE;
        }

        TypeLiteral* getObjectType() const {
            return _value;
        }

        void setObjectType(TypeLiteral* type);

        void disambiguateType();

    protected:
        // used for type assignments
        TypeLiteral* _value;
    };

    class ObjectPropertySymbol : public VariableSymbol {
    public:
        ObjectPropertySymbol(std::string name, Type::Type* type, Position* declaredAt, Type::Type* propertyOf) : VariableSymbol(std::move(name), type, declaredAt, false), _propertyOf(useref(propertyOf)) {}
        
        ~ObjectPropertySymbol() {
            freeref(_propertyOf);
        }

        [[nodiscard]] bool isProperty() const override { return true; }

        Type::Type* propertyOf() const {
            return _propertyOf;
        }
    protected:
        Type::Type* _propertyOf;
    };


    /** Semantic symbol implementation for names referencing variables. */
    class FunctionSymbol : public SemanticSymbol {
    public:
        FunctionSymbol(std::string name, Type::Lambda* type, Position* declaredAt, bool shared) : SemanticSymbol(std::move(name), type, declaredAt, std::move(shared)) {}

        [[nodiscard]] SemanticSymbolKind kind() const override {
            return SemanticSymbolKind::FUNCTION;
        }
    };


    /** Semantic symbol implementation for names referencing variables. */
    class PrologueFunctionSymbol : public FunctionSymbol {
    public:
        PrologueFunctionSymbol(std::string name, Type::Lambda* type, Position* declaredAt, std::string sviName) : FunctionSymbol(std::move(name), type, declaredAt, false), _sviName(sviName) {}

        [[nodiscard]] bool isPrologue() const override { return true; }

        std::string sviName() const {
            return _sviName;
        }
    protected:
        // TODO: remove once import-based prologue is implemented
        std::string _sviName;
    };


    /** Partial symbol table for a specific scope. */
    class ScopeTable : public IStringable {
    public:
        static ScopeTable* prologue();

        ScopeTable() {
            _symbols = new std::map<std::string, SemanticSymbol*>();
        }

        ~ScopeTable() override {
            for (auto p : *_symbols) {
                freeref(p.second);
            }
            delete _symbols;
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

            _symbols->insert(std::make_pair(name, useref(symbol)));
            return true;
        }

        /** Returns true if this scope already has the given name. */
        bool isClashing(const std::string& name) {
            SemanticSymbol* found = lookup(name);
            return found != nullptr;
        }

        /** Add a new variable to this scope. */
        void addVariable(std::string name, Type::Type* type, Position* declaredAt, bool shared) {
            insert(new VariableSymbol(std::move(name), type, declaredAt, std::move(shared)));
        }

        void addObjectProperty(std::string name, Type::Type* type, Position* declaredAt, Type::Type* propertyOf) {
            insert(new ObjectPropertySymbol(std::move(name), type, declaredAt, propertyOf));
        }

        /** Add a new function to this scope. */
        void addFunction(std::string name, Type::Lambda* type, Position* declaredAt, bool shared) {
            insert(new FunctionSymbol(std::move(name), type, declaredAt, std::move(shared)));
        }

        /** Add a new function to this scope as if it were from the Prologue. */
        void addPrologueFunction(std::string name, Type::Lambda* type, Position* declaredAt, std::string sviName) {
            insert(new PrologueFunctionSymbol(std::move(name), type, declaredAt, std::move(sviName)));
        }

        [[nodiscard]] std::string toString() const override {
            return "ScopeTable<#symbols: " + std::to_string(_symbols->size()) + " >";
        }

        /** Print all names in the current scope. */
        void print(std::ostream& out) const {
            for ( const auto& symbol : *_symbols ) {
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

        ~SymbolTable() override {
            for ( auto scope : *_scopes ) delete scope;
            delete _scopes;
        }

        /** Create a new scope. */
        ScopeTable* enter() {
            auto scope = new ScopeTable();
            _scopes->push_front(scope);
            return scope;
        }

        /** Leave the current scope. */
        void leave() {
            if ( _scopes->empty() ) {
                throw std::runtime_error("Attempted to pop non-existent scope.");
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
        SemanticSymbol* lookup(const std::string& name) {
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
        bool isClashing(const std::string& name) {
            return current()->isClashing(name);
        }

        /** Add a new variable to the current scope. */
        void addVariable(std::string name, Type::Type* type, Position* declaredAt, bool shared) {
            return current()->addVariable(std::move(name), type, declaredAt, std::move(shared));
        }

        void addObjectProperty(std::string name, Type::Type* type, Position* declaredAt, Type::Type* propertyOf) {
            return current()->addObjectProperty(std::move(name), type, declaredAt, propertyOf);
        }

        /** Add a new function to the current scope. */
        void addFunction(std::string name, Type::Lambda* type, Position* declaredAt, bool shared) {
            return current()->addFunction(std::move(name), type, declaredAt, std::move(shared));
        }

        [[nodiscard]] std::string toString() const override {
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

#endif
