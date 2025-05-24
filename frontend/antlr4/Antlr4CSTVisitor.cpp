///
/// @file Antlr4CSTVisitor.cpp
/// @brief Antlr4的具体语法树的遍历产生AST
/// @author zenglj (zenglj@live.com)
/// @version 1.1
/// @date 2024-11-23
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// <tr><td>2024-11-23 <td>1.1     <td>zenglj  <td>表达式版增强
/// </table>
///

#include <string>

#include "Antlr4CSTVisitor.h"
#include "AST.h"
#include "AttrType.h"

#define Instanceof(res, type, var) auto res = dynamic_cast<type>(var)

/// @brief 构造函数
MiniCCSTVisitor::MiniCCSTVisitor()
{}

/// @brief 析构函数
MiniCCSTVisitor::~MiniCCSTVisitor()
{}

/// @brief 遍历CST产生AST
/// @param root CST语法树的根结点
/// @return AST的根节点
ast_node * MiniCCSTVisitor::run(MiniCParser::CompileUnitContext * root)
{
    return std::any_cast<ast_node *>(visitCompileUnit(root));
}

/// @brief 非终结运算符compileUnit的遍历
/// @param ctx CST上下文
std::any MiniCCSTVisitor::visitCompileUnit(MiniCParser::CompileUnitContext * ctx)
{
    // compileUnit: (funcDef | varDecl)* EOF
    // 最开始ctx是cstroot
    // 请注意这里必须先遍历全局变量后遍历函数。肯定可以确保全局变量先声明后使用的规则，但有些情况却不能检查出。
    // 事实上可能函数A后全局变量B后函数C，这时在函数A中是不能使用变量B的，需要报语义错误，但目前的处理不会。
    // 因此在进行语义检查时，可能追加检查行号和列号，如果函数的行号/列号在全局变量的行号/列号的前面则需要报语义错误
    // TODO 请追加实现。

    ast_node * temp_node;
    ast_node * compileUnitNode = create_contain_node(ast_operator_type::AST_OP_COMPILE_UNIT);

    // 可能多个变量，因此必须循环遍历
    for (auto varCtx: ctx->varDecl()) {

        // 变量函数定义
        temp_node = std::any_cast<ast_node *>(visitVarDecl(varCtx));
        (void) compileUnitNode->insert_son_node(temp_node);
    }

    // 可能有多个函数，因此必须循环遍历
    for (auto funcCtx: ctx->funcDef()) {

        // 变量函数定义
        temp_node = std::any_cast<ast_node *>(visitFuncDef(funcCtx));
        (void) compileUnitNode->insert_son_node(temp_node);
    }

    return compileUnitNode;
}

/// @brief 非终结运算符funcDef的遍历
/// @param ctx CST上下文
/// @return AST的节点
std::any MiniCCSTVisitor::visitFuncDef(MiniCParser::FuncDefContext * ctx)
{
    // 获取函数返回类型
    auto funcReturnType = std::any_cast<type_attr>(visitFuncType(ctx->funcType()));

    // 创建返回类型节点
    auto returnTypeNode = create_type_node(funcReturnType);

    // 获取函数名
    char * id = strdup(ctx->T_ID()->getText().c_str());
    auto funcNameNode = ast_node::New(id, (int64_t) ctx->T_ID()->getSymbol()->getLine());
    free(id);

    // 获取形参列表（如果存在）
    ast_node * formalParamsNode = nullptr;
    if (ctx->funcFParams()) {
        formalParamsNode = std::any_cast<ast_node *>(visitFuncFParams(ctx->funcFParams()));
    }

    // 获取函数体
    auto blockNode = std::any_cast<ast_node *>(visitBlock(ctx->block()));

    // 调用 AST 中的 create_func_def 函数
    auto funcDefNode = create_func_def(returnTypeNode, funcNameNode, blockNode, formalParamsNode);

    // 返回函数定义节点
    return funcDefNode;
}

