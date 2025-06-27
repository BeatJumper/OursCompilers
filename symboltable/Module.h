///
/// @file Module.h
/// @brief 符号表-模块类
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-09-29
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// </table>
///
#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "ConstInt.h"
#include "ConstFloat.h"
#include "Type.h"
#include "GlobalVariable.h"
#include "Function.h"
#include "ArrayType.h"

class ScopeStack;

///
/// @brief  一个Module代表一个C语言的源文件
///
class Module {

public:
    ///
    /// @brief 构造函数
    /// @param name  模块名
    ///
    Module(std::string _name);

    ///
    /// @brief 缺省的析构函数
    ///
    virtual ~Module() = default;

    ///
    /// @brief 输出IR代码
    /// @return std::string IR代码字符串
    ///
    std::string toIRString();

    ///
    /// @brief 根据后端输出汇编代码
    /// @return std::string 汇编代码字符串
    ///
    // 获取模块的名字
    [[nodiscard]] std::string getName() const
    {
        return name;
    }

    /// @brief 进入作用域，如进入函数体块、语句块等
    void enterScope();

    /// @brief 退出作用域，如退出函数体块、语句块等
    void leaveScope();

    ///
    /// @brief 在遍历抽象语法树的过程中，获取当前正在处理的函数。在函数外处理时返回空指针。
    ///
    Function * getCurrentFunction();

    ///
    /// @brief 设置当前正在处理的函数指针。函数外设置空指针
    /// @param current
    ///
    void setCurrentFunction(Function * current);

    /// @brief 新建函数并放到函数列表中
    /// @param name 函数名
    /// @param returnType 返回值类型
    /// @param params 形参列表
    /// @param builtin 是否内置函数
    /// @return 新建的函数对象实例
    Function *
    newFunction(std::string name, Type * returnType, std::vector<FormalParam *> params = {}, bool builtin = false);

    /// @brief 根据函数名查找函数信息
    /// @param name 函数名
    /// @return 函数信息
    Function * findFunction(std::string name);

    ///
    /// @brief 获取全局变量列表，用于外部遍历全局变量
    /// @return std::vector<GlobalVariable *>&
    ///
    std::vector<GlobalVariable *> & getGlobalVariables()
    {
        return globalVariableVector;
    }

    /// @brief 获得函数列表
    std::vector<Function *> & getFunctionList()
    {
        return funcVector;
    }

    /// @brief 新建一个整型数值的Value，并加入到符号表，用于后续释放空间
    /// \param intVal 整数值
    /// \return 临时Value
    ConstInt * newConstInt(int32_t intVal);
    ConstFloat * newConstFloat(float floatVal);

    /// @brief 新建变量型Value，会根据currentFunc的值进行判断创建全局或者局部变量
    /// ! 该函数只有在AST遍历生成线性IR中使用，其它地方不能使用
    /// @param name 变量ID
    /// @param type 变量类型
    Value * newVarValue(Type * type, std::string name = "");

    /// @brief 查找变量（全局变量或局部变量），会根据作用域栈进行逐级查找。
    /// ! 该函数只有在AST遍历生成线性IR中使用，其它地方不能使用
    /// @param name 变量ID
    /// @return 指针有效则找到，空指针未找到
    Value * findVarValue(std::string name);

    /// @brief 清理Module中管理的所有信息资源
    void Delete();

    /// @brief 输出线性IR指令列表
    /// @param filePath
    void outputIR(const std::string & filePath);

    ///
    /// @brief 对IR指令中没有名字的全部命名
    ///
    void renameIR();

protected:
    /// @brief 根据整数值获取当前符号
    /// \param name 变量名
    /// \return 变量对应的值
    ConstInt * findConstInt(int32_t val);
    ConstFloat * findConstFloat(float floatVal);

    ///
    /// @brief 新建全局变量，要求name必须有效，并且加入到全局符号表中。
    /// @param type 类型
    /// @param name 名字
    /// @return Value* 全局变量
    ///
    GlobalVariable * newGlobalVariable(Type * type, std::string name);

    /// @brief 根据变量名获取当前符号（只管理全局变量）
    /// \param name 变量名
    /// \return 变量对应的值
    GlobalVariable * findGlobalVariable(std::string name);

    /// @brief 直接插入函数到符号表中，不考虑现有的表中是否存在
    /// @param func 函数对象
    void insertFunctionDirectly(Function * func);

    /// @brief Value插入到符号表中
    /// @param val Value信息
    void insertGlobalValueDirectly(GlobalVariable * val);
    void insertConstFloatDirectly(ConstFloat * val);

    /// @brief ConstInt插入到符号表中
    /// @param val Value信息
    void insertConstIntDirectly(ConstInt * val);

private:
    ///
    /// @brief 模块名，也就是要编译的文件名
    ///
    std::string name;

    ///
    /// @brief 所有的类型，便于内存的释放
    ///
    std::vector<Type *> types;

    /// @brief  变量作用域栈
    ScopeStack * scopeStack;

    /// @brief 遍历抽象树过程中的当前处理函数
    Function * currentFunc = nullptr;

    /// @brief 函数映射表，函数名-函数，便于检索
    std::unordered_map<std::string, Function *> funcMap;

    /// @brief  函数列表
    std::vector<Function *> funcVector;

    /// @brief 变量名映射表，变量名-变量，只保存全局变量
    std::unordered_map<std::string, GlobalVariable *> globalVariableMap;

    /// @brief 只保存全局变量
    std::vector<GlobalVariable *> globalVariableVector;

    /// @brief 常量表
    std::unordered_map<int32_t, ConstInt *> constIntMap;

public:
    /// @brief 创建全局常量
    /// @param type 常量类型
    /// @param name 常量名
    /// @param initValue 初始值
    /// @return 全局常量
    GlobalVariable * newGlobalConstant(Type * type, std::string name, Value * initValue);

    /// @brief 在符号表中添加编译时常量值
    /// @param type 类型
    /// @param name 常量名
    /// @param value 常量值
    /// @return 是否成功
    bool addConstValue(Type * type, std::string name, Value * value);

private:
    /// @brief 编译时常量表（用于常量折叠）
    std::unordered_map<std::string, Value *> constValueMap;

public:
    /// @brief 获取i32类型
    /// @return i32类型指针
    Type * getI32Type();

    /// @brief 获取i64类型
    /// @return i64类型指针
    Type * getI64Type();

    /// @brief 获取i8指针类型
    /// @return i8*类型指针
    Type * getI8PtrType();

    /// @brief 创建全局常量数组
    /// @param arrayType 数组类型
    /// @param name 数组名称（可选）
    /// @return 全局常量数组
    GlobalVariable * newGlobalConstArray(ArrayType * arrayType, const std::string & name = "");

private:
    /// @brief 常量整数向量表，用于释放资源
    std::vector<ConstInt *> constIntVector;
    std::vector<ConstFloat *> constFloatVector;

    /// @brief 全局标签计数器，确保标签全局唯一
    int32_t globalLabelCounter;

public:
    /// @brief 新建64位整型常量
    /// @param val 常量值
    /// @return 常量Value
    ConstInt * newConstLong(int64_t val);

    /// @brief 新建指定类型的整型常量
    /// @param val 常量值
    /// @param type 整数类型
    /// @return 常量Value
    ConstInt * newConstInt(int64_t val, Type * type);

    /// @brief 获取下一个全局唯一的标签ID
    /// @return 标签ID
    int32_t getNextLabelId();
};