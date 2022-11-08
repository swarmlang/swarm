#ifndef SWARMVM_ISA
#define SWARMVM_ISA

#include <vector>
#include "../shared/IStringable.h"
#include "../lang/Type.h"
#include "runtime/runtime_functions.h"

/*
 * This file describes the structure of the SVI IR.
 * For implementation classes of each instruction, see the headers
 * located under the `isa/` directory.
 */

namespace swarmc::ISA {

    class Instruction;

    using Instructions = std::vector<Instruction*>;

    /** Unique identifiers for each instruction. */
    enum class Tag {
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
    };

    /** Places where values can be stored. */
    enum class Affinity {
        LOCAL,
        SHARED,
        FUNCTION,
        PRIMITIVE,
    };

    enum class ReferenceTag {
        LOCATION,
        TYPE,
        STRING,
        NUMBER,
        BOOLEAN,
        FUNCTION,
        ENUMERATION,
        MAP,
    };


    /** A reference is a construct that resolves to a value at runtime. */
    class Reference : public IStringable {
    public:
        Reference(ReferenceTag tag) : _tag(tag) {}
        ~Reference() override = default;

        virtual const Type::Type* type() const = 0;

        ReferenceTag tag() const {
            return _tag;
        }

    protected:
        ReferenceTag _tag;
    };

    /** A variable / A value in storage */
    class LocationReference : public Reference {
    public:
        LocationReference(Affinity affinity, std::string name) : Reference(ReferenceTag::LOCATION), _affinity(affinity), _name(name) {}

        static std::string affinityString(Affinity a) {
            if ( a == Affinity::FUNCTION ) return "f";
            if ( a == Affinity::LOCAL ) return "l";
            if ( a == Affinity::SHARED ) return "s";
            if ( a == Affinity::PRIMITIVE ) return "p";
            return "UNKNOWN";
        }

        Affinity affinity() const {
            return _affinity;
        }

        std::string name() const {
            return _name;
        }

        std::string fqName() const {
            return affinityString(_affinity) + ":" + _name;
        }

        std::string toString() const override {
            return "Location<" + affinityString(_affinity) + ":" + _name + ">";
        }

        const Type::Type* type() const override {
            if ( _type == nullptr )
                return new Type::Ambiguous();

            return _type;
        }

        void setType(const Type::Type* t) {
            _type = t;
        }

        virtual bool is(const LocationReference* other) const {
            return (
                other->affinity() == _affinity
                && other->name() == _name
                && other->type()->isAssignableTo(type())
                && type()->isAssignableTo(other->type())
            );
        }

    protected:
        Affinity _affinity;
        std::string _name;
        const Type::Type* _type = nullptr;
    };

    /** A function literal. */
    class FunctionReference : public Reference {
    public:
        explicit FunctionReference(Runtime::IFunction* fn) : Reference(ReferenceTag::FUNCTION), _fn(fn) {}

        std::string toString() const override {
            return "FunctionReference<" + _fn->toString() + ">";
        }

        const Type::Type* type() const override {
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

        Runtime::IFunction* fn() const {
            return _fn;
        }

    protected:
        Runtime::IFunction* _fn;
    };

    /** A type literal */
    class TypeReference : public Reference {
    public:
        TypeReference(const Type::Type* type) : Reference(ReferenceTag::TYPE), _type(type) {}

        std::string toString() const override {
            return "TypeReference<" + _type->toString() + ">";
        }

        const Type::Type* type() const override {
            return Type::Primitive::of(Type::Intrinsic::TYPE);
        }

        const Type::Type* value() const {
            return _type;
        }

    protected:
        const Type::Type* _type;
    };

    /** Helper class for literal values. */
    template <typename T>
    class LiteralReference : public Reference {
    public:
        LiteralReference(ReferenceTag tag, const T value) : Reference(tag), _value(value) {}

        virtual const T value() const {
            return _value;
        }
    protected:
        const T _value;
    };

    /** A literal string value */
    class StringReference : public LiteralReference<std::string> {
    public:
        StringReference(std::string value) : LiteralReference<std::string>(ReferenceTag::STRING, value) {}

        std::string toString() const override {
            return "StringReference<" + _value + ">";
        }

        const Type::Type* type() const {
            return Type::Primitive::of(Type::Intrinsic::STRING);
        }
    };

    /** A literal number value */
    class NumberReference : public LiteralReference<double> {
    public:
        NumberReference(double value) : LiteralReference<double>(ReferenceTag::NUMBER, value) {}

        const Type::Type* type() const {
            return Type::Primitive::of(Type::Intrinsic::NUMBER);
        }

        std::string toString() const override {
            return "NumberReference<" + std::to_string(_value) + ">";
        }
    };