/// @brief 非终结运算符funcType的遍历
/// @param ctx CST上下文
/// @return 函数类型的属性
std::any MiniCCSTVisitor::visitFuncType(MiniCParser::FuncTypeContext * ctx)
{
    type_attr attr{BasicType::TYPE_VOID, -1};
    if (ctx->T_INT()) {
        attr.type = BasicType::TYPE_INT;
        attr.lineno = (int64_t) ctx->T_INT()->getSymbol()->getLine();
    } else if (ctx->T_VOID()) {
        attr.type = BasicType::TYPE_VOID;
        attr.lineno = (int64_t) ctx->T_VOID()->getSymbol()->getLine();
    }
    // TODO 返回float类型的返回值
    return attr;
}

/// @brief 非终结运算符funcFParams的遍历
/// @param ctx CST上下文
/// @return AST的节点
std::any MiniCCSTVisitor::visitFuncFParams(MiniCParser::FuncFParamsContext * ctx)
{
    // 创建形参列表节点
    auto paramsNode = create_contain_node(ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS);

    for (auto paramCtx: ctx->funcFParam()) {
        auto paramNode = std::any_cast<ast_node *>(visitFuncFParam(paramCtx));
        paramsNode->insert_son_node(paramNode);
    }

    return paramsNode;
}

/// @brief 非终结运算符funcFParam的遍历
/// @param ctx CST上下文
/// @return AST的节点
std::any MiniCCSTVisitor::visitFuncFParam(MiniCParser::FuncFParamContext * ctx)
{
    // 获取形参类型
    auto typeAttr = std::any_cast<type_attr>(visitBasicType(ctx->basicType()));

    // 获取形参名称
    auto idNode = ast_node::New(ctx->T_ID()->getText(), (int64_t) ctx->T_ID()->getSymbol()->getLine());

    // 创建形参节点
    return create_var_decl_node(typeAttr, idNode);
}

/// @brief 非终结运算符block的遍历
/// @param ctx CST上下文
/// @return AST的节点
std::any MiniCCSTVisitor::visitBlock(MiniCParser::BlockContext * ctx)
{
    // 识别的文法产生式：block : T_L_BRACE blockItemList? T_R_BRACE';
    if (!ctx->blockItemList()) {
        // 语句块没有语句

        // 为了方便创建一个空的Block节点
        return create_contain_node(ast_operator_type::AST_OP_BLOCK);
    }

    // 语句块含有语句

    // 内部创建Block节点，并把语句加入，这里不需要创建Block节点
    return visitBlockItemList(ctx->blockItemList());
}

/// @brief 非终结运算符blockItemList的遍历
/// @param ctx CST上下文
std::any MiniCCSTVisitor::visitBlockItemList(MiniCParser::BlockItemListContext * ctx)
{
    // 识别的文法产生式：blockItemList : blockItem +;
    // 正闭包 循环 至少一个blockItem
    auto block_node = create_contain_node(ast_operator_type::AST_OP_BLOCK);

    for (auto blockItemCtx: ctx->blockItem()) {

        // 非终结符，需遍历
        auto blockItem = std::any_cast<ast_node *>(visitBlockItem(blockItemCtx));

        // 插入到块节点中
        (void) block_node->insert_son_node(blockItem);
    }

    return block_node;
}

///
/// @brief 非终结运算符blockItem的遍历
/// @param ctx CST上下文
///
std::any MiniCCSTVisitor::visitBlockItem(MiniCParser::BlockItemContext * ctx)
{
    // 识别的文法产生式：blockItem : statement | varDecl
    if (ctx->statement()) {
        // 语句识别

        return visitStatement(ctx->statement());
    } else if (ctx->varDecl()) {
        return visitVarDecl(ctx->varDecl());
    }

    return nullptr;
}

