grammar MiniC;

// 词法规则名总是以大写字母开头

// 语法规则名总是以小写字母开头

// 每个非终结符尽量多包含闭包、正闭包或可选符等的EBNF范式描述

// 若非终结符由多个产生式组成，则建议在每个产生式的尾部追加# 名称来区分，详细可查看非终结符statement的描述

// 语法规则描述：EBNF范式

// 源文件编译单元定义 1. 修改编译单元，添加声明支持
compileUnit: (decl | funcDef)* EOF;

// 2. 添加声明规则
decl: constDecl | varDecl;

// 3. 添加常量声明
constDecl:
	T_CONST basicType constDef (T_COMMA constDef)* T_SEMICOLON;

// 4. 修改常量定义，支持数组
constDef:
	T_ID (T_L_BRACKET constExp T_R_BRACKET)* T_ASSIGN constInitVal;

// 5. 添加常量初值（暂时只支持单个表达式）
constInitVal:
	constExp
	| T_L_BRACE (constInitVal (T_COMMA constInitVal)*)? T_R_BRACE;

// 6. 添加常量表达式
constExp: addExp; // 常量表达式必须能在编译时求值

// 函数定义
funcDef: funcType T_ID T_L_PAREN funcFParams? T_R_PAREN block;

// 函数类型
funcType: T_VOID | T_INT;

// 函数形参表
funcFParams: funcFParam (T_COMMA funcFParam)*;

// 函数形参，支持数组参数
funcFParam:
	basicType T_ID (
		T_L_BRACKET T_R_BRACKET (T_L_BRACKET expr T_R_BRACKET)*
	)?;

// 语句块看用作函数体，这里允许多个语句，并且不含任何语句
block: T_L_BRACE blockItemList? T_R_BRACE;

// 每个ItemList可包含至少一个Item
blockItemList: blockItem+;

// 修改blockItem，添加声明支持
blockItem: statement | decl;

// 变量声明，目前不支持变量含有初值 现在支持了
varDecl: basicType varDef (T_COMMA varDef)* T_SEMICOLON;

// 基本类型
basicType: T_INT;

// 变量定义，支持数组
varDef:
	T_ID (T_L_BRACKET constExp T_R_BRACKET)* (T_ASSIGN initVal)?;

// 添加变量初值，支持数组初始化
initVal:
	expr
	| T_L_BRACE (initVal (T_COMMA initVal)*)? T_R_BRACE;

// 目前语句支持return和赋值语句 TODO: 待确定void是否需要return
statement:
	T_RETURN expr? T_SEMICOLON										# returnStatement
	| lVal T_ASSIGN expr T_SEMICOLON								# assignStatement
	| block															# blockStatement
	| expr? T_SEMICOLON												# expressionStatement
	| T_IF T_L_PAREN cond T_R_PAREN statement (T_ELSE statement)?	# ifElseStatement
	| T_WHILE T_L_PAREN cond T_R_PAREN statement					# whileStatement
	| T_BREAK T_SEMICOLON											# breakStatement
	| T_CONTINUE T_SEMICOLON										# continueStatement;

// 条件表达式 (逻辑或表达式)
cond: lOrExp;

// 逻辑或表达式
lOrExp: lAndExp (T_OR lAndExp)*;

// 逻辑与表达式
lAndExp: eqExp (T_AND eqExp)*;

// 相等性表达式
eqExp: relExp (eqOp relExp)*;

// 相等性运算符
eqOp: T_EQ | T_NE;

// 关系表达式
relExp: addExp (relOp addExp)*;

// 关系运算符
relOp: T_LT | T_GT | T_LE | T_GE;

// 表达式文法 expr : AddExp 表达式目前只支持加法与减法运算
expr: addExp;

// 加减表达式
addExp: mulExp (addOp mulExp)*;

// 乘除模表达式
mulExp: unaryExp (mulOp unaryExp)*;

// 乘除模运算符
mulOp: T_MUL | T_DIV | T_MOD;

// 加减运算符
addOp: T_ADD | T_SUB;

// 一元表达式 (修改：增加单目运算符支持)
unaryExp:
	primaryExp
	| T_ID T_L_PAREN realParamList? T_R_PAREN
	| unaryOp unaryExp;

// 单目运算符 (新增)
unaryOp: T_ADD | T_SUB | T_NOT;

// 基本表达式：括号表达式、整数、左值表达式
primaryExp: T_L_PAREN expr T_R_PAREN | T_DIGIT | lVal;

// 实参列表
realParamList: expr (T_COMMA expr)*;

// 左值表达式，支持数组访问
lVal: T_ID (T_L_BRACKET expr T_R_BRACKET)*;

// 用正规式来进行词法规则的描述

T_L_PAREN: '(';
T_R_PAREN: ')';
T_SEMICOLON: ';';
T_L_BRACE: '{';
T_R_BRACE: '}';

T_L_BRACKET: '[';
T_R_BRACKET: ']';

T_ASSIGN: '=';
T_COMMA: ',';

// 算术运算符
T_ADD: '+';
T_SUB: '-';
T_MUL: '*'; // 乘法
T_DIV: '/'; // 除法
T_MOD: '%'; // 取模

// 关系运算符
T_LT: '<';
T_GT: '>';
T_LE: '<=';
T_GE: '>=';

// 相等性运算符
T_EQ: '==';
T_NE: '!=';

// 逻辑运算符 (新增)
T_AND: '&&'; // 逻辑与
T_OR: '||'; // 逻辑或
T_NOT: '!'; // 逻辑非

// 要注意关键字同样也属于T_ID，因此必须放在T_ID的前面，否则会识别成T_ID
T_RETURN: 'return';
T_INT: 'int';
T_VOID: 'void';
T_IF: 'if';
T_ELSE: 'else';
T_WHILE: 'while';
T_BREAK: 'break';
T_CONTINUE: 'continue';
T_CONST: 'const';

T_ID: [a-zA-Z_][a-zA-Z0-9_]*;

// 整型常量，支持十进制、八进制、十六进制
T_DIGIT:
	('0x' | '0X') [0-9a-fA-F]+ // 十六进制（优先级最高）
	| '0' [0-7]+ // 八进制（不包括单独的0）  
	| '0' // 单独的0（十进制）
	| [1-9] [0-9]*; // 十进制（非零开头）

/* 空白符丢弃 */
WS: [ \r\n\t]+ -> skip;

// 注释处理 - 新增 单行注释：从 // 开始到行末，跳过处理
LINE_COMMENT: '//' ~[\r\n]* -> skip;

// 多行注释：从 /* 开始到 */ 结束，可跨行，跳过处理
BLOCK_COMMENT: '/*' .*? '*/' -> skip;