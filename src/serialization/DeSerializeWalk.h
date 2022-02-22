#ifndef SWARMC_DESERIALIZEWALK_H
#define SWARMC_DESERIALIZEWALK_H

#include <assert.h>
#include "../../lib/json.hpp"
#include "../shared/util/Console.h"
#include "../lang/AST.h"
#include "../lang/Walk.h"

namespace swarmc {
namespace Serialization {
    using namespace Lang;

    using JSONList = std::vector<nlohmann::json>;

    class DeSerializeWalk : public IUsesConsole {
    public:
        DeSerializeWalk() : IUsesConsole() {}
        
        virtual ASTNode* deserialize(std::istream* input) {
            std::string strinput = inputToString(*input);
            auto program = nlohmann::json::parse(strinput);
            return walk(program);
        }
    protected:
        virtual ProgramNode* walkProgramNode(JSONList body, Position* pos) {
            ProgramNode* prog = new ProgramNode();
            for (auto stmt : body) {
                ASTNode* b = walk(stmt);
                assert(b->isStatement());
                prog->pushStatement((StatementNode*) b);
            }
            return prog;
        }

        virtual ExpressionStatementNode* walkExpressionStatementNode(nlohmann::json expjson, Position* pos) {
            ASTNode* expression = walk(expjson);
            assert(expression->isExpression());
            return new ExpressionStatementNode(pos,(StatementExpressionNode*) expression);
        }

        virtual SemanticSymbol* walkSymbol(nlohmann::json symjson) {
            std::string uuid = symjson["uuid"];
            auto loc = _uuidList.find(uuid);
            if (loc != _uuidList.end()) return loc->second;
            std::string name = symjson["name"];
            Type* type = walkType(symjson["type"]);
            Position* pos = new Position(
                symjson["declaredAt"]["startLine"],
                symjson["declaredAt"]["endLine"],
                symjson["declaredAt"]["startCol"],
                symjson["declaredAt"]["endCol"]);
            SemanticSymbol* sym;
            if (type->isFunctionType()) {
                sym = new FunctionSymbol(name,(FunctionType*) type,pos);
                
            } else {
                sym = new VariableSymbol(name,type,pos);
            }
            sym->_uuid = uuid;
            return  sym;
        }

        virtual IdentifierNode* walkIdentifierNode(std::string name, nlohmann::json symjson, Position* pos) {
            IdentifierNode* identifier = new IdentifierNode(pos,name);
            identifier->_symbol = walkSymbol(symjson);
            return identifier;
        }

        virtual MapAccessNode* walkMapAccessNode(std::string end, nlohmann::json endposjson, nlohmann::json pathjson, Position* pos) {
            ASTNode* path = walk(pathjson);
            assert(path->isLVal());         
            Position* endpos = new Position(
                endposjson["startLine"],
                endposjson["endLine"],
                endposjson["startCol"],
                endposjson["endCol"]);
            return new MapAccessNode(pos, (LValNode*) path, new IdentifierNode(endpos, end));
        }

        virtual EnumerableAccessNode* walkEnumerableAccessNode(nlohmann::json pathjson, nlohmann::json indexjson, Position* pos) {
            ASTNode* path = walk(pathjson);
            assert(path->isLVal());
            ASTNode* index = walk(indexjson);
            assert(index->getName() == "IntegerLiteralExpressionNode");
            return new EnumerableAccessNode(pos, (LValNode*) path, (IntegerLiteralExpressionNode*) index);
        }

        virtual PrimitiveTypeNode* walkPrimitiveTypeNode(ValueType type, Position* pos) {
            return new PrimitiveTypeNode(pos, PrimitiveType::of(type));
        }