/// @brief 非终结运算符statement中的遍历
/// @param ctx CST上下文
std::any MiniCCSTVisitor::visitStatement(MiniCParser::StatementContext * ctx)
{
    // 识别的文法产生式：
    /*
    statement:
    T_RETURN expr T_SEMICOLON										# returnStatement
    | lVal T_ASSIGN expr T_SEMICOLON								# assignStatement
    | block															# blockStatement
    | expr? T_SEMICOLON												# expressionStatement
    | T_IF T_L_PAREN cond T_R_PAREN statement (T_ELSE statement)?	# ifElseStatement
    | T_WHILE T_L_PAREN cond T_R_PAREN statement					# whileStatement
    | T_BREAK T_SEMICOLON											# breakStatement
    | T_CONTINUE T_SEMICOLON										# continueStatement;
    */
    // 打印当前访问的语句内容（调试用）
    std::cout << "Visiting statement:" << ctx->getText() << std::endl;

    // 检查是否是 break 语句
    if (Instanceof(breakCtx, MiniCParser::BreakStatementContext *, ctx)) {
        std::cout << "Detected break statement" << std::endl;
        return visitBreakStatement(breakCtx);
    }

    // 检查是否是 continue 语句
    if (Instanceof(continueCtx, MiniCParser::ContinueStatementContext *, ctx)) {
        std::cout << "Detected continue statement" << std::endl;
        return visitContinueStatement(continueCtx);
    }

    // 检查是否是赋值语句
    if (Instanceof(assignCtx, MiniCParser::AssignStatementContext *, ctx)) {
        return visitAssignStatement(assignCtx);
    }

    // 检查是否是 return 语句
    if (Instanceof(returnCtx, MiniCParser::ReturnStatementContext *, ctx)) {
        return visitReturnStatement(returnCtx);
    }

    // 检查是否是块语句
    if (Instanceof(blockCtx, MiniCParser::BlockStatementContext *, ctx)) {
        return visitBlockStatement(blockCtx);
    }

    // 检查是否是表达式语句
    if (Instanceof(exprCtx, MiniCParser::ExpressionStatementContext *, ctx)) {
        return visitExpressionStatement(exprCtx);
    }

    // 检查是否是 if-else 语句
    if (Instanceof(ifElseCtx, MiniCParser::IfElseStatementContext *, ctx)) {
        return visitIfElseStatement(ifElseCtx);
    }

    // 检查是否是 while 语句
    if (Instanceof(whileCtx, MiniCParser::WhileStatementContext *, ctx)) {
        return visitWhileStatement(whileCtx);
    }

    // 如果没有匹配的语句类型，返回空指针
    return nullptr;
}

///
/// @brief 非终结运算符statement中的returnStatement的遍历
/// @param ctx CST上下文
///
std::any MiniCCSTVisitor::visitReturnStatement(MiniCParser::ReturnStatementContext * ctx)
{
    // 识别的文法产生式：returnStatement -> T_RETURN expr T_SEMICOLON

    // 非终结符，表达式expr遍历
    auto exprNode = std::any_cast<ast_node *>(visitExpr(ctx->expr()));

    // 创建返回节点，其孩子为Expr
    return create_contain_node(ast_operator_type::AST_OP_RETURN, exprNode);
}

/// @brief 非终结运算符expr的遍历
/// @param ctx CST上下文
std::any MiniCCSTVisitor::visitExpr(MiniCParser::ExprContext * ctx)
{
    // 识别产生式：expr: addExp;

    return visitAddExp(ctx->addExp());
}

std::any MiniCCSTVisitor::visitAssignStatement(MiniCParser::AssignStatementContext * ctx)
{
    // 识别文法产生式：assignStatement: lVal T_ASSIGN expr T_SEMICOLON

    // 赋值左侧左值Lval遍历产生节点
    auto lvalNode = std::any_cast<ast_node *>(visitLVal(ctx->lVal()));

    // 赋值右侧expr遍历
    auto exprNode = std::any_cast<ast_node *>(visitExpr(ctx->expr()));

    // 创建一个AST_OP_ASSIGN类型的中间节点，孩子为Lval和Expr
    return ast_node::New(ast_operator_type::AST_OP_ASSIGN, lvalNode, exprNode, nullptr);
}

std::any MiniCCSTVisitor::visitBlockStatement(MiniCParser::BlockStatementContext * ctx)
{
    // 识别文法产生式 blockStatement: block

    return visitBlock(ctx->block());
}

std::any MiniCCSTVisitor::visitAddExp(MiniCParser::AddExpContext * ctx)
{
    // 识别的文法产生式：addExp : mulExp (addOp mulExp)*;

    if (ctx->addOp().empty()) {
        // 没有addOp运算符，则说明闭包识别为0，只识别了第一个非终结符mulExp
        return visitMulExp(ctx->mulExp()[0]);
    }

    ast_node *left, *right;

    // 存在addOp运算符
    auto opsCtxVec = ctx->addOp();

    // 有操作符，肯定会进循环，使得right设置正确的值
    for (int k = 0; k < (int) opsCtxVec.size(); k++) {

        // 获取运算符
        ast_operator_type op = std::any_cast<ast_operator_type>(visitAddOp(opsCtxVec[k]));

        if (k == 0) {
            // 左操作数
            left = std::any_cast<ast_node *>(visitMulExp(ctx->mulExp()[k]));
        }

        // 右操作数
        right = std::any_cast<ast_node *>(visitMulExp(ctx->mulExp()[k + 1]));

        // 新建结点作为下一个运算符的右操作符
        left = ast_node::New(op, left, right, nullptr);
    }

    return left;
}

