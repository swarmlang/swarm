#ifndef SWARMC_COMPILE_WALK_H
#define SWARMC_COMPILE_WALK_H

#include <fstream>
#include "Walk.h"
#include "../../vm/isa_meta.h"

namespace swarmc {
namespace Lang {
namespace Walk {

class CompileWalk : Walk<ISA::Instructions*> {
public:
    CompileWalk(std::ostream& out, ASTNode* node) : Walk<ISA::Instructions*>(), _tempCounter(0) {
        ISA::Instructions* program = walk(node);
        for ( auto i : *program ) {
            out << i->toString() << "\n";
        }
    }
protected:
    virtual ISA::Instructions* walkProgramNode(ProgramNode* node) {
        auto instrs = new ISA::Instructions();
        for ( auto stmt : *node->body() ) {
            auto i = walk(stmt);
            instrs->insert(instrs->end(), i->begin(), i->end());
            delete i;
        }
        return instrs;
    }

    virtual ISA::Instructions* walkExpressionStatementNode(ExpressionStatementNode* node) {
        return walk(node->expression());
    }

    virtual ISA::Instructions* walkIdentifierNode(IdentifierNode* node) {
        auto instrs = new ISA::Instructions();
        ISA::Affinity affinity = node->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
        auto ref = new ISA::LocationReference(affinity, "var_" + node->name());
        instrs->push_back(new ISA::AssignValue(ref, ref));
        return instrs;
    }

    virtual ISA::Instructions* walkMapAccessNode(MapAccessNode* node) {
        auto instrs = walk(node->path());
        // FIXME: map location affinity
        auto mapget = new ISA::MapGet(
            new ISA::LocationReference(ISA::Affinity::LOCAL, node->end()->name()), getLocFromAssign(instrs->back()));
        instrs->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), mapget));
        return instrs;
    }

