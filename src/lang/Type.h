#ifndef SWARMC_TYPE_H
#define SWARMC_TYPE_H

#include <map>
#include <sstream>
#include <vector>
#include "../shared/IStringable.h"
#include "../errors/InvalidPrimitiveTypeInstantiation.h"

namespace swarmc {
namespace Lang {
    class TypeLiteral;
}

namespace Type {

    enum class Intrinsic {
        STRING,
        NUMBER,
        BOOLEAN,
        ERROR,
        VOID,
        UNIT,
        TYPE,
        MAP,
        ENUMERABLE,
        LAMBDA0,
        LAMBDA1,
        RESOURCE,
        AMBIGUOUS,
        CONTRADICTION,
    };

    class Type : public IStringable {
    public:
        static std::string intrinsicString(Intrinsic intrinsic) {
            if ( intrinsic == Intrinsic::STRING ) return "STRING";
            if ( intrinsic == Intrinsic::NUMBER ) return "NUMBER";
            if ( intrinsic == Intrinsic::BOOLEAN ) return "BOOLEAN";
            if ( intrinsic == Intrinsic::ERROR ) return "ERROR";
            if ( intrinsic == Intrinsic::VOID ) return "VOID";
            if ( intrinsic == Intrinsic::UNIT ) return "UNIT";
            if ( intrinsic == Intrinsic::TYPE ) return "TYPE";
            if ( intrinsic == Intrinsic::MAP ) return "MAP";
            if ( intrinsic == Intrinsic::ENUMERABLE ) return "ENUMERABLE";
            if ( intrinsic == Intrinsic::LAMBDA0 ) return "LAMBDA0";
            if ( intrinsic == Intrinsic::LAMBDA1 ) return "LAMBDA1";
            if ( intrinsic == Intrinsic::RESOURCE ) return "RESOURCE";
            if ( intrinsic == Intrinsic::AMBIGUOUS ) return "AMBIGUOUS";
            return "CONTRADICTION";
        }

        ~Type() override = default;

        virtual Type* copy() const = 0;

        virtual Intrinsic intrinsic() const {
            return Intrinsic::CONTRADICTION;
        }

        bool isCallable() const {
            return intrinsic() == Intrinsic::LAMBDA0 || intrinsic() == Intrinsic::LAMBDA1;
        }

        bool isIntrinsic() const {
            return intrinsic() != Intrinsic::CONTRADICTION;
        }

        virtual bool shared() const {
            return _shared;
        }

        virtual bool isAssignableTo(const Type* other) const = 0;

    protected:
        bool _shared = false;

        friend class Lang::TypeLiteral;
    };

    class Primitive : public Type {
    public:
        static Primitive* of(Intrinsic intrinsic) {
            return new Primitive(intrinsic);
        }

        static bool isPrimitive(Intrinsic intrinsic) {
            return (
                intrinsic == Intrinsic::STRING
                || intrinsic == Intrinsic::NUMBER
                || intrinsic == Intrinsic::BOOLEAN
                || intrinsic == Intrinsic::ERROR
                || intrinsic == Intrinsic::VOID
                || intrinsic == Intrinsic::UNIT
                || intrinsic == Intrinsic::TYPE
            );
        }

        explicit Primitive(Intrinsic intrinsic) : _intrinsic(intrinsic) {}

        Primitive* copy() const override {
            auto inst = Primitive::of(_intrinsic);
            inst->_shared = shared();
            return inst;
        }

        Intrinsic intrinsic() const override {
            if ( !isPrimitive(_intrinsic) ) {
                return Intrinsic::CONTRADICTION;
            }

            return _intrinsic;
        }

        bool isAssignableTo(const Type* other) const override {
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( !Primitive::isPrimitive(other->intrinsic()) ) return false;
            return other->intrinsic() == intrinsic();
        }

        std::string toString() const override {
            return "Primitive<" + intrinsicString(_intrinsic) + ">";
        }
    protected:
        Intrinsic _intrinsic;
    };

    class Ambiguous : public Type {
    public:
        static Ambiguous* of() {
            return new Ambiguous();
        }

        Intrinsic intrinsic() const override {
            return Intrinsic::AMBIGUOUS;
        }

        std::string toString() const override {
            return Type::intrinsicString(intrinsic());
        }