        Type* walkType(nlohmann::json typejson) {
            ValueType t = (ValueType) typejson["valueType"];
            if ( t == ValueType::TFUNCTION ) {
                Type* ret = walkType(typejson["return"]);
                FunctionType* fn = FunctionType::of(ret, typejson["shared"]);
                for ( auto arg : typejson["arguments"] ) {
                    fn->addArgument(walkType(arg));
                }
                return fn;
            }
            else if ( t == ValueType::TERROR ) {
                assert(false);
            }
            else if ( t == ValueType::TMAP || t == ValueType::TENUMERABLE || t == ValueType::TRESOURCE ) {
                Type* concrete = walkType(typejson["concrete"]);
                return GenericType::of(t, concrete);
            }
            else {
                return PrimitiveType::of(t);
            }
            assert(false);
        }

        virtual TypeNode* walkTypeNode(Type* type,Position* pos) {
            ValueType t = type->valueType();
            if ( t == ValueType::TENUMERABLE ) 
                return new EnumerableTypeNode(pos, walkTypeNode(((GenericType*) type)->concrete(),pos));
            else if ( t == ValueType::TMAP )
                return new MapTypeNode(pos, walkTypeNode(((GenericType*) type)->concrete(),pos));
            else if ( t == ValueType::TRESOURCE || t == ValueType::TFUNCTION || t == ValueType::TERROR )
                assert(false);
            else {
                return new PrimitiveTypeNode(pos,(PrimitiveType*) type);
            }
            assert(false);
        }

        virtual EnumerableTypeNode* walkEnumerableTypeNode(nlohmann::json typejson, Position* pos) {
            TypeNode* type = walkTypeNode(walkType(typejson),pos);
            assert(type->getName() == "EnumerableTypeNode");
            return (EnumerableTypeNode*) type;
        }

        virtual MapTypeNode* walkMapTypeNode(nlohmann::json typejson, Position* pos) {
            TypeNode* type = walkTypeNode(walkType(typejson),pos);
            assert(type->getName() == "MapTypeNode");
            return (MapTypeNode*) type;
        }

        virtual BooleanLiteralExpressionNode* walkBooleanLiteralExpressionNode(bool val, Position* pos) {
            return new BooleanLiteralExpressionNode(pos,val);
        }

        virtual VariableDeclarationNode* walkVariableDeclarationNode(nlohmann::json typejson, nlohmann::json idjson, nlohmann::json valjson, Position* pos) {
            ASTNode* type = walk(typejson);
            console->debug("Got Var Type!");
            ASTNode* identifier = walk(idjson);
            ASTNode* value = walk(valjson);
            assert(type->isType());
            assert(identifier->getName() == "IdentifierNode");
            assert(value->isExpression());
            VariableDeclarationNode* vdn = new VariableDeclarationNode(
                pos,
                (TypeNode*) type,
                (IdentifierNode*) identifier,
                (ExpressionNode*) value
            );
            return vdn;
        }

        virtual CallExpressionNode* walkCallExpressionNode(nlohmann::json idjson,JSONList argsjson,Position* pos) {
            ASTNode* identifier = walk(idjson);
            std::vector<ExpressionNode*>* args = new std::vector<ExpressionNode*>();
            for (auto x : argsjson) {
                ASTNode* exp = walk(x);
                assert(exp->isExpression());
                args->push_back((ExpressionNode*) exp);
            }
            assert(identifier->getName() == "IdentifierNode");
            return new CallExpressionNode(pos, (IdentifierNode*) identifier, args);
        }

        virtual AndNode* walkAndNode(nlohmann::json leftjson, nlohmann::json rightjson,Position* pos) {
            ASTNode* left = walk(leftjson);
            ASTNode* right = walk(rightjson);
            assert(left->isExpression());
            assert(right->isExpression());
            return new AndNode(pos, (ExpressionNode*) left, (ExpressionNode*) right);
        }

        virtual OrNode* walkOrNode(nlohmann::json leftjson, nlohmann::json rightjson,Position* pos) {
            ASTNode* left = walk(leftjson);
            ASTNode* right = walk(rightjson);
            assert(left->isExpression());
            assert(right->isExpression());
            return new OrNode(pos, (ExpressionNode*) left, (ExpressionNode*) right);
        }