    /** A literal boolean value */
    class BooleanReference : public LiteralReference<bool> {
    public:
        BooleanReference(bool value) : LiteralReference<bool>(ReferenceTag::BOOLEAN, value) {}

        std::string toString() const override {
            std::string value = _value ? "true" : "false";
            return "BooleanReference<" + value + ">";
        }

        const Type::Type* type() const {
            return Type::Primitive::of(Type::Intrinsic::BOOLEAN);
        }
    };

    class EnumerationReference : public Reference {
    public:
        explicit EnumerationReference(const Type::Type* innerType) :
            Reference(ReferenceTag::ENUMERATION), _innerType(innerType) {}

        std::string toString() const override {
            return "EnumerationReference<inner: " + _innerType->toString() + ", #items: " + std::to_string(_items.size()) + ">";
        }

        const Type::Enumerable* type() const override {
            return new Type::Enumerable(_innerType);
        }

        virtual void append(Reference* value) {
            _items.push_back(value);
        }

        virtual void prepend(Reference* value) {
            _items.insert(_items.begin(), value);
        }

        virtual bool has(size_t i) const {
            return _items.size() > i;
        }

        virtual Reference* get(size_t i) const {
            return _items.at(i);
        }

        virtual void set(size_t i, Reference* value) {
            _items[i] = value;
        }

        virtual void reserve(size_t len) {
            _items.reserve(_items.size() + len);
        }

        virtual std::vector<Reference*>::size_type length() const {
            return _items.size();
        }
    protected:
        std::vector<Reference*> _items;
        const Type::Type* _innerType;
    };


    class MapReference : public Reference {
    public:
        explicit MapReference(const Type::Type* innerType) :
                Reference(ReferenceTag::MAP), _innerType(innerType) {}

        std::string toString() const override {
            return "MapReference<inner: " + _innerType->toString() + ", #keys: " + std::to_string(_items.size()) + ">";
        }

        const Type::Map* type() const override {
            return new Type::Map(_innerType);
        }

        virtual Reference* get(const std::string& key) const {
            return _items.at(key);
        }

        virtual void set(const std::string& key, Reference* value) {
            _items.insert({key, value});
        }

        virtual bool has(const std::string& key) const {
            return _items.find(key) != _items.end();
        }

        virtual size_t length() const {
            return _items.size();
        }

        virtual EnumerationReference* keys() const {
            auto er = new EnumerationReference(Type::Primitive::of(Type::Intrinsic::STRING));
            for ( const auto& item : _items ) {
                er->append(new StringReference(item.first));
            }
            return er;
        }

    protected:
        std::map<std::string, Reference*> _items;
        const Type::Type* _innerType;
    };


    /** Base class for instructions which are executed in the VM */
    class Instruction : public IStringable {
    public:
        Instruction(Tag tag) : _tag(tag) {}
        ~Instruction() override = default;

        static std::string tagName(Tag tag);

        virtual Tag tag() const {
            return _tag;
        }

    protected:
        Tag _tag;
    };

    /** Class of instructions which take no parameters */
    class NullaryInstruction : public Instruction {
    public:
        NullaryInstruction(Tag tag) : Instruction(tag) {}

        std::string toString() const override {
            return tagName(tag()) + "<>";
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

        std::string toString() const override {
            return tagName(tag()) + "<" + _first->toString() + ">";
        }
    protected:
        TFirst* _first;
    };

    /** Class of instructions which take two parameters */
    template <typename TFirst, typename TSecond>
    class BinaryInstruction : public Instruction {
    public:
        BinaryInstruction(Tag tag, TFirst* first, TSecond* second) : Instruction(tag), _first(first), _second(second) {}

        virtual TFirst* first() const {
            return _first;
        }

        virtual TSecond* second() const {
            return _second;
        }

        std::string toString() const override {
            return tagName(tag()) + "<" + _first->toString() + ", " + _second->toString() + ">";
        }
    protected:
        TFirst* _first;
        TSecond* _second;
    };

    /** Class of instructions which take three parameters */
    template <typename TFirst, typename TSecond, typename TThird>
    class TrinaryInstruction : public Instruction {
    public:
        TrinaryInstruction(Tag tag, TFirst* first, TSecond* second, TThird* third) : Instruction(tag), _first(first), _second(second), _third(third) {}

        virtual TFirst* first() const {
            return _first;
        }

        virtual TSecond* second() const {
            return _second;
        }

        virtual TThird* third() const {
            return _third;
        }

        std::string toString() const override {
            return tagName(tag()) + "<" + _first->toString() + ", " + _second->toString() + ", " + _third->toString() + ">";
        }
    protected:
        TFirst* _first;
        TSecond* _second;
        TThird* _third;
    };
}

#endif