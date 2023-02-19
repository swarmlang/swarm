#ifndef SWARMVM_ISA
#define SWARMVM_ISA

#include <utility>
#include <vector>
#include "../shared/nslib.h"
#include "../lang/Type.h"
#include "runtime/runtime_functions.h"
#include "runtime/interfaces.h"
#include "runtime/fabric.h"

using namespace nslib;

/*
 * This file describes the structure of the SVI IR.
 * For implementation classes of each instruction, see the headers
 * located under the `isa/` directory.
 */

namespace swarmc::ISA {

    class Instruction;

    using Instructions = std::vector<Instruction *>;

    /** Unique identifiers for each instruction. */
    enum class Tag : std::size_t {
        POSITION,
        BEGINFN,
        FNPARAM,
        RETURN0,
        RETURN1,
        CURRY,
        CALL0,
        CALL1,
        CALLIF0,
        CALLIF1,
        CALLELSE0,
        CALLELSE1,
        PUSHCALL0,
        PUSHCALL1,
        PUSHCALLIF0,
        PUSHCALLIF1,
        PUSHCALLELSE0,
        PUSHCALLELSE1,
        DRAIN,
        EXIT,
        OUT,
        ERR,
        STREAMINIT,
        STREAMPUSH,
        STREAMPOP,
        STREAMCLOSE,
        STREAMEMPTY,
        TYPIFY,
        ASSIGNVALUE,
        ASSIGNEVAL,
        LOCK,
        UNLOCK,
        EQUAL,
        SCOPEOF,
        TYPEOF,
        COMPATIBLE,
        AND,
        OR,
        XOR,
        NAND,
        NOR,
        NOT,
        MAPINIT,
        MAPSET,
        MAPGET,
        MAPLENGTH,
        MAPKEYS,
        ENUMINIT,
        ENUMAPPEND,
        ENUMPREPEND,
        ENUMLENGTH,
        ENUMGET,
        ENUMSET,
        ENUMERATE,
        STRCONCAT,
        STRLENGTH,
        STRSLICEFROM,
        STRSLICEFROMTO,
        PLUS,
        MINUS,
        TIMES,
        DIVIDE,
        POWER,
        MOD,
        NEG,
        GT,
        GTE,
        LT,
        LTE,
        WHILE,
        WITH,
        PUSHEXHANDLER1,
        PUSHEXHANDLER2,
        POPEXHANDLER,
        RAISE,
        RESUME,
        OTYPEINIT,
        OTYPEPROP,
        OTYPEDEL,
        OTYPEGET,
        OTYPEFINALIZE,
        OTYPESUBSET,
    };

    /** Places where values can be stored. */
    enum class Affinity : std::size_t {
        LOCAL,
        SHARED,
        FUNCTION,
        PRIMITIVE,
        OBJECTPROP,
    };

    /** Broad types of references built-in to the runtime. */
    enum class ReferenceTag : std::size_t {
        LOCATION,
        TYPE,
        STRING,
        NUMBER,
        BOOLEAN,
        FUNCTION,
        STREAM,
        RESOURCE,
        ENUMERATION,
        MAP,
        VOID,
        OTYPE,
        OBJECT,
    };
}

namespace nslib {

    [[nodiscard]] std::string s(swarmc::ISA::ReferenceTag v);

}


namespace swarmc::ISA {
    /** A reference is a construct that resolves to a value at runtime. */
    class Reference : public IStringable, public serial::ISerializable, public IRefCountable {
    public:
        explicit Reference(ReferenceTag tag) : _tag(tag) {}
        ~Reference() override = default;

        /** Get the type of this reference. */
        [[nodiscard]] virtual Type::Type* type() const = 0;

        [[nodiscard]] virtual InlineRefHandle<Type::Type> typei() const {
            return inlineref<Type::Type>(type());
        }

        /** Get copy of object */
        [[nodiscard]] virtual Reference* copy() const = 0;

        virtual bool isEqualTo(const Reference* other) const = 0;

