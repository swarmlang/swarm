#ifndef SWARM_024_SHARED_VARIABLES_H
#define SWARM_024_SHARED_VARIABLES_H

#include <assert.h>
#include <sstream>
#include <iostream>
#include "Test.h"
#include "../pipeline/Pipeline.h"

namespace swarmc {
namespace Test {

    class SharedVariablesTest : public Test {
    public:
        SharedVariablesTest() : Test() {}
    
        bool run() override {
            return (varDeclTest() 
                    && accessTest() 
                    && enumStatement()
                    && withStatement());
        }

        bool varDeclTest() {
            std::stringstream program;
            program << "enumerable<map<map<number>>> v1 = [{w:{a:1},x:{b:2}},{} of map<number>];\n"
                    << "shared enumerable<map<map<number>>> v2 = [{y:{c:3},z:{d:4}},{} of map<number>];\n";

            Pipeline pipeline(&program);
            Lang::ProgramNode* pgNode = pipeline.targetASTSymbolicTyped();

            assert(pgNode->body()->at(0)->getTag() == Lang::ASTNodeTag::VARIABLEDECLARATION);
            assert(pgNode->body()->at(1)->getTag() == Lang::ASTNodeTag::VARIABLEDECLARATION);
            Lang::VariableDeclarationNode* line1 = (Lang::VariableDeclarationNode*) pgNode->body()->at(0);
            Lang::VariableDeclarationNode* line2 = (Lang::VariableDeclarationNode*) pgNode->body()->at(1);

            assert(line1->id()->symbol()->shared() == false);
            assert(line2->id()->symbol()->shared() == true);

            return true;
        }

        bool accessTest() {
            std::stringstream program;
            program << "enumerable<map<number>> e1 = [{a:1,b:2},{} of number];\n"
                    << "shared enumerable<map<number>> e2 = [{c:3,d:4},{} of number];\n"
                    << "e1[0][a] = 3;\n"
                    << "e2[0][d] = 5;\n";

            Pipeline pipeline(&program);
            Lang::ProgramNode* pgNode = pipeline.targetASTSymbolicTyped();

            assert(pgNode->body()->at(2)->getTag() == Lang::ASTNodeTag::EXPRESSIONSTATEMENT);
            assert(pgNode->body()->at(3)->getTag() == Lang::ASTNodeTag::EXPRESSIONSTATEMENT);
            Lang::ExpressionStatementNode* line3 = (Lang::ExpressionStatementNode*) pgNode->body()->at(2);
            Lang::ExpressionStatementNode* line4 = (Lang::ExpressionStatementNode*) pgNode->body()->at(3);

            assert(line3->expression()->getTag() == Lang::ASTNodeTag::ASSIGN);
            assert(line4->expression()->getTag() == Lang::ASTNodeTag::ASSIGN);
            Lang::AssignExpressionNode* line3Exp = (Lang::AssignExpressionNode*) line3->expression();
            Lang::AssignExpressionNode* line4Exp = (Lang::AssignExpressionNode*) line4->expression();

            assert(line3Exp->dest()->getTag() == Lang::ASTNodeTag::MAPACCESS);
            assert(line4Exp->dest()->getTag() == Lang::ASTNodeTag::MAPACCESS);
            Lang::MapAccessNode* lval1 = (Lang::MapAccessNode*) line3Exp->dest();
            Lang::MapAccessNode* lval2 = (Lang::MapAccessNode*) line4Exp->dest();

            // the shared function recursively calls to the leftmost id, so this should
            // test for all sub-lvals as well
            assert(lval1->shared() == false);
            assert(lval2->shared() == true);

            return true;
        }

        bool enumStatement() {
            std::stringstream program;

            program << "enumerable<number> e1 = [69,420,69420];\n"
                    << "shared enumerable<string> e2 = [\"a\",\"b\",\"c\"];\n"
                    << "enumerate e1 as funnynums {\n"
                    << "}\n"
                    << "enumerate e1 as shared funniernums {\n"
                    << "}\n"
                    << "enumerate e2 as strs {\n"
                    << "}\n"
                    << "enumerate e2 as shared sstrs {\n"
                    << "}\n";

            Pipeline pipeline(&program);
            Lang::ProgramNode* pgNode = pipeline.targetASTSymbolicTyped();

            assert(pgNode->body()->at(2)->getTag() == Lang::ASTNodeTag::ENUMERATE);
            assert(pgNode->body()->at(3)->getTag() == Lang::ASTNodeTag::ENUMERATE);
            assert(pgNode->body()->at(4)->getTag() == Lang::ASTNodeTag::ENUMERATE);
            assert(pgNode->body()->at(5)->getTag() == Lang::ASTNodeTag::ENUMERATE);
            Lang::EnumerationStatement* stmt1 = (Lang::EnumerationStatement*) pgNode->body()->at(2);
            Lang::EnumerationStatement* stmt2 = (Lang::EnumerationStatement*) pgNode->body()->at(3);
            Lang::EnumerationStatement* stmt3 = (Lang::EnumerationStatement*) pgNode->body()->at(4);
            Lang::EnumerationStatement* stmt4 = (Lang::EnumerationStatement*) pgNode->body()->at(5);
        
            assert(stmt1->local()->shared() == false);
            assert(stmt2->local()->shared() == true);
            assert(stmt3->local()->shared() == false);
            assert(stmt4->local()->shared() == true);

            return true;
        }

        bool withStatement() {
            std::stringstream program;

            program << "with fileContents(\"file.txt\") as contents {\n"
                    << "}\n"
                    << "with fileContents(\"file.txt\") as shared contents {\n"
                    << "}\n";

            Pipeline pipeline(&program);
            Lang::ProgramNode* pgNode = pipeline.targetASTSymbolicTyped();

            assert(pgNode->body()->at(0)->getTag() == Lang::ASTNodeTag::WITH);
            assert(pgNode->body()->at(1)->getTag() == Lang::ASTNodeTag::WITH);
            Lang::WithStatement* stmt1 = (Lang::WithStatement*) pgNode->body()->at(0);
            Lang::WithStatement* stmt2 = (Lang::WithStatement*) pgNode->body()->at(1);
        
            assert(stmt1->local()->shared() == false);
            assert(stmt2->local()->shared() == true);

            return true;
        }
    };

}
}

#endif //SWARM_024_SHARED_VARIABLES_H