        bool isAssignableTo(const Type* other) const override {
            return true;
        }

        Ambiguous* copy() const override {
            return Ambiguous::of();
        }
    };

    class Map : public Type {
    public:
        explicit Map(const Type* values) : _values(values) {}

        Intrinsic intrinsic() const override {
            return Intrinsic::MAP;
        }

        const Type* values() const {
            return _values;
        }

        Map* copy() const override {
            auto inst = new Map(_values->copy());
            inst->_shared = shared();
            return inst;
        }

        bool isAssignableTo(const Type* other) const override {
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::MAP ) return false;
            return _values->isAssignableTo(((Map*) other)->values());
        }

        std::string toString() const override {
            return Type::intrinsicString(intrinsic()) + "<" + _values->toString() + ">";
        }
    protected:
        const Type* _values;
    };

    class Enumerable : public Type {
    public:
        explicit Enumerable(const Type* values) : _values(values) {}

        Intrinsic intrinsic() const override {
            return Intrinsic::ENUMERABLE;
        }

        const Type* values() const {
            return _values;
        }

        Enumerable* copy() const override {
            auto inst = new Enumerable(_values->copy());
            inst->_shared = shared();
            return inst;
        }

        bool isAssignableTo(const Type* other) const override {
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::ENUMERABLE ) return false;
            return _values->isAssignableTo(((Enumerable*) other)->values());
        }

        std::string toString() const override {
            return Type::intrinsicString(intrinsic()) + "<" + _values->toString() + ">";
        }
    protected:
        const Type* _values;
    };

    class Resource : public Type {
    public:
        explicit Resource(Type* yields) : _yields(yields) {}

        Intrinsic intrinsic() const override {
            return Intrinsic::RESOURCE;
        }

        const Type* yields() const {
            return _yields;
        }

        Resource* copy() const override {
            auto inst = new Resource(_yields->copy());
            inst->_shared = shared();
            return inst;
        }

        bool isAssignableTo(const Type* other) const override {
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::RESOURCE ) return false;
            return _yields->isAssignableTo(((Resource*) other)->yields());
        }

        std::string toString() const override {
            return Type::intrinsicString(intrinsic()) + "<" + _yields->toString() + ">";
        }
    protected:
        Type* _yields;
    };

    class Lambda : public Type {
    public:
        explicit Lambda(Type* returns): _returns(returns) {}

        const Type* returns() const {
            return _returns;
        }

        virtual std::vector<const Type*> params() const {
            return {};
        }

    protected:
        Type* _returns;
    };

    class Lambda0 : public Lambda {
    public:
        explicit Lambda0(Type* returns) : Lambda(returns) {}

        Intrinsic intrinsic() const override {
            return Intrinsic::LAMBDA0;
        }

        Lambda0* copy() const override {
            auto inst = new Lambda0(_returns->copy());
            inst->_shared = shared();
            return inst;
        }

        bool isAssignableTo(const Type* other) const {
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::LAMBDA0 ) return false;
            return _returns->isAssignableTo(((Lambda0*) other)->returns());
        }

        std::string toString() const override {
            return ":: " + _returns->toString();
        }
    };

    class Lambda1 : public Lambda {
    public:
        explicit Lambda1(Type* param, Type* returns) : Lambda(returns), _param(param) {}

        Intrinsic intrinsic() const override {
            return Intrinsic::LAMBDA1;
        }

        const Type* param() const {
            return _param;
        }

        Lambda1* copy() const override {
            auto inst = new Lambda1(_param->copy(), _returns->copy());
            inst->_shared = shared();
            return inst;
        }

        bool isAssignableTo(const Type* other) const {
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::LAMBDA1 ) return false;
            return _returns->isAssignableTo(((Lambda1*) other)->returns()) && _param->isAssignableTo(((Lambda1*) other)->param());
        }

        std::string toString() const override {
            return _param->toString() + " :: " + _returns->toString();
        }

        std::vector<const Type*> params() const override {
            std::vector<const Type*> p = {_param};

            if ( returns()->intrinsic() == Intrinsic::LAMBDA1 ) {
                auto curried = ((Lambda1*) returns())->params();
                for ( auto subp : curried ) {
                    p.push_back(subp);
                }
            }

            return p;
        }

    protected:
        Type* _param;
    };
}
}

#endif