    virtual ISA::Instructions* walkEnumerableAccessNode(EnumerableAccessNode* node) {
        auto instrs = walk(node->index());
        auto llval = walk(node->path());
        auto enumget = new ISA::EnumGet(getLocFromAssign(llval->back()), getLocFromAssign(instrs->back()));
        instrs->insert(instrs->end(), llval->begin(), llval->end());
        instrs->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), enumget));
        return instrs;
    }

    virtual ISA::Instructions* walkTypeLiteral(swarmc::Lang::TypeLiteral *node) {
        auto instrs = new ISA::Instructions();
        auto ref = new ISA::TypeReference(node->value());
        instrs->push_back(new ISA::AssignValue(makeLocation(ISA::Affinity::PRIMITIVE), ref));
        return instrs;
    }

    virtual ISA::Instructions* walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
        auto instrs = new ISA::Instructions();
        auto ref = new ISA::BooleanReference(node->value());
        instrs->push_back(new ISA::AssignValue(makeLocation(ISA::Affinity::PRIMITIVE), ref));
        return instrs;
    }

    virtual ISA::Instructions* walkVariableDeclarationNode(VariableDeclarationNode* node) {
        auto instrs = walk(node->value());

        // Create location from variable name
        auto aff = node->id()->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
        ISA::LocationReference* loc = new ISA::LocationReference(aff, "var_" + node->id()->name());

        // acquire temporary location from previous instruction
        instrs->push_back(new ISA::AssignValue(loc, getLocFromAssign(instrs->back())));

        return instrs;
    }

    virtual ISA::Instructions* walkCallExpressionNode(CallExpressionNode* node) {
        return nullptr;
    }

    virtual ISA::Instructions* walkIIFExpressionNode(IIFExpressionNode* node) {
        return nullptr;
    }

    virtual ISA::Instructions* walkAndNode(AndNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::And(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkOrNode(OrNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Or(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkEqualsNode(EqualsNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::IsEqual(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkNotEqualsNode(NotEqualsNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        auto loc = makeLocation(ISA::Affinity::LOCAL);
        left->push_back(new ISA::AssignEval(loc, new ISA::IsEqual(lhs, rhs)));
        left->push_back(new ISA::AssignEval(loc, new ISA::Not(loc)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkAddNode(AddNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Plus(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkSubtractNode(SubtractNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Minus(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkMultiplyNode(MultiplyNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Times(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkDivideNode(DivideNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Divide(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkModulusNode(ModulusNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Mod(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkPowerNode(PowerNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Power(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkConcatenateNode(ConcatenateNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());
        left->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::StringConcat(lhs, rhs)));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkNegativeExpressionNode(NegativeExpressionNode* node) {
        auto instrs = walk(node->exp());
        auto loc = getLocFromAssign(instrs->back());
        instrs->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Negative(loc)));
        return instrs;
    }

    virtual ISA::Instructions* walkNotNode(NotNode* node) {
        auto instrs = walk(node->exp());
        auto loc = getLocFromAssign(instrs->back());
        instrs->push_back(new ISA::AssignEval(makeLocation(ISA::Affinity::LOCAL), new ISA::Not(loc)));
        return instrs;
    }

    virtual ISA::Instructions* walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
        auto instrs = new ISA::Instructions();
        auto loc = makeLocation(ISA::Affinity::LOCAL);
        assert(node->type()->intrinsic() == Type::Intrinsic::ENUMERABLE);
        auto innerType = ((Type::Enumerable*)node->type())->values();
        auto map = new ISA::AssignEval(loc, new ISA::EnumInit(new ISA::TypeReference(innerType)));
        instrs->push_back(map);
        for ( auto exp : *node->actuals() ) {
            auto expinstrs = walk(exp);
            auto enumapp = new ISA::EnumAppend(getLocFromAssign(expinstrs->back()), loc);
            instrs->insert(instrs->end(), expinstrs->begin(), expinstrs->end());
            instrs->push_back(enumapp);
            delete expinstrs;
        }
        // need to end with an assign because future operations will look at the end of instrs for the lloc
        instrs->push_back(new ISA::AssignValue(loc, loc));
        return instrs;
    }

    virtual ISA::Instructions* walkEnumerationStatement(EnumerationStatement* node) {
        return nullptr;
    }

    virtual ISA::Instructions* walkCapturedBlockStatementNode(CapturedBlockStatementNode* node) {
        return nullptr;
    }

    virtual ISA::Instructions* walkWithStatement(WithStatement* node) {
        return nullptr;
    }

    virtual ISA::Instructions* walkIfStatement(IfStatement* node) {
        return nullptr;
    }

    virtual ISA::Instructions* walkWhileStatement(WhileStatement* node) {
        return nullptr;
    }

    virtual ISA::Instructions* walkContinueNode(ContinueNode* node) {
        return nullptr;
    }

    virtual ISA::Instructions* walkBreakNode(BreakNode* node) {
        return nullptr;
    }

    virtual ISA::Instructions* walkReturnStatementNode(ReturnStatementNode* node) {
        ISA::Instructions* instrs = nullptr;
        if ( node->value() == nullptr ) {
            instrs = new ISA::Instructions();
            instrs->push_back(new ISA::Return0());
        } else {
            instrs = walk(node->value());
            instrs->push_back(new ISA::Return1(getLocFromAssign(instrs->back()));
        }
        return instrs;
    }

    virtual ISA::Instructions* walkMapStatementNode(MapStatementNode* node) {
        return walk(node->value());
    }

    virtual ISA::Instructions* walkMapNode(MapNode* node) {
        auto instrs = new ISA::Instructions();
        auto loc = makeLocation(ISA::Affinity::LOCAL);
        assert(node->type()->intrinsic() == Type::Intrinsic::MAP);
        auto innerType = ((Type::Map*)node->type())->values();
        auto map = new ISA::AssignEval(loc, new ISA::MapInit(new ISA::TypeReference(innerType)));
        instrs->push_back(map);
        for ( auto stmt : *node->body() ) {
            // FIXME: do map ids inherit sharedness from the map?
            auto id = new ISA::LocationReference(ISA::Affinity::LOCAL, stmt->id()->name());
            auto mapstmt = walk(stmt);
            auto mapset = new ISA::MapSet(id, getLocFromAssign(mapstmt->back()), loc);
            instrs->insert(instrs->end(), mapstmt->begin(), mapstmt->end());
            instrs->push_back(mapset);
            delete mapstmt;
        }
        // need to end with an assign because future operations will look at the end of instrs for the lloc
        instrs->push_back(new ISA::AssignValue(loc, loc));
        return instrs;
    }

    virtual ISA::Instructions* walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
        auto instrs = new ISA::Instructions();
        auto ref = new ISA::StringReference(node->value());
        instrs->push_back(new ISA::AssignValue(makeLocation(ISA::Affinity::PRIMITIVE), ref));
        return instrs;
    }

    virtual ISA::Instructions* walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
        auto instrs = new ISA::Instructions();
        auto ref = new ISA::NumberReference(node->value());
        instrs->push_back(new ISA::AssignValue(makeLocation(ISA::Affinity::PRIMITIVE), ref));
        return instrs;
    }

    virtual ISA::Instructions* walkAssignExpressionNode(AssignExpressionNode* node) {
        auto instrs = walk(node->value());
        auto value = getLocFromAssign(instrs->back());
        if ( node->dest()->getName() == "IdentifierNode" ) {
            auto id = (IdentifierNode*) node->dest();
            auto aff = id->shared() ? ISA::Affinity::SHARED : ISA::Affinity::LOCAL;
            instrs->push_back(
                new ISA::AssignValue(
                    new ISA::LocationReference(aff, "var_" + id->name()), value));
        } else if ( node->dest()->getName() == "EnumerableAccessNode" ) {
            auto en = (EnumerableAccessNode*) node->dest();
            auto index = walk(en->index());
            auto path = walk(en->path());
            instrs->insert(instrs->end(), index->begin(), index->end());
            instrs->insert(instrs->end(), path->begin(), path->end());
            instrs->push_back(
                new ISA::EnumSet(
                    getLocFromAssign(path->back()), 
                    getLocFromAssign(index->back()), 
                    value));
        } else {
            auto ma = (MapAccessNode*) node->dest();
            auto path = walk(ma->path());
            instrs->insert(instrs->end(), path->begin(), path->end());
            // FIXME: map key affinity
            instrs->push_back(
                new ISA::MapSet(
                    new ISA::LocationReference(ISA::Affinity::LOCAL, ma->end()->name()), 
                    value,
                    getLocFromAssign(path->back())));
        }
        return instrs;
    }

    virtual ISA::Instructions* walkUnitNode(UnitNode* node) {
        // this should never be called
        return new ISA::Instructions();
    }

    virtual ISA::Instructions* walkFunctionNode(FunctionNode* node) {
        return nullptr;
    }

    virtual ISA::Instructions* walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
        auto left = walk(node->left());
        auto right = walk(node->right());
        auto lhs = getLocFromAssign(left->back());
        auto rhs = getLocFromAssign(right->back());
        left->insert(left->end(), right->begin(), right->end());

        // determine which instruction
        ISA::Instruction* instr;
        switch (node->comparisonType()) {
        case NumberComparisonType::GREATER_THAN:
            instr = new ISA::GreaterThan(lhs, rhs);
            break;
        case NumberComparisonType::GREATER_THAN_OR_EQUAL:
            instr = new ISA::GreaterThanOrEqual(lhs, rhs);
            break;
        case NumberComparisonType::LESS_THAN:
            instr = new ISA::LessThan(lhs, rhs);
            break;
        default:
            instr = new ISA::LessThanOrEqual(lhs, rhs);
            break;
        }
        left->push_back(new ISA::AssignEval(lhs, instr));
        delete right;
        return left;
    }

    virtual ISA::Instructions* walkTagResourceNode(TagResourceNode* node) {
        return nullptr;
    }

    virtual std::string toString() const {
        return "CompileWalk<>";
    }

private:
    int _tempCounter;

    ISA::LocationReference* makeLocation(ISA::Affinity affinity) {
        return new ISA::LocationReference(affinity, "temp" + std::to_string(_tempCounter++));
    }

    ISA::LocationReference* getLocFromAssign(ISA::Instruction* instr) {
        assert(instr->tag() == ISA::Tag::ASSIGNEVAL || instr->tag() == ISA::Tag::ASSIGNVALUE);
        if (instr->tag() == ISA::Tag::ASSIGNEVAL) {
            return ((ISA::AssignEval*)instr)->first();
        }
        return ((ISA::AssignValue*)instr)->first();
    }
};

}
}
}

#endif