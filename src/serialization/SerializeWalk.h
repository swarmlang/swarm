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
        virtual std::string toJSON(ASTNode* node) {
            return walk(node)->dump();
        }

        virtual nlohmann::json* walk(ASTNode* node) {
            nlohmann::json* obj = Lang::Walk<nlohmann::json*>::walk(node);
            nlohmann::json json = *obj;
            json["position"] = *walkPosition(node->position());
            json["astNodeName"] = node->getName();
            return obj;
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
            nlohmann::json json = *obj;

            std::vector<nlohmann::json>* body = getVector();

            for ( auto stmt : *node->body() ) {
                body->push_back(*walk(stmt));
            }

            json["body"] = *obj;

            return obj;
        }

        virtual nlohmann::json* walkExpressionStatementNode(ExpressionStatementNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["expression"] = *walk(node->expression());

            return obj;
        }

        virtual nlohmann::json* walkIdentifierNode(IdentifierNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["name"] = node->name();
            json["symbol"] = *walkSemanticSymbol(node->symbol());

            return obj;
        }

        virtual nlohmann::json* walkPrimitiveTypeNode(PrimitiveTypeNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["type"] = *walkType(node->type());

            return obj;
        }

        virtual nlohmann::json* walkEnumerableTypeNode(EnumerableTypeNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["type"] = *walkType(node->type());

            return obj;
        }

        virtual nlohmann::json* walkMapTypeNode(MapTypeNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["type"] = *walkType(node->type());

            return obj;
        }

        virtual nlohmann::json* walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["value"] = node->value();

            return obj;
        }

        virtual nlohmann::json* walkVariableDeclarationNode(VariableDeclarationNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["typeNode"] = *walk(node->typeNode());
            json["identifier"] = *walk(node->id());
            json["value"] = *walk(node->value());

            return obj;
        }

        virtual nlohmann::json* walkCallExpressionNode(CallExpressionNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["identifier"] = *walk(node->id());

            std::vector<nlohmann::json>* args = getVector();
            for ( auto arg : *node->args() ) {
                args->push_back(*walk(arg));
            }

            json["arguments"] = *args;

            return obj;
        }

        virtual nlohmann::json* walkAndNode(AndNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["left"] = *walk(node->left());
            json["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkOrNode(OrNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["left"] = *walk(node->left());
            json["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkEqualsNode(EqualsNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["left"] = *walk(node->left());
            json["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkNotEqualsNode(NotEqualsNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["left"] = *walk(node->left());
            json["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkAddNode(AddNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["left"] = *walk(node->left());
            json["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkAddAssignExpressionNode(AddAssignExpressionNode* node) {
            return walkAssignExpressionNode(node);
        }

        virtual nlohmann::json* walkSubtractNode(SubtractNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["left"] = *walk(node->left());
            json["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkMultiplyNode(MultiplyNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["left"] = *walk(node->left());
            json["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkMultiplyAssignExpressionNode(MultiplyAssignExpressionNode* node) {
            return walkAssignExpressionNode(node);
        }

        virtual nlohmann::json* walkDivideNode(DivideNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["left"] = *walk(node->left());
            json["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkModulusNode(ModulusNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["left"] = *walk(node->left());
            json["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkPowerNode(PowerNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["left"] = *walk(node->left());
            json["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkConcatenateNode(ConcatenateNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["left"] = *walk(node->left());
            json["right"] = *walk(node->right());

            return obj;
        }

        virtual nlohmann::json* walkNotNode(NotNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["expression"] = *walk(node->exp());

            return obj;
        }

        virtual nlohmann::json* walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            std::vector<nlohmann::json>* actuals = getVector();
            for ( auto actual : *node->actuals() ) {
                actuals->push_back(*walk(actual));
            }

            return obj;
        }

        virtual nlohmann::json* walkEnumerationStatement(EnumerationStatement* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["enumerable"] = *walk(node->enumerable());
            json["local"] = *walk(node->local());

            std::vector<nlohmann::json>* body = getVector();
            for ( auto stmt : *node->body() ) {
                body->push_back(*walk(stmt));
            }

            json["body"] = *body;

            return obj;
        }

        virtual nlohmann::json* walkWithStatement(WithStatement* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["resource"] = *walk(node->resource());
            json["local"] = *walk(node->local());

            std::vector<nlohmann::json>* body = getVector();
            for ( auto stmt : *node->body() ) {
                body->push_back(*walk(stmt));
            }

            json["body"] = *body;

            return obj;
        }

        virtual nlohmann::json* walkIfStatement(IfStatement* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["condition"] = *walk(node->condition());

            std::vector<nlohmann::json>* body = getVector();
            for ( auto stmt : *node->body() ) {
                body->push_back(*walk(stmt));
            }

            json["body"] = *body;

            return obj;
        }

        virtual nlohmann::json* walkWhileStatement(WhileStatement* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["condition"] = *walk(node->condition());

            std::vector<nlohmann::json>* body = getVector();
            for ( auto stmt : *node->body() ) {
                body->push_back(*walk(stmt));
            }

            json["body"] = *body;

            return obj;
        }

        virtual nlohmann::json* walkMapStatementNode(MapStatementNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["identifier"] = *walk(node->id());
            json["value"] = *walk(node->value());

            return obj;
        }

        virtual nlohmann::json* walkMapNode(MapNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            std::vector<nlohmann::json>* body = getVector();
            for ( auto stmt : *node->body() ) {
                body->push_back(*walk(stmt));
            }

            json["body"] = *body;

            return obj;
        }

        virtual nlohmann::json* walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["value"] = node->value();

            return obj;
        }

        virtual nlohmann::json* walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["value"] = node->value();

            return obj;
        }

        virtual nlohmann::json* walkAssignExpressionNode(AssignExpressionNode* node) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["dest"] = *walk(node->dest());
            json["value"] = *walk(node->value());

            return obj;
        }

        virtual nlohmann::json* walkPosition(const Position* pos) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["startLine"] = pos->startLine();
            json["endLine"] = pos->endLine();
            json["startCol"] = pos->startCol();
            json["endCol"] = pos->endCol();

            return obj;
        }

        virtual nlohmann::json* walkSemanticSymbol(SemanticSymbol* sym) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["name"] = sym->name();
            json["kind"] = sym->kind();
            json["type"] = *walkType(sym->type());
            json["declaredAt"] = *walkPosition(sym->declaredAt());

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
            nlohmann::json json = *obj;

            json["valueType"] = type->valueType();

            return obj;
        }

        virtual nlohmann::json* walkGenericType(const GenericType* type) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["valueType"] = type->valueType();
            json["concrete"] = *walkType(type->concrete());

            return obj;
        }

        virtual nlohmann::json* walkFunctionType(const FunctionType* type) {
            nlohmann::json* obj = getJSON();
            nlohmann::json json = *obj;

            json["valueType"] = type->valueType();
            json["return"] = *walkType(type->returnType());

            std::vector<nlohmann::json>* args = getVector();
            for ( auto arg : *type->getArgumentTypes() ) {
                args->push_back(*walkType(arg));
            }

            json["arguments"] = *args;

            return obj;
        }
    };

}
}

#endif