        virtual EqualsNode* walkEqualsNode(nlohmann::json leftjson, nlohmann::json rightjson,Position* pos) {
            ASTNode* left = walk(leftjson);
            ASTNode* right = walk(rightjson);
            assert(left->isExpression());
            assert(right->isExpression());
            return new EqualsNode(pos, (ExpressionNode*) left, (ExpressionNode*) right);
        }

        virtual NotEqualsNode* walkNotEqualsNode(nlohmann::json leftjson, nlohmann::json rightjson,Position* pos) {
            ASTNode* left = walk(leftjson);
            ASTNode* right = walk(rightjson);
            assert(left->isExpression());
            assert(right->isExpression());
            return new NotEqualsNode(pos, (ExpressionNode*) left, (ExpressionNode*) right);
        }

        virtual AddNode* walkAddNode(nlohmann::json leftjson, nlohmann::json rightjson,Position* pos) {
            ASTNode* left = walk(leftjson);
            ASTNode* right = walk(rightjson);
            assert(left->isExpression());
            assert(right->isExpression());
            return new AddNode(pos, (ExpressionNode*) left, (ExpressionNode*) right);
        }

        virtual AddAssignExpressionNode* walkAddAssignExpressionNode(nlohmann::json destjson, nlohmann::json expjson, Position* pos) {
            ASTNode* dest = walk(destjson);
            ASTNode* exp = walk(expjson);
            assert(dest->isLVal());
            assert(exp->isExpression());
            return new AddAssignExpressionNode(pos,(LValNode*) dest,(ExpressionNode*) exp);
        }

        virtual SubtractNode* walkSubtractNode(nlohmann::json leftjson, nlohmann::json rightjson,Position* pos) {
            ASTNode* left = walk(leftjson);
            ASTNode* right = walk(rightjson);
            assert(left->isExpression());
            assert(right->isExpression());
            return new SubtractNode(pos, (ExpressionNode*) left, (ExpressionNode*) right);
        }

        virtual MultiplyNode* walkMultiplyNode(nlohmann::json leftjson, nlohmann::json rightjson,Position* pos) {
            ASTNode* left = walk(leftjson);
            ASTNode* right = walk(rightjson);
            assert(left->isExpression());
            assert(right->isExpression());
            return new MultiplyNode(pos, (ExpressionNode*) left, (ExpressionNode*) right);
        }

        virtual MultiplyAssignExpressionNode* walkMultiplyAssignExpressionNode(nlohmann::json destjson, nlohmann::json expjson, Position* pos) {
            ASTNode* dest = walk(destjson);
            ASTNode* exp = walk(expjson);
            assert(dest->isLVal());
            assert(exp->isExpression());
            return new MultiplyAssignExpressionNode(pos,(LValNode*) dest,(ExpressionNode*) exp);
        }

        virtual DivideNode* walkDivideNode(nlohmann::json leftjson, nlohmann::json rightjson,Position* pos) {
            ASTNode* left = walk(leftjson);
            ASTNode* right = walk(rightjson);
            assert(left->isExpression());
            assert(right->isExpression());
            return new DivideNode(pos, (ExpressionNode*) left, (ExpressionNode*) right);
        }

        virtual ModulusNode* walkModulusNode(nlohmann::json leftjson, nlohmann::json rightjson,Position* pos) {
            ASTNode* left = walk(leftjson);
            ASTNode* right = walk(rightjson);
            assert(left->isExpression());
            assert(right->isExpression());
            return new ModulusNode(pos, (ExpressionNode*) left, (ExpressionNode*) right);
        }

        virtual PowerNode* walkPowerNode(nlohmann::json leftjson, nlohmann::json rightjson,Position* pos) {
            ASTNode* left = walk(leftjson);
            ASTNode* right = walk(rightjson);
            assert(left->isExpression());
            assert(right->isExpression());
            return new PowerNode(pos, (ExpressionNode*) left, (ExpressionNode*) right);
        }

