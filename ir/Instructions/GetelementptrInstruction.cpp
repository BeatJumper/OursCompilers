///
/// @file GetelementptrInstruction.cpp
/// @brief getelementptr指令，用于计算数组元素地址
///
/// @author kangyk (2921006018@qq.com)
/// @version 1.0
/// @date 2025-05-25
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2025-05-25 <td>1.0     <td>kangyk  <td>新建
/// </table>
///

#include "GetelementptrInstruction.h"
#include "VoidType.h"
#include "PointerType.h"
#include "ArrayType.h"
#include "IntegerType.h"

/// @brief 构造函数（单索引版本）
/// @param _func 所属函数
/// @param _basePtr 基础指针
/// @param _index 索引值
/// @param _inbounds 是否inbounds
GetelementptrInstruction::GetelementptrInstruction(Function * _func, Value * _basePtr, Value * _index, bool _inbounds)
    : Instruction(_func, IRInstOperator::IRINST_OP_GEP, nullptr), basePtr(_basePtr), firstIndex(_index),
      secondIndex(nullptr), inbounds(_inbounds)
{
    addOperand(_basePtr);
    addOperand(_index);

    // 设置返回类型为元素指针类型
    Type * baseType = _basePtr->getType();

    if (baseType->isPointerType()) {
        const PointerType * ptrType = static_cast<const PointerType *>(baseType);
        const Type * pointeeType = ptrType->getPointeeType();
        this->type = new PointerType(pointeeType);
    } else {
        this->type = baseType; // 保持原类型
    }
}

/// @brief 构造函数（双索引版本）
/// @param _func 所属函数
/// @param _basePtr 基础指针
/// @param _firstIndex 第一个索引
/// @param _secondIndex 第二个索引
/// @param _inbounds 是否inbounds
GetelementptrInstruction::GetelementptrInstruction(Function * _func,
                                                   Value * _basePtr,
                                                   Value * _firstIndex,
                                                   Value * _secondIndex,
                                                   bool _inbounds)
    : Instruction(_func, IRInstOperator::IRINST_OP_GEP, nullptr), basePtr(_basePtr), firstIndex(_firstIndex),
      secondIndex(_secondIndex), inbounds(_inbounds)
{
    addOperand(_basePtr);
    addOperand(_firstIndex);
    addOperand(_secondIndex);

    // 设置返回类型为元素指针类型
    Type * baseType = _basePtr->getType();

    if (baseType->isArrayType()) {
        const ArrayType * arrayType = static_cast<const ArrayType *>(baseType);
        Type * elementType = arrayType->getElementType();
        this->type = new PointerType(elementType);
    } else if (baseType->isPointerType()) {
        // 处理指针类型（如多维数组访问的第二层）
        const PointerType * ptrType = static_cast<const PointerType *>(baseType);
        const Type * pointeeType = ptrType->getPointeeType();

        if (pointeeType->isArrayType()) {
            const ArrayType * arrayType = static_cast<const ArrayType *>(pointeeType);
            Type * elementType = arrayType->getElementType();
            this->type = new PointerType(elementType);
        } else {
            this->type = baseType; // 保持原类型
        }
    } else {
        this->type = baseType; // 保持原类型
    }
}

/// @brief 转换成IR指令字符串
void GetelementptrInstruction::toString(std::string & str)
{
    str = getIRName() + " = getelementptr ";

    if (inbounds) {
        str += "inbounds ";
    }

    // 获取基础类型
    Type * baseType = basePtr->getType();
    if (baseType->isArrayType()) {
        str += baseType->toString() + ", " + baseType->toString() + "* " + basePtr->getIRName();
        str += ", i64 " + firstIndex->getIRName();
        if (secondIndex) {
            str += ", i64 " + secondIndex->getIRName();
        }
    } else if (baseType->isPointerType()) {
        // 处理指针类型
        const PointerType * ptrType = static_cast<const PointerType *>(baseType);
        const Type * pointeeType = ptrType->getPointeeType();
        str += pointeeType->toString() + ", " + baseType->toString() + " " + basePtr->getIRName();
        str += ", i64 " + firstIndex->getIRName();
        if (secondIndex) {
            str += ", i64 " + secondIndex->getIRName();
        }
    }
}