        [[nodiscard]] ReferenceTag tag() const {
            return _tag;
        }

        [[nodiscard]] serial::tag_t getSerialKey() const override {
            return s(tag());
        }

    protected:
        ReferenceTag _tag;
    };

    /** A variable / A value in storage */
    class LocationReference : public Reference {
    public:
        LocationReference(Affinity affinity, std::string name) : Reference(ReferenceTag::LOCATION), _affinity(affinity), _name(std::move(name)) {}

        ~LocationReference() override {
            freeref(_type);
        }

        /** Convert the given affinity value to a human-readable representation. */
        static std::string affinityString(Affinity a) {
            if ( a == Affinity::FUNCTION ) return "f";
            if ( a == Affinity::LOCAL ) return "l";
            if ( a == Affinity::SHARED ) return "s";
            if ( a == Affinity::PRIMITIVE ) return "p";
            if ( a == Affinity::OBJECTPROP ) return "o";
            return "UNKNOWN";
        }

        bool isEqualTo(const Reference* other) const override {
            if ( other->tag() == ReferenceTag::LOCATION ) {
                auto loc = (LocationReference*) other;
                if ( loc->affinity() == affinity() && loc->name() == name() ) return true;
            }

            throw Errors::SwarmError("Cannot directly compare the equality of two different locations.");
        }

        /** Get the storage affinity of this location. */
        [[nodiscard]] Affinity affinity() const {
            return _affinity;
        }

        /** Get the variable name of this location. */
        [[nodiscard]] std::string name() const {
            return _name;
        }

        /** Get the location-prefixed name of this location (e.g. `l:my_var`) */
        [[nodiscard]] std::string fqName() const {
            return affinityString(_affinity) + ":" + _name;
        }

        [[nodiscard]] std::string toString() const override {
            return "Location<" + affinityString(_affinity) + ":" + _name + ">";
        }

        [[nodiscard]] Type::Type* type() const override {
            if ( _type == nullptr )
                return new Type::Ambiguous();

            return _type;
        }

        void setType(Type::Type* t) {
            if ( _type != t ) {
                freeref(_type);
                _type = useref(t);
            }
        }

        /** Returns true if the given location refers to the same place as this one. */
        virtual bool is(const LocationReference* other) const {
            return (
                other->affinity() == _affinity
                && other->name() == _name
                && other->type()->isAssignableTo(type())
                && type()->isAssignableTo(other->type())
            );
        }

        [[nodiscard]] LocationReference* copy() const override {
            auto t = new LocationReference(_affinity, _name);
            if ( _type != nullptr ) t->setType(_type);
            return t;
        }

    protected:
        Affinity _affinity;
        std::string _name;
        Type::Type* _type = nullptr;
    };


    class StreamReference : public Reference {
    public:
        explicit StreamReference(Runtime::IStream* stream) : Reference(ReferenceTag::STREAM), _stream(useref(stream)) {}

        ~StreamReference() override {
            freeref(_stream);
        }

        [[nodiscard]] std::string toString() const override {
            return "StreamReference<" + _stream->toString() + ">";
        }

        [[nodiscard]] Type::Stream* type() const override {
            return Type::Stream::of(_stream->innerType());
        }

        [[nodiscard]] Runtime::IStream* stream() const {
            return _stream;
        }

        bool isEqualTo(const Reference* other) const override {
            if ( other->tag() != ReferenceTag::STRING ) return false;
            auto stream = (StreamReference*) other;
            return stream->stream()->id() == _stream->id();
        }

        [[nodiscard]] StreamReference* copy() const override {
            // FIXME: might need a copy of _stream
            return new StreamReference(_stream);
        }

    protected:
        Runtime::IStream* _stream;
    };


    class ResourceReference : public Reference {
    public:
        explicit ResourceReference(Runtime::IResource* resource) : Reference(ReferenceTag::RESOURCE), _resource(resource) {}

        [[nodiscard]] std::string toString() const override {
            return "ResourceReference<" + _resource->toString() + ">";
        }

