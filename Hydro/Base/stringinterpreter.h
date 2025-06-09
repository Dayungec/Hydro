#ifndef STRINGINTERPRETER_H
#define STRINGINTERPRETER_H

#include "autoptr.h"
#include "Exp/Expression.h"

namespace SGIS{

class SGIS_EXPORT BaseStringInterpreter{
public:
    BaseStringInterpreter();
    virtual~BaseStringInterpreter();
    bool InputExpressionString(string Exp);
    bool InputBoolExpressionString(string Exp);
    ExpressionDataType GetExpressionDataType();
    string GetErrorMessage(){return ErrorMsg;};//获取错误信息
    bool GetIsExpressionValid(){return IsExpressionValid;};
    AutoPtr<Expression>GetValue();
    void ReturnValue(Variant&oV);
    string GetExpressionString(){return ExpressionString;};
    string GetInterpreterString();
public:
    Expression*exp;
protected:
    void SimplifyExpresion();
    virtual FunctionClass*QueryOtherFunctionKind(string str,int&kind)=0;
    virtual int QueryOtherExpressionKindByString(string&str)=0;
    virtual bool AddOtherOperation(int kind,string str,ExpressionClass*newops)=0;
    virtual bool QueryOtherSymbolInString(int&NowPin,string str)=0;
    bool ResetOperation(ExpressionClass*newops);
    int FindMateBracket(int frompursue,string str);
    int FindFormerMateBraket(int frompursue,string str);
    int FindMateSymbol(int frompursue,string str,char sym);
    int GetExpressionKindByString(string&str);
    bool AddOperation(string str,ExpressionClass*newops);
    FunctionClass*GetFunctionKind(string str,int&kind);
    bool InterpreteFunctionContent(string str,FunctionClass*newops);
    FunctionClass*GetFunction(string str,int leftbracketposi);
    bool GetExpressionByString(string str,ExpressionClass*newops);
    bool GetSubString(string&str1,int fromposi,int toposi,string&str2);
protected:
    bool IsExpressionValid;
    string ErrorMsg;
    string ExpressionString;
};

class SGIS_EXPORT StringInterpreter :public BaseStringInterpreter
{
public:
    StringInterpreter(AutoPtr<VirtualTable>pVirtualTable=NULL,AutoPtr<ConstantValues>ConstantCol=NULL);
    virtual~StringInterpreter();
    AutoPtr<VirtualTable>GetVirtualTable();
    AutoPtr<ConstantValues>GetConstantCol();
    void SetVirtualTable(AutoPtr<VirtualTable>Table);
    void SetConstantCol(AutoPtr<ConstantValues>pCol);
    bool InputExpressionString(string Exp);
    bool InputExpressionStringNoMsg(string Exp);
    bool InputBoolExpressionString(string Exp);
    bool InputBoolExpressionStringNoMsg(string Exp);
    AutoPtr<Expression>GetValue();
    void ReturnValue(Variant&oV);
protected:
    FunctionClass*QueryOtherFunctionKind(string str,int&kind);
    int QueryOtherExpressionKindByString(string&str);
    bool AddOtherOperation(int kind,string str,ExpressionClass*newops);
    bool QueryOtherSymbolInString(int&NowPin,string str);
private:
    AutoPtr<VirtualTable>pVirtualTable;
    AutoPtr<ConstantValues>pConstantCol;
};

class SGIS_EXPORT GridInterpreter :public BaseStringInterpreter
{
public:
   GridInterpreter(AutoPtr<VirtualGrids>Grids=NULL,AutoPtr<ConstantValues>ConstantCol=NULL);
   virtual~GridInterpreter();
   AutoPtr<VirtualGrids>GetGrids();
   AutoPtr<ConstantValues>GetConstantCol();
   void SetGrids(AutoPtr<VirtualGrids>Grids);
   void SetConstantCol(AutoPtr<ConstantValues>pCol);
   bool InputExpressionString(string Exp);
   bool InputBoolExpressionString(string Exp);
   AutoPtr<Expression>GetValue();
   void ReturnValue(Variant&oV);
protected:
   FunctionClass*QueryOtherFunctionKind(string str,int&kind);
   int QueryOtherExpressionKindByString(string&str);
   bool AddOtherOperation(int kind,string str,ExpressionClass*newops);
   bool QueryOtherSymbolInString(int&NowPin,string str);
private:
   AutoPtr<VirtualGrids>pGrids;
   AutoPtr<ConstantValues>pConstantCol;
};
}

#endif // STRINGINTERPRETER_H
