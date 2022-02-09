#ifndef SWARMC_SERIALIZEWALK_H
#define SWARMC_SERIALIZEWALK_H

#include <string>
#include <vector>
#include "../../lib/json.hpp"
#include "../shared/RefPool.h"
#include "../lang/Type.h"
#include "../lang/AST.h"
#include "../lang/Walk.h"

namespace swarmc {
namespace Serialization {

    using namespace Lang;

    /**
     * An AST walk that serializes the AST to a JSON object.
     */
    class SerializeWalk : public Lang::Walk<nlohmann::json*> {
    public:
        SerializeWalk() : Lang::Walk<nlohmann::json*>() {}

        virtual std::string toJSON(ASTNode* node) {
            return walk(node)->dump(4);
        }

        virtual nlohmann::json* walk(ASTNode* node) {
            nlohmann::json* obj = Lang::Walk<nlohmann::json*>::walk(node);
            (*obj)["position"] = *walkPosition(node->position());
            (*obj)["astNodeName"] = node->getName();

            return obj;
        }

        virtual std::string toString() const {
            return "SerializeWalk<>";
        }

    protected:
        RefPool<nlohmann::json> _jsons;
        RefPool<std::vector<nlohmann::json>> _vectors;

        virtual nlohmann::json* getJSON() {
            return _jsons.allocInstance(new nlohmann::json)->get();
        }

        virtual std::vector<nlohmann::json>* getVector() {
            return _vectors.allocInstance(new std::vector<nlohmann::json>)->get();
        }
    
        virtual nlohmann::json* walkProgramNode(ProgramNode* node) {
            nlohmann::json* obj = getJSON();

            std::vector<nlohmann::json>* body = getVector();

            for ( auto stmt : *node->body() ) {
                body->push_back(*walk(stmt));
            }

            (*obj)["body"] = *body;

            return obj;
        }

        virtual nlohmann::json* walkExpressionStatementNode(ExpressionStatementNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["expression"] = *walk(node->expression());

            return obj;
        }

        virtual nlohmann::json* walkIdentifierNode(IdentifierNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["name"] = node->name();
            (*obj)["symbol"] = *walkSemanticSymbol(node->symbol());

            return obj;
        }

        virtual nlohmann::json* walkPrimitiveTypeNode(PrimitiveTypeNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["type"] = *walkType(node->type());

            return obj;
        }

        virtual nlohmann::json* walkEnumerableTypeNode(EnumerableTypeNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["type"] = *walkType(node->type());

            return obj;
        }

        virtual nlohmann::json* walkMapTypeNode(MapTypeNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["type"] = *walkType(node->type());

            return obj;
        }

        virtual nlohmann::json* walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["value"] = node->value();

            return obj;
        }

        virtual nlohmann::json* walkVariableDeclarationNode(VariableDeclarationNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["typeNode"] = *walk(node->typeNode());
            (*obj)["identifier"] = *walk(node->id());
            (*obj)["value"] = *walk(node->value());

            return obj;
        }

        virtual nlohmann::json* walkCallExpressionNode(CallExpressionNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["identifier"] = *walk(node->id());

            std::vector<nlohmann::json>* args = getVector();
            for ( auto arg : *node->args() ) {
                args->push_back(*walk(arg));
            }

            (*obj)["arguments"] = *args;

            return obj;
        }