        virtual ConcatenateNode* walkConcatenateNode(nlohmann::json leftjson, nlohmann::json rightjson,Position* pos) {
            ASTNode* left = walk(leftjson);
            ASTNode* right = walk(rightjson);
            assert(left->isExpression());
            assert(right->isExpression());
            return new ConcatenateNode(pos, (ExpressionNode*) left, (ExpressionNode*) right);
        }

        virtual NumericComparisonExpressionNode* walkNumericComparisonExpressionNode(nlohmann::json leftjson, nlohmann::json rightjson, std::string comparisonType, Position* pos) {
            ASTNode* left = walk(leftjson);
            ASTNode* right = walk(rightjson);
            assert(left->isExpression());
            assert(right->isExpression());

            auto compType = NumberComparisonType::LESS_THAN;
            if ( comparisonType == "LESS_THAN_OR_EQUAL" ) compType = Lang::NumberComparisonType::LESS_THAN_OR_EQUAL;
            if ( comparisonType == "GREATER_THAN" ) compType = Lang::NumberComparisonType::GREATER_THAN;
            if ( comparisonType == "GREATER_THAN_OR_EQUAL" ) compType = Lang::NumberComparisonType::GREATER_THAN_OR_EQUAL;

            return new NumericComparisonExpressionNode(pos, compType, (ExpressionNode*) left, (ExpressionNode*) right);
        }

        virtual NegativeExpressionNode* walkNegativeExpressionNode(nlohmann::json expjson, Position* pos) {
            ASTNode* exp = walk(expjson);
            assert(exp->isExpression());
            return new NegativeExpressionNode(pos, (ExpressionNode*) exp);
        }

        virtual NotNode* walkNotNode(nlohmann::json expjson, Position* pos) {
            ASTNode* exp = walk(expjson);
            assert(exp->isExpression());
            return new NotNode(pos, (ExpressionNode*) exp);
        }

        virtual EnumerationLiteralExpressionNode* walkEnumerationLiteralExpressionNode(JSONList actualsjson, Position* pos) {
            ExpressionList* actuals = new ExpressionList();

            for (auto a : actualsjson) {
                ASTNode* exp = walk(a);
                assert(exp->isExpression());
                actuals->push_back((ExpressionNode*) exp);
            }

            return new EnumerationLiteralExpressionNode(pos,actuals);
        }

        virtual EnumerationStatement* walkEnumerationStatement(nlohmann::json enumjson, nlohmann::json localjson, JSONList body,Position* pos) {
            ASTNode* enumerable = walk(enumjson);
            ASTNode* local = walk(localjson);
            assert(enumerable->getName() == "IdentifierNode");
            assert(local->getName() == "IdentifierNode");

            EnumerationStatement* es = new EnumerationStatement(
                pos,
                (IdentifierNode*) enumerable,
                (IdentifierNode*) local,
                false);
            
            for (auto stmt : body) {
                ASTNode* s = walk(stmt);
                assert(s->isStatement());
                es->pushStatement((StatementNode*) s);
            }

            return es;
        }

        virtual WithStatement* walkWithStatement(JSONList body, nlohmann::json localjson, nlohmann::json resjson, Position* pos) {
            ASTNode* local = walk(localjson);
            ASTNode* res = walk(resjson);
            assert(local->getName() == "IdentifierNode");
            assert(res->isExpression());

            WithStatement* with = new WithStatement(pos,(ExpressionNode*) res,(IdentifierNode*) local, false);

            for (auto stmt : body) {
                ASTNode* s = walk(stmt);
                assert(s->isStatement());
                with->pushStatement((StatementNode*) s);
            }

            return with;
        }

