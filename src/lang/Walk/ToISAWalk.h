#ifndef SWARMC_TO_ISA_WALK_H
#define SWARMC_TO_ISA_WALK_H

#include "Walk.h"
#include "ASTMapReduce.h"
#include "../../vm/isa_meta.h"
#include "../../vm/walk/DeferrableLocationsWalk.h"
#include "../../vm/walk/SharedLocationsWalk.h"
#include "SharedLocationsWalk.h"

#define TO_ISA_FUNCTION_PREFIX "FUNC_"
#define TO_ISA_SUBFUNCTION_PREFIX "SUBFUNC_"
#define TO_ISA_FUNC_CONTROL_FLOW_BREAKER "cfb"
#define TO_ISA_LOOP_CONTROL_FLOW_BREAKER "cfbLoop"
#define TO_ISA_BREAK "break"
#define TO_ISA_WHILE_PREFIX "WHILE_"
#define TO_ISA_WHILE_COND_OUTER_PREFIX "WHILECONDOUTER_"
#define TO_ISA_WHILE_COND_INNER_PREFIX "WHILECONDINNER_"
#define TO_ISA_WHILE_COND_LOCATION "whileCondition"
#define TO_ISA_ENUMERATION_PREFIX "ENUM_"
#define TO_ISA_WITH_PREFIX "WITH_"
#define TO_ISA_RETURN_LOCATION "retVal"
#define TO_ISA_VARIABLE_PREFIX "var_"
#define TO_ISA_TMP_PREFIX "tmp"
#define TO_ISA_MAP_KEY_PREFIX "mkey_"
#define TO_ISA_OBJECT_DEFAULT_PREFIX "defval_"
#define TO_ISA_OBJECT_INSTANCE "objinst_"

namespace swarmc::Lang {

class DeferredLocationScope;

namespace Walk {

using ISAFormalList = std::vector<std::tuple<Type::Type*, ISA::Affinity, std::string, SemanticSymbol*>>;
// list of pairs mapping ObjectType to locations where the default values are stored
using TypeConstructorData = std::list<std::pair<Type::Object*, std::map<std::string, ISA::LocationReference*>>>;

class ToISAWalk : public Walk<ISA::Instructions*> {
public:
    ToISAWalk();
    ~ToISAWalk();
protected:
    ISA::Instructions* walkProgramNode(ProgramNode* node) override;
    ISA::Instructions* walkExpressionStatementNode(ExpressionStatementNode* node) override;
    ISA::Instructions* walkIdentifierNode(IdentifierNode* node) override;
    ISA::Instructions* walkEnumerableAccessNode(EnumerableAccessNode* node) override;
    ISA::Instructions* walkEnumerableAppendNode(EnumerableAppendNode* node) override;
    ISA::Instructions* walkMapAccessNode(MapAccessNode* node) override;
    ISA::Instructions* walkClassAccessNode(ClassAccessNode* node) override;
    ISA::Instructions* walkIncludeStatementNode(IncludeStatementNode* node) override;
    ISA::Instructions* walkTypeLiteral(swarmc::Lang::TypeLiteral* node) override;
    ISA::Instructions* walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) override;
    ISA::Instructions* walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) override;
    ISA::Instructions* walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) override;
    ISA::Instructions* walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) override;
    ISA::Instructions* walkMapStatementNode(MapStatementNode* node) override;
    ISA::Instructions* walkMapNode(MapNode* node) override;
    ISA::Instructions* walkAssignExpressionNode(AssignExpressionNode* node) override;
    ISA::Instructions* walkVariableDeclarationNode(VariableDeclarationNode* node) override;
    ISA::Instructions* walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) override;
    ISA::Instructions* walkUseNode(UseNode* node) override;
    ISA::Instructions* walkReturnStatementNode(ReturnStatementNode* node) override;
    ISA::Instructions* walkFunctionNode(FunctionNode* node) override;
    ISA::Instructions* walkConstructorNode(ConstructorNode* node) override;
    ISA::Instructions* walkTypeBodyNode(TypeBodyNode* node) override;
    ISA::Instructions* walkCallExpressionNode(CallExpressionNode* node) override;
    ISA::Instructions* walkDeferCallExpressionNode(DeferCallExpressionNode* node) override;
    ISA::Instructions* walkAndNode(AndNode* node) override;
    ISA::Instructions* walkOrNode(OrNode* node) override;
    ISA::Instructions* walkEqualsNode(EqualsNode* node) override;
    ISA::Instructions* walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) override;
    ISA::Instructions* walkNotEqualsNode(NotEqualsNode* node) override;
    ISA::Instructions* walkAddNode(AddNode* node) override;
    ISA::Instructions* walkSubtractNode(SubtractNode* node) override;
    ISA::Instructions* walkMultiplyNode(MultiplyNode* node) override;
    ISA::Instructions* walkDivideNode(DivideNode* node) override;
    ISA::Instructions* walkModulusNode(ModulusNode* node) override;
    ISA::Instructions* walkPowerNode(PowerNode* node) override;
    ISA::Instructions* walkNthRootNode(NthRootNode* node) override;
    ISA::Instructions* walkNegativeExpressionNode(NegativeExpressionNode* node) override;
    ISA::Instructions* walkNotNode(NotNode* node) override;
    ISA::Instructions* walkEnumerationStatement(EnumerationStatement* node) override;
    ISA::Instructions* walkWithStatement(WithStatement* node) override;
    ISA::Instructions* walkIfStatement(IfStatement* node) override;
    ISA::Instructions* walkWhileStatement(WhileStatement* node) override;
    ISA::Instructions* walkContinueNode(ContinueNode* node) override;
    ISA::Instructions* walkBreakNode(BreakNode* node) override;

    [[nodiscard]] std::string toString() const override {
        return "ToISAWalk<>";
    }