        virtual nlohmann::json* walkAndNode(AndNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["left"] = *walk(node->left());
            (*obj)["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkOrNode(OrNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["left"] = *walk(node->left());
            (*obj)["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkEqualsNode(EqualsNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["left"] = *walk(node->left());
            (*obj)["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkNotEqualsNode(NotEqualsNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["left"] = *walk(node->left());
            (*obj)["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkAddNode(AddNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["left"] = *walk(node->left());
            (*obj)["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkAddAssignExpressionNode(AddAssignExpressionNode* node) {
            return walkAssignExpressionNode(node);
        }

        virtual nlohmann::json* walkSubtractNode(SubtractNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["left"] = *walk(node->left());
            (*obj)["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkMultiplyNode(MultiplyNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["left"] = *walk(node->left());
            (*obj)["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkMultiplyAssignExpressionNode(MultiplyAssignExpressionNode* node) {
            return walkAssignExpressionNode(node);
        }

        virtual nlohmann::json* walkDivideNode(DivideNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["left"] = *walk(node->left());
            (*obj)["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkModulusNode(ModulusNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["left"] = *walk(node->left());
            (*obj)["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkPowerNode(PowerNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["left"] = *walk(node->left());
            (*obj)["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkConcatenateNode(ConcatenateNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["left"] = *walk(node->left());
            (*obj)["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkNotNode(NotNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["expression"] = *walk(node->exp());

            return obj;
        }

        virtual nlohmann::json* walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
            nlohmann::json* obj = getJSON();

            std::vector<nlohmann::json>* actuals = getVector();
            for ( auto actual : *node->actuals() ) {
                actuals->push_back(*walk(actual));
            }

            (*obj)["actuals"] = *actuals;

            return obj;
        }

        virtual nlohmann::json* walkEnumerationStatement(EnumerationStatement* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["enumerable"] = *walk(node->enumerable());
            (*obj)["local"] = *walk(node->local());

            std::vector<nlohmann::json>* body = getVector();
            for ( auto stmt : *node->body() ) {
                body->push_back(*walk(stmt));
            }

            (*obj)["body"] = *body;

            return obj;
        }

        virtual nlohmann::json* walkWithStatement(WithStatement* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["resource"] = *walk(node->resource());
            (*obj)["local"] = *walk(node->local());

            std::vector<nlohmann::json>* body = getVector();
            for ( auto stmt : *node->body() ) {
                body->push_back(*walk(stmt));
            }

            (*obj)["body"] = *body;

            return obj;
        }

        virtual nlohmann::json* walkIfStatement(IfStatement* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["condition"] = *walk(node->condition());

            std::vector<nlohmann::json>* body = getVector();
            for ( auto stmt : *node->body() ) {
                body->push_back(*walk(stmt));
            }

            (*obj)["body"] = *body;

            return obj;
        }

        virtual nlohmann::json* walkWhileStatement(WhileStatement* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["condition"] = *walk(node->condition());

            std::vector<nlohmann::json>* body = getVector();
            for ( auto stmt : *node->body() ) {
                body->push_back(*walk(stmt));
            }

            (*obj)["body"] = *body;

            return obj;
        }

        virtual nlohmann::json* walkMapStatementNode(MapStatementNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["mapStatementIdentifier"] = node->id()->name();  // doesn't have type information
            (*obj)["value"] = *walk(node->value());

            return obj;
        }

        virtual nlohmann::json* walkMapNode(MapNode* node) {
            nlohmann::json* obj = getJSON();

            std::vector<nlohmann::json>* body = getVector();
            for ( auto stmt : *node->body() ) {
                body->push_back(*walk(stmt));
            }

            (*obj)["body"] = *body;

            return obj;
        }

        virtual nlohmann::json* walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["value"] = node->value();

            return obj;
        }

        virtual nlohmann::json* walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["value"] = node->value();

            return obj;
        }

        virtual nlohmann::json* walkAssignExpressionNode(AssignExpressionNode* node) {
            nlohmann::json* obj = getJSON();

            (*obj)["dest"] = *walk(node->dest());
            (*obj)["value"] = *walk(node->value());

            return obj;
        }

        virtual nlohmann::json* walkPosition(const Position* pos) {
            nlohmann::json* obj = getJSON();

            (*obj)["startLine"] = pos->startLine();
            (*obj)["endLine"] = pos->endLine();
            (*obj)["startCol"] = pos->startCol();
            (*obj)["endCol"] = pos->endCol();

            return obj;
        }

        virtual nlohmann::json* walkSemanticSymbol(SemanticSymbol* sym) {
            nlohmann::json* obj = getJSON();

            (*obj)["name"] = sym->name();
            (*obj)["uuid"] = sym->uuid();
            (*obj)["kind"] = sym->kind();
            (*obj)["type"] = *walkType(sym->type());
            (*obj)["declaredAt"] = *walkPosition(sym->declaredAt());

            return obj;
        }

        virtual nlohmann::json* walkType(const Type* type) {
            if ( type->isPrimitiveType() ) {
                return walkPrimitiveType((PrimitiveType*) type);
            } else if ( type->isGenericType() ) {
                return walkGenericType((GenericType*) type);
            } else if ( type->isFunctionType() ) {
                return walkFunctionType((FunctionType*) type);
            }

            throw Errors::SwarmError("Cannot serialize invalid type.");
        }

        virtual nlohmann::json* walkPrimitiveType(const PrimitiveType* type) {
            nlohmann::json* obj = getJSON();

            (*obj)["valueType"] = type->valueType();

            return obj;
        }

        virtual nlohmann::json* walkGenericType(const GenericType* type) {
            nlohmann::json* obj = getJSON();

            (*obj)["valueType"] = type->valueType();
            (*obj)["concrete"] = *walkType(type->concrete());

            return obj;
        }

        virtual nlohmann::json* walkFunctionType(const FunctionType* type) {
            nlohmann::json* obj = getJSON();

            (*obj)["valueType"] = type->valueType();
            (*obj)["return"] = *walkType(type->returnType());

            std::vector<nlohmann::json>* args = getVector();
            for ( auto arg : *type->getArgumentTypes() ) {
                args->push_back(*walkType(arg));
            }

            (*obj)["arguments"] = *args;

            return obj;
        }
    };

}
}

#endif