/// @brief 非终结运算符addOp的遍历
/// @param ctx CST上下文
std::any MiniCCSTVisitor::visitAddOp(MiniCParser::AddOpContext * ctx)
{
    // 识别的文法产生式：addOp : T_ADD | T_SUB

    if (ctx->T_ADD()) {
        return ast_operator_type::AST_OP_ADD;
    } else {
        return ast_operator_type::AST_OP_SUB;
    }
}

std::any MiniCCSTVisitor::visitUnaryExp(MiniCParser::UnaryExpContext * ctx)
{
    // 识别文法产生式：unaryExp: primaryExp | T_ID T_L_PAREN realParamList? T_R_PAREN | unaryOp unaryExp;

    if (ctx->primaryExp()) {
        // 普通表达式
        return visitPrimaryExp(ctx->primaryExp());
    } else if (ctx->T_ID()) {
        // 函数调用
        // 创建函数调用名终结符节点
        ast_node * funcname_node = ast_node::New(ctx->T_ID()->getText(), (int64_t) ctx->T_ID()->getSymbol()->getLine());

        // 实参列表
        ast_node * paramListNode = nullptr;

        // 函数调用
        if (ctx->realParamList()) {
            // 有参数
            paramListNode = std::any_cast<ast_node *>(visitRealParamList(ctx->realParamList()));
        }

        // 创建函数调用节点，其孩子为被调用函数名和实参
        return create_func_call(funcname_node, paramListNode);
    } else if (ctx->unaryOp()) {
        // 单目运算符表达式
        auto op = std::any_cast<ast_operator_type>(visitUnaryOp(ctx->unaryOp()));
        auto operand = std::any_cast<ast_node *>(visitUnaryExp(ctx->unaryExp()));

        // 创建单目运算符节点
        return ast_node::New(op, operand, nullptr, nullptr);
    }

    return nullptr;
}

std::any MiniCCSTVisitor::visitPrimaryExp(MiniCParser::PrimaryExpContext * ctx)
{
    // 识别文法产生式 primaryExp: T_L_PAREN expr T_R_PAREN | T_DIGIT | lVal;

    ast_node * node = nullptr;

    if (ctx->T_DIGIT()) {
        // 无符号整型字面量
        // 识别 primaryExp: T_DIGIT

        uint32_t val = (uint32_t) stoull(ctx->T_DIGIT()->getText());
        int64_t lineNo = (int64_t) ctx->T_DIGIT()->getSymbol()->getLine();
        node = ast_node::New(digit_int_attr{val, lineNo});
    } else if (ctx->lVal()) {
        // 具有左值的表达式
        // 识别 primaryExp: lVal
        node = std::any_cast<ast_node *>(visitLVal(ctx->lVal()));
    } else if (ctx->expr()) {
        // 带有括号的表达式
        // primaryExp: T_L_PAREN expr T_R_PAREN
        node = std::any_cast<ast_node *>(visitExpr(ctx->expr()));
    }

    return node;
}

std::any MiniCCSTVisitor::visitLVal(MiniCParser::LValContext * ctx)
{
    // 识别文法产生式：lVal: T_ID;
    // 获取ID的名字
    auto varId = ctx->T_ID()->getText();

    // 获取行号
    int64_t lineNo = (int64_t) ctx->T_ID()->getSymbol()->getLine();

    return ast_node::New(varId, lineNo);
}

