#ifndef SWARMC_TYPE_H
#define SWARMC_TYPE_H

#include <map>
#include <set>
#include <sstream>
#include <utility>
#include <vector>
#include "../shared/nslib.h"
#include "../errors/InvalidPrimitiveTypeInstantiation.h"
#include "../errors/RuntimeError.h"

using namespace nslib;

namespace swarmc::Lang {
    class TypeLiteral;
    class LValNode;
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
        OTYPE,
        OBJECT_PROTO,
        OBJECT,
        THIS,
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
            if ( intrinsic == Intrinsic::OTYPE ) return "OTYPE";
            if ( intrinsic == Intrinsic::OBJECT_PROTO ) return "OBJECT_PROTO";
            if ( intrinsic == Intrinsic::OBJECT ) return "OBJECT";
            if ( intrinsic == Intrinsic::THIS ) return "THIS";
            return "CONTRADICTION";
        }

        Type() {
            _id = _nextId++;
        }

        ~Type() override = default;

        [[nodiscard]] serial::tag_t getSerialKey() const override {
            return s(intrinsic());
        }

        [[nodiscard]] virtual Type* copy() const = 0;

        virtual void transform(std::function<Type*(Type*)> visitor) {
            std::vector<std::size_t> visited;
            transformRecursively(std::move(visitor), visited);
        }

        virtual void transformRecursively(std::function<Type*(Type*)>, std::vector<std::size_t>&) = 0;

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

        virtual bool isAssignableTo(const Type* other) const = 0;

        virtual bool isAssignableTo(const InlineRefHandle<Type>& other) const {
            return isAssignableTo(other.get());
        }

        [[nodiscard]] virtual std::size_t getId() const {
            return _id;
        }

    protected:
        friend class Lang::TypeLiteral;
        std::size_t _id;
        static std::size_t _nextId;
    };

    class Primitive : public Type {
    public:
        static Primitive* of(Intrinsic intrinsic) {
            auto mapIter = _primitives.find(intrinsic);
            if ( mapIter != _primitives.end() ) {
                return mapIter->second;
            }

            auto inst = useref(new Primitive(intrinsic));
            GC_ON_SHUTDOWN(inst)
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
                || intrinsic == Intrinsic::OTYPE
                || intrinsic == Intrinsic::THIS
            );
        }

        explicit Primitive(Intrinsic intrinsic) : Type(), _intrinsic(intrinsic) {}

        [[nodiscard]] serial::tag_t getSerialKey() const override {
            return "Type::Primitive";
        }

        [[nodiscard]] Primitive* copy() const override {
            return Primitive::of(_intrinsic);
        }

        void transformRecursively(std::function<Type*(Type*)>, std::vector<std::size_t>& visited) override {
            visited.push_back(_id);
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
            if ( intrinsic() == Intrinsic::TYPE ) {
                return other->intrinsic() == Intrinsic::TYPE
                    || other->intrinsic() == Intrinsic::OTYPE;
            }
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

        explicit Opaque(std::string name) : Type(), _name(std::move(name)) {}

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

        void transformRecursively(std::function<Type*(Type*)>, std::vector<std::size_t>& visited) override {
            visited.push_back(_id);
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

        static Ambiguous* partial(Lang::LValNode* lval) {
            return new Ambiguous(lval);
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

        void transformRecursively(std::function<Type*(Type*)>, std::vector<std::size_t>& visited) override {
            visited.push_back(_id);
        }

        explicit Ambiguous(Lang::LValNode* lval = nullptr) : Type(), _lval(lval) {}
    protected:

        static Ambiguous* _inst;
        Lang::LValNode* _lval;
    };

    class Map : public Type {
    public:
        explicit Map(Type* values) : Type(), _values(useref(values)) {}

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
            return new Map(_values->copy());
        }

        void transformRecursively(std::function<Type*(Type*)> visitor, std::vector<std::size_t>& visited) override {
            visited.push_back(_id);
            if ( stl::contains(visited, _values->getId()) ) {
                return;
            }

            _values->transformRecursively(visitor, visited);
            _values = swapref(_values, visitor(_values));
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
        explicit Enumerable(Type* values) : Type(), _values(useref(values)) {}

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
            return new Enumerable(_values->copy());
        }

        void transformRecursively(std::function<Type*(Type*)> visitor, std::vector<std::size_t>& visited) override {
            visited.push_back(_id);
            if ( stl::contains(visited, _values->getId()) ) {
                return;
            }
            _values->transformRecursively(visitor, visited);
            _values = swapref(_values, visitor(_values));
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

        explicit Resource(Type* yields) : Type(), _yields(useref(yields)) {}

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
            return new Resource(_yields);
        }

        void transformRecursively(std::function<Type*(Type*)> visitor, std::vector<std::size_t>& visited) override {
            visited.push_back(_id);
            if ( stl::contains(visited, _yields->getId()) ) {
                return;
            }
            _yields->transformRecursively(visitor, visited);
            _yields = swapref(_yields, visitor(_yields));
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

        explicit Stream(Type* inner) : Type(), _inner(useref(inner)) {}

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
            return new Stream(_inner->copy());
        }

        void transformRecursively(std::function<Type*(Type*)> visitor, std::vector<std::size_t>& visited) override {
            visited.push_back(_id);
            if ( stl::contains(visited, _inner->getId()) ) {
                return;
            }
            _inner->transformRecursively(visitor, visited);
            _inner = swapref(_inner, visitor(_inner));
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
        explicit Lambda(Type* returns): Type(), _returns(useref(returns)) {}

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

        void transformRecursively(std::function<Type*(Type*)> visitor, std::vector<std::size_t>& visited) override {
            visited.push_back(_id);
            if ( stl::contains(visited, _returns->getId()) ) {
                return;
            }
            _returns->transformRecursively(visitor, visited);
            _returns = swapref(_returns, visitor(_returns));
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
            return new Lambda0(_returns->copy());
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
            return new Lambda1(_param->copy(), _returns->copy());
        }

        void transformRecursively(std::function<Type*(Type*)> visitor, std::vector<std::size_t>& visited) override {
            Lambda::transformRecursively(visitor, visited);
            if ( stl::contains(visited, _param->getId()) ) {
                return;
            }
            _param->transformRecursively(visitor, visited);
            _param = swapref(_param, visitor(_param));
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

    class Object : public Type {
    public:
        explicit Object(Object* parent = nullptr) : Type(), _parent(useref(parent)) {}

        ~Object() override {
            freeref(_parent);
        }

        [[nodiscard]] Intrinsic intrinsic() const override {
            if ( _final ) return Intrinsic::OBJECT;
            return Intrinsic::OBJECT_PROTO;
        }

        [[nodiscard]] Object* copy() const override {
            auto inst = new Object;
            inst->_final = _final;
            inst->_parent = _parent;
            inst->_properties = _properties;
            for ( const auto& p : inst->_properties ) useref(p.second);
            return inst;
        }

        void transformRecursively(std::function<Type*(Type*)> visitor, std::vector<std::size_t>& visited) override {
            visited.push_back(_id);

            if ( _parent != nullptr && !stl::contains(visited, _parent->getId()) ) {
                _parent->transformRecursively(visitor, visited);
            }

            for (auto& _property : _properties) {
                if ( stl::contains(visited, _property.second->getId()) ) {
                    continue;
                }

                _property.second->transformRecursively(visitor, visited);
                _property.second = swapref(_property.second, visitor(_property.second));
            }
        }

        [[nodiscard]] bool isFinal() const {
            return _final;
        }

        [[nodiscard]] bool isAssignableTo(const Type* other) const override {
            if ( other->intrinsic() != Intrinsic::OBJECT ) {
                // Can only assign to finalized object types
                return false;
            }

            if ( !_final ) {
                // Object prototypes can only be assigned to p:THIS in prototypical types
                return other->intrinsic() == Intrinsic::THIS;
            }

            auto otherObject = dynamic_cast<const Object*>(other);
            auto otherIter = otherObject->_properties.begin();
            for ( ; otherIter != otherObject->_properties.end(); ++otherIter ) {
                auto thisResult = _properties.find(otherIter->first);
                if ( thisResult == _properties.end() ) {
                    // We don't have a required property on the base type
                    return false;
                }

                if ( !thisResult->second->isAssignableTo(otherIter->second) ) {
                    // Our property has an incompatible type
                    return false;
                }
            }

            return true;
        }

        Object* defineProperty(const std::string& name, Type* type) {
            if ( _final ) {
                throw Errors::RuntimeError(
                    Errors::RuntimeExCode::MutateFinalizedObject,
                    "Cannot define property on finalized object type"
                );
            }

            checkParentCompatibility(name, type);

            auto existing = _properties.find(name);
            if ( existing != _properties.end() ) {
                freeref(existing->second);
            }

            _properties[name] = useref(type);
            return this;
        }

        Object* deleteProperty(const std::string& name) {
            if ( _final ) {
                throw Errors::RuntimeError(
                    Errors::RuntimeExCode::MutateFinalizedObject,
                    "Cannot delete property on finalized object type"
                );
            }

            auto existing = _properties.find(name);
            if ( existing != _properties.end() ) {
                _properties.erase(existing);
            }
            return this;
        }

        [[nodiscard]] Type* getProperty(const std::string& name) {
            auto match = _properties.find(name);
            if ( match != _properties.end() ) {
                return match->second;
            }

            if ( _parent != nullptr ) {
                return _parent->getProperty(name);
            }

            return nullptr;
        }

        [[nodiscard]] Object* finalize() const {
            auto inst = copy();
            inst->_final = true;

            // Replace all instances of p:THIS with the actual object type
            inst->transform([inst](Type* t) -> Type* {
                if ( t->intrinsic() == Intrinsic::THIS ) {
                    return inst;
                }

                return t;
            });

            return inst;
        }

        [[nodiscard]] std::map<std::string, Type*> getProperties() const {
            return _properties;
        }

        [[nodiscard]] std::size_t numberOfProperties() const {
            auto size = _properties.size();
            if ( _parent != nullptr ) {
                size += _parent->numberOfProperties();
            }
            return size;
        }

        [[nodiscard]] Object* getParent() const {
            return _parent;
        }

        [[nodiscard]] std::string toString() const override {
            return "Type::Object<#prop: " + s(_properties.size()) + ", parent: " + s(_parent) + ">";
        }
    protected:
        bool _final = false;
        std::map<std::string, Type*> _properties;
        Object* _parent = nullptr;

        void checkParentCompatibility(const std::string& name, const Type* type) const {
            if ( _parent == nullptr ) {
                return;  // We are the parent.
            }

            auto parentMatch = _parent->getProperty(name);
            if ( parentMatch != nullptr && !type->isAssignableTo(parentMatch) ) {
                throw Errors::RuntimeError(
                    Errors::RuntimeExCode::ChildObjectTypeConflict,
                    "Property '" + name + "' on child object type is not assignable to the parent type (expected: " + s(parentMatch) + ", got: " + s(type) + ")"
                );
            }
        }
    };
}

#endif
