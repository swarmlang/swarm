#include "SharedLocationsWalk.h"
#include "../../vm/isa_meta.h"

namespace swarmc::Lang::Walk {

[[nodiscard]] bool SharedLocations::has(const ISA::LocationReference* loc) const {
    if ( loc->affinity() != ISA::Affinity::SHARED ) return false;
    assert( SharedLocations::_locRefToSymbol.count(loc->fqName()) );
    return _map.count(SharedLocations::_locRefToSymbol.at(loc->fqName())) != 0;
}

[[nodiscard]] bool SharedLocations::locked(const ISA::LocationReference* loc) const {
    if ( has(loc) ) {
        return _locked.at(SharedLocations::_locRefToSymbol.at(loc->fqName()));
    }
    return false;
}

void SharedLocations::dec(const ISA::LocationReference* loc) {
    auto sym = SharedLocations::_locRefToSymbol.at(loc->fqName());
    assert( _map.at(sym) > 0 );
    _map.at(sym) -= 1;
    _locked.at(sym) = true;
}

void SharedLocations::remove(const ISA::LocationReference* loc) {
    auto sym = SharedLocations::_locRefToSymbol.at(loc->fqName());
    _map.erase(sym);
    _locked.erase(sym);
}

bool SharedLocations::shouldUnlock(const ISA::LocationReference* loc) const {
    auto sym = SharedLocations::_locRefToSymbol.at(loc->fqName());
    return _map.at(sym) == 0;
}

[[nodiscard]] SharedLocationsMap::size_type SharedLocations::size() const {
    return _map.size();
}

[[nodiscard]] SharedLocationsMap::iterator SharedLocations::begin() noexcept {
    return _map.begin();
}

[[nodiscard]] SharedLocationsMap::iterator SharedLocations::end() noexcept {
    return _map.end();
}

void SharedLocations::registerLoc(const ISA::LocationReference* loc, const SemanticSymbol* sym) {
    if ( loc->affinity() != ISA::Affinity::SHARED ) return;
    nslib::Logging::get()->get("AST Shared Locations Walk")
        ->debug(s(sym->declaredAt()) + " Registering shared variable " + sym->name());
    if ( _locRefToSymbol.count(loc->fqName()) )_locRefToSymbol.at(loc->fqName()) = sym;
    else _locRefToSymbol.insert({ loc->fqName(), sym });
}

std::map<std::string, const SemanticSymbol*> SharedLocations::_locRefToSymbol;

SharedLocationsWalk::SharedLocationsWalk() : Walk<SharedLocationsMap>("AST Shared Locations Walk") {}

SharedLocations SharedLocationsWalk::getLocs(ASTNode* node) {
    SharedLocationsWalk swalk;
    auto locs = swalk.walk(node);
    // because SVI instructions are atomic, we only need to lock
    // locations that appear more than once in a statement
    SharedLocations sharedLocs;
    std::copy_if(locs.begin(), locs.end(), std::inserter(sharedLocs._map, sharedLocs._map.begin()), [](auto p) {
        return p.second > 1;
    });
    std::string found = "";
    for ( auto p : sharedLocs._map ) {
        sharedLocs._locked.insert({ p.first, false });
        found += p.first->name() + ":" + s(p.second) + ", ";
    }
    if ( found.size() > 0 ) {
        swalk.logger->debug(s(node) + " lockable locations: {" + found.substr(0, found.size()-2) + "}");
    }

    return sharedLocs;
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkProgramNode(ProgramNode* node) {
    return {};
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkExpressionStatementNode(ExpressionStatementNode* node) {
    return walk(node->expression());
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkIdentifierNode(IdentifierNode* node) {
    if ( node->symbol()->shared() ) {
        return { { node->symbol(), 1 } };
    }
    return {};
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkEnumerableAccessNode(EnumerableAccessNode* node) {
    auto sharedLocs = walk(node->path());
    combine(sharedLocs, walk(node->index()));
    return sharedLocs;
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkEnumerableAppendNode(EnumerableAppendNode* node) {
    return walk(node->path());
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkMapAccessNode(MapAccessNode* node) {
    return walk(node->path());
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkClassAccessNode(ClassAccessNode* node) {
    return walk(node->path());
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkIncludeStatementNode(IncludeStatementNode* node) {
    return {};
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkTypeLiteral(TypeLiteral* node) {
    return {};
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkBooleanLiteralExpressionNode(BooleanLiteralExpressionNode* node) {
    return {};
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkStringLiteralExpressionNode(StringLiteralExpressionNode* node) {
    return {};
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkNumberLiteralExpressionNode(NumberLiteralExpressionNode* node) {
    return {};
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkEnumerationLiteralExpressionNode(EnumerationLiteralExpressionNode* node) {
    SharedLocationsMap sharedLocs;

    for ( auto value : *node->actuals() ) {
        combine(sharedLocs, walk(value));
    }

    return sharedLocs;
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkMapStatementNode(MapStatementNode* node) {
    return walk(node->value());
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkMapNode(MapNode* node) {
    return {}; //FIXME?
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkAssignExpressionNode(AssignExpressionNode* node) {
    auto sharedLocs = walk(node->dest());
    combine(sharedLocs, walk(node->value()));
    return sharedLocs;
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkVariableDeclarationNode(VariableDeclarationNode* node) {
    auto sharedLocs = walk(node->typeNode());
    combine(sharedLocs, walk(node->assignment()));
    return sharedLocs;
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkUninitializedVariableDeclarationNode(UninitializedVariableDeclarationNode* node) {
    return {};
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkUseNode(UseNode* node) {
    auto sharedLocs = SharedLocationsMap();
    for ( auto id : *node->ids() ) {
        combine(sharedLocs, walk(id));
    }
    return sharedLocs;
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkReturnStatementNode(ReturnStatementNode* node) {
    if ( node->value() != nullptr ) {
        return walk(node->value());
    }
    return {};
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkFunctionNode(FunctionNode* node) {
    return {};
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkConstructorNode(ConstructorNode* node) {
    return {};
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkTypeBodyNode(TypeBodyNode* node) {
    return {}; // FIXME?
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkCallExpressionNode(CallExpressionNode* node) {
    auto sharedLocs = walk(node->func());
    for ( auto arg : *node->args() ) {
        combine(sharedLocs, walk(arg));
    }
    return sharedLocs;
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkDeferCallExpressionNode(DeferCallExpressionNode* node) {
    return walk(node->call());
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkAndNode(AndNode* node) {
    return walkBinaryExpressionNode(node);
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkOrNode(OrNode* node) {
    return walkBinaryExpressionNode(node);
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkEqualsNode(EqualsNode* node) {
    return walkBinaryExpressionNode(node);
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkNumericComparisonExpressionNode(NumericComparisonExpressionNode* node) {
    return walkBinaryExpressionNode(node);
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkNotEqualsNode(NotEqualsNode* node) {
    return walkBinaryExpressionNode(node);
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkAddNode(AddNode* node) {
    return walkBinaryExpressionNode(node);
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkSubtractNode(SubtractNode* node) {
    return walkBinaryExpressionNode(node);
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkMultiplyNode(MultiplyNode* node) {
    return walkBinaryExpressionNode(node);
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkDivideNode(DivideNode* node) {
    return walkBinaryExpressionNode(node);
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkModulusNode(ModulusNode* node) {
    return walkBinaryExpressionNode(node);
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkPowerNode(PowerNode* node) {
    return walkBinaryExpressionNode(node);
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkNegativeExpressionNode(NegativeExpressionNode* node) {
    return walkUnaryExpressionNode(node);
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkSqrtNode(SqrtNode* node) {
    return walkUnaryExpressionNode(node);
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkNotNode(NotNode* node) {
    return walkUnaryExpressionNode(node);
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkEnumerationStatement(EnumerationStatement* node) {
    return walk(node->enumerable());
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkWithStatement(WithStatement* node) {
    return walk(node->resource());
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkIfStatement(IfStatement* node) {
    return walk(node->condition());
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkWhileStatement(WhileStatement* node) {
    return walk(node->condition());
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkContinueNode(ContinueNode* node) {
    return {};
}

[[nodiscard]] SharedLocationsMap SharedLocationsWalk::walkBreakNode(BreakNode* node) {
    return {};
}

SharedLocationsMap SharedLocationsWalk::walkBinaryExpressionNode(BinaryExpressionNode* node) {
    auto sharedLocs = walk(node->left());
    combine(sharedLocs, walk(node->right()));
    return sharedLocs;
}

SharedLocationsMap SharedLocationsWalk::walkUnaryExpressionNode(UnaryExpressionNode* node) {
    return walk(node->exp());
}

void SharedLocationsWalk::combine(SharedLocationsMap& first, SharedLocationsMap&& second) {
    for ( auto p : second ) {
        if ( first.count(p.first) ) first.at(p.first) += p.second;
        else first.insert({ p.first, p.second});
    }
}

}