std::any MiniCCSTVisitor::visitVarDecl(MiniCParser::VarDeclContext * ctx)
{
    // varDecl: basicType varDef (T_COMMA varDef)* T_SEMICOLON;

    // 声明语句节点
    ast_node * stmt_node = create_contain_node(ast_operator_type::AST_OP_DECL_STMT);

    // 类型节点
    type_attr typeAttr = std::any_cast<type_attr>(visitBasicType(ctx->basicType()));

    for (auto & varCtx: ctx->varDef()) {
        // 变量名节点
        ast_node * id_node = std::any_cast<ast_node *>(visitVarDef(varCtx));

        // 创建类型节点
        ast_node * type_node = create_type_node(typeAttr);

        // 创建变量定义节点
        ast_node * decl_node = ast_node::New(ast_operator_type::AST_OP_VAR_DECL, type_node, id_node, nullptr);

        // 插入到变量声明语句
        (void) stmt_node->insert_son_node(decl_node);
    }

    return stmt_node;
}

std::any MiniCCSTVisitor::visitVarDef(MiniCParser::VarDefContext * ctx)
{
    // varDef: T_ID (T_ASSIGN expr)?;

    auto varId = ctx->T_ID()->getText();

    // 获取行号
    int64_t lineNo = (int64_t) ctx->T_ID()->getSymbol()->getLine();

    // 创建变量名节点
    auto varNode = ast_node::New(varId, lineNo);

    // 如果存在初值
    if (ctx->expr()) {
        // 遍历表达式节点
        auto initExprNode = std::any_cast<ast_node *>(visitExpr(ctx->expr()));
        // 创建赋值节点，将变量名和初值作为子节点
        auto assignNode = ast_node::New(ast_operator_type::AST_OP_ASSIGN, varNode, initExprNode, nullptr);

        // varNode->insert_son_node(initExprNode);
        return assignNode;
    }

    // 没有初值，直接返回变量名节点
    return varNode;
}

std::any MiniCCSTVisitor::visitBasicType(MiniCParser::BasicTypeContext * ctx)
{
    // basicType: T_INT;
    type_attr attr{BasicType::TYPE_VOID, -1};
    if (ctx->T_INT()) {
        attr.type = BasicType::TYPE_INT;
        attr.lineno = (int64_t) ctx->T_INT()->getSymbol()->getLine();
    }

    return attr;
}

std::any MiniCCSTVisitor::visitRealParamList(MiniCParser::RealParamListContext * ctx)
{
    // 识别的文法产生式：realParamList : expr (T_COMMA expr)*;

    auto paramListNode = create_contain_node(ast_operator_type::AST_OP_FUNC_REAL_PARAMS);

    for (auto paramCtx: ctx->expr()) {

        auto paramNode = std::any_cast<ast_node *>(visitExpr(paramCtx));

        paramListNode->insert_son_node(paramNode);
    }

    return paramListNode;
}

std::any MiniCCSTVisitor::visitExpressionStatement(MiniCParser::ExpressionStatementContext * ctx)
{
    // 识别文法产生式  expr ? T_SEMICOLON #expressionStatement;
    if (ctx->expr()) {
        // 表达式语句

        // 遍历expr非终结符，创建表达式节点后返回
        return visitExpr(ctx->expr());
    } else {
        // 空语句

        // 直接返回空指针，需要再把语句加入到语句块时要注意判断，空语句不要加入
        return nullptr;
    }
}

std::any MiniCCSTVisitor::visitIfElseStatement(MiniCParser::IfElseStatementContext * ctx)
{
    // 遍历条件表达式
    auto condNode = std::any_cast<ast_node *>(visitCond(ctx->cond()));

    // 遍历then分支
    auto thenNode = std::any_cast<ast_node *>(visitStatement(ctx->statement(0)));

    // 遍历else分支（如果存在）
    ast_node * elseNode = nullptr;
    if (ctx->statement().size() > 1) {
        elseNode = std::any_cast<ast_node *>(visitStatement(ctx->statement(1)));
    }

    // 创建if语句AST节点
    return create_if_node(condNode, thenNode, elseNode);
}

std::any MiniCCSTVisitor::visitWhileStatement(MiniCParser::WhileStatementContext * ctx)
{
    // 遍历条件表达式
    auto condNode = std::any_cast<ast_node *>(visitCond(ctx->cond()));

    // 遍历循环体
    auto bodyNode = std::any_cast<ast_node *>(visitStatement(ctx->statement()));

    // 创建while语句AST节点
    return create_while_node(condNode, bodyNode);
}

