#ifndef SWARMC_TYPE_H
#define SWARMC_TYPE_H

#include <map>
#include <sstream>
#include <vector>
#include "../shared/IStringable.h"
#include "../errors/InvalidPrimitiveTypeInstantiation.h"

namespace swarmc {
namespace Lang {

    /** All possible value types in the language. */
    enum ValueType {
        TSTRING = 0,    // strings
        TNUM,           // numbers
        TBOOL,          // booleans
        TENUMERABLE,    // enumeration lists
        TMAP,           // maps
        TERROR,         // special error type (internal use only)
        TFUNCTION,      // functions
        TUNIT,          // statement return type
        TRESOURCE,
    };

    /** All possible kinds of types. */
    enum TypeKind {
        KPRIMITIVE = 0, // primitive value
        KGENERIC,       // single-parametric type
        KFUNCTION,      // callable (...D)->R function
    };

    /** Base class for type instances. */
    class Type : public IStringable {
    public:
        virtual ~Type() {}

        /** Returns true if the given ValueType is a primitive type. */
        static bool isPrimitiveValueType(ValueType t);

        /** Get the string representation of the given ValueType. */
        static std::string valueTypeToString(ValueType t);

        /** Implements IStringable. */
        virtual std::string toString() const = 0;

        /** Determines whether a type is equivalent to this one. */
        virtual bool is(const Type* other) const = 0;

        /** Get the ValueType of this class. */
        virtual ValueType valueType() const {
            return _type;
        }

        /** Get the type-kind of this type. */
        virtual TypeKind kind() const = 0;

        virtual bool isPrimitiveType() const {
            return false;
        }

        virtual bool isGenericType() const {
            return false;
        }

        virtual bool isFunctionType() const {
            return false;
        }

    protected:
        Type(ValueType t) : _type(t) {}
        ValueType _type;
    };

    /** Type class of primitive types. */
    class PrimitiveType : public Type {
    public:

        /**
         * Get the PrimitiveType instance for a given ValueType.
         *
         * @example PrimitiveType::of(ValueType::TSTRING);
         */
        static PrimitiveType* of(ValueType t) {
            static std::map<ValueType, PrimitiveType*> instances;

            auto i = instances.find(t);
            if ( i != instances.end() ) {
                return i->second;
            }

            PrimitiveType* instance = new PrimitiveType(t);
            instances.insert(std::pair<ValueType, PrimitiveType*>(t, instance));
            return instance;
        }

        /** Implements IStringable. */
        virtual std::string toString() const override {
            return "T<" + Type::valueTypeToString(_type) + ">";
        }

        virtual bool is(const Type* other) const override {
            return (
                other->kind() == KPRIMITIVE
                && other->valueType() == _type
            );
        }

        virtual TypeKind kind() const override {
            return KPRIMITIVE;
        }

        virtual bool isPrimitiveType() const override {
            return true;
        }
    private:
        PrimitiveType(ValueType t) : Type(t) {
            if ( !Type::isPrimitiveValueType(t) ) {
                throw Errors::InvalidPrimitiveTypeInstantiationError();
            }
        }
    };


    /** Base class for types that accept another type as a parameter. */
    class GenericType : public Type {
    public:
        static GenericType* of(ValueType t, Type* concrete) {
            return new GenericType(t, concrete);
        }

        virtual std::string toString() const override {
            return "T<" + Type::valueTypeToString(_type) + "<" + _concrete->toString() + ">>";
        }

        virtual TypeKind kind() const override {
            return KGENERIC;
        }

        virtual bool is(const Type* other) const override {
            if ( other->kind() != KGENERIC ) {
                return false;
            }

            // We know this is a generic now.
            GenericType* otherGeneric = (GenericType*) other;
            return (
                otherGeneric->valueType() == _type
                && otherGeneric->_concrete->is(_concrete)
            );
        }

        /** Get the Type of the parameter type. */
        Type* concrete() const {
            return _concrete;
        }

        virtual bool isGenericType() const override {
            return true;
        }
    protected:
        GenericType(ValueType t, Type* concrete) : Type(t), _concrete(concrete) {}
        Type* _concrete;
    };


    /** Base class for callable function types with multiple domain types and a single range type. */
    class FunctionType : public Type {
    public:
        static FunctionType* of(Type* returnType) {
            return new FunctionType(returnType);
        }

        virtual std::string toString() const override {
            std::stringstream s;
            s << "T<(";
            bool first = false;

            for (auto arg : _args) {
                if ( !first ) {
                    s << ", ";
                } else {
                    first = false;
                }

                s << arg->toString();
            }

            s << ") -> " << _return->toString();
            return s.str();
        }

        virtual TypeKind kind() const override {
            return KFUNCTION;
        }

        virtual bool is(const Type* other) const override {
            if ( other->kind() != KFUNCTION ) {
                return false;
            }

            // We know this is a function type.
            FunctionType* otherFunction = (FunctionType*) other;

            // Check return type and arg count
            if (
                (otherFunction->_args.size() != _args.size())
                || !otherFunction->_return->is(_return)
            ) return false;

            // Check arg types
            for ( size_t i = 0; i < _args.size(); i += 1 ) {
                if ( !_args[i]->is(otherFunction->_args[i]) ) {
                    return false;
                }
            }

            return true;
        }

        /** Returns true if this type is provided by the runtime environment. */
        bool isBuiltin() const {
            return _builtin;
        }

        /** Add an argument to the function's signature. */
        void addArgument(Type* arg) {
            _args.push_back(arg);
        }

        const Type* returnType() const {
            return _return;
        }

        std::vector<Type*>* getArgumentTypes() const {
            std::vector<Type*>* types = new std::vector<Type*>();

            for ( auto type : _args ) {
                types->push_back(type);
            }

            return types;
        }

        virtual bool isFunctionType() const override {
            return true;
        }

    protected:
        FunctionType(Type* returnType) : Type(TFUNCTION), _return(returnType) {}
        std::vector<Type*> _args;
        Type* _return;
        bool _builtin = false;
    };
}
}

#endif
