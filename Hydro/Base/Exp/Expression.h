#ifndef EXPRESSION_H
#define EXPRESSION_H
#include "VirtualFields.h"

namespace SGIS{

class LikeInterp
{
public:
    LikeInterp(string Filter);
    virtual~LikeInterp();
    bool IsInFilter(string FileName);
    string GetFilter()
    {
        return Filter;
    };
protected:
    string Filter;
    vector<string>exps;
};

class IntegerClass;
class NumericClass;
class StringClass;
class DateClass;
class LogicClass;

class SGIS_EXPORT Expression{
public:
    Expression();
    virtual~Expression();
    int GetExpressionKind();
    void ReturnInteger(Variant&oV);
    void ReturnNumeric(Variant&oV);
    void ReturnLogic(Variant&oV);
    void ReturnString(Variant&oV);
    void ReturnDate(Variant&oV);
    virtual Expression*GetValue()=0;
    virtual bool CanSimplified()=0;
    virtual Expression*GetSimplifyExpression()=0;
    virtual Expression*Clone()=0;
    virtual ExpressionDataType GetDataType()=0;//得到表达式的值类型
    virtual IntegerClass*ReturnNewInteger();
    virtual NumericClass*ReturnNewNumeric();
    virtual LogicClass*ReturnNewLogic();
    virtual StringClass*ReturnNewString();
    virtual DateClass*ReturnNewDate();
    virtual string GetInterpreteString()=0;
    virtual void ReturnValue(Variant&oV)=0;
protected:
    void MakeInteger(Variant&oV);
    void MakeNumeric(Variant&oV);
    void MakeLogic(Variant&oV);
    void MakeString(Variant&oV);
    void MakeDate(Variant&oV);
protected:
    //0-整数类;1-数值类;2-逻辑类;3-字符串类;4-日期类;5-表达式类;6-函数类;7--字段类;8--PIN
    int ExpressionKind;
    double DecimalPrecision;
};

class IntegerClass :public Expression
{
public:
    IntegerClass()
    {
        ExpressionKind=0;
    };
    virtual~IntegerClass()
    {
    };
    Expression*GetValue()
    {
        IntegerClass*newV=new IntegerClass;
        newV->Value=Value;
        return newV;
    };
    void ReturnValue(Variant&oV)
    {
        oV.Clear();
        oV.vt=VRT_I4;
        oV.lVal=Value;
    };
    Expression*Clone(){return GetValue();};
    ExpressionDataType GetDataType()
    {
        return exInt;
    };
    IntegerClass*ReturnNewInteger();
    NumericClass*ReturnNewNumeric();
    LogicClass*ReturnNewLogic();
    StringClass*ReturnNewString();
    DateClass*ReturnNewDate();
    bool CanSimplified(){return true;};
    Expression*GetSimplifyExpression(){return GetValue();};
    string GetInterpreteString();
    int Value;
};

class  NumericClass :public Expression
{
public:
    NumericClass()
    {
        ExpressionKind=1;
    };
    virtual ~NumericClass()
    {
    };
    ExpressionDataType GetDataType();
    Expression*GetValue()
    {
        NumericClass*newV=new NumericClass;
        newV->Value=Value;
        return newV;
    };
    void ReturnValue(Variant&oV)
    {
        oV.Clear();
        oV.vt=VRT_R4;
        oV.fltVal=Value;
    };
    Expression*Clone(){return GetValue();};
    IntegerClass*ReturnNewInteger();
    NumericClass*ReturnNewNumeric();
    LogicClass*ReturnNewLogic();
    StringClass*ReturnNewString();
    DateClass*ReturnNewDate();
    bool CanSimplified(){return true;};
    Expression*GetSimplifyExpression(){return GetValue();};
    string GetInterpreteString();
    float Value;
};

class  LogicClass :public Expression
{
public:
    LogicClass()
    {
        ExpressionKind=2;
    };
    virtual ~LogicClass()
    {
    };
    ExpressionDataType GetDataType()
    {
        return exLog;
    };
    Expression*GetValue()
    {
        LogicClass*newV=new LogicClass;
        newV->Value=Value;
        return newV;
    };
    void ReturnValue(Variant&oV)
    {
        oV.Clear();
        oV.vt=VRT_BOOL;
        oV.boolVal=(Value?true:false);
    };
    Expression*Clone(){return GetValue();};
    IntegerClass*ReturnNewInteger();
    NumericClass*ReturnNewNumeric();
    LogicClass*ReturnNewLogic();
    StringClass*ReturnNewString();
    DateClass*ReturnNewDate();
    bool CanSimplified(){return true;};
    Expression*GetSimplifyExpression(){return GetValue();};
    string GetInterpreteString();
    bool Value;
};

class  StringClass :public Expression
{
public:
    StringClass()
    {
        ExpressionKind=3;
    };
    virtual ~StringClass()
    {
    };
    bool IsNumeric();//字符串是否为合法的数值,如.3,0.12,67都是有效的
    bool IsInterger();
    ExpressionDataType GetDataType();
    Expression*GetValue()
    {
        StringClass*newV=new StringClass;
        newV->Value=Value;
        return newV;
    };
    void ReturnValue(Variant&oV)
    {
        oV.Clear();
        oV=Variant(Value);
    };
    Expression*Clone(){return GetValue();};
    IntegerClass*ReturnNewInteger();
    NumericClass*ReturnNewNumeric();
    LogicClass*ReturnNewLogic();
    StringClass*ReturnNewString();
    DateClass*ReturnNewDate();
    bool CanSimplified(){return true;};
    Expression*GetSimplifyExpression(){return GetValue();};
    string GetInterpreteString();
    void InterpreteString();
    string Value;
};

class  DateClass :public Expression
{
public:
    DateClass()
    {
        ExpressionKind=4;
        year=2001;month=1;day=1;
    };
    virtual ~DateClass()
    {
    };
    ExpressionDataType GetDataType();
    Expression*GetValue()
    {
        DateClass*newV=new DateClass;
        newV->SetValue(year,month,day);
        return newV;
    };
    void ReturnValue(Variant&oV)
    {
        oV.Clear();
        DateTime dt(year,month,day);
        oV.vt=VRT_DATETIME;
        oV.date=dt.ToDouble();
    };
    Expression*Clone(){return GetValue();};
    bool IsDateValid();   //合法格式:YYYY-MM-DD  或  YYYY/MM/DD
    void SetValue(string V);
    void SetValue(int y,int m,int d);
    string GetDateAsString();
    long DiffDays(DateClass*todate);
    void DateAdd(long DiffDays);
    int GetYear();
    int GetMonth();
    int GetDay();
    bool ValidDateString(string Date);
    IntegerClass*ReturnNewInteger();
    NumericClass*ReturnNewNumeric();
    LogicClass*ReturnNewLogic();
    StringClass*ReturnNewString();
    DateClass*ReturnNewDate();
    bool CanSimplified(){return true;};
    Expression*GetSimplifyExpression(){return GetValue();};
    string GetInterpreteString();
private:
    int year,month,day;
};

class  ExpressionClass :public Expression
{
public:
    ExpressionClass()
    {
        ExpressionKind=5;
        dtype=exNul;
    };
    virtual ~ExpressionClass();
    ExpressionDataType GetDataType();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    Expression*Clone();
    void AddElement(Expression*newel);//增加一个新元素
    void AddOps(OperationType newop);//增加一个新运算符
    bool OrderExpression(string&ErrorMsg);
    bool CanSimplified();
    Expression*GetSimplifyExpression();
    int GetOpsNum(){return ops.size();};
    int GetElsNum(){return els.size();};
    OperationType GetOp(int index){return ops[index];};
    Expression*GetEl(int index){return els[index];};
    int GetOpNumNoNot();
    string GetInterpreteString();
private:
    bool OrderNotExpression(string&ErrorMsg);
    bool OrderAndOrExpression(string&ErrorMsg);
    bool OrderLogiExpression(string&ErrorMsg);
    bool OrderRelationExpression(string&ErrorMsg);
    bool OrderNum1Expression(string&ErrorMsg);
    bool OrderLikeExpression(string&ErrorMsg);
    bool OrderNum2Expression(string&ErrorMsg);
    bool OrderNum3Expression(string&ErrorMsg);
    bool OrderNumExpression(string&ErrorMsg);
protected:
    bool CanRemoveBracket();
    LogicClass*AndValue(Expression*V1,Expression*V2);
    LogicClass*OrValue(Expression*V1,Expression*V2);
    LogicClass*EqualValue(Expression*V1,Expression*V2);
    LogicClass*BigValue(Expression*V1,Expression*V2);
    LogicClass*SmallValue(Expression*V1,Expression*V2);
    LogicClass*BigOrEqualValue(Expression*V1,Expression*V2);
    LogicClass*SmallOrEqualValue(Expression*V1,Expression*V2);
    LogicClass*NotEqualValue(Expression*V1,Expression*V2);
    Expression*AddValue(Expression*V1,Expression*V2);
    Expression*MinusValue(Expression*V1,Expression*V2);
    Expression*MultiplyValue(Expression*V1,Expression*V2);
    Expression*DivideValue(Expression*V1,Expression*V2);
    Expression*ModValue(Expression*V1,Expression*V2);
    LogicClass*LikeValue(Expression*V1,Expression*V2);
    void AndValue(Variant&oV1,Expression*V2);
    void OrValue(Variant&oV1,Expression*V2);
    void EqualValue(Variant&oV1,Expression*V2);
    void BigValue(Variant&oV1,Expression*V2);
    void SmallValue(Variant&oV1,Expression*V2);
    void BigOrEqualValue(Variant&oV1,Expression*V2);
    void SmallOrEqualValue(Variant&oV1,Expression*V2);
    void  NotEqualValue(Variant&oV1,Expression*V2);
    void AddValue(Variant&oV1,Expression*V2);
    void MinusValue(Variant&oV1,Expression*V2);
    void MultiplyValue(Variant&oV1,Expression*V2);
    void DivideValue(Variant&oV1,Expression*V2);
    void ModValue(Variant&oV1,Expression*V2);
    void LikeValue(Variant&oV1,Expression*V2);
protected:
    vector<Expression*>els;
    vector<OperationType>ops;
    ExpressionDataType dtype;
};

class  FunctionClass :public Expression
{
public:
    FunctionClass()
    {
        ExpressionKind=6;
    };
    virtual ~FunctionClass();
    Expression*GetSimplifyExpression();
    virtual ExpressionDataType GetDataType()=0;
    void AddElement(Expression*newel);
    bool CanSimplified();
    virtual bool IsExpressionValid(string&ErrorMsg)=0;//判断函数是否合法,包括els元素个数是否与参数个数要求相同,以及类型判断,一个函数各参数类型必须匹配,该函数还返回错误信息ErrorMsg
    int GetElsNum(){return els.size();};
    Expression*GetEl(int index){return els[index];};
protected:
    vector<Expression*>els;//函数的参数,如果无参,则els为空
};

class  NotFunction :public FunctionClass
{
public:
    NotFunction(){};
    virtual ~NotFunction(){};
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    Expression*Clone();
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
};

class  AbsFunction :public FunctionClass
{
public:
    AbsFunction(){};
    virtual ~AbsFunction(){};
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    Expression*Clone();
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  SqrFunction :public FunctionClass
{
public:
    SqrFunction(){};
    virtual ~SqrFunction(){};
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    Expression*Clone();
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  RoundFunction :public FunctionClass
{
public:
    RoundFunction(){};
    virtual ~RoundFunction(){};
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    Expression*Clone();
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  ExpFunction :public FunctionClass
{
public:
    ExpFunction(){};
    virtual ~ExpFunction(){};
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    Expression*Clone();
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  LogFunction :public FunctionClass
{
public:
    LogFunction(){};
    virtual ~LogFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  LnFunction :public FunctionClass
{
public:
    LnFunction(){};
    virtual ~LnFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  SinFunction :public FunctionClass
{
public:
    SinFunction(){};
    virtual ~SinFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  CosFunction :public FunctionClass
{
public:
    CosFunction(){};
    virtual ~CosFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  TanFunction :public FunctionClass
{
public:
    TanFunction(){};
    virtual ~TanFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  AsinFunction :public FunctionClass
{
public:
    AsinFunction(){};
    virtual ~AsinFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  AcosFunction :public FunctionClass
{
public:
    AcosFunction(){};
    virtual ~AcosFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  AtanFunction :public FunctionClass
{
public:
    AtanFunction(){};
    virtual ~AtanFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  TruncFunction :public FunctionClass
{
public:
    TruncFunction(){};
    virtual ~TruncFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  MinFunction :public FunctionClass
{
public:
    MinFunction()
    {
        dtype=exNul;
    };
    virtual ~MinFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
    ExpressionDataType dtype;
};

class  MaxFunction :public FunctionClass
{
public:
    MaxFunction()
    {
        dtype=exNul;
    };
    virtual ~MaxFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
    ExpressionDataType dtype;
};
class  MeanFunction :public FunctionClass
{
public:
    MeanFunction(){};
    virtual ~MeanFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
};
class  SumFunction :public FunctionClass
{
public:
    SumFunction()
    {
        dtype=exNul;
    };
    virtual ~SumFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
    ExpressionDataType dtype;
};
class  FmodFunction :public FunctionClass
{
public:
    FmodFunction(){};
    virtual ~FmodFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  PowFunction :public FunctionClass
{
public:
    PowFunction(){};
    virtual ~PowFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  CLngFunction :public FunctionClass
{
public:
    CLngFunction(){};
    virtual ~CLngFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  CDblFunction :public FunctionClass
{
public:
    CDblFunction(){};
    virtual ~CDblFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  IIFFunction :public FunctionClass
{
public:
    IIFFunction(){};
    virtual ~IIFFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  CStrFunction :public FunctionClass
{
public:
    CStrFunction(){};
    virtual ~CStrFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  LengthFunction :public FunctionClass
{
public:
    LengthFunction(){};
    virtual ~LengthFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  LCaseFunction :public FunctionClass
{
public:
    LCaseFunction(){};
    virtual ~LCaseFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  UCaseFunction :public FunctionClass
{
public:
    UCaseFunction(){};
    virtual ~UCaseFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  LTrimFunction :public FunctionClass
{
public:
    LTrimFunction(){};
    virtual ~LTrimFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  RTrimFunction :public FunctionClass
{
public:
    RTrimFunction(){};
    virtual ~RTrimFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  TrimFunction :public FunctionClass
{
public:
    TrimFunction(){};
    virtual ~TrimFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  SpaceFunction :public FunctionClass
{
public:
    SpaceFunction(){};
    virtual ~SpaceFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  LeftFunction :public FunctionClass
{
public:
    LeftFunction(){};
    virtual ~LeftFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  RightFunction :public FunctionClass
{
public:
    RightFunction(){};
    virtual ~RightFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  InStrFunction :public FunctionClass
{
public:
    InStrFunction(){};
    virtual ~InStrFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  InStrRevFunction :public FunctionClass
{
public:
    InStrRevFunction(){};
    virtual ~InStrRevFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  MidFunction :public FunctionClass
{
public:
    MidFunction(){};
    virtual ~MidFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  CDateFunction :public FunctionClass
{
public:
    CDateFunction(){};
    virtual ~CDateFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  NowFunction :public FunctionClass
{
public:
    NowFunction(){};
    virtual ~NowFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  DayFunction :public FunctionClass
{
public:
    DayFunction(){};
    virtual ~DayFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  MonthFunction :public FunctionClass
{
public:
    MonthFunction(){};
    virtual ~MonthFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  YearFunction :public FunctionClass
{
public:
    YearFunction(){};
    virtual ~YearFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  DateDiffFunction :public FunctionClass
{
public:
    DateDiffFunction(){};
    virtual ~DateDiffFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  DateAddFunction :public FunctionClass
{
public:
    DateAddFunction(){};
    virtual ~DateAddFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  LFillFunction :public FunctionClass
{
public:
    LFillFunction(){};
    virtual ~LFillFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};
class  RFillFunction :public FunctionClass
{
public:
    RFillFunction(){};
    virtual ~RFillFunction(){};
    Expression*Clone();
    Expression*GetValue();
    void ReturnValue(Variant&oV);
    ExpressionDataType GetDataType();
    bool IsExpressionValid(string&ErrorMsg);
    string GetInterpreteString();
private:
};

class  VirtualField :public FunctionClass
{
public:
   VirtualField();
   VirtualField(VirtualTable*VirtualTable,string FieldName);
   virtual ~VirtualField(){};
   Expression*Clone();
   Expression*GetValue();
   void ReturnValue(Variant&oV);
   ExpressionDataType GetDataType();
   string GetInterpreteString();
   bool CanSimplified();
protected:
   LONG FieldIndex;
   bool IsExpressionValid(string&ErrorMsg);
   VirtualTable*pVirtualTable;
   string FieldName;
};

class Grid :public FunctionClass
{
public:
   Grid();
   Grid(VirtualGrids*Grids,string Name);
   virtual ~Grid(){};
   Expression*Clone();
   Expression*GetValue();
   void ReturnValue(Variant&oV);
   ExpressionDataType GetDataType();
   string GetInterpreteString();
   bool CanSimplified();
protected:
   LONG GridIndex;
   bool IsExpressionValid(string&ErrorMsg);
   VirtualGrids*pGrids;
   string GridName;
};

class PinExpressionClass :public Expression
{
public:
   PinExpressionClass()
   {
      ExpressionKind=8;
      other=NULL;
   };
   virtual ~PinExpressionClass();
   Expression*GetSimplifyExpression();
   ExpressionDataType GetDataType();
   void SetPin(Expression*exp,string label,string labelinfo);
   bool CanSimplified();
   Expression*Clone();
   Expression*GetValue();
   void ReturnValue(Variant&oV);
   string GetInterpreteString();
   string GetLabel(){return Label;};
   string GetLabelInfo(){return LabelInfo;};
protected:
   Expression*other;//函数的参数,如果无参,则els为空
   string Label;
   string LabelInfo;
};

class ConstantValue :public FunctionClass
{
public:
   ConstantValue();
   ConstantValue(ConstantValues*pCol,string Name);
   virtual ~ConstantValue(){};
   Expression*Clone();
   Expression*GetValue();
   void ReturnValue(Variant&oV);
   ExpressionDataType GetDataType();
   string GetInterpreteString();
protected:
   bool IsExpressionValid(string&ErrorMsg);
   ExpressionDataType type;
   LONG ConstantIndex;
   ConstantValues*pConstantCol;
   string ConstantName;
};


}


#endif // EXPRESSION_H