std::any MiniCCSTVisitor::visitCond(MiniCParser::CondContext * ctx)
{
    // 条件表达式现在是逻辑或表达式
    return visitLOrExp(ctx->lOrExp());
}

std::any MiniCCSTVisitor::visitRelExp(MiniCParser::RelExpContext * ctx)
{
    // 识别文法产生式：relExp: addExp (relOp addExp)*;

    if (ctx->relOp().empty()) {
        // 没有关系运算符，直接返回addExp的AST节点
        return visitAddExp(ctx->addExp(0));
    }

    ast_node * left = std::any_cast<ast_node *>(visitAddExp(ctx->addExp(0)));

    for (size_t i = 0; i < ctx->relOp().size(); ++i) {
        // 获取关系运算符
        auto op = std::any_cast<ast_operator_type>(visitRelOp(ctx->relOp(i)));

        // 获取右操作数
        auto right = std::any_cast<ast_node *>(visitAddExp(ctx->addExp(i + 1)));

        // 创建条件表达式节点
        left = create_cond_node(left, op, right);
    }

    return left;
}

std::any MiniCCSTVisitor::visitRelOp(MiniCParser::RelOpContext * ctx)
{
    if (ctx->T_LT()) {
        return ast_operator_type::AST_OP_LT; // 小于
    } else if (ctx->T_GT()) {
        return ast_operator_type::AST_OP_GT; // 大于
    } else if (ctx->T_LE()) {
        return ast_operator_type::AST_OP_LE; // 小于等于
    } else if (ctx->T_GE()) {
        return ast_operator_type::AST_OP_GE; // 大于等于
    }

    return nullptr;
}

/// @brief 非终结符BreakStatement的分析
/// @param ctx CST上下文
/// @return std::any AST的节点
std::any MiniCCSTVisitor::visitBreakStatement(MiniCParser::BreakStatementContext * ctx)
{
    // 创建一个AST_OP_BREAK类型的节点
    return ast_node::New(ast_operator_type::AST_OP_BREAK, nullptr, ctx->T_BREAK()->getSymbol()->getLine());
}

/// @brief 非终结符ContinueStatement的分析
/// @param ctx CST上下文
/// @return std::any AST的节点
std::any MiniCCSTVisitor::visitContinueStatement(MiniCParser::ContinueStatementContext * ctx)
{
    // 创建一个AST_OP_CONTINUE类型的节点
    return ast_node::New(ast_operator_type::AST_OP_CONTINUE, nullptr, ctx->T_CONTINUE()->getSymbol()->getLine());
}

/// @brief 非终结运算符mulExp的遍历
/// @param ctx CST上下文
/// @return AST的节点
std::any MiniCCSTVisitor::visitMulExp(MiniCParser::MulExpContext * ctx)
{
    // 识别的文法产生式：mulExp : unaryExp (mulOp unaryExp)*;

    if (ctx->mulOp().empty()) {
        // 没有mulOp运算符，则说明闭包识别为0，只识别了第一个非终结符unaryExp
        return visitUnaryExp(ctx->unaryExp()[0]);
    }

    ast_node *left, *right;

    // 存在mulOp运算符
    auto opsCtxVec = ctx->mulOp();

    // 有操作符，肯定会进循环，使得right设置正确的值
    for (int k = 0; k < (int) opsCtxVec.size(); k++) {

        // 获取运算符
        ast_operator_type op = std::any_cast<ast_operator_type>(visitMulOp(opsCtxVec[k]));

        if (k == 0) {
            // 左操作数
            left = std::any_cast<ast_node *>(visitUnaryExp(ctx->unaryExp()[k]));
        }

        // 右操作数
        right = std::any_cast<ast_node *>(visitUnaryExp(ctx->unaryExp()[k + 1]));

        // 新建结点作为下一个运算符的右操作符
        left = ast_node::New(op, left, right, nullptr);
    }

    return left;
}

/// @brief 非终结运算符mulOp的遍历
/// @param ctx CST上下文
/// @return AST的节点
std::any MiniCCSTVisitor::visitMulOp(MiniCParser::MulOpContext * ctx)
{
    // 识别的文法产生式：mulOp : T_MUL | T_DIV | T_MOD

    if (ctx->T_MUL()) {
        return ast_operator_type::AST_OP_MUL;
    } else if (ctx->T_DIV()) {
        return ast_operator_type::AST_OP_DIV;
    } else if (ctx->T_MOD()) {
        return ast_operator_type::AST_OP_MOD;
    }

    return nullptr;
}

