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
    class IdentifierNode;
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
        OBJECT_PROTO,
        OBJECT,
        THIS,
    };
}

namespace nslib {
    [[nodiscard]] std::string s(swarmc::Type::Intrinsic v);
}

namespace swarmc::Type {
    using AssignableCache = std::map<std::size_t, std::set<std::size_t>>;

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
            if ( intrinsic == Intrinsic::OBJECT_PROTO ) return "OBJECT_PROTO";
            if ( intrinsic == Intrinsic::OBJECT ) return "OBJECT";
            if ( intrinsic == Intrinsic::THIS ) return "THIS";
            return "CONTRADICTION";
        }

        [[nodiscard]] static nslib::Monitor<AssignableCache>& getAssignableCache();

        Type() {
            _id = _nextId++;
            getAssignableCache().access<void>([this](AssignableCache* cache) {
                (*cache)[_id] = std::set<std::size_t>();
            });
        }

        ~Type() override = default;

        [[nodiscard]] serial::tag_t getSerialKey() const override {
            return s(intrinsic());
        }

        [[nodiscard]] virtual Type* copy() const {
            std::map<const Type*, Type*> visited;
            return copyRec(visited);
        }

        [[nodiscard]] virtual Type* copyRec(std::map<const Type*, Type*>&) const = 0;

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

        [[nodiscard]] virtual Type* disambiguateStatically() { return this; }

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
                || intrinsic == Intrinsic::THIS
            );
        }

        explicit Primitive(Intrinsic intrinsic) : Type(), _intrinsic(intrinsic) {}

        [[nodiscard]] serial::tag_t getSerialKey() const override {
            return "Type::Primitive";
        }

        [[nodiscard]] virtual Primitive* copy() const override {
            return Primitive::of(_intrinsic);
        }

        void transformRecursively(std::function<Type*(Type*)>, std::vector<std::size_t>& visited) override {}

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
                    || other->intrinsic() == Intrinsic::OBJECT;  // FIXME: is the type of an otype object?
            }
            return other->intrinsic() == intrinsic();
        }

        [[nodiscard]] std::string toString() const override {
            return "Primitive<" + intrinsicString(_intrinsic) + ">";
        }
    protected:
        static std::map<Intrinsic, Primitive*> _primitives;
        Intrinsic _intrinsic;

        [[nodiscard]] virtual Type* copyRec(std::map<const Type*, Type*>&) const override {
            return Primitive::of(_intrinsic);
        }
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
            Framework::onShutdown([inst]() { freeref(inst); });
            return inst;
        }

        explicit Opaque(std::string name) : Type(), _name(std::move(name)) {}

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::OPAQUE;
        }

        [[nodiscard]] std::string name() const {
            return _name;
        }

        [[nodiscard]] std::string toString() const override {
            return "Opaque<" + _name + ">";
        }

        bool isAssignableTo(const Type* other) const override {
            return other->intrinsic() == Intrinsic::OPAQUE && ((const Opaque*) other)->_name == _name;
        }

        [[nodiscard]] virtual Opaque* copy() const override {
            return Opaque::of(_name);
        }

        void transformRecursively(std::function<Type*(Type*)>, std::vector<std::size_t>& visited) override {}

    protected:
        static std::map<std::string, Opaque*> _opaques;
        std::string _name;

        [[nodiscard]] virtual Type* copyRec(std::map<const Type*, Type*>&) const override {
            return Opaque::of(_name);
        }
    };


    class Ambiguous : public Type {
    public:
        static Ambiguous* of() {
            if ( _inst == nullptr ) {
                _inst = new Ambiguous();
            }

            return _inst;
        }

        static Ambiguous* partial(Lang::IdentifierNode* id) {
            return new Ambiguous(id);
        }

        Lang::IdentifierNode* id() const {
            return _typeid;
        }

        [[nodiscard]] Intrinsic intrinsic() const override {
            return Intrinsic::AMBIGUOUS;
        }

        [[nodiscard]] std::string toString() const override;

        bool isAssignableTo(const Type* other) const override {
            return true;
        }

        [[nodiscard]] virtual Ambiguous* copy() const override {
            if ( _typeid == nullptr ) return Ambiguous::of();
            return Ambiguous::partial(_typeid);
        }

        void transformRecursively(std::function<Type*(Type*)> visitor, std::vector<std::size_t>& visited) override {
            visited.push_back(_id);
            visitor(this);
        }

        explicit Ambiguous(Lang::IdentifierNode* id = nullptr);
        ~Ambiguous();

        [[nodiscard]] Type* disambiguateStatically() override;
    protected:
        static Ambiguous* _inst;
        Lang::IdentifierNode* _typeid;

        [[nodiscard]] Ambiguous* copyRec(std::map<const Type*, Type*>& visited) const override {
            if ( _typeid == nullptr ) return Ambiguous::of();
            if ( visited.count(this) == 0 ) visited[this] = Ambiguous::partial(_typeid);
            return (Ambiguous*)visited[this];
        }
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

        [[nodiscard]] virtual Map* copy() const override {
            std::map<const Type*, Type*> visited;
            return copyRec(visited);
        }

        void transformRecursively(std::function<Type*(Type*)> visitor, std::vector<std::size_t>& visited) override {
            if ( stl::contains(visited, _values->getId()) ) {
                return;
            }

            _values->transformRecursively(visitor, visited);
            _values = swapref(_values, visitor(_values));
        }

        bool isAssignableTo(const Type* other) const override {
            if ( other->getId() == _id ) return true;
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::MAP ) return false;
            return _values->isAssignableTo(((Map*) other)->values());
        }

        [[nodiscard]] std::string toString() const override {
            return Type::intrinsicString(intrinsic()) + "<" + _values->toString() + ">";
        }

        [[nodiscard]] Map* disambiguateStatically() override {
            _values = _values->disambiguateStatically();
            return this;
        }
    protected:
        Type* _values;

        [[nodiscard]] Map* copyRec(std::map<const Type*, Type*>& visited) const override {
            if ( visited.count(this) == 0 ) visited[this] = new Map(_values->copyRec(visited));
            return (Map*)visited[this];
        }
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

        [[nodiscard]] virtual Enumerable* copy() const override {
            std::map<const Type*, Type*> visited;
            return copyRec(visited);
        }

        void transformRecursively(std::function<Type*(Type*)> visitor, std::vector<std::size_t>& visited) override {
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

        [[nodiscard]] Enumerable* disambiguateStatically() override {
            _values = _values->disambiguateStatically();
            return this;
        }
    protected:
        Type* _values;

        [[nodiscard]] Enumerable* copyRec(std::map<const Type*, Type*>& visited) const override {
            if ( visited.count(this) == 0 ) visited[this] = new Enumerable(_values->copyRec(visited));
            return (Enumerable*)visited[this];
        }
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

        [[nodiscard]] virtual Resource* copy() const override {
            std::map<const Type*, Type*> visited;
            return copyRec(visited);
        }

        void transformRecursively(std::function<Type*(Type*)> visitor, std::vector<std::size_t>& visited) override {
            if ( stl::contains(visited, _yields->getId()) ) {
                return;
            }
            _yields->transformRecursively(visitor, visited);
            _yields = swapref(_yields, visitor(_yields));
        }

        bool isAssignableTo(const Type* other) const override {
            if ( other->getId() == _id ) return true;
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::RESOURCE ) return false;
            return _yields->isAssignableTo(((Resource*) other)->yields());
        }

        [[nodiscard]] std::string toString() const override {
            return Type::intrinsicString(intrinsic()) + "<" + _yields->toString() + ">";
        }

        [[nodiscard]] Resource* disambiguateStatically() override {
            _yields = _yields->disambiguateStatically();
            return this;
        }
    protected:
        Type* _yields;

        [[nodiscard]] Resource* copyRec(std::map<const Type*, Type*>& visited) const override {
            if ( visited.count(this) == 0 ) visited[this] = new Resource(_yields->copyRec(visited));
            return (Resource*)visited[this];
        }
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

        [[nodiscard]] virtual Stream* copy() const override {
            std::map<const Type*, Type*> visited;
            return copyRec(visited);
        }

        void transformRecursively(std::function<Type*(Type*)> visitor, std::vector<std::size_t>& visited) override {
            if ( stl::contains(visited, _inner->getId()) ) {
                return;
            }
            _inner->transformRecursively(visitor, visited);
            _inner = swapref(_inner, visitor(_inner));
        }

        bool isAssignableTo(const Type* other) const override {
            if ( other->getId() == _id ) return true;
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::STREAM ) return false;
            return _inner->isAssignableTo(((Stream*) other)->inner());
        }

        [[nodiscard]] std::string toString() const override {
            return Type::intrinsicString(intrinsic()) + "<" + _inner->toString() + ">";
        }

        [[nodiscard]] Stream* disambiguateStatically() override {
            _inner = _inner->disambiguateStatically();
            return this;
        }

    protected:
        Type* _inner;

        [[nodiscard]] Stream* copyRec(std::map<const Type*, Type*>& visited) const override {
            if ( visited.count(this) == 0 ) visited[this] = new Stream(_inner->copyRec(visited));
            return (Stream*)visited[this];
        }
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

        [[nodiscard]] virtual Lambda0* copy() const override {
            std::map<const Type*, Type*> visited;
            return copyRec(visited);
        }

        bool isAssignableTo(const Type* other) const override {
            if ( other->getId() == _id ) return true;
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::LAMBDA0 ) return false;
            return _returns->isAssignableTo(((Lambda0*) other)->returns());
        }

        [[nodiscard]] std::string toString() const override {
            return ":: " + _returns->toString();
        }

        [[nodiscard]] Lambda0* disambiguateStatically() override {
            _returns = _returns->disambiguateStatically();
            return this;
        }
    protected:
        [[nodiscard]] Lambda0* copyRec(std::map<const Type*, Type*>& visited) const override {
            if ( visited.count(this) == 0 ) visited[this] = new Lambda0(_returns->copyRec(visited));
            return (Lambda0*)visited[this];
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

        [[nodiscard]] virtual Lambda1* copy() const override {
            std::map<const Type*, Type*> visited;
            return copyRec(visited);
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
            if ( other->getId() == _id ) return true;
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;
            if ( other->intrinsic() != Intrinsic::LAMBDA1 ) return false;
            return _returns->isAssignableTo(((Lambda1*) other)->returns()) && ((Lambda1*) other)->param()->isAssignableTo(_param);
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

        [[nodiscard]] Lambda1* disambiguateStatically() override {
            _param = _param->disambiguateStatically();
            _returns = _returns->disambiguateStatically();
            return this;
        }

    protected:
        Type* _param;
        
        [[nodiscard]] Lambda1* copyRec(std::map<const Type*, Type*>& visited) const override {
            if ( visited.count(this) == 0 ) 
                visited[this] = new Lambda1(_param->copyRec(visited), _returns->copyRec(visited));
            return (Lambda1*)visited[this];
        }
    };

    class Object : public Type {
    public:
        explicit Object(Object* parent = nullptr) : Type(), _parent(useref(parent)) {}

        ~Object() override {
            freeref(_parent);
            for (auto p : _properties) freeref(p.second);
        }

        [[nodiscard]] Intrinsic intrinsic() const override {
            if ( _final ) return Intrinsic::OBJECT;
            return Intrinsic::OBJECT_PROTO;
        }

        [[nodiscard]] virtual Object* copy() const override {
            std::map<const Type*, Type*> visited;
            return copyRec(visited);
        }

        void transformRecursively(std::function<Type*(Type*)> visitor, std::vector<std::size_t>& visited) override {
            visited.push_back(_id);  // otypes can create circular types, so add any objects we see to the visited list

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
            if ( other->getId() == _id ) return true;
            if ( other->intrinsic() == Intrinsic::AMBIGUOUS ) return true;

            if ( other->intrinsic() != Intrinsic::OBJECT ) {
                // Can only assign to finalized object types
                return false;
            }

            // FIXME: p:THIS blows this all to hell. And by hell, I mean off the end of the stack space.
            // The problem:
            // (1) p:THIS
                // o:METHOD1 of type p:THIS :: p:STRING
                    // Is p:THIS assignable to p:THIS? to check, goto (1)

            if ( !_final ) {
                // Object prototypes can only be assigned to p:THIS in prototypical types
                return other->intrinsic() == Intrinsic::THIS;
            }

            // Even without p:THIS a recursive type will still nuke the stack haha
            // If we arrived at a duplicate, we basically are saying for types `a` and `b`:
            // a == b <-> a == b
            // which is of course true, ergo we can return true without recursing further
            auto cachedIds = getAssignableCache().access<std::set<std::size_t>>([this](AssignableCache* cache) {
                return (*cache).at(_id);
            });
            if ( stl::contains(cachedIds, other->getId()) ) return true;

            auto otherObject = dynamic_cast<const Object*>(other);
            auto properties = otherObject->getCollapsedProperties();
            auto otherIter = properties.begin();

            // assume objects to be equal until proven otherwise (because of recursive types)
            getAssignableCache().access<void>([this, other](AssignableCache* cache) {
                (*cache)[_id].insert(other->getId());
            });

            for ( ; otherIter != properties.end(); ++otherIter ) {
                auto thisResult = getProperty(otherIter->first);
                if ( thisResult == nullptr ) {
                    // We don't have a required property on the base type
                    getAssignableCache().access<void>([this, other](AssignableCache* cache) {
                        (*cache)[_id].erase(other->getId());
                    });
                    return false;
                }

                if ( !thisResult->isAssignableTo(otherIter->second) ) {
                    // Our property has an incompatible type
                    getAssignableCache().access<void>([this, other](AssignableCache* cache) {
                        (*cache)[_id].erase(other->getId());
                    });
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
                _properties.erase(existing);
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
                freeref(existing->second);
                _properties.erase(existing);
            }
            return this;
        }

        [[nodiscard]] Type* getProperty(const std::string& name) const {
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

        [[nodiscard]] std::map<std::string, Type*> getCollapsedProperties() const {
            std::map<std::string, Type*> map;
            return getCollapsedProperties(map);
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
            if ( _final )
                return "Type::Object<#prop: " + s(_properties.size()) + ", parent: " + s(_parent) + ">";
            return "Type::Object<final: false, #prop: " + s(_properties.size()) + ", parent: " + s(_parent) + ">";
        }
    protected:
        bool _final = false;
        std::map<std::string, Type*> _properties;
        Object* _parent = nullptr;

        [[nodiscard]] std::map<std::string, Type*> getCollapsedProperties(std::map<std::string, Type*>& map) const {
            if ( _parent != nullptr ) {
                map = _parent->getCollapsedProperties(map);
            }

            for ( const auto& pair : _properties ) {
                map[pair.first] = pair.second;
            }

            return map;
        }

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

        [[nodiscard]] Object* copyRec(std::map<const Type*, Type*>& visited) const override {
            if ( visited.count(this) == 0 ) {
                auto inst = new Object(_parent);
                visited[this] = inst;
                inst->_final = _final;
                for ( const auto& p : _properties ) {
                    inst->_properties.insert({ p.first, useref(p.second->copyRec(visited)) });
                }
            }
            return (Object*)visited[this];
        }
    };
}

#endif