        virtual IfStatement* walkIfStatement(JSONList body, nlohmann::json condjson, Position* pos) {
            ASTNode* cond = walk(condjson);
            assert(cond->isExpression());

            IfStatement* ifstmt = new IfStatement(pos,(ExpressionNode*) cond);

            for (auto stmt : body) {
                ASTNode* s = walk(stmt);
                assert(s->isStatement());
                ifstmt->pushStatement((StatementNode*) s);
            }

            return ifstmt;
        }

        virtual WhileStatement* walkWhileStatement(JSONList body, nlohmann::json condjson, Position* pos) {
            ASTNode* cond = walk(condjson);
            assert(cond->isExpression());

            WhileStatement* whilstmt = new WhileStatement(pos,(ExpressionNode*) cond);

            for (auto stmt : body) {
                ASTNode* s = walk(stmt);
                assert(s->isStatement());
                whilstmt->pushStatement((StatementNode*) s);
            }

            return whilstmt;
        }

        virtual MapStatementNode* walkMapStatementNode(std::string fakeid, nlohmann::json valjson, Position* pos) {
            IdentifierNode* identifier = new IdentifierNode(pos,fakeid);
            ASTNode* exp = walk(valjson);
            assert(exp->isExpression());
            return new MapStatementNode(pos, identifier, (ExpressionNode*) exp);
        }

        virtual MapNode* walkMapNode(JSONList bodyjson, Position* pos) {
            MapBody* body = new MapBody();
            for (auto stmt : bodyjson) {
                ASTNode* mapstmt = walk(stmt);
                assert(mapstmt->getName() == "MapStatementNode");
                body->push_back((MapStatementNode*) mapstmt);
            }

            return new MapNode(pos,body);
        }

        virtual StringLiteralExpressionNode* walkStringLiteralExpressionNode(std::string value, Position* pos) {
            return new StringLiteralExpressionNode(pos,value);
        }

        virtual NumberLiteralExpressionNode* walkNumberLiteralExpressionNode(double value, Position* pos) {
            return new NumberLiteralExpressionNode(pos,value);
        }

        virtual AssignExpressionNode* walkAssignExpressionNode(nlohmann::json destjson, nlohmann::json valjson, Position* pos) {
            ASTNode* dest = walk(destjson);
            ASTNode* val = walk(valjson);
            assert(dest->isLVal());
            assert(val->isExpression());
            return new AssignExpressionNode(pos,(LValNode*) dest,(ExpressionNode*) val);
        }

        virtual IntegerLiteralExpressionNode* walkIntegerLiteralExpressionNode(double value, Position* pos) {
            return new IntegerLiteralExpressionNode(pos, (int)value);
        }

        virtual UnitNode* walkUnitNode(Position* pos) {
            return new UnitNode(pos);
        }

        std::string inputToString(std::istream& input) {
            std::string output, temp;
            while (input >> temp) output += temp;
            return output;
        }

