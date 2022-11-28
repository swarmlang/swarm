#ifndef SWARMC_TYPE_H
#define SWARMC_TYPE_H

#include <map>
#include <sstream>
#include <vector>
#include "../shared/nslib.h"
#include "../errors/InvalidPrimitiveTypeInstantiation.h"

using namespace nslib;

namespace swarmc {
namespace Lang {
    class TypeLiteral;
}

namespace Type {
    class Type;
}

namespace Type {

    enum class Intrinsic: size_t {
        STRING,
        NUMBER,
        BOOLEAN,
        ERROR,
        VOID,
        UNIT,
        TYPE,
        MAP,
        ENUMERABLE,
        STREAM,
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
            if ( intrinsic == Intrinsic::STREAM ) return "STREAM";
            if ( intrinsic == Intrinsic::LAMBDA0 ) return "LAMBDA0";
            if ( intrinsic == Intrinsic::LAMBDA1 ) return "LAMBDA1";
            if ( intrinsic == Intrinsic::RESOURCE ) return "RESOURCE";
            if ( intrinsic == Intrinsic::AMBIGUOUS ) return "AMBIGUOUS";
            return "CONTRADICTION";
        }

        ~Type() override = default;

        [[nodiscard]] virtual Type* copy() const = 0;

        [[nodiscard]] virtual Type* copy(bool shared) const {
            auto e = copy();
            e->_shared = shared;
            return e;
        }

        [[nodiscard]] virtual Intrinsic intrinsic() const {
            return Intrinsic::CONTRADICTION;
        }

        [[nodiscard]] bool isCallable() const {
            return intrinsic() == Intrinsic::LAMBDA0 || intrinsic() == Intrinsic::LAMBDA1;
        }

        [[nodiscard]] bool isIntrinsic() const {
            return intrinsic() != Intrinsic::CONTRADICTION;
        }

        [[nodiscard]] bool isAmbiguous() const {
            return intrinsic() == Intrinsic::AMBIGUOUS;
        }

        [[nodiscard]] virtual bool shared() const {
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

        [[nodiscard]] Primitive* copy() const override {
            auto inst = Primitive::of(_intrinsic);
            inst->_shared = shared();
            return inst;
        }

        [[nodiscard]] Intrinsic intrinsic() const override {
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

        [[nodiscard]] std::string toString() const override {
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

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::AMBIGUOUS;
        }

        [[nodiscard]] std::string toString() const override {
            return Type::intrinsicString(intrinsic());
        }

        bool isAssignableTo(const Type* other) const override {
            return true;
        }

        [[nodiscard]] Ambiguous* copy() const override {
            return Ambiguous::of();
        }
    };

    class Map : public Type {
    public:
        explicit Map(const Type* values) : _values(values) {}

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::MAP;
        }

        [[nodiscard]] const Type* values() const {
            return _values;
        }

        [[nodiscard]] Map* copy() const override {
            auto inst = new Map(_values->copy());
            inst->_shared = shared();
            return inst;
        }

        bool isAssignableTo(const Type* other) const override {
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::MAP ) return false;
            return _values->isAssignableTo(((Map*) other)->values());
        }

        [[nodiscard]] std::string toString() const override {
            return Type::intrinsicString(intrinsic()) + "<" + _values->toString() + ">";
        }
    protected:
        const Type* _values;
    };

    class Enumerable : public Type {
    public:
        explicit Enumerable(const Type* values) : _values(values) {}

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::ENUMERABLE;
        }

        [[nodiscard]] const Type* values() const {
            return _values;
        }

        [[nodiscard]] Enumerable* copy() const override {
            auto inst = new Enumerable(_values->copy());
            inst->_shared = shared();
            return inst;
        }

        bool isAssignableTo(const Type* other) const override {
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::ENUMERABLE ) return false;
            return _values->isAssignableTo(((Enumerable*) other)->values());
        }

        [[nodiscard]] std::string toString() const override {
            return Type::intrinsicString(intrinsic()) + "<" + _values->toString() + ">";
        }
    protected:
        const Type* _values;
    };

    class Resource : public Type {
    public:
        static Resource* of(const Type* inner) {
            return new Resource(inner);
        }

        explicit Resource(const Type* yields) : _yields(yields) {}

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::RESOURCE;
        }

        [[nodiscard]] const Type* yields() const {
            return _yields;
        }

        [[nodiscard]] Resource* copy() const override {
            auto inst = new Resource(_yields);
            inst->_shared = shared();
            return inst;
        }

        bool isAssignableTo(const Type* other) const override {
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::RESOURCE ) return false;
            return _yields->isAssignableTo(((Resource*) other)->yields());
        }

        [[nodiscard]] std::string toString() const override {
            return Type::intrinsicString(intrinsic()) + "<" + _yields->toString() + ">";
        }
    protected:
        const Type* _yields;
    };

    class Stream : public Type {
    public:
        static Stream* of(const Type* inner) {
            return new Stream(inner);
        }

        explicit Stream(const Type* inner) : _inner(inner) {}

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::STREAM;
        }

        [[nodiscard]] const Type* inner() const {
            return _inner;
        }

        [[nodiscard]] Stream* copy() const override {
            auto inst = new Stream(_inner->copy());
            inst->_shared = shared();
            return inst;
        }

        bool isAssignableTo(const Type* other) const override {
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::STREAM ) return false;
            return _inner->isAssignableTo(((Stream*) other)->inner());
        }

        [[nodiscard]] std::string toString() const override {
            return Type::intrinsicString(intrinsic()) + "<" + _inner->toString() + ">";
        }

    protected:
        const Type* _inner;
    };

    class Lambda : public Type {
    public:
        explicit Lambda(const Type* returns): _returns(returns) {}

        [[nodiscard]] const Type* returns() const {
            return _returns;
        }

        [[nodiscard]] virtual std::vector<const Type*> params() const {
            return {};
        }

    protected:
        const Type* _returns;
    };

    class Lambda0 : public Lambda {
    public:
        explicit Lambda0(const Type* returns) : Lambda(returns) {}

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::LAMBDA0;
        }

        [[nodiscard]] Lambda0* copy() const override {
            auto inst = new Lambda0(_returns->copy());
            inst->_shared = shared();
            return inst;
        }

        bool isAssignableTo(const Type* other) const override {
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::LAMBDA0 ) return false;
            return _returns->isAssignableTo(((Lambda0*) other)->returns());
        }

        [[nodiscard]] std::string toString() const override {
            return ":: " + _returns->toString();
        }
    };

    class Lambda1 : public Lambda {
    public:
        explicit Lambda1(const Type* param, const Type* returns) : Lambda(returns), _param(param) {}

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::LAMBDA1;
        }

        [[nodiscard]] const Type* param() const {
            return _param;
        }

        [[nodiscard]] Lambda1* copy() const override {
            auto inst = new Lambda1(_param->copy(), _returns->copy());
            inst->_shared = shared();
            return inst;
        }

        bool isAssignableTo(const Type* other) const override {
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::LAMBDA1 ) return false;
            return _returns->isAssignableTo(((Lambda1*) other)->returns()) && _param->isAssignableTo(((Lambda1*) other)->param());
        }

        [[nodiscard]] std::string toString() const override {
            return _param->toString() + " :: " + _returns->toString();
        }

        [[nodiscard]] std::vector<const Type*> params() const override {
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
        const Type* _param;
    };
}
}

#endif