std::any MiniCCSTVisitor::visitLOrExp(MiniCParser::LOrExpContext * ctx)
{
    // 识别文法产生式：lOrExp: lAndExp (T_OR lAndExp)*;

    if (ctx->T_OR().empty()) {
        // 没有逻辑或运算符，直接返回lAndExp的AST节点
        return visitLAndExp(ctx->lAndExp(0));
    }

    ast_node * left = std::any_cast<ast_node *>(visitLAndExp(ctx->lAndExp(0)));

    for (size_t i = 0; i < ctx->T_OR().size(); ++i) {
        // 获取右操作数
        auto right = std::any_cast<ast_node *>(visitLAndExp(ctx->lAndExp(i + 1)));

        // 创建逻辑或表达式节点
        left = create_cond_node(left, ast_operator_type::AST_OP_OR, right);
    }

    return left;
}

/// @brief 非终结运算符lAndExp的遍历
/// @param ctx CST上下文
/// @return AST的节点
std::any MiniCCSTVisitor::visitLAndExp(MiniCParser::LAndExpContext * ctx)
{
    // 识别文法产生式：lAndExp: eqExp (T_AND eqExp)*;

    if (ctx->T_AND().empty()) {
        // 没有逻辑与运算符，直接返回eqExp的AST节点
        return visitEqExp(ctx->eqExp(0));
    }

    ast_node * left = std::any_cast<ast_node *>(visitEqExp(ctx->eqExp(0)));

    for (size_t i = 0; i < ctx->T_AND().size(); ++i) {
        // 获取右操作数
        auto right = std::any_cast<ast_node *>(visitEqExp(ctx->eqExp(i + 1)));

        // 创建逻辑与表达式节点
        left = create_cond_node(left, ast_operator_type::AST_OP_AND, right);
    }

    return left;
}

/// @brief 非终结运算符eqExp的遍历
/// @param ctx CST上下文
/// @return AST的节点
std::any MiniCCSTVisitor::visitEqExp(MiniCParser::EqExpContext * ctx)
{
    // 识别文法产生式：eqExp: relExp (eqOp relExp)*;

    if (ctx->eqOp().empty()) {
        // 没有相等性运算符，直接返回relExp的AST节点
        return visitRelExp(ctx->relExp(0));
    }

    ast_node * left = std::any_cast<ast_node *>(visitRelExp(ctx->relExp(0)));

    for (size_t i = 0; i < ctx->eqOp().size(); ++i) {
        // 获取相等性运算符
        auto op = std::any_cast<ast_operator_type>(visitEqOp(ctx->eqOp(i)));

        // 获取右操作数
        auto right = std::any_cast<ast_node *>(visitRelExp(ctx->relExp(i + 1)));

        // 创建相等性表达式节点
        left = create_cond_node(left, op, right);
    }

    return left;
}

/// @brief 非终结运算符eqOp的遍历
/// @param ctx CST上下文
/// @return AST的节点
std::any MiniCCSTVisitor::visitEqOp(MiniCParser::EqOpContext * ctx)
{
    // 识别文法产生式：eqOp : T_EQ | T_NE

    if (ctx->T_EQ()) {
        return ast_operator_type::AST_OP_EQ; // 等于
    } else if (ctx->T_NE()) {
        return ast_operator_type::AST_OP_NE; // 不等于
    }

    return nullptr;
}

/// @brief 非终结运算符unaryOp的遍历
/// @param ctx CST上下文
/// @return AST的节点
std::any MiniCCSTVisitor::visitUnaryOp(MiniCParser::UnaryOpContext * ctx)
{
    // 识别文法产生式：unaryOp : T_ADD | T_SUB | T_NOT

    if (ctx->T_ADD()) {
        return ast_operator_type::AST_OP_POSITIVE; // 正号
    } else if (ctx->T_SUB()) {
        return ast_operator_type::AST_OP_NEGATIVE; // 负号
    } else if (ctx->T_NOT()) {
        return ast_operator_type::AST_OP_NOT; // 逻辑非
    }

    return nullptr;
}