        ASTNode* walk(nlohmann::json prog) {
            std::string name = prog["astNodeName"];
            console->debug("Walking "+name);
            Position* pos = new Position(
                prog["position"]["startLine"],
                prog["position"]["endLine"],
                prog["position"]["startCol"],
                prog["position"]["endCol"]);
            if ( name == "ProgramNode" ) return walkProgramNode(prog["body"],pos);
            else if ( name == "ExpressionStatementNode" ) return walkExpressionStatementNode(prog["expression"],pos);
            else if ( name == "IdentifierNode" ) return walkIdentifierNode(prog["name"],prog["symbol"],pos);
            else if ( name == "MapAccessNode" ) return walkMapAccessNode(prog["end"],prog["end_pos"],prog["path"],pos);
            else if ( name == "EnumerableAccessNode" ) return walkEnumerableAccessNode(prog["path"],prog["index"],pos);
            else if ( name == "PrimitiveTypeNode" ) return walkPrimitiveTypeNode(prog["type"]["valueType"],pos);
            else if ( name == "EnumerableTypeNode" ) return walkEnumerableTypeNode(prog["type"],pos);
            else if ( name == "MapTypeNode" ) return walkMapTypeNode(prog["type"],pos);
            else if ( name == "BooleanLiteralExpressionNode" ) return walkBooleanLiteralExpressionNode(prog["value"],pos);
            else if ( name == "VariableDeclarationNode" ) return walkVariableDeclarationNode(prog["typeNode"],prog["identifier"],prog["value"],pos);
            else if ( name == "CallExpressionNode" ) return walkCallExpressionNode(prog["identifier"],prog["arguments"],pos);
            else if ( name == "AndNode" ) return walkAndNode(prog["left"],prog["right"],pos);
            else if ( name == "OrNode" ) return walkOrNode(prog["left"],prog["right"],pos);
            else if ( name == "EqualsNode" ) return walkEqualsNode(prog["left"],prog["right"],pos);
            else if ( name == "NotEqualsNode" ) return walkNotEqualsNode(prog["left"],prog["right"],pos);
            else if ( name == "AddNode" ) return walkAddNode(prog["left"],prog["right"],pos);
            else if ( name == "AddAssignExpressionNode" ) return walkAddAssignExpressionNode(prog["dest"],prog["value"],pos);
            else if ( name == "SubtractNode" ) return walkSubtractNode(prog["left"],prog["right"],pos);
            else if ( name == "MultiplyNode" ) return walkMultiplyNode(prog["left"],prog["right"],pos);
            else if ( name == "MultiplyAssignExpressionNode" ) return walkMultiplyAssignExpressionNode(prog["dest"],prog["value"],pos);
            else if ( name == "DivideNode" ) return walkDivideNode(prog["left"],prog["right"],pos);
            else if ( name == "ModulusNode" ) return walkModulusNode(prog["left"],prog["right"],pos);
            else if ( name == "PowerNode" ) return walkPowerNode(prog["left"],prog["right"],pos);
            else if ( name == "ConcatenateNode" ) return walkConcatenateNode(prog["left"],prog["right"],pos);
            else if ( name == "NegativeExpressionNode" ) return walkNegativeExpressionNode(prog["expression"],pos);
            else if ( name == "NotNode" ) return walkNotNode(prog["expression"],pos);
            else if ( name == "EnumerationLiteralExpressionNode" ) return walkEnumerationLiteralExpressionNode(prog["actuals"],pos);
            else if ( name == "EnumerationStatement" ) return walkEnumerationStatement(prog["enumerable"],prog["local"],prog["body"],pos);
            else if ( name == "WithStatement" ) return walkWithStatement(prog["body"],prog["local"],prog["resource"],pos);
            else if ( name == "IfStatement" ) return walkIfStatement(prog["body"],prog["condition"],pos);
            else if ( name == "WhileStatement" ) return walkWhileStatement(prog["body"],prog["condition"],pos);
            else if ( name == "MapStatementNode" ) return walkMapStatementNode(prog["mapStatementIdentifier"],prog["value"],pos);
            else if ( name == "MapNode" ) return walkMapNode(prog["body"],pos);
            else if ( name == "StringLiteralExpressionNode" ) return walkStringLiteralExpressionNode(prog["value"],pos);
            else if ( name == "NumberLiteralExpressionNode" ) return walkNumberLiteralExpressionNode(prog["value"],pos);
            else if ( name == "AssignExpressionNode" ) return walkAssignExpressionNode(prog["dest"],prog["value"],pos);
            else if ( name == "IntegerLiteralExpressionNode" ) return walkIntegerLiteralExpressionNode(prog["value"],pos);
            else if ( name == "UnitNode" ) return walkUnitNode(pos);
            else if ( name == "NumericComparisonExpressionNode" ) return walkNumericComparisonExpressionNode(prog["left"], prog["right"], prog["comparisonType"], pos);
            assert(false);
        }
    
    private:
        std::unordered_map<std::string,SemanticSymbol*> _uuidList;
    };
}
}

#endif