#ifndef SWARMC_TYPE_H
#define SWARMC_TYPE_H

#include <map>
#include <sstream>
#include <vector>
#include "../shared/nslib.h"
#include "../errors/InvalidPrimitiveTypeInstantiation.h"

using namespace nslib;

namespace swarmc::Lang {
    class TypeLiteral;
}

namespace swarmc::Type {
    enum class Intrinsic : std::size_t {
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
        OPAQUE,
    };
}

namespace nslib {
    [[nodiscard]] std::string s(swarmc::Type::Intrinsic v);
}

namespace swarmc::Type {
    class Type : public IStringable, public serial::ISerializable, public IRefCountable {
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
            if ( intrinsic == Intrinsic::OPAQUE ) return "OPAQUE";
            return "CONTRADICTION";
        }

        ~Type() override = default;

        [[nodiscard]] serial::tag_t getSerialKey() const override {
            return s(intrinsic());
        }

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

        virtual bool isAssignableTo(const InlineRefHandle<Type>& other) const {
            return isAssignableTo(other.get());
        }

    protected:
        bool _shared = false;

        friend class Lang::TypeLiteral;
    };

    class Primitive : public Type {
    public:
        static Primitive* of(Intrinsic intrinsic) {
            auto mapIter = _primitives.find(intrinsic);
            if ( mapIter != _primitives.end() ) {
                return mapIter->second;
            }

            // FIXME: need to freeref these when the program ends
            auto inst = useref(new Primitive(intrinsic));
            _primitives[intrinsic] = inst;
            return inst;
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

        [[nodiscard]] serial::tag_t getSerialKey() const override {
            return "Type::Primitive";
        }

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
        static std::map<Intrinsic, Primitive*> _primitives;
        Intrinsic _intrinsic;
    };


    class Opaque : public Type {
    public:
        [[nodiscard]] static Opaque* of(const std::string& name) {
            auto mapIter = _opaques.find(name);
            if ( mapIter != _opaques.end() ) {
                return mapIter->second;
            }

            // FIXME: need to freeref these when the program ends
            auto inst = useref(new Opaque(name));
            _opaques[name] = inst;
            return inst;
        }

        explicit Opaque(std::string name) : _name(std::move(name)) {}

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::OPAQUE;
        }

        [[nodiscard]] std::string toString() const override {
            return "Opaque<" + _name + ">";
        }

        bool isAssignableTo(const Type* other) const override {
            return other->intrinsic() == Intrinsic::OPAQUE && ((const Opaque*) other)->_name == _name;
        }

        [[nodiscard]] Opaque* copy() const override {
            return Opaque::of(_name);
        }

    protected:
        static std::map<std::string, Opaque*> _opaques;
        std::string _name;
    };


    class Ambiguous : public Type {
    public:
        static Ambiguous* of() {
            if ( _inst == nullptr ) {
                _inst = new Ambiguous();
            }

            return _inst;
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

    protected:
        static Ambiguous* _inst;
    };

    class Map : public Type {
    public:
        explicit Map(Type* values) : _values(useref(values)) {}

        ~Map() override {
            freeref(_values);
        }

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::MAP;
        }

        [[nodiscard]] Type* values() const {
            return _values;
        }

        [[nodiscard]] InlineRefHandle<Type> valuesi() const {
            return inlineref<Type>(values());
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
        Type* _values;
    };

    class Enumerable : public Type {
    public:
        explicit Enumerable(Type* values) : _values(useref(values)) {}

        ~Enumerable() override {
            freeref(_values);
        }

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::ENUMERABLE;
        }

        [[nodiscard]] Type* values() const {
            return _values;
        }

        [[nodiscard]] InlineRefHandle<Type> valuesi() const {
            return inlineref<Type>(_values);
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
        Type* _values;
    };

    class Resource : public Type {
    public:
        static Resource* of(Type* inner) {
            return new Resource(inner);
        }

        explicit Resource(Type* yields) : _yields(useref(yields)) {}

        ~Resource() override {
            freeref(_yields);
        }

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::RESOURCE;
        }

        [[nodiscard]] Type* yields() const {
            return _yields;
        }

        [[nodiscard]] InlineRefHandle<Type> yieldsi() const {
            return inlineref<Type>(yields());
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
        Type* _yields;
    };

    class Stream : public Type {
    public:
        static Stream* of(Type* inner) {
            return new Stream(inner);
        }

        explicit Stream(Type* inner) : _inner(useref(inner)) {}

        ~Stream() override {
            freeref(_inner);
        }

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::STREAM;
        }

        [[nodiscard]] Type* inner() const {
            return _inner;
        }

        [[nodiscard]] InlineRefHandle<Type> inneri() const {
            return inlineref<Type>(inner());
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
        Type* _inner;
    };

    class Lambda : public Type {
    public:
        explicit Lambda(Type* returns): _returns(useref(returns)) {}

        ~Lambda() override {
            freeref(_returns);
        }

        [[nodiscard]] Type* returns() const {
            return _returns;
        }

        [[nodiscard]] InlineRefHandle<Type> returnsi() const {
            return inlineref<Type>(returns());
        }

        [[nodiscard]] virtual std::vector<const Type*> params() const {
            return {};
        }

    protected:
        Type* _returns;
    };

    class Lambda0 : public Lambda {
    public:
        explicit Lambda0(Type* returns) : Lambda(returns) {}

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
        explicit Lambda1(Type* param, Type* returns) : Lambda(returns), _param(useref(param)) {}

        ~Lambda1() override {
            freeref(_param);
        }

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::LAMBDA1;
        }

        [[nodiscard]] Type* param() const {
            return _param;
        }

        [[nodiscard]] InlineRefHandle<Type> parami() const {
            return inlineref<Type>(_param);
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
        Type* _param;
    };
}

#endif
