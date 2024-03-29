#pragma once

const char* test_output = R"(ProgramNode<#body: 29>
  IncludeStatementNode<ClassAccessNode<path: IdentifierNode<name: Other> id: File>>
  IncludeStatementNode<ClassAccessNode<path: ClassAccessNode<path: IdentifierNode<name: Other> id: Other> id: File>>
  IncludeStatementNode<ClassAccessNode<path: ClassAccessNode<path: IdentifierNode<name: Other> id: Other> id: File>>
  VariableDeclarationNode<name: log, shared:false>
    Type<Primitive<STRING> :: Primitive<VOID>>
    FunctionNode<type: Type<Primitive<STRING> :: Primitive<VOID>>>
  VariableDeclarationNode<name: t1, shared:false>
    Type<ENUMERABLE<Primitive<NUMBER>>>
    EnumerationLiteralExpressionNode<#actuals: 3>
      NumberLiteralExpressionNode<#value: 1.000000>
      NumberLiteralExpressionNode<#value: 2.000000>
      NumberLiteralExpressionNode<#value: 3.000000>
  VariableDeclarationNode<name: t2, shared:true>
    Type<ENUMERABLE<ENUMERABLE<Primitive<STRING>>>>
    EnumerationLiteralExpressionNode<#actuals: 0>
  VariableDeclarationNode<name: t3, shared:false>
    Type<Primitive<NUMBER>>
    NumberLiteralExpressionNode<#value: 4.000000>
  VariableDeclarationNode<name: t4, shared:true>
    Type<Primitive<STRING>>
    StringLiteralExpressionNode<#value: 'zoinks'>
  VariableDeclarationNode<name: t5, shared:false>
    Type<MAP<Primitive<NUMBER> :: Primitive<NUMBER>>>
    MapNode<#body: 0>
  VariableDeclarationNode<name: t6, shared:true>
    Type<MAP<ENUMERABLE<Primitive<NUMBER> :: Primitive<STRING>>>>
    MapNode<#body: 2>
      MapStatementNode<id: a>
        EnumerationLiteralExpressionNode<#actuals: 1>
          FunctionNode<type: Type<Primitive<NUMBER> :: Primitive<STRING>>>
            ReturnStatementNode<lval: StringLiteralExpressionNode<#value: 'hello'>>
              StringLiteralExpressionNode<#value: 'hello'>
      MapStatementNode<id: b>
        EnumerationLiteralExpressionNode<#actuals: 2>
          FunctionNode<type: Type<Primitive<NUMBER> :: Primitive<STRING>>>
            ReturnStatementNode<lval: StringLiteralExpressionNode<#value: 'goodbye'>>
              StringLiteralExpressionNode<#value: 'goodbye'>
          FunctionNode<type: Type<Primitive<NUMBER> :: Primitive<STRING>>>
            ReturnStatementNode<lval: StringLiteralExpressionNode<#value: 'zoinks'>>
              StringLiteralExpressionNode<#value: 'zoinks'>
  EnumerationStatement<e: MapAccessNode<path: IdentifierNode<name: t6> id: b>, as: joe, #body: 1>
    WithStatement<r: CallExpressionNode<#args: 2>, as: _>
      ExpressionStatementNode<AssignExpressionNode<lval: EnumerableAccessNode<path: IdentifierNode<name: t1>, index: NumberLiteralExpressionNode<#value: 1.000000>>>>
        AssignExpressionNode<lval: EnumerableAccessNode<path: IdentifierNode<name: t1>, index: NumberLiteralExpressionNode<#value: 1.000000>>>
          NumberLiteralExpressionNode<#value: 4.000000>
  EnumerationStatement<e: MapAccessNode<path: IdentifierNode<name: t6> id: b>, as: var, #body: 1>
    WithStatement<r: CallExpressionNode<#args: 2>, as: w>
      ExpressionStatementNode<CallExpressionNode<#args: 1>>
        CallExpressionNode<#args: 1>
          IdentifierNode<name: log>
          StringLiteralExpressionNode<#value: 'string'>
  EnumerationStatement<e: MapAccessNode<path: IdentifierNode<name: t6> id: b>, as: v, #body: 0>
  EnumerationStatement<e: MapAccessNode<path: IdentifierNode<name: t6> id: b>, as: v, #body: 0>
  EnumerationStatement<e: MapAccessNode<path: IdentifierNode<name: t6> id: b>, as: v, #body: 0>
  EnumerationStatement<e: MapAccessNode<path: IdentifierNode<name: t6> id: b>, as: v, #body: 0>
  WhileStatement<w: while AndNode<> then, #body: 9>
    IfStatement<f: if EqualsNode<> then, #body: 1>
      ContinueNode<>
    IfStatement<f: if EqualsNode<> then, #body: 3>
      VariableDeclarationNode<name: func, shared:false>
        Type<:: Primitive<VOID>>
        FunctionNode<type: Type<:: Primitive<VOID>>>
          ReturnStatementNode<>
      VariableDeclarationNode<name: func2, shared:true>
        Type<:: Primitive<BOOLEAN>>
        FunctionNode<type: Type<:: Primitive<BOOLEAN>>>
          ReturnStatementNode<lval: NotNode<>>
            NotNode<>
              OrNode<>
                BoolLiteralNode<of: 1>
                BoolLiteralNode<of: 0>
      BreakNode<>
    ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: t3>>>
      AssignExpressionNode<lval: IdentifierNode<name: t3>>
        AddNode<>
          IdentifierNode<name: t3>
          NumberLiteralExpressionNode<#value: 0.100000>
    ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: t3>>>
      AssignExpressionNode<lval: IdentifierNode<name: t3>>
        SubtractNode<>
          IdentifierNode<name: t3>
          NumberLiteralExpressionNode<#value: 1.100000>
    ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: t3>>>
      AssignExpressionNode<lval: IdentifierNode<name: t3>>
        MultiplyNode<>
          IdentifierNode<name: t3>
          NumberLiteralExpressionNode<#value: 1.000000>
    ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: t3>>>
      AssignExpressionNode<lval: IdentifierNode<name: t3>>
        DivideNode<>
          IdentifierNode<name: t3>
          NumberLiteralExpressionNode<#value: 1.000000>
    ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: t3>>>
      AssignExpressionNode<lval: IdentifierNode<name: t3>>
        PowerNode<>
          IdentifierNode<name: t3>
          NumberLiteralExpressionNode<#value: 1.000000>
    ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: t3>>>
      AssignExpressionNode<lval: IdentifierNode<name: t3>>
        ModulusNode<>
          IdentifierNode<name: t3>
          NumberLiteralExpressionNode<#value: 1.000000>
    ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: t4>>>
      AssignExpressionNode<lval: IdentifierNode<name: t4>>
        AddNode<>
          IdentifierNode<name: t4>
          StringLiteralExpressionNode<#value: 's'>
  ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: t3>>>
    AssignExpressionNode<lval: IdentifierNode<name: t3>>
      AddNode<>
        NumberLiteralExpressionNode<#value: 1.000000>
        ModulusNode<>
          DivideNode<>
            SubtractNode<>
              NumberLiteralExpressionNode<#value: 2.000000>
              NumberLiteralExpressionNode<#value: 3.000000>
            NegativeExpressionNode<>
              NumberLiteralExpressionNode<#value: 4.000000>
          PowerNode<>
            NumberLiteralExpressionNode<#value: 1.000000>
            NumberLiteralExpressionNode<#value: 5.000000>
  VariableDeclarationNode<name: t7, shared:false>
    Type<Primitive<BOOLEAN>>
    BoolLiteralNode<of: 1>
  ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: t7>>>
    AssignExpressionNode<lval: IdentifierNode<name: t7>>
      AndNode<>
        IdentifierNode<name: t7>
        BoolLiteralNode<of: 0>
  ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: t7>>>
    AssignExpressionNode<lval: IdentifierNode<name: t7>>
      OrNode<>
        IdentifierNode<name: t7>
        BoolLiteralNode<of: 1>
  ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: t4>>>
    AssignExpressionNode<lval: IdentifierNode<name: t4>>
      AddNode<>
        IdentifierNode<name: t4>
        IdentifierNode<name: t4>
  VariableDeclarationNode<name: t8, shared:false>
    Type<Primitive<NUMBER> :: Primitive<TYPE> :: Primitive<TYPE>>
    FunctionNode<type: Type<Primitive<NUMBER> :: Primitive<TYPE> :: Primitive<TYPE>>>
      ReturnStatementNode<lval: Type<Primitive<NUMBER>>>
        Type<Primitive<NUMBER>>
  VariableDeclarationNode<name: t9, shared:true>
    Type<:: Primitive<TYPE>>
    FunctionNode<type: Type<:: Primitive<TYPE>>>
      ReturnStatementNode<lval: Type<Primitive<NUMBER>>>
        Type<Primitive<NUMBER>>
  ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: t8>>>
    AssignExpressionNode<lval: IdentifierNode<name: t8>>
      IIFExpressionNode<func: FunctionNode<type: Type<:: Primitive<NUMBER> :: Primitive<TYPE> :: Primitive<TYPE>>>, #args: 0>
  VariableDeclarationNode<name: t11, shared:false>
    Type<Primitive<TYPE> :: Primitive<TYPE>>
    CallExpressionNode<#args: 1>
      IdentifierNode<name: t8>
      NumberLiteralExpressionNode<#value: 69.000000>
  VariableDeclarationNode<name: UserDefined, shared:false>
    Type<Primitive<TYPE>>
    TypeBodyNode<#type:Type::Object<#prop: 3, parent: (nullptr)>>
      UninitializedVariableDeclarationNode<name: uninit>
        Type<Primitive<NUMBER>>
      VariableDeclarationNode<name: regFunc, shared:false>
        Type<:: Primitive<NUMBER> :: Type::Object<#prop: 3, parent: (nullptr)>>
        FunctionNode<type: Type<:: Primitive<NUMBER> :: Type::Object<#prop: 3, parent: (nullptr)>>>
          ReturnStatementNode<lval: FunctionNode<type: Type<Primitive<NUMBER> :: Type::Object<#prop: 3, parent: (nullptr)>>>>
            FunctionNode<type: Type<Primitive<NUMBER> :: Type::Object<#prop: 3, parent: (nullptr)>>>
              ReturnStatementNode<lval: CallExpressionNode<#args: 1>>
                CallExpressionNode<#args: 1>
                  IdentifierNode<name: UserDefined>
                  IdentifierNode<name: i>
      VariableDeclarationNode<name: copy, shared:false>
        Type<Type::Object<#prop: 3, parent: (nullptr)> :: Type::Object<#prop: 3, parent: (nullptr)>>
        FunctionNode<type: Type<Type::Object<#prop: 3, parent: (nullptr)> :: Type::Object<#prop: 3, parent: (nullptr)>>>
          ReturnStatementNode<lval: CallExpressionNode<#args: 1>>
            CallExpressionNode<#args: 1>
              IdentifierNode<name: UserDefined>
              ClassAccessNode<path: IdentifierNode<name: orig> id: uninit>
                IdentifierNode<name: orig>
      ConstructorNode<name: constructor1>
        FunctionNode<type: Type<:: Primitive<VOID>>>
          ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: uninit>>>
            AssignExpressionNode<lval: IdentifierNode<name: uninit>>
              NumberLiteralExpressionNode<#value: 5.000000>
      ConstructorNode<name: constructor2>
        FunctionNode<type: Type<Primitive<NUMBER> :: Primitive<VOID>>>
          ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: uninit>>>
            AssignExpressionNode<lval: IdentifierNode<name: uninit>>
              NumberLiteralExpressionNode<#value: 5.000000>
          IfStatement<f: if NumericComparisonExpressionNode<type: GREATER_THAN> then, #body: 1>
            ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: uninit>>>
              AssignExpressionNode<lval: IdentifierNode<name: uninit>>
                IdentifierNode<name: n>
  VariableDeclarationNode<name: obj, shared:false>
    Type<Type::Object<#prop: 3, parent: (nullptr)>>
    CallExpressionNode<#args: 0>
      IdentifierNode<name: UserDefined>
  ExpressionStatementNode<AssignExpressionNode<lval: IdentifierNode<name: obj>>>
    AssignExpressionNode<lval: IdentifierNode<name: obj>>
      CallExpressionNode<#args: 1>
        CallExpressionNode<#args: 0>
          ClassAccessNode<path: IdentifierNode<name: obj> id: regFunc>
            IdentifierNode<name: obj>
        NumberLiteralExpressionNode<#value: 4.000000>
)";