        [[nodiscard]] Type::Resource* type() const override {
            return Type::Resource::of(_resource->innerType());
        }

        Runtime::IResource* resource() {
            return _resource;
        }

        bool isEqualTo(const Reference*) const override {
            return false;
        }

        [[nodiscard]] ResourceReference* copy() const override {
            return new ResourceReference(_resource);
        }
    protected:
        Runtime::IResource* _resource;
    };


    /** A function literal. */
    class FunctionReference : public Reference {
    public:
        explicit FunctionReference(Runtime::IFunction* fn) : Reference(ReferenceTag::FUNCTION), _fn(useref(fn)) {}

        ~FunctionReference() override {
            freeref(_fn);
        }

        [[nodiscard]] std::string toString() const override {
            return "FunctionReference<" + _fn->toString() + ">";
        }

        [[nodiscard]] Type::Type* type() const override {
            auto params = _fn->paramTypes();
            auto returnType = _fn->returnType();
            if ( params.empty() ) {
                return new Type::Lambda0(returnType);
            }

            Type::Lambda1* t = nullptr;
            for ( auto it = params.rbegin(); it < params.rend(); ++it ) {
                if ( t == nullptr ) {
                    t = new Type::Lambda1(*it, returnType);
                } else {
                    t = new Type::Lambda1(*it, t);
                }
            }
            return t;
        }

        /** Get the runtime function implementation. */
        [[nodiscard]] Runtime::IFunction* fn() const {
            return _fn;
        }

        bool isEqualTo(const Reference* other) const override {
            return false;  // function equality is undefined
        }

        [[nodiscard]] FunctionReference* copy() const override {
            // FIXME: might need acopy of _fn
            return new FunctionReference(_fn);
        }

    protected:
        Runtime::IFunction* _fn;
    };

    /** A type literal */
    class TypeReference : public Reference {
    public:
        explicit TypeReference(Type::Type* type) : Reference(ReferenceTag::TYPE), _type(useref(type)) {}

        ~TypeReference() override {
            freeref(_type);
        }

        [[nodiscard]] std::string toString() const override {
            return "TypeReference<" + _type->toString() + ">";
        }

