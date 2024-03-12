#include "ASTMapReduce.h"

namespace swarmc::Lang::Walk {

ASTMapReduce<bool>* removeRedundantCFB() {
    static ASTMapReduce<bool> rrCFB = ASTMapReduce<bool>(
        "AST Remove Post-CFB",
        [](ASTNode* node) {
            bool flag = false;
            auto log = Logging::get()->get("AST Remove Post-CFB");
            if ( node->isBlock() || node->getTag() == ASTNodeTag::FUNCTION ) {
                auto bsn = (node->getTag() == ASTNodeTag::FUNCTION)
                    ? ((FunctionNode*)node)->body()
                    : ((BlockStatementNode*)node)->body();
                auto i = bsn->begin();
                for ( ; i != bsn->end(); i++ ) {
                    if ( (*i)->getTag() == ASTNodeTag::RETURN
                        || (*i)->getTag() == ASTNodeTag::CONTINUE
                        || (*i)->getTag() == ASTNodeTag::BREAK )
                    {
                        i++;
                        break;
                    }
                }

                // remove post-return/continue/break statements
                if ( i != bsn->end() ) {
                    auto p = new Position((*i)->position(), bsn->back()->position());
                    log->debug(s(p) + " Remove dead statements");
                    GC_LOCAL_REF(p)
                    while ( i != bsn->end() ) {
                        freeref(*i);
                        i = bsn->erase(i);
                    }
                    flag = true;
                }

                // remove unnecessary continue
                if ( node->getTag() == ASTNodeTag::WHILE ) {
                    if ( bsn->size() > 0 && bsn->back()->getTag() == ASTNodeTag::CONTINUE ) {
                        log->debug(s(bsn->back()->position()) + " Removed continue at end of While statement");
                        freeref(bsn->back());
                        bsn->resize(bsn->size() - 1);
                        flag = true;
                    }
                }

                // remove unnecessary return
                if ( node->getTag() == ASTNodeTag::FUNCTION ) {
                    if ( bsn->size() > 0 && bsn->back()->getTag() == ASTNodeTag::RETURN && ((ReturnStatementNode*)bsn->back())->value() == nullptr ) {
                        log->debug(s(bsn->back()->position()) + " Removed void return at end of function");
                        freeref(bsn->back());
                        bsn->resize(bsn->size() - 1);
                        flag = true;
                    }
                }
            }
            return flag;
        },
        [](bool l, bool r) { return l || r; }
    );

    return &rrCFB;
}

ASTMapReduce<bool>* hasReturn() {
    static auto hr = ASTMapReduce<bool>(
        "AST Has Return",
        [](ASTNode* n) {
            return n->getTag() == ASTNodeTag::RETURN;
        },
        [](bool l, bool r) { return l || r; },
        [](ASTNode* n) {
            return n->getTag() == ASTNodeTag::FUNCTION
                || n->getTag() == ASTNodeTag::ENUMERATE;
        }
    );
    return &hr;
}

ASTMapReduce<bool>* hasContinue() {
    static ASTMapReduce<bool> hc = ASTMapReduce<bool>(
        "AST Has Continue",
        [](ASTNode* n) {
            return n->getTag() == ASTNodeTag::CONTINUE;
        },
        [](bool l, bool r) { return l || r; },
        [](ASTNode* n) {
            return n->getTag() == ASTNodeTag::FUNCTION
                || n->getTag() == ASTNodeTag::WHILE
                || n->getTag() == ASTNodeTag::ENUMERATE;
        }
    );
    return &hc;
}

ASTMapReduce<bool>* hasBreak() {
    static ASTMapReduce<bool> hb = ASTMapReduce<bool>(
        "AST Has Break",
        [](ASTNode* n) {
            return n->getTag() == ASTNodeTag::BREAK;
        },
        [](bool l, bool r) { return l || r; },
        [](ASTNode* n) {
            return n->getTag() == ASTNodeTag::FUNCTION
                || n->getTag() == ASTNodeTag::WHILE
                || n->getTag() == ASTNodeTag::ENUMERATE;
        }
    );
    return &hb;
}

}