#ifndef EXPRESSIONDATATYPE_H
#define EXPRESSIONDATATYPE_H


namespace SGIS{

enum OperationType
{
    opAdd = 0,
    opMinus = 1,
    opMultiply = 2,
    opDivide = 3,
    opMod = 4,
    opEqual = 5,
    opBig  = 6,
    opBigOrEqual  = 7,
    opSmall  = 8,
    opSmallOrEqual = 9,
    opNotEqual  =10,
    opNot      =11,
    opAnd  = 12,
    opOr =13,
    opLike=14
};

enum ExpressionDataType
{
    exInt = 0,
    exNum = 1,
    exLog = 2,
    exStr = 3,
    exDat = 4,
    exNul = 5
};

}

#endif // EXPRESSIONDATATYPE_H