        /** Get the type of the reference itself (always of type type). */
        [[nodiscard]] Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::TYPE);
        }

        /** Get the actual type this value is holding. */
        [[nodiscard]] virtual Type::Type* value() const {
            return _type;
        }

        [[nodiscard]] virtual InlineRefHandle<Type::Type> valuei() const {
            return inlineref<Type::Type>(value());
        }

        bool isEqualTo(const Reference* other) const override {
            if ( other->tag() != _tag ) return false;
            auto ref = (TypeReference*) other;
            return ref->type()->isAssignableTo(type()) && type()->isAssignableTo(ref->type());
        }

        [[nodiscard]] TypeReference* copy() const override {
            return new TypeReference(_type->copy());
        }

    protected:
        Type::Type* _type;
    };

    class ObjectTypeReference : public TypeReference {
    public:
        explicit ObjectTypeReference(Type::Object* type) : TypeReference(type) {
            _tag = ReferenceTag::OTYPE;
        }

        [[nodiscard]] std::string toString() const override {
            return "ObjectTypeReference<" + _type->toString() + ">";
        }

        [[nodiscard]] Type::Object* value() const override {
            return dynamic_cast<Type::Object*>(_type);
        }

        [[nodiscard]] InlineRefHandle<Type::Object> otypei() const {
            return inlineref<Type::Object>(value());
        }

        [[nodiscard]] bool isFinal() const {
            return value()->isFinal();
        }

        void replace(Type::Object* type) {
            if ( isFinal() ) {
                throw Errors::RuntimeError(
                    Errors::RuntimeExCode::MutateFinalizedObject,
                    "Cannot mutate finalized object type"
                );
            }

            _type = swapref(_type, type);
        }

        void finalize() {
            replace(value()->finalize());
        }
    };

    class VoidReference : public Reference {
    public:
        VoidReference() : Reference(ReferenceTag::VOID) {}

        [[nodiscard]] std::string toString() const override {
            return "VoidReference<>";
        }

        [[nodiscard]] Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::VOID);
        }

        bool isEqualTo(const Reference* other) const override {
            return other->tag() == ReferenceTag::VOID;
        }

        [[nodiscard]] VoidReference* copy() const override {
            return new VoidReference;
        }
    };

    /** Helper class for literal values. */
    template <typename T>
    class LiteralReference : public Reference {
    public:
        LiteralReference(ReferenceTag tag, const T value) : Reference(tag), _value(value) {}

        /** The literal value this reference is wrapping. */
        [[nodiscard]] virtual T value() const {
            return _value;
        }
    protected:
        const T _value;
    };

    /** A literal string value */
    class StringReference : public LiteralReference<std::string> {
    public:
        explicit StringReference(std::string value) : LiteralReference<std::string>(ReferenceTag::STRING, std::move(value)) {}

        [[nodiscard]] std::string toString() const override {
            return "StringReference<" + _value + ">";
        }

        [[nodiscard]] Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::STRING);
        }

        [[nodiscard]] bool isEqualTo(const Reference* other) const override {
            if ( other->tag() != tag() ) return false;
            auto ref = (StringReference*) other;
            return ref->value() == value();
        }

        [[nodiscard]] StringReference* copy() const override {
            return new StringReference(_value);
        }
    };

    /** A literal number value */
    class NumberReference : public LiteralReference<double> {
    public:
        explicit NumberReference(double value) : LiteralReference<double>(ReferenceTag::NUMBER, value) {}

        [[nodiscard]] Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        [[nodiscard]] std::string toString() const override {
            return "NumberReference<" + std::to_string(_value) + ">";
        }

        bool isEqualTo(const Reference* other) const override {
            if ( other->tag() != tag() ) return false;
            auto ref = (NumberReference*) other;
            return ref->value() == value();
        }

        [[nodiscard]] NumberReference* copy() const override {
            return new NumberReference(_value);
        }
    };

    /** A literal boolean value */
    class BooleanReference : public LiteralReference<bool> {
    public:
        explicit BooleanReference(bool value) : LiteralReference<bool>(ReferenceTag::BOOLEAN, value) {}

        [[nodiscard]] std::string toString() const override {
            std::string value = _value ? "true" : "false";
            return "BooleanReference<" + value + ">";
        }

        [[nodiscard]] Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }

        bool isEqualTo(const Reference* other) const override {
            if ( other->tag() != tag() ) return false;
            auto ref = (BooleanReference*) other;
            return ref->value() == value();
        }

        [[nodiscard]] BooleanReference* copy() const override {
            return new BooleanReference(_value);
        }
    };

    /**
     * Reference representing an enumerable list of values.
     */
    class EnumerationReference : public Reference {
    public:
        explicit EnumerationReference(Type::Type* innerType) :
            Reference(ReferenceTag::ENUMERATION), _innerType(useref(innerType)) {}

        ~EnumerationReference() override {
            for ( auto elem : _items ) freeref(elem);
            freeref(_innerType);
        }

        [[nodiscard]] std::string toString() const override {
            return "EnumerationReference<inner: " + _innerType->toString() + ", #items: " + std::to_string(_items.size()) + ">";
        }

        [[nodiscard]] Type::Enumerable* type() const override {
            return new Type::Enumerable(_innerType);
        }

        /** Add an item to the end of this enumeration. */
        virtual void append(Reference* value) {
            _items.push_back(useref(value));
        }

        /** Add an item to the beginning of this enumeration. */
        virtual void prepend(Reference* value) {
            _items.insert(_items.begin(), useref(value));
        }

        /** Returns true if this enumeration has an item at the given index. */
        [[nodiscard]] virtual bool has(std::size_t i) const {
            return _items.size() > i;
        }

        /** Returns the item at the given index. */
        [[nodiscard]] virtual Reference* get(std::size_t i) const {
            return _items.at(i);
        }

        /** Inserts the given item into the enumeration at the specified index. */
        virtual void set(std::size_t i, Reference* value) {
            if ( i < _items.size() ) {
                freeref(_items[i]);
            }

            _items[i] = useref(value);
        }

        /** Pre-allocate space for the given number of items. */
        virtual void reserve(std::size_t len) {
            _items.reserve(_items.size() + len);
        }

        /** Returns the number of items in this enumeration. */
        [[nodiscard]] virtual std::vector<Reference*>::size_type length() const {
            return _items.size();
        }

        bool isEqualTo(const Reference* other) const override {
            if ( other->tag() != tag() ) return false;
            auto ref = (EnumerationReference*) other;
            if ( ref->length() != length() ) return false;
            for ( std::size_t i = 0; i < length(); i += 1 ) {
                if ( !get(i)->isEqualTo(ref->get(i)) ) {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] EnumerationReference* copy() const override {
            auto e = new EnumerationReference(_innerType->copy());
            for ( auto item : _items ) {
                e->append(item->copy());
            }
            return e;
        }
    protected:
        std::vector<Reference*> _items;
        Type::Type* _innerType;
    };


    /**
     * A reference representing a string -> value mapping.
     */
    class MapReference : public Reference {
    public:
        explicit MapReference(Type::Type* innerType) :
                Reference(ReferenceTag::MAP), _innerType(useref(innerType)) {}

        ~MapReference() override {
            for ( const auto& pair : _items ) freeref(pair.second);
            freeref(_innerType);
        }

        [[nodiscard]] std::string toString() const override {
            return "MapReference<inner: " + _innerType->toString() + ", #keys: " + std::to_string(_items.size()) + ">";
        }

        [[nodiscard]] Type::Map* type() const override {
            return new Type::Map(_innerType);
        }

        /** Get the element at the given key. */
        [[nodiscard]] virtual Reference* get(const std::string& key) const {
            return _items.at(key);
        }

        /** Store the given element at the given key. */
        virtual void set(const std::string& key, Reference* value) {
            _items.insert({key, useref(value)});
        }

        /** Returns true if this map contains an element at the given key. */
        [[nodiscard]] virtual bool has(const std::string& key) const {
            return _items.find(key) != _items.end();
        }

        /** Get the number of entries in this map. */
        [[nodiscard]] virtual std::size_t length() const {
            return _items.size();
        }

        /** Get an enumeration of the keys of this map. */
        [[nodiscard]] virtual EnumerationReference* keys() const {
            auto er = new EnumerationReference(Type::Primitive::of(Type::Intrinsic::STRING));
            for ( const auto& item : _items ) {
                er->append(new StringReference(item.first));
            }
            return er;
        }

        bool isEqualTo(const Reference* other) const override {
            if ( other->tag() != tag() ) return false;
            auto ref = (MapReference*) other;
            if ( ref->length() != length() ) return false;
            return std::all_of(_items.begin(), _items.end(), [ref](const std::pair<std::string, Reference*>& item) {
                return item.second->isEqualTo(ref->get(item.first));
            });
        }

        [[nodiscard]] MapReference* copy() const override {
            auto m = new MapReference(_innerType->copy());
            for ( const auto& item : _items ) {
                m->set(item.first, item.second->copy());
            }
            return m;
        }

    protected:
        std::map<std::string, Reference*> _items;
        Type::Type* _innerType;
    };


    /** Base class for instructions which are executed in the VM */
    class Instruction : public IStringable, public IRefCountable {
    public:
        explicit Instruction(Tag tag) : _tag(tag) {}
        ~Instruction() override = default;

        static std::string tagName(Tag tag);

        [[nodiscard]] virtual Tag tag() const {
            return _tag;
        }

        [[nodiscard]] virtual Instruction* copy() const = 0;

        [[nodiscard]] virtual bool isNullary() const {
            return false;
        }

        [[nodiscard]] virtual bool isUnary() const {
            return false;
        }

        [[nodiscard]] virtual bool isBinary() const {
            return false;
        }

        [[nodiscard]] virtual bool isTrinary() const {
            return false;
        }

    protected:
        Tag _tag;
    };

    /** Class of instructions which take no parameters */
    class NullaryInstruction : public Instruction {
    public:
        explicit NullaryInstruction(Tag tag) : Instruction(tag) {}

        [[nodiscard]] std::string toString() const override {
            return tagName(tag()) + "<>";
        }

        [[nodiscard]] bool isNullary() const override {
            return true;
        }
    };

    /** Class of instructions which take a single parameter */
    template <typename TFirst>
    class UnaryInstruction : public Instruction {
    public:
        UnaryInstruction(Tag tag, TFirst* first) : Instruction(tag), _first(first) {}

        virtual TFirst* first() const {
            return _first;
        }

        virtual void setFirst(TFirst* first) {
            _first = first;
        }

        [[nodiscard]] std::string toString() const override {
            return tagName(tag()) + "<" + _first->toString() + ">";
        }

        [[nodiscard]] bool isUnary() const override {
            return true;
        }
    protected:
        TFirst* _first;
    };

    /** Class of instructions which take two parameters */
    template <typename TFirst, typename TSecond>
    class BinaryInstruction : public Instruction {
    public:
        BinaryInstruction(Tag tag, TFirst* first, TSecond* second) : Instruction(tag), _first(first), _second(second) {}

        [[nodiscard]] virtual TFirst* first() const {
            return _first;
        }

        [[nodiscard]] virtual TSecond* second() const {
            return _second;
        }

        virtual void setFirst(TFirst* first) {
            _first = first;
        }

        virtual void setSecond(TSecond* second) {
            _second = second;
        }

        [[nodiscard]] std::string toString() const override {
            return tagName(tag()) + "<" + _first->toString() + ", " + _second->toString() + ">";
        }

        [[nodiscard]] bool isBinary() const override {
            return true;
        }
    protected:
        TFirst* _first;
        TSecond* _second;
    };

    class BinaryReferenceInstruction : public BinaryInstruction<Reference, Reference> {
    public:
        BinaryReferenceInstruction(Tag tag, Reference* first, Reference* second):
            BinaryInstruction<Reference, Reference>(tag, useref(first), useref(second)) {}

        ~BinaryReferenceInstruction() override {
            freeref(_first);
            freeref(_second);
        }
    };

    /** Class of instructions which take three parameters */
    template <typename TFirst, typename TSecond, typename TThird>
    class TrinaryInstruction : public Instruction {
    public:
        TrinaryInstruction(Tag tag, TFirst* first, TSecond* second, TThird* third) : Instruction(tag), _first(first), _second(second), _third(third) {}

        [[nodiscard]] virtual TFirst* first() const {
            return _first;
        }

        [[nodiscard]] virtual TSecond* second() const {
            return _second;
        }

        [[nodiscard]] virtual TThird* third() const {
            return _third;
        }

        virtual void setFirst(TFirst* first) {
            _first = first;
        }

        virtual void setSecond(TSecond* second) {
            _second = second;
        }

        virtual void setThird(TThird* third) {
            _third = third;
        }

        [[nodiscard]] std::string toString() const override {
            return tagName(tag()) + "<" + _first->toString() + ", " + _second->toString() + ", " + _third->toString() + ">";
        }

        [[nodiscard]] bool isTrinary() const override {
            return true;
        }
    protected:
        TFirst* _first;
        TSecond* _second;
        TThird* _third;
    };

    class TrinaryReferenceInstruction : public TrinaryInstruction<Reference, Reference, Reference> {
    public:
        TrinaryReferenceInstruction(Tag tag, Reference* first, Reference* second, Reference* third):
                TrinaryInstruction<Reference, Reference, Reference>(tag, useref(first), useref(second), useref(third)) {}

        ~TrinaryReferenceInstruction() override {
            freeref(_first);
            freeref(_second);
            freeref(_third);
        }
    };
}

#endif