private:
    ISA::Instructions* walkUnaryExpressionNode(UnaryExpressionNode* node);
    ISA::Instructions* walkBinaryExpressionNode(BinaryExpressionNode* node);
    ISA::Instructions* walkStatementList(StatementListWrapper*, bool, bool, bool);

    /** makeLocation but with autogenerated name */
    ISA::LocationReference* makeTmp(ISA::Affinity affinity, ISA::Instructions* instrs);

    /**
     * Creates a new location reference with the given `affinity` and `name`
     * `instrs` is the list of instructions a ScopeOf should be appended to, nullptr if no ScopeOf necessary
     */
    ISA::LocationReference* makeLocation(ISA::Affinity affinity, std::string name, ISA::Instructions* instrs);

    /** Get the location assigned to in the (last - offset) instruction of `instrs` */
    ISA::LocationReference* getLastLoc(ISA::Instructions* instrs, std::size_t offset=0) const;

    /** It makes a function.
     * `loop`: if the function is a while loop
     * `newScope`: if we are in a function literal (i.e. do we need to scopeof retVal or cfb)
     * `with`: are we a with function
     */
    ISA::Instructions* makeFunction(std::string name, ISAFormalList* formals, Type::Type* retType,
        StatementListWrapper* node, bool loop, bool newScope, bool with);

    /** Transforms swarm function call to equivalent SVI instructions */
    ISA::Instructions* callToInstruction(CallExpressionNode*);

    /** Wraps parent constructor calls in a flag marking the lack of creation of new instances*/
    ISA::Instructions* parentCall(CallExpressionNode*);

    /** Transforms `FormalList` into `ISAFormalList` */
    ISAFormalList* extractFormals(FormalList*) const;

    /** Get a valid reference for `type`, replacing `comp` with `THIS` */
    ISA::TypeReference* getTypeRef(Type::Type* type, Type::Type* comp=nullptr);

    /** Recursively replaces all instances of `comp` with instances of `Type::Primitive::of(Type::Intrinsic::THIS)` in `type` */
    Type::Type* thisify(Type::Type* type, Type::Type* comp) const;

    /** Scans for what type a property belongs to (in the case that we are constructing types within types) */
    std::size_t scanConstructing(std::string name) const;

    /** appends second instructions to first, deletes second */
    void append(ISA::Instructions*, ISA::Instructions*) const;

    /** appends an instruction to the end of a list of instructions. Add drain logic if the variable refers to a deferred call */
    void append(ISA::Instructions*, ISA::Instruction*);

    /** Adds a position annotation */
    ISA::Instructions* position(ASTNode*) const;

    /** adds an AssignEval instruction, removes location from possible deferred locations */
    ISA::Instructions* assignEval(ISA::LocationReference*, ISA::Instruction*);

    /** adds an AssignValue instruction, removes location from possible deferred locations */
    ISA::Instructions* assignValue(ISA::LocationReference*, ISA::Reference*, bool);

    std::size_t _tempCounter = 0;
    std::size_t _depth = 0;
    std::size_t _loopDepth = 0;
    bool _functionOuterScope = false;
    bool _parentCall = false;
    DeferredLocationScope* _deferredResults;
    ISA::DeferrableLocationsWalk _combLocations;
    ISA::SharedLocationsWalk _sharedLocsWalkISA;
    SharedLocations _sharedLocs;

    // locationref cache
    std::map<ISA::Affinity, std::map<std::string, ISA::LocationReference*>> _locMap;
    // typeref cache
    std::map<std::size_t, ISA::TypeReference*> _typeMap;
    // top of stack is the type of the object whose constructor is being compiled
    TypeConstructorData _constructing;
    TypeConstructorData _defaultValues;
    const std::map<std::string, Type::Type*>& getInstructionAsFunc();
};

}
}

#endif
