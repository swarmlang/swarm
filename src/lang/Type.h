#ifndef SWARMC_TYPE_H
#define SWARMC_TYPE_H

#include <map>
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
    };

    /** Base class for type instances. */
    class Type : public IStringable {
    public:
        /** Returns true if the given ValueType is a primitive type. */
        static bool isPrimitiveValueType(ValueType t);

        /** Get the string representation of the given ValueType. */
        static std::string valueTypeToString(ValueType t);

        /** Implements IStringable. */
        virtual std::string toString() const = 0;

        /** Returns true if this type is of the given ValueType. */
        virtual bool isValueType(ValueType t) { return false; }
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
            return "PrimitiveType<t: " + Type::valueTypeToString(_type) + ">";
        }

        /** Returns true if this type is of the given ValueType. */
        virtual bool isValueType(ValueType t) override {
            return t == _type;
        }

        /** Get the ValueType of this class. */
        virtual ValueType valueType() const {
            return _type;
        }
    private:
        ValueType _type;
        PrimitiveType(ValueType t) : _type(t) {
            if ( !Type::isPrimitiveValueType(t) ) {
                throw Errors::InvalidPrimitiveTypeInstantiationError();
            }
        }
    };
}
}

#endif
