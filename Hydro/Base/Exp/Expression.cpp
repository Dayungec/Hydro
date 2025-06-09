#include "Expression.h"
#include "math.h"
#include "../StringHelper.h"
namespace SGIS{


LikeInterp::LikeInterp(string filter)
{
    Filter=filter;
    int len=Filter.length();
    int Pos=0;
    while(Pos<len)
    {
        int fPos1=Filter.find("?",Pos);
        if(fPos1==-1) fPos1=len;
        int fPos2=Filter.find("*",Pos);
        if(fPos2==-1) fPos2=len;
        int tPos=fPos1;
        if(fPos2<fPos1) tPos=fPos2;
        if(tPos==len)
        {
            exps.push_back(Filter.substr(Pos,len-Pos));
            Pos=len;
        }
        else if(fPos1<fPos2)
        {
            if(fPos1-Pos>0) exps.push_back(Filter.substr(Pos,fPos1-Pos));
            exps.push_back("?");
            Pos=fPos1+1;
        }
        else
        {
            if(fPos2-Pos>0) exps.push_back(Filter.substr(Pos,fPos2-Pos));
            exps.push_back("*");
            Pos=fPos2+1;
        }
    }
}
LikeInterp::~LikeInterp()
{

}
bool LikeInterp::IsInFilter(string FileName)
{
    StringHelper::MakeUpper(FileName);
    int Size=exps.size();
    if(Size==0) return false;
    int len=FileName.length();
    if(len==0) return false;
    int Pin=0;
    bool IsAny=true;
    for(int k=0;k<Size;k++)
    {
        string exp=exps[k];
        StringHelper::MakeUpper(exp);
        if(exp=="*")
        {
            IsAny=true;
            continue;
        }
        if(exp=="?")
        {
            if(Pin>=len)
                return false;
            else if(k==Size-1)
            {
                if(Pin+1!=len) return false;
            }
            Pin++;
            IsAny=false;
            continue;
        }
        int expsize=exp.length();
        if(Pin+expsize>len) return false;
        if(!IsAny)
        {
            string str=FileName.substr(Pin,expsize);
            StringHelper::MakeUpper(str);
            if(str!=exp) return false;
            if(k==Size-1)
            {
                return true;
            }
        }
        else
        {
            Pin=FileName.find(exp,Pin);
            if(Pin==-1) return false;
            if(k==Size-1)
            {
                return true;
            }
            Pin=Pin+expsize;
        }
    }
    return true;
}


Expression::Expression(){
    DecimalPrecision=0.000001;
}
Expression::~Expression(){

}
int Expression::GetExpressionKind(){
    return ExpressionKind;
}

void Expression::MakeInteger(Variant&oV)
{
    oV.ChangeType(VRT_I4);
}
void Expression::MakeNumeric(Variant&oV)
{
    oV.ChangeType(VRT_R4);
}
void Expression::MakeLogic(Variant&oV)
{
    oV.ChangeType(VRT_BOOL);
}
void Expression::MakeString(Variant&oV)
{
    oV.ChangeType(VRT_BSTR);
}
void Expression::MakeDate(Variant&oV)
{
    oV.ChangeType(VRT_DATETIME);
}
void Expression::ReturnInteger(Variant&oV)
{
    ReturnValue(oV);
    oV.ChangeType(VRT_I4);
}
void Expression::ReturnNumeric(Variant&oV)
{
    ReturnValue(oV);
    oV.ChangeType(VRT_R4);
}
void Expression::ReturnLogic(Variant&oV)
{
    ReturnValue(oV);
    oV.ChangeType(VRT_BOOL);
}
void Expression::ReturnString(Variant&oV)
{
    ReturnValue(oV);
    oV.ChangeType(VRT_BSTR);
}
void Expression::ReturnDate(Variant&oV)
{
    ReturnValue(oV);
    oV.ChangeType(VRT_DATETIME);
}

IntegerClass*Expression::ReturnNewInteger()
{
    Expression*exp=GetValue();
    if(exp==NULL) return NULL;
    IntegerClass*pNew=exp->ReturnNewInteger();
    delete exp;
    return pNew;
}

NumericClass*Expression::ReturnNewNumeric()
{
    Expression*exp=GetValue();
    if(exp==NULL) return NULL;
    NumericClass*pNew=exp->ReturnNewNumeric();
    delete exp;
    return pNew;
}
LogicClass*Expression::ReturnNewLogic()
{
    Expression*exp=GetValue();
    if(exp==NULL) return NULL;
    LogicClass*pNew=exp->ReturnNewLogic();
    delete exp;
    return pNew;
}
StringClass*Expression::ReturnNewString()
{
    Expression*exp=GetValue();
    if(exp==NULL) return NULL;
    StringClass*pNew=exp->ReturnNewString();
    delete exp;
    return pNew;
}
DateClass*Expression::ReturnNewDate()
{
    Expression*exp=GetValue();
    if(exp==NULL) return NULL;
    DateClass*pNew=exp->ReturnNewDate();
    delete exp;
    return pNew;
}

string IntegerClass::GetInterpreteString()
{
    return to_string(Value);
}
IntegerClass*IntegerClass::ReturnNewInteger()
{
    IntegerClass*newV=new IntegerClass;
    newV->Value=Value;
    return newV;
}
NumericClass*IntegerClass::ReturnNewNumeric()
{
    float fV=(float)Value;
    NumericClass*newV=new NumericClass;
    newV->Value=fV;
    return newV;
}
LogicClass*IntegerClass::ReturnNewLogic()
{
    LogicClass*newV=new LogicClass;
    newV->Value=(Value==0);
    return newV;
}
StringClass*IntegerClass::ReturnNewString()
{
    StringClass*newV=new StringClass;
    newV->Value=to_string(Value);
    return newV;
}

DateClass*IntegerClass::ReturnNewDate()
{
    DateClass*newV=new DateClass;
    DateTime dt(Value);
    newV->SetValue(dt.GetYear(),dt.GetMonth(),dt.GetDay());
    return newV;
}

ExpressionDataType NumericClass::GetDataType()
{
    return exNum;
}
string NumericClass::GetInterpreteString()
{
    return to_string(Value);
}

IntegerClass*NumericClass::ReturnNewInteger()
{
    int iV=(int)Value;
    IntegerClass*newV=new IntegerClass;
    newV->Value=iV;
    return newV;
}
NumericClass*NumericClass::ReturnNewNumeric()
{
    NumericClass*newV=new NumericClass;
    newV->Value=Value;
    return newV;
}
LogicClass*NumericClass::ReturnNewLogic()
{
    LogicClass*newV=new LogicClass;
    newV->Value=(!fabs(Value)<DecimalPrecision);
    return newV;
}
StringClass*NumericClass::ReturnNewString()
{
    StringClass*newV=new StringClass;
    newV->Value=to_string(Value);
    return newV;
}
DateClass*NumericClass::ReturnNewDate()
{
    DateClass*newV=new DateClass;
    DateTime dt(Value);
    newV->SetValue(dt.GetYear(),dt.GetMonth(),dt.GetDay());
    return newV;
}

string LogicClass::GetInterpreteString()
{
    if(!Value) return "False";
    return "True";
}

IntegerClass*LogicClass::ReturnNewInteger()
{
    int intV=0;
    if(Value) intV=1;
    IntegerClass*newV=new IntegerClass;
    newV->Value=intV;
    return newV;
}
NumericClass*LogicClass::ReturnNewNumeric()
{
    float fV=0;
    if(Value) fV=1;
    NumericClass*newV=new NumericClass;
    newV->Value=fV;
    return newV;
}
LogicClass*LogicClass::ReturnNewLogic()
{
    LogicClass*newV=new LogicClass;
    newV->Value=Value;
    return newV;
}
StringClass*LogicClass::ReturnNewString()
{
    string Va="True";
    if(!Value)
        Va="False";
    StringClass*newV=new StringClass;
    newV->Value=Va;
    return newV;
}
DateClass*LogicClass::ReturnNewDate()
{
    DateClass*newV=new DateClass;
    DateTime dt(Value);
    newV->SetValue(dt.GetYear(),dt.GetMonth(),dt.GetDay());
    return newV;
}

ExpressionDataType StringClass::GetDataType()
{
    return exStr;
}

string StringClass::GetInterpreteString()
{
    return Value;
}

void StringClass::InterpreteString()
{
    int len=Value.length();
    for(int k=0;k<len;k++)
    {
        char ca=Value[k];
        if((ca=='n')||(ca=='N'))
        {
            if(k>0)
            {
                if('\\'==Value[k-1])
                {
                    Value.erase(std::begin(Value)+k-1);
                    Value[k-1]='\n';
                    len--;
                    k--;
                }
            }
        }
        else if((ca=='t')||(ca=='T'))
        {
            if(k>0)
            {
                if('\\'==Value[k-1])
                {
                    Value.erase(std::begin(Value)+k-1);
                    Value[k-1]='\t';
                    len--;
                    k--;
                }
            }
        }
        else if((ca=='r')||(ca=='R'))
        {
            if(k>0)
            {
                if('\\'==Value[k-1])
                {
                    Value.erase(std::begin(Value)+k-1);
                    Value[k-1]='\r';
                    len--;
                    k--;
                }
            }
        }
        else if(ca=='\"')
        {
            if(k>0)
            {
                if('\\'==Value[k-1])
                {
                    Value.erase(std::begin(Value)+k-1);
                    Value[k-1]='\"';
                    len--;
                    k--;
                }
            }
        }
    }
}

bool StringClass::IsNumeric()
{
    int pointnumber=0;
    int len=Value.length();
    char ca[2];
    for(int k=0;k<len;k++)
    {
        ca[0]=Value[k];
        ca[1]='\0';
        if(ca[0]=='.')
        {
            if((k==len-1)||(k==0)) return false;
            pointnumber++;
        }
        else
        {
            int num=atoi(ca);
            if((num==0)&&(ca[0]!='0'))
            {
                return false;
            }
            if((num<0)||(num>9))
            {
                return false;
            }
        }
    }
    if(pointnumber>1) return false;
    return true;
}

bool StringClass::IsInterger()
{
    int pointnumber=0;
    int len=Value.length();
    for(int k=0;k<len;k++)
    {
        char ca[2];
        ca[0]=Value[k];
        ca[1]='\0';
        if(ca[0]=='.')
        {
            if((k==len-1)||(k==0)) return false;
            pointnumber++;
        }
        else
        {
            int num=atoi(ca);
            if((num==0)&&(ca[0]!='0'))
            {
                return false;
            }
            if((num<0)||(num>9))
            {
                return false;
            }
        }
    }
    if(pointnumber==0) return true;
    return false;
}

IntegerClass*StringClass::ReturnNewInteger()
{
    int intV=atoi(Value.c_str());
    IntegerClass*newV=new IntegerClass;
    newV->Value=intV;
    return newV;
}
NumericClass*StringClass::ReturnNewNumeric()
{
    float fV=atof(Value.c_str());
    NumericClass*newV=new NumericClass;
    newV->Value=fV;
    return newV;
}
LogicClass*StringClass::ReturnNewLogic()
{
    string Va=Value;
    LogicClass*newV=new LogicClass;
    if((Va=="true")||(Va=="t")||(Va=="1"))
        newV->Value=true;
    else
        newV->Value=false;
    return newV;
}
StringClass*StringClass::ReturnNewString()
{
    StringClass*newV=new StringClass;
    newV->Value=Value;
    return newV;
}
DateClass*StringClass::ReturnNewDate()
{
    DateClass*newV=new DateClass;
    newV->SetValue(Value);
    return newV;
}

ExpressionDataType DateClass::GetDataType()
{
    return exDat;
}
string DateClass::GetInterpreteString()
{
    return GetDateAsString();
}

bool DateClass::IsDateValid()
{
    return false;
}

void DateClass::SetValue(string V)
{
    ValidDateString(V);
}

void DateClass::SetValue(int y,int m,int d)
{
    if((y>9999)||(y<0)) return;
    if((m>12)||(m<=0))  return;
    if((d>31)||(d<=0))  return;
    if(m==2)
    {
        if((y%4==0)&&(y%400!=0))
        {
            if(d>29)
            {
                return;
            }
        }
        else if(d>28)
        {
            return;
        }
    }
    else if((m<8)&&((m%2)==1))
    {
        if(d>31)
        {
            return;
        }
    }
    else if((m>7)&&((m%2)==0))
    {
        if(d>31)
        {
            return;
        }
    }
    else if(d>30)
    {
        return;
    }
    year=y;
    month=m;
    day=d;
}

string DateClass::GetDateAsString()
{
    DateTime dt(year,month,day);
    return dt.ToString(tstDay);
}

long DateClass::DiffDays(DateClass*todate)
{
    DateTime dt1(year,month,day);
    DateTime dt2(todate->GetYear(),todate->GetMonth(),todate->GetDay());
    return (dt2-dt1).TotalDays();
}
void DateClass::DateAdd(long DiffDays)
{
    DateTime dt(year,month,day);
    dt+=TimeSpan(DiffDays,0,0,0);
    year=dt.GetYear();
    month=dt.GetMonth();
    day=dt.GetDay();
}
int DateClass::GetYear()
{
    return year;
}

int DateClass::GetMonth()
{
    return month;
}

int DateClass::GetDay()
{
    return day;
}

bool DateClass::ValidDateString(string Date)
{
    string Year,Month,Day;
    int len=Date.length();
    int y,m,d;
    if(Date.find('-')!=-1)
    {
        int DtNum=0;
        for(int i=0;i<len;i++)
        {
            char d=Date[i];
            if(d=='-')
            {
                DtNum++;
                if(DtNum>2) return false;
                if(i==len-1) return false;
            }
            else
            {
                int k=atoi(&d);
                if((k==0)&&(d!='0')) return false;
                if(DtNum==0)
                    Year.insert(begin(Year)+Year.length(),d);
                else if(DtNum==1)
                    Month.insert(begin(Month)+Month.length(),d);
                else
                    Day.insert(begin(Day)+Day.length(),d);
            }
        }
        if(DtNum!=2) return false;
    }
    else
    {
        for(int i=0;i<len;i++)
        {
            char d=Date[i];
            int k=atoi(&d);
            if((k==0)&&(d!='0')) return false;
            if(i<4)
                Year.insert(begin(Year)+Year.length(),d);
            else if(i<6)
                Month.insert(begin(Month)+Month.length(),d);
            else
                Day.insert(begin(Day)+Day.length(),d);
        }
    }
    y=atoi(Year.c_str());
    if((y>9999)||(y<0))
    {
        return false;
    }
    m=atoi(Month.c_str());
    if((m>12)||(m<=0))
    {
        return false;
    }
    d=atoi(Day.c_str());
    if((d>31)||(d<=0))
    {
        return false;
    }
    if(m==2)
    {
        if((y%4==0)&&(y%400!=0))
        {
            if(d>29)
            {
                return false;
            }
        }
        else if(d>28)
        {
            return false;
        }
    }
    else if((m<8)&&((m%2)==1))
    {
        if(d>31)
        {
            return false;
        }
    }
    else if((m>7)&&((m%2)==0))
    {
        if(d>31)
        {
            return false;
        }
    }
    else if(d>30)
    {
        return false;
    }
    year=y;
    month=m;
    day=d;
    return true;
}

IntegerClass*DateClass::ReturnNewInteger()
{
    DateTime dt(year,month,day);
    IntegerClass*newV=new IntegerClass;
    newV->Value=dt.ToDouble();
    return newV;
}
NumericClass*DateClass::ReturnNewNumeric()
{
    DateTime dt(year,month,day);
    NumericClass*newV=new NumericClass;
    newV->Value=dt.ToDouble();
    return newV;
}
LogicClass*DateClass::ReturnNewLogic()
{
    DateTime dt(year,month,day);
    LogicClass*newV=new LogicClass();
    newV->Value=dt.ToDouble();
    return newV;
}
StringClass*DateClass::ReturnNewString()
{
    StringClass*newV=new StringClass;
    newV->Value=GetDateAsString();
    return newV;
}
DateClass*DateClass::ReturnNewDate()
{
    DateClass*newV=new DateClass;
    newV->year=year;
    newV->month=month;
    newV->day=day;
    return newV;
}


ExpressionClass::~ExpressionClass()
{
    for(int k=els.size()-1;k>=0;k--)
        delete els[k];
    els.clear();
    ops.clear();
}

void ExpressionClass::AddElement(Expression*newel)
{
    els.push_back(newel);
}

void ExpressionClass::AddOps(OperationType newop)
{
    ops.push_back(newop);
}

int ExpressionClass::GetOpNumNoNot()
{
    int num=0;
    for(int k=0;k<ops.size();k++)
    {
        if(ops[k]!=opNot) num++;
    }
    return num;
}
bool ExpressionClass::OrderNotExpression(string&ErrorMsg)
{
    for(int k=0;k<ops.size();k++)
    {
        if(ops[k]==opNot)
        {
            int index=ops.size()-1;
            int j;
            for(j=k+1;j<ops.size();j++)
            {
                if(ops[j]==opNot)
                {
                    ErrorMsg="逻辑非不匹配!";
                    return false;
                }
                else if((ops[j]==opAnd)||(ops[j]==opOr))
                {
                    index=j-1;
                    break;
                }
            }
            if(index>els.size()-1)
            {
                ErrorMsg="逻辑非不匹配!";
                return false;
            }
            ExpressionClass*newExp=new ExpressionClass;
            for(j=k;j<=index;j++)
            {
                newExp->AddElement(els[k]);
                els.erase(begin(els)+k);
                if(j==k)
                    ops.erase(begin(ops)+k);
                else
                {
                    newExp->AddOps(ops[k]);
                    ops.erase(begin(ops)+k);
                }
            }
            if(!newExp->OrderExpression(ErrorMsg))
            {
                delete newExp;
                return false;
            }
            if(newExp->GetDataType()!=exLog)
            {
                ErrorMsg="逻辑非!要求数据为逻辑型，运算之前要做强制转换";
            }
            NotFunction*newNot=new NotFunction;
            newNot->AddElement(newExp);
            els.insert(begin(els)+k,newNot);
            k=k-1;
        }
    }
    return true;
}

bool ExpressionClass::OrderAndOrExpression(string&ErrorMsg)
{
    int LastPosi=0;
    for(int k=0;k<ops.size();k++)
    {
        if((ops[k]==opAnd)||(ops[k]==opOr))
        {
            if(k>=els.size())
            {
                ErrorMsg="逻辑运算符&&或||不匹配!";
                return false;
            }
            ExpressionClass*newExp=new ExpressionClass;
            for(int j=LastPosi;j<=k;j++)
            {
                newExp->AddElement(els[LastPosi]);
                els.erase(begin(els)+LastPosi);
                if(j<k)
                {
                    newExp->AddOps(ops[LastPosi]);
                    ops.erase(begin(ops)+LastPosi);
                }
            }
            if(!newExp->OrderExpression(ErrorMsg))
            {
                delete newExp;
                return false;
            }
            if(newExp->GetDataType()!=exLog)
            {
                ErrorMsg="逻辑运算符要求数据为逻辑型，运算之前要做强制转换";
            }
            els.insert(begin(els)+LastPosi,newExp);
            k=LastPosi;
            LastPosi++;
        }
    }
    if(LastPosi>0)
    {
        int size=ops.size();
        if(size>=els.size())
        {
            ErrorMsg="逻辑运算符&&或||不匹配!";
            return false;
        }
        if(LastPosi<size)
        {
            ExpressionClass*newExp=new ExpressionClass;
            for(int k=LastPosi;k<=size;k++)
            {
                newExp->AddElement(els[LastPosi]);
                els.erase(begin(els)+LastPosi);
                if(k<size)
                {
                    newExp->AddOps(ops[LastPosi]);
                    ops.erase(begin(ops)+LastPosi);
                }
            }
            if(!newExp->OrderExpression(ErrorMsg))
            {
                delete newExp;
                return false;
            }
            if(newExp->GetDataType()!=exLog)
            {
                ErrorMsg="逻辑运算符要求数据为逻辑型，运算之前要做强制转换";
            }
            els.insert(begin(els)+LastPosi,newExp);
        }
        else
        {
            if(els[size]->GetDataType()!=exLog)
            {
                ErrorMsg="逻辑运算符要求数据为逻辑型，运算之前要做强制转换";
            }
        }
    }
    return true;
}
bool ExpressionClass::OrderLogiExpression(string&ErrorMsg)
{
    if(!OrderNotExpression(ErrorMsg)) return false;
    if(!OrderAndOrExpression(ErrorMsg)) return false;
    int LastPosi=0;
    for(int k=0;k<ops.size();k++)
    {
        if(ops[k]==opOr)
        {
            if(k>=els.size())
            {
                ErrorMsg="逻辑运算符||不匹配!";
                return false;
            }
            ExpressionClass*newExp=new ExpressionClass;
            for(int j=LastPosi;j<=k;j++)
            {
                newExp->AddElement(els[LastPosi]);
                els.erase(begin(els)+LastPosi);
                if(j<k)
                {
                    newExp->AddOps(ops[LastPosi]);
                    ops.erase(begin(ops)+LastPosi);
                }
            }
            if(!newExp->OrderExpression(ErrorMsg))
            {
                delete newExp;
                return false;
            }
            if(newExp->GetDataType()!=exLog)
            {
                ErrorMsg="逻辑运算符||要求数据为逻辑型，运算之前要做强制转换";
            }
            els.insert(begin(els)+LastPosi,newExp);
            k=LastPosi;
            LastPosi++;
        }
    }
    if(LastPosi>0)
    {
        int size=ops.size();
        if(size>=els.size())
        {
            ErrorMsg="逻辑运算符||不匹配!";
            return false;
        }
        if(LastPosi<size)
        {
            ExpressionClass*newExp=new ExpressionClass;
            for(int k=LastPosi;k<=size;k++)
            {
                newExp->AddElement(els[LastPosi]);
                els.erase(begin(els)+LastPosi);
                if(k<size)
                {
                    newExp->AddOps(ops[LastPosi]);
                    ops.erase(begin(ops)+LastPosi);
                }
            }
            if(!newExp->OrderExpression(ErrorMsg))
            {
                delete newExp;
                return false;
            }
            if(newExp->GetDataType()!=exLog)
            {
                ErrorMsg="逻辑运算符要求数据为逻辑型，运算之前要做强制转换";
            }
            els.insert(begin(els)+LastPosi,newExp);
        }
        else
        {
            if(els[size]->GetDataType()!=exLog)
            {
                ErrorMsg="逻辑运算符要求数据为逻辑型，运算之前要做强制转换";
            }
        }
    }
    if(ops.size()!=els.size()-1)
    {
        ErrorMsg="非法逻辑运算表达式";
        return false;
    }
    return true;
}

bool ExpressionClass::OrderRelationExpression(string&ErrorMsg)
{
    int LastPosi=0;
    for(int k=0;k<ops.size();k++)
    {
        if((ops[k]==opEqual)||(ops[k]==opBig)||(ops[k]==opEqual)||(ops[k]==opBigOrEqual)||(ops[k]==opSmall)||(ops[k]==opSmallOrEqual)||(ops[k]==opNotEqual))
        {
            if(k>=els.size())
            {
                ErrorMsg="关系运算符不匹配!";
                return false;
            }
            ExpressionClass*newExp=new ExpressionClass;
            for(int j=LastPosi;j<=k;j++)
            {
                newExp->AddElement(els[LastPosi]);
                els.erase(begin(els)+LastPosi);
                if(j<k)
                {
                    newExp->AddOps(ops[LastPosi]);
                    ops.erase(begin(ops)+LastPosi);
                }
            }
            if(!newExp->OrderExpression(ErrorMsg))
            {
                delete newExp;
                return false;
            }
            els.insert(begin(els)+LastPosi,newExp);
            k=LastPosi;
            LastPosi++;
        }
    }
    if(LastPosi>0)
    {
        int size=ops.size();
        if(size>=els.size())
        {
            ErrorMsg="关系运算符不匹配!";
            return false;
        }
        if(LastPosi<size)
        {
            ExpressionClass*newExp=new ExpressionClass;
            for(int k=LastPosi;k<=size;k++)
            {
                newExp->AddElement(els[LastPosi]);
                els.erase(begin(els)+LastPosi);
                if(k<size)
                {
                    newExp->AddOps(ops[LastPosi]);
                    ops.erase(begin(ops)+LastPosi);
                }
            }
            if(!newExp->OrderExpression(ErrorMsg))
            {
                delete newExp;
                return false;
            }
            els.insert(begin(els)+LastPosi,newExp);
        }
    }
    if(LastPosi>1)
    {
        ErrorMsg="非法关系计算,关系运算符后不能继续跟关系运算符!";
        return false;
    }
    if(ops.size()!=els.size()-1)
    {
        ErrorMsg="非法关系运算表达式";
        return false;
    }
    if(LastPosi==1)
    {
        ExpressionDataType left,right;
        left=els[0]->GetDataType();
        right=els[1]->GetDataType();
        if((left==exInt)||(left==exNum))
        {
            if(right==exLog)
            {
                ErrorMsg="企图比较数值和逻辑值大小，运算之前要强制转换后面的数据类型";
            }
            else if(right==exStr)
            {
                ErrorMsg="企图比较数值和字符串大小，运算之前要强制转换后面的数据类型";
            }
            else if(right==exDat)
            {
                ErrorMsg="企图比较数值和日期大小，运算之前要强制转换后面的数据类型";
            }
        }
        else if(left==exLog)
        {
            if((right==exInt)||(right==exNum))
            {
                ErrorMsg="企图比较数值和逻辑值大小，运算之前要强制转换后面的数据类型";
            }
            else if(right==exStr)
            {
                ErrorMsg="企图比较逻辑值和字符串大小，运算之前要强制转换后面的数据类型";
            }
            else if(right==exDat)
            {
                ErrorMsg="企图比较逻辑值和日期大小，运算之前要强制转换后面的数据类型";
            }
        }
        else if(left==exStr)
        {
            if((right==exInt)||(right==exNum))
            {
                ErrorMsg="企图比较数值和字符串大小，运算之前要强制转换后面的数据类型";
            }
            else if(right==exLog)
            {
                ErrorMsg="企图比较逻辑值和字符串大小，运算之前要强制转换后面的数据类型";
            }
            else if(right==exDat)
            {
                ErrorMsg="企图比较逻辑值和字符串大小，运算之前要强制转换后面的数据类型";
            }
        }
        else if(left==exDat)
        {
            if((right==exInt)||(right==exNum))
            {
                ErrorMsg="企图比较数值和日期大小，运算之前要强制转换后面的数据类型";
            }
            else if(right==exLog)
            {
                ErrorMsg="企图比较逻辑值和日期大小，运算之前要强制转换后面的数据类型";
            }
            else if(right==exStr)
            {
                ErrorMsg="企图比较日期和字符串大小，运算之前要强制转换后面的数据类型";
            }
        }
        else if(left==exNul)
        {
            if((right==exInt)||(right==exNum))
            {
                ErrorMsg="企图比较一个不清楚数据类型的数据和数值大小，运算之前要强制转换后面的数据类型";
            }
            else if(right==exLog)
            {
                ErrorMsg="企图比较一个不清楚数据类型的数据和逻辑值大小，运算之前要强制转换后面的数据类型";
            }
            else if(right==exStr)
            {
                ErrorMsg="企图比较一个不清楚数据类型的数据和字符串大小，运算之前要强制转换后面的数据类型";
            }
        }
    }
    return true;
}
bool ExpressionClass::OrderNum1Expression(string&ErrorMsg)
{
    int LastPosi=0;
    for(int k=0;k<ops.size();k++)
    {
        if((ops[k]==opAdd)||(ops[k]==opMinus)||(ops[k]==opMultiply)||(ops[k]==opDivide)||(ops[k]==opMod)||(ops[k]==opLike))
        {
            if(k>=els.size())
            {
                ErrorMsg="运算符不匹配!";
                return false;
            }
            ExpressionClass*newExp=new ExpressionClass;
            for(int j=LastPosi;j<=k;j++)
            {
                newExp->AddElement(els[LastPosi]);
                els.erase(begin(els)+LastPosi);
                if(j<k)
                {
                    newExp->AddOps(ops[LastPosi]);
                    ops.erase(begin(ops)+LastPosi);
                }
            }
            if(!newExp->OrderExpression(ErrorMsg))
            {
                delete newExp;
                return false;
            }
            els.insert(begin(els)+LastPosi,newExp);
            k=LastPosi;
            LastPosi++;
        }
    }
    if(LastPosi>0)
    {
        int size=ops.size();
        if(size>=els.size())
        {
            ErrorMsg="运算符不匹配!";
            return false;
        }
        if(LastPosi<size)
        {
            ExpressionClass*newExp=new ExpressionClass;
            for(int k=LastPosi;k<=size;k++)
            {
                newExp->AddElement(els[LastPosi]);
                els.erase(begin(els)+LastPosi);
                if(k<size)
                {
                    newExp->AddOps(ops[LastPosi]);
                    ops.erase(begin(ops)+LastPosi);
                }
            }
            if(!newExp->OrderExpression(ErrorMsg))
            {
                delete newExp;
                return false;
            }
            els.insert(begin(els)+LastPosi,newExp);
        }
    }
    if(ops.size()!=els.size()-1)
    {
        ErrorMsg="非法表达式";
        return false;
    }
    return true;
}
bool ExpressionClass::OrderLikeExpression(string&ErrorMsg)
{
    int LastPosi=0;
    for(int k=0;k<ops.size();k++)
    {
        if(ops[k]==opLike)
        {
            if(k>=els.size())
            {
                ErrorMsg="运算符不匹配!";
                return false;
            }
            ExpressionClass*newExp=new ExpressionClass;
            for(int j=LastPosi;j<=k;j++)
            {
                newExp->AddElement(els[LastPosi]);
                els.erase(begin(els)+LastPosi);
                if(j<k)
                {
                    newExp->AddOps(ops[LastPosi]);
                    ops.erase(begin(ops)+LastPosi);
                }
            }
            if(!newExp->OrderExpression(ErrorMsg))
            {
                delete newExp;
                return false;
            }
            els.insert(begin(els)+LastPosi,newExp);
            k=LastPosi;
            LastPosi++;
        }
    }
    if(LastPosi>0)
    {
        int size=ops.size();
        if(size>=els.size())
        {
            ErrorMsg="运算符不匹配!";
            return false;
        }
        if(LastPosi<size)
        {
            ExpressionClass*newExp=new ExpressionClass;
            for(int k=LastPosi;k<=size;k++)
            {
                newExp->AddElement(els[LastPosi]);
                els.erase(begin(els)+LastPosi);
                if(k<size)
                {
                    newExp->AddOps(ops[LastPosi]);
                    ops.erase(begin(ops)+LastPosi);
                }
            }
            if(!newExp->OrderExpression(ErrorMsg))
            {
                delete newExp;
                return false;
            }
            els.insert(begin(els)+LastPosi,newExp);
        }
    }
    if(ops.size()!=els.size()-1)
    {
        ErrorMsg="非法表达式";
        return false;
    }
    return true;
}
bool ExpressionClass::OrderNum2Expression(string&ErrorMsg)
{
    int LastPosi=0;
    for(int k=0;k<ops.size();k++)
    {
        if((ops[k]==opAdd)||(ops[k]==opMinus))
        {
            if(k>=els.size())
            {
                ErrorMsg="运算符不匹配!";
                return false;
            }
            ExpressionClass*newExp=new ExpressionClass;
            for(int j=LastPosi;j<=k;j++)
            {
                newExp->AddElement(els[LastPosi]);
                els.erase(begin(els)+LastPosi);
                if(j<k)
                {
                    newExp->AddOps(ops[LastPosi]);
                    ops.erase(begin(ops)+LastPosi);
                }
            }
            if(!newExp->OrderExpression(ErrorMsg))
            {
                delete newExp;
                return false;
            }
            els.insert(begin(els)+LastPosi,newExp);
            k=LastPosi;
            LastPosi++;
        }
    }
    if(LastPosi>0)
    {
        int size=ops.size();
        if(size>=els.size())
        {
            ErrorMsg="运算符不匹配!";
            return false;
        }
        if(LastPosi<size)
        {
            ExpressionClass*newExp=new ExpressionClass;
            for(int k=LastPosi;k<=size;k++)
            {
                newExp->AddElement(els[LastPosi]);
                els.erase(begin(els)+LastPosi);
                if(k<size)
                {
                    newExp->AddOps(ops[LastPosi]);
                    ops.erase(begin(ops)+LastPosi);
                }
            }
            if(!newExp->OrderExpression(ErrorMsg))
            {
                delete newExp;
                return false;
            }
            els.insert(begin(els)+LastPosi,newExp);
        }
    }
    if(ops.size()!=els.size()-1)
    {
        ErrorMsg="非法表达式";
        return false;
    }
    return true;
}

bool ExpressionClass::OrderNum3Expression(string&ErrorMsg)
{
    for(int k=0;k<ops.size();k++)
    {
        if(ops[k]==opMod)
        {
            if(k+1>=els.size())
            {
                ErrorMsg="非法表达式";
                return false;
            }
            ExpressionClass*newV=new ExpressionClass;
            newV->AddElement(els[k]);
            newV->AddElement(els[k+1]);
            newV->AddOps(opMod);
            ops.erase(begin(ops)+k);
            els.erase(begin(els)+k);
            els.erase(begin(els)+k);
            els.insert(begin(els)+k,newV);
            k--;
        }
    }
    return true;
}
bool ExpressionClass::OrderNumExpression(string&ErrorMsg)
{
    if(OrderNum1Expression(ErrorMsg)==false) return false;
    if(OrderLikeExpression(ErrorMsg)==false) return false;
    if(OrderNum2Expression(ErrorMsg)==false) return false;
    if(OrderNum3Expression(ErrorMsg)==false) return false;
    if(ops.size()==0) return true;
    if((ops[0]==opMultiply)||(ops[0]==opDivide)||(ops[0]==opMod))
    {
        for(int k=els.size()-1;k>=0;k--)
        {
            ExpressionDataType type=els[k]->GetDataType();
            if((type!=exNum)&&(type!=exInt)&&(type!=exNul))
            {
                ErrorMsg="非数值型数据不能进行乘*、除\\、求余%运算，运算之前将强制转换为数值";
            }
        }
    }
    else if((ops[0]==opAdd)||(ops[0]==opMinus))
    {
        ExpressionDataType ftype=els[0]->GetDataType();
        int k;
        for(k=els.size()-1;k>=0;k--)
        {
            ExpressionDataType type=els[k]->GetDataType();
            if((type==exNum)&&((ftype==exInt)||(ftype==exLog))) ftype=exNum;
            if((type!=exNum)&&(type!=exInt)&&(type!=exStr)&&(type!=exNul))
            {
                ErrorMsg="非数值和字符串型数据不能进行加+、减-运算，运算之前将依据第一个数据类型强制转换为数值或字符";
            }
        }
        for(k=els.size()-1;k>=0;k--)
        {
            ExpressionDataType type=els[k]->GetDataType();
            if(ftype!=type)
            {
                if((ftype==exInt)&&(type!=exNum)&&(type!=exNul))
                {
                    ErrorMsg="+、-运算要求数据类型一致，非整数参数将强制转换为整数";
                    break;
                }
                if((ftype==exNum)&&(type!=exInt)&&(type!=exNul))
                {
                    ErrorMsg="+、-运算要求数据类型一致，非数值参数将强制转换为数值";
                    break;
                }
                if(ftype==exLog)
                {
                    ErrorMsg="+、-运算要求数据类型一致，非整数参数将强制转换为整数";
                    break;
                }
                if(ftype==exStr)
                {
                    ErrorMsg="+、-运算要求数据类型一致，非字符参数将强制转换为字符";
                    break;
                }
                if(ftype==exDat)
                {
                    ErrorMsg="+、-运算要求数据类型一致，非字符参数将强制转换为字符";
                    break;
                }
            }
        }
    }
    else if(ops[0]==opLike)
    {
        ExpressionDataType ftype=els[0]->GetDataType();
        if(els.size()>2)
        {
            ErrorMsg="Like运算符计算非法";
            return false;
        }
        if((ftype!=exStr)||(els[1]->GetDataType()!=exStr))
        {
            ErrorMsg="Like运算符要求两边必须为字符串类型";
            return false;
        }
    }
    return true;
}

bool ExpressionClass::OrderExpression(string&ErrorMsg)
{
    if(!OrderLogiExpression(ErrorMsg)) return false;
    if(!OrderRelationExpression(ErrorMsg)) return false;
    if(!OrderNumExpression(ErrorMsg)) return false;
    return true;
}

bool ExpressionClass::CanSimplified()
{
    if(ops.size()==0)
    {
        if(els.size()==0) return true;
        Expression*exp=els[0];
        if(exp->GetExpressionKind()==5)
        {
            ExpressionClass*sub=(ExpressionClass*)exp;
            if(sub->GetOpsNum()==0) return true;
        }
        return els[0]->CanSimplified();
    }
    int num=0;
    for(int k=els.size()-1;k>=0;k--)
    {
        if((els[k]!=NULL)&&(els[k]->CanSimplified()))
        {
            if(els[k]->GetExpressionKind()>4) return true;
            num++;
        }
    }
    if(num>1) return true;
    return false;
}

Expression*ExpressionClass::GetSimplifyExpression()
{
    if(ops.size()==0)
    {
        Expression*exp=els[0];
        if(exp==NULL) return NULL;
        if(exp->GetExpressionKind()==5)
        {
            ExpressionClass*sub=(ExpressionClass*)exp;
            if(sub->GetOpsNum()==0)
            {
                if(sub->GetEl(0)==NULL) return NULL;
                return sub->GetEl(0)->GetSimplifyExpression();
            }
        }
        return exp->GetSimplifyExpression();
    }
    int k;
    for(k=0;k<els.size();k++)
    {
        if((els[k]!=NULL)&&(els[k]->CanSimplified()))
        {
            if(els[k]->GetExpressionKind()>4)
            {
                Expression*newV=els[k]->GetSimplifyExpression();
                delete els[k];
                els[k]=newV;
            }
        }
    }
    bool CanAllSimplify=true;
    int simplifynum=0;
    for(k=0;k<els.size();k++)
    {
        if(els[k]==NULL)
            CanAllSimplify=false;
        else if(!els[k]->CanSimplified())
        {
            CanAllSimplify=false;
        }
        else
            simplifynum++;
    }
    if(CanAllSimplify)
    {
        Expression*V=GetValue();
        return V;
    }
    if(simplifynum<2) return Clone();
    OperationType op;
    if(GetOpsNum()==GetElsNum())
    {
        op=ops[0];
        ops.erase(begin(ops)+0);
        if(op==opMinus)
        {
            for(int p=0;p<ops.size();p++)
            {
                if(ops[p]==opAdd)
                    ops[p]=opMinus;
                else
                    ops[p]=opAdd;
            }
        }
        else if(op==opDivide)
        {
            for(int p=0;p<ops.size();p++)
            {
                if(ops[p]==opMultiply)
                    ops[p]=opDivide;
                else
                    ops[p]=opMultiply;
            }
        }
    }
    vector<ExpressionClass*>Vs;
    vector<bool>CanSims;
    ExpressionClass*V;
    for(k=0;k<els.size();k++)
    {
        Expression*exp=els[k];
        bool CanSim=false;
        if(exp!=NULL) CanSim=exp->CanSimplified();
        if(k==0)
        {
            V=new ExpressionClass();
            Vs.push_back(V);
            V->AddElement(exp->Clone());
            CanSims.push_back(CanSim);
            continue;
        }
        if(CanSims[CanSims.size()-1]^CanSim)
        {
            V=new ExpressionClass();
            Vs.push_back(V);
            V->AddElement(exp->Clone());
            CanSims.push_back(CanSim);
        }
        else
        {
            V->AddElement(exp->Clone());
        }
        V->AddOps(ops[k-1]);
    }
    ExpressionClass*V1=Vs[0];
    vector<OperationType>rsps;
    for(int k=1;k<Vs.size();k++)
    {
        ExpressionClass*V1=Vs[k];
        op=V1->ops[0];
        rsps.push_back(op);
        V1->ops.erase(begin(ops)+0);
        if(!CanSims[k]) continue;
        if(op==opMinus)
        {
            for(int p=0;p<V1->ops.size();p++)
            {
                if(V1->ops[p]==opAdd)
                    V1->ops[p]=opMinus;
                else
                    V1->ops[p]=opAdd;
            }
        }
        else if(op==opDivide)
        {
            for(int p=0;p<V1->ops.size();p++)
            {
                if(V1->ops[p]==opMultiply)
                    V1->ops[p]=opDivide;
                else
                    V1->ops[p]=opMultiply;
            }
        }
    }
    ExpressionClass*NewExp=new ExpressionClass;
    for(int k=0;k<Vs.size();k++)
    {
        ExpressionClass*V=Vs[k];
        if(k>0) NewExp->AddOps(rsps[k-1]);
        if(CanSims[k])
        {
            Expression*newV=V->GetValue();
            NewExp->AddElement(newV);
            delete V;
        }
        else
        {
            for(int p=0;p<V->ops.size();p++)
                NewExp->AddOps(V->ops[p]);
            for(int p=0;p<V->els.size();p++)
                NewExp->AddElement(V->els[p]);
            V->ops.clear();
            V->els.clear();
            delete V;
        }
    }
    return NewExp;
}


ExpressionDataType ExpressionClass::GetDataType()
{
    if(dtype!=exNul) return dtype;
    if(ops.size()==0)
    {
        if(els[0]==NULL) return exNul;
        dtype=els[0]->GetDataType();
        return dtype;
    }
    OperationType op=ops[0];
    if((op==opAnd)||(op==opOr))
    {
        dtype=exLog;
        return exLog;
    }
    if((op==opEqual)||(op==opBig)||(op==opBigOrEqual)||(op==opSmall)||(op==opSmallOrEqual)||(op==opNotEqual))
    {
        dtype=exLog;
        return exLog;
    }
    if(op==opLike)
    {
        dtype=exLog;
        return exLog;
    }
    if(op==opMod)
    {
        dtype=exInt;
        return exInt;
    }
    if(op==opMinus)
    {
        bool IsNumeric=false;
        for(int k=0;k<els.size();k++)
        {
            if((els[k]==NULL)||(els[k]->GetDataType()==exNum)||(els[k]->GetDataType()==exNul))
            {
                IsNumeric=true;
                break;
            }
        }
        if(IsNumeric)
        {
            dtype=exNum;
            return exNum;
        }
        dtype=exInt;
        return exInt;
    }
    else if(op==opDivide)
    {
        dtype=exNum;
        return exNum;
    }
    if(els[0]==NULL)
        return exNul;
    else if(els[0]->GetDataType()==exStr)
    {
        dtype=exStr;
        return exStr;
    }
    else if(els[0]->GetDataType()==exDat)
    {
        dtype=exStr;
        return exStr;
    }
    else if(els[0]->GetDataType()==exNul)
    {
        dtype=exNul;
        return exNul;
    }
    bool IsNumeric=false;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
        {
            if((els[k]->GetDataType()==exNum)||(els[k]->GetDataType()==exNul))
            {
                IsNumeric=true;
                break;
            }
        }
    }
    if(IsNumeric)
    {
        dtype=exNum;
        return exNum;
    }
    dtype=exInt;
    return exInt;
}

LogicClass*ExpressionClass::AndValue(Expression*V1,Expression*V2)
{
    LogicClass*newV1=V1->ReturnNewLogic();
    LogicClass*newV2=V2->ReturnNewLogic();
    newV1->Value=((newV1->Value)&&(newV2->Value));
    delete newV2;
    return newV1;
}
LogicClass*ExpressionClass::OrValue(Expression*V1,Expression*V2)
{
    LogicClass*newV1=V1->ReturnNewLogic();
    LogicClass*newV2=V2->ReturnNewLogic();
    newV1->Value=((newV1->Value)||(newV2->Value));
    delete newV2;
    return newV1;
}
LogicClass*ExpressionClass::EqualValue(Expression*V1,Expression*V2)
{
    Expression*newV1=V1->GetValue();
    Expression*newV2=V2->GetValue();
    switch(newV1->GetDataType())
    {
    case exInt:
    case exNum:
    {
        LogicClass*V=new LogicClass;
        NumericClass*t1=newV1->ReturnNewNumeric();
        NumericClass*t2=newV2->ReturnNewNumeric();
        V->Value=(fabs(t1->Value-t2->Value)<=DecimalPrecision);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exLog:
    {
        LogicClass*V=new LogicClass;
        LogicClass*t1=newV1->ReturnNewLogic();
        LogicClass*t2=newV2->ReturnNewLogic();
        V->Value=(t1->Value==t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exStr:
    {
        LogicClass*V=new LogicClass;
        StringClass*t1=newV1->ReturnNewString();
        StringClass*t2=newV2->ReturnNewString();
        V->Value=(t1->Value==t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exDat:
    {
        LogicClass*V=new LogicClass;
        DateClass*t1=newV1->ReturnNewDate();
        DateClass*t2=newV2->ReturnNewDate();
        V->Value=(t1->DiffDays(t2)==0);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    }
    delete newV1;delete newV2;
    return NULL;
}
LogicClass*ExpressionClass::BigValue(Expression*V1,Expression*V2)
{
    Expression*newV1=V1->GetValue();
    Expression*newV2=V2->GetValue();
    switch(newV1->GetDataType())
    {
    case exInt:
    case exNum:
    {
        LogicClass*V=new LogicClass;
        NumericClass*t1=newV1->ReturnNewNumeric();
        NumericClass*t2=newV2->ReturnNewNumeric();
        V->Value=(t1->Value>t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exLog:
    {
        LogicClass*V=new LogicClass;
        LogicClass*t1=newV1->ReturnNewLogic();
        LogicClass*t2=newV2->ReturnNewLogic();
        V->Value=(t1->Value>t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exStr:
    {
        LogicClass*V=new LogicClass;
        StringClass*t1=newV1->ReturnNewString();
        StringClass*t2=newV2->ReturnNewString();
        V->Value=(t1->Value>t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exDat:
    {
        LogicClass*V=new LogicClass;
        DateClass*t1=newV1->ReturnNewDate();
        DateClass*t2=newV2->ReturnNewDate();
        V->Value=(t1->DiffDays(t2)>0);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    }
    delete newV1;delete newV2;
    return NULL;
}
LogicClass*ExpressionClass::SmallValue(Expression*V1,Expression*V2)
{
    Expression*newV1=V1->GetValue();
    Expression*newV2=V2->GetValue();
    switch(newV1->GetDataType())
    {
    case exInt:
    case exNum:
    {
        LogicClass*V=new LogicClass;
        NumericClass*t1=newV1->ReturnNewNumeric();
        NumericClass*t2=newV2->ReturnNewNumeric();
        V->Value=(t1->Value<t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exLog:
    {
        LogicClass*V=new LogicClass;
        LogicClass*t1=newV1->ReturnNewLogic();
        LogicClass*t2=newV2->ReturnNewLogic();
        V->Value=(t1->Value<t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exStr:
    {
        LogicClass*V=new LogicClass;
        StringClass*t1=newV1->ReturnNewString();
        StringClass*t2=newV2->ReturnNewString();
        V->Value=(t1->Value<t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exDat:
    {
        LogicClass*V=new LogicClass;
        DateClass*t1=newV1->ReturnNewDate();
        DateClass*t2=newV2->ReturnNewDate();
        V->Value=(t1->DiffDays(t2)<0);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    }
    delete newV1;delete newV2;
    return NULL;
}
LogicClass*ExpressionClass::BigOrEqualValue(Expression*V1,Expression*V2)
{
    Expression*newV1=V1->GetValue();
    Expression*newV2=V2->GetValue();
    switch(newV1->GetDataType())
    {
    case exInt:
    case exNum:
    {
        LogicClass*V=new LogicClass;
        NumericClass*t1=newV1->ReturnNewNumeric();
        NumericClass*t2=newV2->ReturnNewNumeric();
        V->Value=(t1->Value>=t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exLog:
    {
        LogicClass*V=new LogicClass;
        LogicClass*t1=newV1->ReturnNewLogic();
        LogicClass*t2=newV2->ReturnNewLogic();
        V->Value=(t1->Value>=t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exStr:
    {
        LogicClass*V=new LogicClass;
        StringClass*t1=newV1->ReturnNewString();
        StringClass*t2=newV2->ReturnNewString();
        V->Value=(t1->Value>=t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exDat:
    {
        LogicClass*V=new LogicClass;
        DateClass*t1=newV1->ReturnNewDate();
        DateClass*t2=newV2->ReturnNewDate();
        V->Value=(t1->DiffDays(t2)>=0);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    }
    delete newV1;delete newV2;
    return NULL;
}
LogicClass*ExpressionClass::SmallOrEqualValue(Expression*V1,Expression*V2)
{
    Expression*newV1=V1->GetValue();
    Expression*newV2=V2->GetValue();
    switch(newV1->GetDataType())
    {
    case exInt:
    case exNum:
    {
        LogicClass*V=new LogicClass;
        NumericClass*t1=newV1->ReturnNewNumeric();
        NumericClass*t2=newV2->ReturnNewNumeric();
        V->Value=(t1->Value<=t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exLog:
    {
        LogicClass*V=new LogicClass;
        LogicClass*t1=newV1->ReturnNewLogic();
        LogicClass*t2=newV2->ReturnNewLogic();
        V->Value=(t1->Value<=t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exStr:
    {
        LogicClass*V=new LogicClass;
        StringClass*t1=newV1->ReturnNewString();
        StringClass*t2=newV2->ReturnNewString();
        V->Value=(t1->Value<=t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exDat:
    {
        LogicClass*V=new LogicClass;
        DateClass*t1=newV1->ReturnNewDate();
        DateClass*t2=newV2->ReturnNewDate();
        V->Value=(t1->DiffDays(t2)<=0);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    }
    delete newV1;delete newV2;
    return NULL;
}

LogicClass*ExpressionClass::NotEqualValue(Expression*V1,Expression*V2)
{
    Expression*newV1=V1->GetValue();
    Expression*newV2=V2->GetValue();
    switch(newV1->GetDataType())
    {
    case exInt:
    case exNum:
    {
        LogicClass*V=new LogicClass;
        NumericClass*t1=newV1->ReturnNewNumeric();
        NumericClass*t2=newV2->ReturnNewNumeric();
        V->Value=(fabs(t1->Value-t2->Value)>DecimalPrecision);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exLog:
    {
        LogicClass*V=new LogicClass;
        LogicClass*t1=newV1->ReturnNewLogic();
        LogicClass*t2=newV2->ReturnNewLogic();
        V->Value=(t1->Value!=t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exStr:
    {
        LogicClass*V=new LogicClass;
        StringClass*t1=newV1->ReturnNewString();
        StringClass*t2=newV2->ReturnNewString();
        V->Value=(t1->Value!=t2->Value);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    case exDat:
    {
        LogicClass*V=new LogicClass;
        DateClass*t1=newV1->ReturnNewDate();
        DateClass*t2=newV2->ReturnNewDate();
        V->Value=(t1->DiffDays(t2)!=0);
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    }
    delete newV1;delete newV2;
    return NULL;
}

Expression*ExpressionClass::AddValue(Expression*V1,Expression*V2)
{
    Expression*newV1=V1->GetValue();
    Expression*newV2=V2->GetValue();
    ExpressionDataType dt1,dt2;
    dt1=newV1->GetDataType();
    dt2=newV2->GetDataType();
    if((dt1==exStr)||(dt1==exDat))
    {
        StringClass*V=new StringClass;
        StringClass*t1=newV1->ReturnNewString();
        StringClass*t2=newV2->ReturnNewString();
        V->Value=t1->Value+t2->Value;
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    else if((dt1==exNum)||(dt2==exNum))
    {
        NumericClass*V=new NumericClass;
        NumericClass*t1=newV1->ReturnNewNumeric();
        NumericClass*t2=newV2->ReturnNewNumeric();
        V->Value=t1->Value+t2->Value;
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    else
    {
        IntegerClass*V=new IntegerClass;
        IntegerClass*t1=newV1->ReturnNewInteger();
        IntegerClass*t2=newV2->ReturnNewInteger();
        V->Value=t1->Value+t2->Value;
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
}

Expression*ExpressionClass::MinusValue(Expression*V1,Expression*V2)
{
    Expression*newV1=V1->GetValue();
    Expression*newV2=V2->GetValue();
    ExpressionDataType dt1,dt2;
    dt1=newV1->GetDataType();
    dt2=newV2->GetDataType();
    if((dt1==exStr)||(dt1==exDat))
    {
        StringClass*V=new StringClass;
        StringClass*t1=newV1->ReturnNewString();
        StringClass*t2=newV2->ReturnNewString();
        t2->Value=StringHelper::TrimLeft(t2->Value);
        V->Value=t1->Value+t2->Value;
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    else if((dt1==exNum)||(dt2==exNum))
    {
        NumericClass*V=new NumericClass;
        NumericClass*t1=newV1->ReturnNewNumeric();
        NumericClass*t2=newV2->ReturnNewNumeric();
        V->Value=t1->Value-t2->Value;
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    else
    {
        IntegerClass*V=new IntegerClass;
        IntegerClass*t1=newV1->ReturnNewInteger();
        IntegerClass*t2=newV2->ReturnNewInteger();
        V->Value=t1->Value-t2->Value;
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
}

Expression*ExpressionClass::MultiplyValue(Expression*V1,Expression*V2)
{
    Expression*newV1=V1->GetValue();
    Expression*newV2=V2->GetValue();
    ExpressionDataType dt1,dt2;
    dt1=newV1->GetDataType();
    dt2=newV2->GetDataType();
    if(((dt1==exInt)||(dt1==exLog))&&((dt2==exInt)||(dt2==exLog)))
    {
        IntegerClass*V=new IntegerClass;
        IntegerClass*t1=newV1->ReturnNewInteger();
        IntegerClass*t2=newV2->ReturnNewInteger();
        V->Value=t1->Value*t2->Value;
        delete newV1;delete newV2;delete t1;delete t2;
        return V;
    }
    NumericClass*V=new NumericClass;
    NumericClass*t1=newV1->ReturnNewNumeric();
    NumericClass*t2=newV2->ReturnNewNumeric();
    V->Value=t1->Value*t2->Value;
    delete newV1;delete newV2;delete t1;delete t2;
    return V;
}

Expression*ExpressionClass::DivideValue(Expression*V1,Expression*V2)
{
    NumericClass*newV1=V1->ReturnNewNumeric();
    NumericClass*newV2=V2->ReturnNewNumeric();
    newV1->Value=newV1->Value/newV2->Value;
    delete newV2;
    return newV1;
}
Expression*ExpressionClass::ModValue(Expression*V1,Expression*V2)
{
    IntegerClass*newV1=V1->ReturnNewInteger();
    IntegerClass*newV2=V2->ReturnNewInteger();
    if(newV2->Value==0)
        newV1->Value=0;
    else
        newV1->Value=newV1->Value%newV2->Value;
    delete newV2;
    return newV1;
}
LogicClass*ExpressionClass::LikeValue(Expression*V1,Expression*V2)
{
    StringClass*newV1=V1->ReturnNewString();
    StringClass*newV2=V2->ReturnNewString();
    LogicClass*rt=new LogicClass();
    rt->Value=(newV1->Value.find(newV2->Value)>=0);
    delete newV2;
    delete newV2;
    return rt;
}
void ExpressionClass::AndValue(Variant&oV1,Expression*V2)
{
    MakeLogic(oV1);
    Variant toV;
    V2->ReturnLogic(toV);
    oV1.boolVal=((oV1.boolVal)&&(toV.boolVal));
}
void ExpressionClass::OrValue(Variant&oV1,Expression*V2)
{
    MakeLogic(oV1);
    Variant toV;
    V2->ReturnLogic(toV);
    oV1.boolVal=((oV1.boolVal)||(toV.boolVal));
}
void ExpressionClass::EqualValue(Variant&oV1,Expression*V2)
{
    Variant toV;
    switch(oV1.vt)
    {
    case VRT_I4:
    {
        V2->ReturnNumeric(toV);
        bool bV=(fabs(oV1.lVal-toV.fltVal)<=DecimalPrecision);
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_R4:
    {
        V2->ReturnNumeric(toV);
        bool bV=(fabs(oV1.fltVal-toV.fltVal)<=DecimalPrecision);
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_BOOL:
    {
        V2->ReturnLogic(toV);
        oV1.boolVal=(oV1.boolVal==toV.boolVal);
        return;
    }
    case VRT_BSTR:
    {
        string soV1=(string)(*oV1.bstrVal);
        V2->ReturnString(toV);
        bool bV=(soV1==(string)(*toV.bstrVal));
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_DATE:
    case VRT_DATETIME:
    {
        V2->ReturnDate(toV);
        bool bV=(oV1.date==toV.date);
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    }
}
void ExpressionClass::BigValue(Variant&oV1,Expression*V2)
{
    Variant toV;
    switch(oV1.vt)
    {
    case VRT_I4:
    {
        V2->ReturnNumeric(toV);
        bool bV=oV1.lVal>toV.fltVal;
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_R4:
    {
        V2->ReturnNumeric(toV);
        bool bV=oV1.fltVal>toV.fltVal;
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_BOOL:
    {
        V2->ReturnLogic(toV);
        oV1.boolVal=(oV1.boolVal>toV.boolVal);
        return;
    }
    case VRT_BSTR:
    {
        string soV1=(string)(*oV1.bstrVal);
        V2->ReturnString(toV);
        bool bV=(soV1>(string)(*toV.bstrVal));
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_DATE:
    case VRT_DATETIME:
    {
        V2->ReturnDate(toV);
        bool bV=(oV1.date>toV.date);
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    }
}
void ExpressionClass::SmallValue(Variant&oV1,Expression*V2)
{
    Variant toV;
    switch(oV1.vt)
    {
    case VRT_I4:
    {
        V2->ReturnNumeric(toV);
        bool bV=oV1.lVal<toV.fltVal;
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_R4:
    {
        V2->ReturnNumeric(toV);
        bool bV=oV1.fltVal<toV.fltVal;
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_BOOL:
    {
        V2->ReturnLogic(toV);
        oV1.boolVal=(oV1.boolVal<toV.boolVal);
        return;
    }
    case VRT_BSTR:
    {
        string soV1=(string)(*oV1.bstrVal);
        V2->ReturnString(toV);
        bool bV=(soV1<(string)(*toV.bstrVal));
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_DATE:
    case VRT_DATETIME:
    {
        V2->ReturnDate(toV);
        bool bV=(oV1.date<toV.date);
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    }
}
void ExpressionClass::BigOrEqualValue(Variant&oV1,Expression*V2)
{
    Variant toV;
    switch(oV1.vt)
    {
    case VRT_I4:
    {
        V2->ReturnNumeric(toV);
        bool bV=oV1.lVal>=toV.fltVal;
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_R4:
    {
        V2->ReturnNumeric(toV);
        bool bV=oV1.fltVal>=toV.fltVal;
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_BOOL:
    {
        V2->ReturnLogic(toV);
        oV1.boolVal=(oV1.boolVal>=toV.boolVal);
        return;
    }
    case VRT_BSTR:
    {
        string soV1=(string)(*oV1.bstrVal);
        V2->ReturnString(toV);
        bool bV=(soV1>=(string)(*toV.bstrVal));
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_DATE:
    case VRT_DATETIME:
    {
        V2->ReturnDate(toV);
        bool bV=(oV1.date>=toV.date);
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    }
}
void ExpressionClass::SmallOrEqualValue(Variant&oV1,Expression*V2)
{
    Variant toV;
    switch(oV1.vt)
    {
    case VRT_I4:
    {
        V2->ReturnNumeric(toV);
        bool bV=oV1.lVal<=toV.fltVal;
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_R4:
    {
        V2->ReturnNumeric(toV);
        bool bV=oV1.fltVal<=toV.fltVal;
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_BOOL:
    {
        V2->ReturnLogic(toV);
        oV1.boolVal=(oV1.boolVal<=toV.boolVal);
        return;
    }
    case VRT_BSTR:
    {
        string soV1=(string)(*oV1.bstrVal);
        V2->ReturnString(toV);
        bool bV=(soV1<=(string)(*toV.bstrVal));
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_DATE:
    case VRT_DATETIME:
    {
        V2->ReturnDate(toV);
        bool bV=(oV1.date<=toV.date);
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    }
}

void ExpressionClass::NotEqualValue(Variant&oV1,Expression*V2)
{
    Variant toV;
    switch(oV1.vt)
    {
    case VRT_I4:
    {
        V2->ReturnNumeric(toV);
        bool bV=(fabs(oV1.lVal-toV.fltVal)>DecimalPrecision);
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_R4:
    {
        V2->ReturnNumeric(toV);
        bool bV=(fabs(oV1.fltVal-toV.fltVal)>DecimalPrecision);
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_BOOL:
    {
        V2->ReturnLogic(toV);
        oV1.boolVal=(oV1.boolVal!=toV.boolVal);
        return;
    }
    case VRT_BSTR:
    {
        string soV1=(string)(*oV1.bstrVal);
        V2->ReturnString(toV);
        bool bV=(soV1!=(string)(*toV.bstrVal));
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    case VRT_DATE:
    case VRT_DATETIME:
    {
        V2->ReturnDate(toV);
        bool bV=(oV1.date!=toV.date);
        oV1.vt=VRT_BOOL;
        oV1.boolVal=bV;
        return;
    }
    }
}

void ExpressionClass::AddValue(Variant&oV1,Expression*V2)
{
    ExpressionDataType dt;
    dt=V2->GetDataType();
    if((oV1.vt==VRT_BSTR)||(oV1.vt==VRT_DATE)||(oV1.vt==VRT_DATETIME))
    {
        if(oV1.vt==VRT_DATE||oV1.vt==VRT_DATETIME) MakeString(oV1);
        Variant toV;
        V2->ReturnString(toV);
        string sV=(string)(*oV1.bstrVal)+(string)(*toV.bstrVal);
        oV1=Variant(sV);
        return;
    }
    else if((oV1.vt==VRT_R4)||(dt==exNum))
    {
        if(oV1.vt!=VRT_R4) MakeNumeric(oV1);
        Variant toV;
        V2->ReturnNumeric(toV);
        oV1.fltVal+=toV.fltVal;
        return;
    }
    else
    {
        if(oV1.vt!=VRT_I4) MakeInteger(oV1);
        Variant toV;
        V2->ReturnInteger(toV);
        oV1.lVal+=toV.lVal;
        return;
    }
}

void ExpressionClass::MinusValue(Variant&oV1,Expression*V2)
{
    ExpressionDataType dt;
    dt=V2->GetDataType();
    if((oV1.vt==VRT_BSTR)||(oV1.vt==VRT_DATE)||(oV1.vt==VRT_DATETIME))
    {
        if(oV1.vt==VRT_DATE||oV1.vt==VRT_DATETIME) MakeString(oV1);
        Variant toV;
        V2->ReturnString(toV);
        string sV=(string)(*toV.bstrVal);
        sV=StringHelper::TrimLeft(sV);
        sV=(string)(*oV1.bstrVal)+sV;
        oV1=Variant(sV);
        return;
    }
    else if((oV1.vt==VRT_R4)||(dt==exNum))
    {
        if(oV1.vt!=VRT_R4) MakeNumeric(oV1);
        Variant toV;
        V2->ReturnNumeric(toV);
        oV1.fltVal-=toV.fltVal;
        return;
    }
    else
    {
        if(oV1.vt!=VRT_I4) MakeInteger(oV1);
        Variant toV;
        V2->ReturnInteger(toV);
        oV1.lVal-=toV.lVal;
        return;
    }
}
void ExpressionClass::MultiplyValue(Variant&oV1,Expression*V2)
{
    ExpressionDataType dt;
    dt=V2->GetDataType();
    if(((oV1.vt==VRT_I4)||(oV1.vt==VRT_BOOL))&&((dt==exInt)||(dt==exLog)))
    {
        if(oV1.vt!=VRT_I4) MakeInteger(oV1);
        Variant toV;
        V2->ReturnInteger(toV);
        oV1.lVal=oV1.lVal*toV.lVal;
        return;
    }
    if(oV1.vt!=VRT_R4) MakeNumeric(oV1);
    Variant toV;
    V2->ReturnNumeric(toV);
    oV1.fltVal=oV1.fltVal*toV.fltVal;
}

void ExpressionClass::DivideValue(Variant&oV1,Expression*V2)
{
    if(oV1.vt!=VRT_R4) MakeNumeric(oV1);
    Variant toV;
    V2->ReturnNumeric(toV);
    oV1.fltVal=oV1.fltVal/toV.fltVal;
}
void ExpressionClass::ModValue(Variant&oV1,Expression*V2)
{
    if(oV1.vt!=VRT_I4) MakeInteger(oV1);
    Variant toV;
    V2->ReturnInteger(toV);
    oV1.lVal=oV1.lVal%toV.lVal;
}
void ExpressionClass::LikeValue(Variant&oV1,Expression*V2)
{
    if(oV1.vt!=VRT_BSTR) MakeString(oV1);
    Variant toV;
    V2->ReturnString(toV);
    oV1.vt=VRT_BOOL;
    string sV=(string)(*oV1.bstrVal);
    LikeInterp filter((string)(*toV.bstrVal));
    oV1.boolVal=filter.IsInFilter(sV);
}
Expression*ExpressionClass::Clone()
{
    ExpressionClass*newV=new ExpressionClass;
    int k;
    for(k=0;k<ops.size();k++) newV->AddOps(ops[k]);
    for(k=0;k<els.size();k++)
    {
        if(els[k]==NULL)
            newV->AddElement(NULL);
        else
            newV->AddElement(els[k]->Clone());
    }
    return newV;
}
bool ExpressionClass::CanRemoveBracket()
{
    if(els.size()==1)
    {
        if((els[0]!=NULL)&&(els[0]->GetExpressionKind()==5))
        {
            return ((ExpressionClass*)els[0])->CanRemoveBracket();
        }
        string str=els[0]->GetInterpreteString();
        if((str.length()>1)&&(str.substr(0,1)=="-")) return false;
        return true;
    }
    bool IsValid=true;
    for(int k=0;k<els.size()-1;k++)
    {
        switch(ops[k])
        {
        case opAdd:
        case opMinus:
        case opMod:
        case opEqual:
        case opBig:
        case opBigOrEqual:
        case opSmall:
        case opSmallOrEqual:
        case opNotEqual:
        case opNot:
        case opAnd:
        case opOr:
            IsValid=false;
            break;
        }
    }
    if(IsValid) return true;
    for(int k=0;k<els.size()-1;k++)
    {
        if(ops[k]!=opMod)
        {
            return false;
        }
    }
    return true;
}
string ExpressionClass::GetInterpreteString()
{
    if(els.size()==1)
    {
        if(els[0]==NULL) return "?";
        return els[0]->GetInterpreteString();
    }
    string Info;
    if(els[0]==NULL)
        Info="?";
    else
        Info=els[0]->GetInterpreteString();
    if((ops[0]==opMultiply)||(ops[0]==opDivide)||(ops[0]==opMod))
    {
        Expression*el=els[0];
        if(el->GetExpressionKind()==5)
        {
            ExpressionClass*exp=(ExpressionClass*)el;
            if(!exp->CanRemoveBracket()) Info="("+Info+")";
        }
    }
    for(int k=0;k<ops.size();k++)
    {
        switch(ops[k])
        {
        case opAdd:
            Info=Info+"+";
            break;
        case opMinus:
            Info=Info+"-";
            break;
        case opMultiply:
            Info=Info+"*";
            break;
        case opDivide:
            Info=Info+"/";
            break;
        case opMod:
            Info=Info+"%";
            break;
        case opEqual:
            Info=Info+"=";
            break;
        case opBig:
            Info=Info+">";
            break;
        case opBigOrEqual:
            Info=Info+">=";
            break;
        case opSmall:
            Info=Info+"<";
            break;
        case opSmallOrEqual:
            Info=Info+"<=";
            break;
        case opNotEqual:
            Info=Info+"!=";
            break;
        case opNot:
            Info=Info+" ! ";
            break;
        case opAnd:
            Info=Info+" && ";
            break;
        case opOr:
            Info=Info+" || ";
            break;
        }
        if(els[k+1]==NULL)
            Info=Info+"?";
        else
        {
            Expression*el=els[k+1];
            string str=el->GetInterpreteString();
            bool NeedContinue=true;
            if((ops[k]==opAdd)||(ops[k]==opMinus)||(ops[k]==opMultiply)||(ops[k]==opDivide)||(ops[k]==opMod))
            {
                if((str.length()>0)&&(str.substr(0,1)=="-"))
                {
                    str="("+str+")";
                    NeedContinue=false;
                }
            }
            if((el->GetExpressionKind()==5)&&(NeedContinue))
            {
                ExpressionClass*exp=(ExpressionClass*)el;
                if((ops[k]==opAdd)||(ops[k]==opMinus)||(ops[k]==opMultiply)||(ops[k]==opDivide)||(ops[k]==opMod))
                {
                    if(!exp->CanRemoveBracket())
                        str="("+str+")";
                }
            }
            Info=Info+str;
        }
    }
    if(Info.length()>2)
    {
        if(Info.substr(0,2)=="0-")
            Info=Info.substr(1,Info.length()-1);
    }
    return Info;
}

Expression*ExpressionClass::GetValue()
{
    int Size=ops.size();
    if(Size==0)
    {
        return els[0]->GetValue();
    }
    Expression*newV=els[0]->GetValue();
    for(int k=0;k<Size;k++)
    {
        switch(ops[k])
        {
        case opOr:
        {
            Expression*tempV=OrValue(newV,els[k+1]);
            delete newV;
            newV=tempV;
            break;
        }
        case opAnd:
        {
            Expression*tempV=AndValue(newV,els[k+1]);
            delete newV;
            newV=tempV;
            break;
        }
        case opEqual:
        {
            Expression*tempV=EqualValue(newV,els[k+1]);
            delete newV;
            newV=tempV;
            break;
        }
        case opBig:
        {
            Expression*tempV=BigValue(newV,els[k+1]);
            delete newV;
            newV=tempV;
            break;
        }
        case opSmall:
        {
            Expression*tempV=SmallValue(newV,els[k+1]);
            delete newV;
            newV=tempV;
            break;
        }
        case opBigOrEqual:
        {
            Expression*tempV=BigOrEqualValue(newV,els[k+1]);
            delete newV;
            newV=tempV;
            break;
        }
        case opSmallOrEqual:
        {
            Expression*tempV=SmallOrEqualValue(newV,els[k+1]);
            delete newV;
            newV=tempV;
            break;
        }
        case opNotEqual:
        {
            Expression*tempV=NotEqualValue(newV,els[k+1]);
            delete newV;
            newV=tempV;
            break;
        }
        case opAdd:
        {
            Expression*tempV=AddValue(newV,els[k+1]);
            delete newV;
            newV=tempV;
            break;
        }
        case opMinus:
        {
            Expression*tempV=MinusValue(newV,els[k+1]);
            delete newV;
            newV=tempV;
            break;
        }
        case opMultiply:
        {
            Expression*tempV=MultiplyValue(newV,els[k+1]);
            delete newV;
            newV=tempV;
            break;
        }
        case opDivide:
        {
            Expression*tempV=DivideValue(newV,els[k+1]);
            delete newV;
            newV=tempV;
            break;
        }
        case opMod:
        {
            Expression*tempV=ModValue(newV,els[k+1]);
            delete newV;
            newV=tempV;
            break;
        }
        case opLike:
        {
            Expression*tempV=LikeValue(newV,els[k+1]);
            delete newV;
            newV=tempV;
            break;
        }
        }
    }
    return newV;
}
void ExpressionClass::ReturnValue(Variant&oV)
{
    int Size=ops.size();
    if(Size==0)
    {
        els[0]->ReturnValue(oV);
        return;
    }
    els[0]->ReturnValue(oV);
    for(int k=0;k<Size;k++)
    {
        switch(ops[k])
        {
        case opOr:
        {
            OrValue(oV,els[k+1]);
            break;
        }
        case opAnd:
        {
            AndValue(oV,els[k+1]);
            break;
        }
        case opEqual:
        {
            EqualValue(oV,els[k+1]);
            break;
        }
        case opBig:
        {
            BigValue(oV,els[k+1]);
            break;
        }
        case opSmall:
        {
            SmallValue(oV,els[k+1]);
            break;
        }
        case opBigOrEqual:
        {
            BigOrEqualValue(oV,els[k+1]);
            break;
        }
        case opSmallOrEqual:
        {
            SmallOrEqualValue(oV,els[k+1]);
            break;
        }
        case opNotEqual:
        {
            NotEqualValue(oV,els[k+1]);
            break;
        }
        case opAdd:
        {
            AddValue(oV,els[k+1]);
            break;
        }
        case opMinus:
        {
            MinusValue(oV,els[k+1]);
            break;
        }
        case opMultiply:
        {
            MultiplyValue(oV,els[k+1]);
            break;
        }
        case opDivide:
        {
            DivideValue(oV,els[k+1]);
            break;
        }
        case opMod:
        {
            ModValue(oV,els[k+1]);
            break;
        }
        case opLike:
        {
            LikeValue(oV,els[k+1]);
            break;
        }
        }
    }
}

FunctionClass::~FunctionClass()
{
    for(int k=els.size()-1;k>=0;k--)
        delete els[k];
    els.clear();
}

void FunctionClass::AddElement(Expression*newel)
{
    els.push_back(newel);
}

bool FunctionClass::CanSimplified()
{
    if(ExpressionKind==7)
        return false;
    else if(ExpressionKind==12)
        return false;
    else if(ExpressionKind==9)
        return false;
    bool CanAll=true;
    for(int k=els.size()-1;k>=0;k--)
    {
        if((els[k]!=NULL)&&(els[k]->CanSimplified()))
        {
            if(els[k]->GetExpressionKind()>4) return true;
        }
    }
    return false;
}

Expression*FunctionClass::GetSimplifyExpression()
{
    int k;
    if(ExpressionKind==7)
        return Clone();
    else if(ExpressionKind==12)
        return Clone();
    else if(ExpressionKind==9)
        return Clone();
    for(k=els.size()-1;k>=0;k--)
    {
        if(els[k]->CanSimplified())
        {
            if((els[k]!=NULL)&&(els[k]->GetExpressionKind()>4))
            {
                Expression*newExp=els[k]->GetSimplifyExpression();
                delete els[k];
                els[k]=newExp;
            }
        }
    }
    bool CanAllSimplify=true;
    for(k=els.size()-1;k>=0;k--)
    {
        if((els[k]==NULL)||(!els[k]->CanSimplified()))
        {
            CanAllSimplify=false;
        }
    }
    if(!CanAllSimplify) return Clone();
    return GetValue();
}

Expression*NotFunction::Clone()
{
    NotFunction*newf=new NotFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

Expression*NotFunction::GetValue()
{
    Expression*exp=els[0]->GetValue();
    LogicClass*log=exp->ReturnNewLogic();
    delete exp;
    log->Value=!(log->Value);
    return log;
}
void NotFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnLogic(oV);
    oV.boolVal=(oV.boolVal?true:false);
}
string NotFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "! ?";
    return " ! "+els[0]->GetInterpreteString();
}

ExpressionDataType NotFunction::GetDataType()
{
    return exLog;
}

bool NotFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="逻辑非!不匹配";
        return false;
    }
    ExpressionDataType type=els[0]->GetDataType();
    ErrorMsg="";
    if((type!=exLog)&&(type!=exNul))
        ErrorMsg="逻辑非!要求数据为逻辑型,将强制转换为逻辑型";
    return true;
}

Expression*AbsFunction::Clone()
{
    AbsFunction*newf=new AbsFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string AbsFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "abs(?)";
    return "abs("+els[0]->GetInterpreteString()+")";
}

Expression*AbsFunction::GetValue()
{
    if(els[0]->GetDataType()==exInt)
    {
        Expression*V=els[0]->GetValue();
        IntegerClass*iV=V->ReturnNewInteger();
        iV->Value=abs(iV->Value);
        delete V;
        return iV;
    }
    else
    {
        Expression*V=els[0];
        NumericClass*fV=V->ReturnNewNumeric();
        fV->Value=fabs(fV->Value);
        delete V;
        return fV;
    }
    return NULL;
}
void AbsFunction::ReturnValue(Variant&oV)
{
    if(els[0]->GetDataType()==exInt)
    {
        els[0]->ReturnInteger(oV);
        oV.lVal=abs(oV.lVal);
    }
    else
    {
        els[0]->ReturnNumeric(oV);
        oV.fltVal=fabs(oV.fltVal);
    }
}
ExpressionDataType AbsFunction::GetDataType()
{
    if(els[0]->GetDataType()==exInt)
    {
        return exInt;
    }
    return exNum;
}
bool AbsFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数abs要求参数个数为1";
        return false;
    }
    ExpressionDataType type=els[0]->GetDataType();
    ErrorMsg="";
    if((type==exInt)||(type==exNum)) return true;
    if(type!=exNul) ErrorMsg="函数abs要求参数类型为数值型,将强制转换数据";
    return true;
}

ExpressionDataType SqrFunction::GetDataType()
{
    return exNum;
}
bool SqrFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数sqr要求参数个数为1";
        return false;
    }
    ExpressionDataType type=els[0]->GetDataType();
    ErrorMsg="";
    if((type==exInt)||(type==exNum)) return true;
    if(type!=exNul) ErrorMsg="函数sqr要求参数类型为数值型,将强制转换数据";
    return true;
}
string SqrFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "sqr(?)";
    return "sqr("+els[0]->GetInterpreteString()+")";
}

Expression*SqrFunction::Clone()
{
    SqrFunction*newf=new SqrFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}


Expression*SqrFunction::GetValue()
{
    Expression*V=els[0]->GetValue();
    NumericClass*fV=V->ReturnNewNumeric();
    delete V;
    fV->Value=sqrt(fV->Value);
    return fV;
}
void SqrFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
    oV.fltVal=sqrt(oV.fltVal);
}
ExpressionDataType RoundFunction::GetDataType()
{
    return exInt;
}
bool RoundFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数round要求参数个数为1";
        return false;
    }
    ExpressionDataType type=els[0]->GetDataType();
    ErrorMsg="";
    if((type==exInt)||(type==exNum)) return true;
    if(type!=exNul) ErrorMsg="函数round要求参数类型为数值型,将强制转换数据";
    return true;
}

Expression*RoundFunction::Clone()
{
    RoundFunction*newf=new RoundFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string RoundFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "round(?)";
    return "round("+els[0]->GetInterpreteString()+")";
}

Expression*RoundFunction::GetValue()
{
    Expression*V=els[0]->GetValue();
    NumericClass*fV=V->ReturnNewNumeric();
    delete V;
    IntegerClass*V1=new IntegerClass;
    V1->Value=fV->Value;
    if(fV->Value>0)
    {
        if(fV->Value-V1->Value>=0.5)
            V1->Value++;
    }
    else
    {
        if(V1->Value-fV->Value>=0.5)
            V1->Value--;
    }
    delete fV;
    return V1;
}
void RoundFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
    oV.ChangeType(VRT_I4);
}
ExpressionDataType ExpFunction::GetDataType()
{
    return exNum;
}
bool ExpFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数exp要求参数个数为1";
        return false;
    }
    ExpressionDataType type=els[0]->GetDataType();
    ErrorMsg="";
    if((type==exInt)||(type==exNum)) return true;
    if(type!=exNul) ErrorMsg="函数exp要求参数类型为数值型,将强制转换数据";
    return true;
}

Expression*ExpFunction::Clone()
{
    ExpFunction*newf=new ExpFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}
string ExpFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "exp(?)";
    return "exp("+els[0]->GetInterpreteString()+")";
}

Expression*ExpFunction::GetValue()
{
    NumericClass*V=els[0]->ReturnNewNumeric();
    V->Value=exp(V->Value);
    return V;
}
void ExpFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
    oV.fltVal=exp(oV.fltVal);
}
ExpressionDataType LogFunction::GetDataType()
{
    return exNum;
}

bool LogFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数lg要求参数个数为1";
        return false;
    }
    ExpressionDataType type=els[0]->GetDataType();
    ErrorMsg="";
    if((type==exInt)||(type==exNum)) return true;
    if(type!=exNul) ErrorMsg="函数lg要求参数类型为数值型,将强制转换数据";
    return true;
}

Expression*LogFunction::Clone()
{
    LogFunction*newf=new LogFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string LogFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "log(?)";
    return "log("+els[0]->GetInterpreteString()+")";
}

Expression*LogFunction::GetValue()
{
    NumericClass*V=els[0]->ReturnNewNumeric();
    V->Value=log10(V->Value);
    return V;
}
void LogFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
    oV.fltVal=log10(oV.fltVal);
}
ExpressionDataType LnFunction::GetDataType()
{
    return exNum;
}

bool LnFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数ln要求参数个数为1";
        return false;
    }
    ExpressionDataType type=els[0]->GetDataType();
    ErrorMsg="";
    if((type==exInt)||(type==exNum)) return true;
    if(type!=exNul) ErrorMsg="函数ln要求参数类型为数值型,将强制转换数据";
    return true;
}

Expression*LnFunction::Clone()
{
    LnFunction*newf=new LnFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string LnFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "ln(?)";
    return "ln("+els[0]->GetInterpreteString()+")";
}

Expression*LnFunction::GetValue()
{
    NumericClass*V=els[0]->ReturnNewNumeric();
    V->Value=log(V->Value);
    return V;
}
void LnFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
    oV.fltVal=log(oV.fltVal);
}
ExpressionDataType SinFunction::GetDataType()
{
    return exNum;
}

bool SinFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数sin要求参数个数为1";
        return false;
    }
    ExpressionDataType type=els[0]->GetDataType();
    ErrorMsg="";
    if((type==exInt)||(type==exNum)) return true;
    if(type!=exNul) ErrorMsg="函数sin要求参数类型为数值型,将强制转换数据";
    return true;
}

Expression*SinFunction::Clone()
{
    SinFunction*newf=new SinFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string SinFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "sin(?)";
    return "sin("+els[0]->GetInterpreteString()+")";
}

Expression*SinFunction::GetValue()
{
    NumericClass*V=els[0]->ReturnNewNumeric();
    V->Value=sin(V->Value);
    return V;
}
void SinFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
    oV.fltVal=sin(oV.fltVal);
}
ExpressionDataType CosFunction::GetDataType()
{
    return exNum;
}

bool CosFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数cos要求参数个数为1";
        return false;
    }
    ExpressionDataType type=els[0]->GetDataType();
    ErrorMsg="";
    if((type==exInt)||(type==exNum)) return true;
    if(type!=exNul) ErrorMsg="函数cos要求参数类型为数值型,将强制转换数据";
    return true;
}

Expression*CosFunction::Clone()
{
    CosFunction*newf=new CosFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string CosFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "cos(?)";
    return "cos("+els[0]->GetInterpreteString()+")";
}

Expression*CosFunction::GetValue()
{
    NumericClass*V=els[0]->ReturnNewNumeric();
    V->Value=cos(V->Value);
    return V;
}
void CosFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
    oV.fltVal=cos(oV.fltVal);
}
ExpressionDataType TanFunction::GetDataType()
{
    return exNum;
}

bool TanFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数tan要求参数个数为1";
        return false;
    }
    ExpressionDataType type=els[0]->GetDataType();
    ErrorMsg="";
    if((type==exInt)||(type==exNum)) return true;
    if(type!=exNul) ErrorMsg="函数tan要求参数类型为数值型,将强制转换数据";
    return true;
}

Expression*TanFunction::Clone()
{
    TanFunction*newf=new TanFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string TanFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "tan(?)";
    return "tan("+els[0]->GetInterpreteString()+")";
}

Expression*TanFunction::GetValue()
{
    NumericClass*V=els[0]->ReturnNewNumeric();
    V->Value=tan(V->Value);
    return V;
}
void TanFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
    oV.fltVal=tan(oV.fltVal);
}
ExpressionDataType AsinFunction::GetDataType()
{
    return exNum;
}

bool AsinFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数asin要求参数个数为1";
        return false;
    }
    ExpressionDataType type=els[0]->GetDataType();
    ErrorMsg="";
    if((type==exInt)||(type==exNum)) return true;
    if(type!=exNul) ErrorMsg="函数asin要求参数类型为数值型,将强制转换数据";
    return true;
}

Expression*AsinFunction::Clone()
{
    AsinFunction*newf=new AsinFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string AsinFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "asin(?)";
    return "asin("+els[0]->GetInterpreteString()+")";
}

Expression*AsinFunction::GetValue()
{
    NumericClass*V=els[0]->ReturnNewNumeric();
    V->Value=asin(V->Value);
    return V;
}
void AsinFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
    oV.fltVal=asin(oV.fltVal);
}

ExpressionDataType AcosFunction::GetDataType()
{
    return exNum;
}

bool AcosFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数acos要求参数个数为1";
        return false;
    }
    ExpressionDataType type=els[0]->GetDataType();
    ErrorMsg="";
    if((type==exInt)||(type==exNum)) return true;
    if(type!=exNul) ErrorMsg="函数acos要求参数类型为数值型,将强制转换数据";
    return true;
}

Expression*AcosFunction::Clone()
{
    AcosFunction*newf=new AcosFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string AcosFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "acos(?)";
    return "acos("+els[0]->GetInterpreteString()+")";
}

Expression*AcosFunction::GetValue()
{
    NumericClass*V=els[0]->ReturnNewNumeric();
    V->Value=acos(V->Value);
    return V;
}
void AcosFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
    oV.fltVal=acos(oV.fltVal);
}
ExpressionDataType AtanFunction::GetDataType()
{
    return exNum;
}

bool AtanFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数atan要求参数个数为1";
        return false;
    }
    ExpressionDataType type=els[0]->GetDataType();
    ErrorMsg="";
    if((type==exInt)||(type==exNum)) return true;
    if(type!=exNul) ErrorMsg="函数atan要求参数类型为数值型,将强制转换数据";
    return true;
}

Expression*AtanFunction::Clone()
{
    AtanFunction*newf=new AtanFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string AtanFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "atan(?)";
    return "atan("+els[0]->GetInterpreteString()+")";
}

Expression*AtanFunction::GetValue()
{
    NumericClass*V=els[0]->ReturnNewNumeric();
    V->Value=atan(V->Value);
    return V;
}
void AtanFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
    oV.fltVal=atan(oV.fltVal);
}
ExpressionDataType TruncFunction::GetDataType()
{
    return exInt;
}

bool TruncFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数trunc要求参数个数为1";
        return false;
    }
    ExpressionDataType type=els[0]->GetDataType();
    ErrorMsg="";
    if((type==exInt)||(type==exNum)) return true;
    if(type!=exNul) ErrorMsg="函数trunc要求参数类型为数值型,将强制转换数据";
    return true;
}

Expression*TruncFunction::Clone()
{
    TruncFunction*newf=new TruncFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string TruncFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "trunc(?)";
    return "trunc("+els[0]->GetInterpreteString()+")";
}

Expression*TruncFunction::GetValue()
{
    return els[0]->ReturnNewInteger();
}
void TruncFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
    int iV=oV.fltVal;
    oV.vt=VRT_I4;
    oV.lVal=iV;
}
ExpressionDataType MinFunction::GetDataType()
{
    if(dtype!=exNul) return dtype;
    bool HasNum=false;
    for(int k=els.size()-1;k>=0;k--)
    {
        if(els[k]==NULL) return exNum;
        ExpressionDataType type=els[k]->GetDataType();
        if((type!=exInt)&&(type!=exLog))
        {
            HasNum=true;
            break;
        }
    }
    if(HasNum)
        dtype= exNum;
    else
        dtype=exInt;
    return dtype;
}

bool MinFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()<2)
    {
        ErrorMsg="函数min要求参数个数大于等于2";
        return false;
    }
    ErrorMsg="";
    for(int k=els.size()-1;k>=0;k--)
    {
        ExpressionDataType type=els[k]->GetDataType();
        if((type!=exInt)&&(type!=exNum)&&(type!=exNul))
        {
            ErrorMsg="函数min要求参数类型为数值型,将强制转换数据";
        }
    }
    return true;
}

Expression*MinFunction::Clone()
{
    MinFunction*newf=new MinFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string MinFunction::GetInterpreteString()
{
    int Size=els.size();
    string sInfo="";
    for(int k=0;k<Size;k++)
    {
        string Label="?";
        if(els[k]!=NULL) Label=els[k]->GetInterpreteString();
        if(k>0)
            sInfo=sInfo+","+Label;
        else
            sInfo=Label;
    }
    return "min("+sInfo+")";
}

Expression*MinFunction::GetValue()
{
    int Size=els.size();
    ExpressionDataType dt=GetDataType();
    if(dt==exInt)
    {
        int minv;
        int num=0;
        for(int k=0;k<Size;k++)
        {
            Expression*exp=els[k];
            IntegerClass*V=exp->ReturnNewInteger();
            if(num==0)
                minv=V->Value;
            else if(V->Value<minv)
            {
                minv=V->Value;
            }
            delete V;
            num++;
        }
        IntegerClass*iV=new IntegerClass;
        iV->Value=minv;
        return iV;
    }
    else
    {
        float minv;
        int num=0;
        for(int k=0;k<Size;k++)
        {
            Expression*exp=els[k];
            NumericClass*V=exp->ReturnNewNumeric();
            if(num==0)
                minv=V->Value;
            else if(V->Value<minv)
            {
                minv=V->Value;
            }
            delete V;
            num++;
        }
        NumericClass*iV=new NumericClass;
        iV->Value=minv;
        return iV;
    }
    return NULL;
}
void MinFunction::ReturnValue(Variant&oV)
{
    int Size=els.size();
    ExpressionDataType dt=GetDataType();
    Variant toV;
    if(dt==exInt)
    {
        int minv;
        int num=0;
        for(int k=0;k<Size;k++)
        {
            Expression*exp=els[k];
            exp->ReturnInteger(toV);
            if(num==0)
                minv=toV.lVal;
            else if(toV.lVal<minv)
            {
                minv=toV.lVal;
            }
            num++;
        }
        oV.vt=VRT_I4;
        oV.lVal=minv;
    }
    else
    {
        float minv;
        int num=0;
        for(int k=0;k<Size;k++)
        {
            Expression*exp=els[k];
            exp->ReturnNumeric(toV);
            if(num==0)
                minv=toV.fltVal;
            else if(toV.fltVal<minv)
            {
                minv=toV.fltVal;
            }
            num++;
        }
        oV.vt=VRT_R4;
        oV.fltVal=minv;
    }
}
ExpressionDataType MaxFunction::GetDataType()
{
    if(dtype!=exNul) return dtype;
    bool HasNum=false;
    for(int k=els.size()-1;k>=0;k--)
    {
        if(els[k]==NULL) return exNum;
        ExpressionDataType type=els[k]->GetDataType();
        if((type!=exInt)&&(type!=exLog))
        {
            HasNum=true;
            break;
        }
    }
    if(HasNum)
        dtype= exNum;
    else
        dtype=exInt;
    return dtype;
}

bool MaxFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()<2)
    {
        ErrorMsg="函数max要求参数个数大于等于2";
        return false;
    }
    ErrorMsg="";
    for(int k=els.size()-1;k>=0;k--)
    {
        ExpressionDataType type=els[k]->GetDataType();
        if((type!=exInt)&&(type!=exNum)&&(type!=exNul))
        {
            ErrorMsg="函数max要求参数类型为数值型,将强制转换数据";
        }
    }
    return true;
}

Expression*MaxFunction::Clone()
{
    MaxFunction*newf=new MaxFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string MaxFunction::GetInterpreteString()
{
    int Size=els.size();
    string sInfo="";
    for(int k=0;k<Size;k++)
    {
        string Label="?";
        if(els[k]!=NULL) Label=els[k]->GetInterpreteString();
        if(k>0)
            sInfo=sInfo+","+Label;
        else
            sInfo=Label;
    }
    return "min("+sInfo+")";
}

Expression*MaxFunction::GetValue()
{
    int Size=els.size();
    ExpressionDataType dt=GetDataType();
    if(dt==exInt)
    {
        int maxv;
        int num=0;
        for(int k=0;k<Size;k++)
        {
            Expression*exp=els[k];
            IntegerClass*V=exp->ReturnNewInteger();
            if(num==0)
                maxv=V->Value;
            else if(V->Value>maxv)
            {
                maxv=V->Value;
            }
            delete V;
            num++;
        }
        IntegerClass*iV=new IntegerClass;
        iV->Value=maxv;
        return iV;
    }
    else
    {
        float maxv;
        int num=0;
        for(int k=0;k<Size;k++)
        {
            Expression*exp=els[k];
            NumericClass*V=exp->ReturnNewNumeric();
            if(num==0)
                maxv=V->Value;
            else if(V->Value>maxv)
            {
                maxv=V->Value;
            }
            delete V;
            num++;
        }
        NumericClass*iV=new NumericClass;
        iV->Value=maxv;
        return iV;
    }
    return NULL;
}
void MaxFunction::ReturnValue(Variant&oV)
{
    int Size=els.size();
    ExpressionDataType dt=GetDataType();
    Variant toV;
    if(dt==exInt)
    {
        int maxv;
        int num=0;
        for(int k=0;k<Size;k++)
        {
            Expression*exp=els[k];
            exp->ReturnInteger(toV);
            if(num==0)
                maxv=toV.lVal;
            else if(toV.lVal>maxv)
            {
                maxv=toV.lVal;
            }
            num++;
        }
        oV.vt=VRT_I4;
        oV.lVal=maxv;
    }
    else
    {
        float maxv;
        int num=0;
        for(int k=0;k<Size;k++)
        {
            Expression*exp=els[k];
            exp->ReturnNumeric(toV);
            if(num==0)
                maxv=toV.fltVal;
            else if(toV.fltVal>maxv)
            {
                maxv=toV.fltVal;
            }
            num++;
        }
        oV.vt=VRT_R4;
        oV.fltVal=maxv;
    }
}
ExpressionDataType MeanFunction::GetDataType()
{
    return exNum;
}

bool MeanFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()<2)
    {
        ErrorMsg="函数mean要求参数个数大于等于2";
        return false;
    }
    ErrorMsg="";
    for(int k=els.size()-1;k>=0;k--)
    {
        ExpressionDataType type=els[k]->GetDataType();
        if((type!=exInt)&&(type!=exNum)&&(type!=exNul))
        {
            ErrorMsg="函数mean要求参数类型为数值型,将强制转换数据";
        }
    }
    return true;
}

Expression*MeanFunction::Clone()
{
    MeanFunction*newf=new MeanFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string MeanFunction::GetInterpreteString()
{
    int Size=els.size();
    string sInfo="";
    for(int k=0;k<Size;k++)
    {
        string Label="?";
        if(els[k]!=NULL) Label=els[k]->GetInterpreteString();
        if(k>0)
            sInfo=sInfo+","+Label;
        else
            sInfo=Label;
    }
    return "mean("+sInfo+")";
}

Expression*MeanFunction::GetValue()
{
    int Size=els.size();
    float meanv=0;
    int num=0;
    for(int k=0;k<Size;k++)
    {
        Expression*exp=els[k];
        NumericClass*V=exp->ReturnNewNumeric();
        meanv+=V->Value;
        delete V;
        num++;
    }
    if(num==0) return 0;
    NumericClass*iV=new NumericClass;
    iV->Value=meanv/num;
    return iV;
}
void MeanFunction::ReturnValue(Variant&oV)
{
    int Size=els.size();
    Variant toV;
    float meanv=0;
    int num=0;
    for(int k=0;k<Size;k++)
    {
        Expression*exp=els[k];
        exp->ReturnNumeric(toV);
        meanv+=toV.fltVal;
        num++;
    }
    oV.vt=VRT_R4;
    if(num==0)
        oV.fltVal=0;
    else
        oV.fltVal=meanv/num;
}
ExpressionDataType SumFunction::GetDataType()
{
    if(dtype!=exNul) return dtype;
    bool HasNum=false;
    for(int k=els.size()-1;k>=0;k--)
    {
        if(els[k]==NULL) return exNum;
        ExpressionDataType type=els[k]->GetDataType();
        if((type!=exInt)&&(type!=exLog))
        {
            HasNum=true;
            break;
        }
    }
    if(HasNum)
        dtype= exNum;
    else
        dtype=exInt;
    return dtype;
}

bool SumFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()<2)
    {
        ErrorMsg="函数sum要求参数个数大于等于2";
        return false;
    }
    ErrorMsg="";
    for(int k=els.size()-1;k>=0;k--)
    {
        ExpressionDataType type=els[k]->GetDataType();
        if((type!=exInt)&&(type!=exNum)&&(type!=exNul))
        {
            ErrorMsg="函数sum要求参数类型为数值型,将强制转换数据";
        }
    }
    return true;
}

Expression*SumFunction::Clone()
{
    SumFunction*newf=new SumFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string SumFunction::GetInterpreteString()
{
    int Size=els.size();
    string sInfo="";
    for(int k=0;k<Size;k++)
    {
        string Label="?";
        if(els[k]!=NULL) Label=els[k]->GetInterpreteString();
        if(k>0)
            sInfo=sInfo+","+Label;
        else
            sInfo=Label;
    }
    return "sum("+sInfo+")";
}

Expression*SumFunction::GetValue()
{
    int Size=els.size();
    ExpressionDataType dt=GetDataType();
    if(dt==exInt)
    {
        int sumv=0;
        int num=0;
        for(int k=0;k<Size;k++)
        {
            Expression*exp=els[k];
            IntegerClass*V=exp->ReturnNewInteger();
            sumv+=V->Value;
            delete V;
            num++;
        }
        IntegerClass*iV=new IntegerClass;
        iV->Value=sumv;
        return iV;
    }
    else
    {
        float sumv=0;
        int num=0;
        for(int k=0;k<Size;k++)
        {
            Expression*exp=els[k];
            NumericClass*V=exp->ReturnNewNumeric();
            sumv+=V->Value;
            delete V;
            num++;
        }
        NumericClass*iV=new NumericClass;
        iV->Value=sumv;
        return iV;
    }
    return NULL;
}
void SumFunction::ReturnValue(Variant&oV)
{
    int Size=els.size();
    ExpressionDataType dt=GetDataType();
    if(dt==exInt)
    {
        Variant toV;
        int sumv=0;
        for(int k=0;k<Size;k++)
        {
            Expression*exp=els[k];
            exp->ReturnInteger(toV);
            sumv+=toV.lVal;
        }
        oV.vt=VRT_I4;
        oV.lVal=sumv;
    }
    else
    {
        Variant toV;
        float sumv=0;
        for(int k=0;k<Size;k++)
        {
            Expression*exp=els[k];
            exp->ReturnNumeric(toV);
            sumv+=toV.fltVal;
        }
        oV.vt=VRT_R4;
        oV.fltVal=sumv;
    }
}
ExpressionDataType FmodFunction::GetDataType()
{
    return exNum;
}

bool FmodFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=2)
    {
        ErrorMsg="函数fmod要求参数个数为2";
        return false;
    }
    ExpressionDataType type1=els[0]->GetDataType();
    ExpressionDataType type2=els[1]->GetDataType();
    ErrorMsg="";
    if(((type1==exInt)||(type1==exNum)||(type1==exNul))&&((type2==exInt)||(type2==exNum)||(type2==exNul))) return true;
    ErrorMsg="函数fmod要求参数类型为数值型,将强制转换数据";
    return true;
}

Expression*FmodFunction::Clone()
{
    FmodFunction*newf=new FmodFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string FmodFunction::GetInterpreteString()
{
    string F="?",S="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    if(els[1]!=NULL) S=els[1]->GetInterpreteString();
    return "fmod("+F+","+S+")";
}

Expression*FmodFunction::GetValue()
{
    NumericClass*V1=els[0]->ReturnNewNumeric();
    NumericClass*V2=els[1]->ReturnNewNumeric();
    V1->Value=fmod(V1->Value,V2->Value);
    delete V2;
    return V1;
}
void FmodFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
    Variant toV;
    els[1]->ReturnNumeric(toV);
    oV.fltVal=fmod(oV.fltVal,toV.fltVal);
}
ExpressionDataType PowFunction::GetDataType()
{
    return exNum;
}

bool PowFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=2)
    {
        ErrorMsg="函数pow要求参数个数为2";
        return false;
    }
    ExpressionDataType type1=els[0]->GetDataType();
    ExpressionDataType type2=els[1]->GetDataType();
    ErrorMsg="";
    if(((type1==exInt)||(type1==exNum)||(type1==exNul))&&((type2==exInt)||(type2==exNum)||(type2==exNul))) return true;
    ErrorMsg="函数pow要求参数类型为数值型,将强制转换数据";
    return true;
}

Expression*PowFunction::Clone()
{
    PowFunction*newf=new PowFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string PowFunction::GetInterpreteString()
{
    string F="?",S="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    if(els[1]!=NULL) S=els[1]->GetInterpreteString();
    return "pow("+F+","+S+")";
}

Expression*PowFunction::GetValue()
{
    NumericClass*V1=els[0]->ReturnNewNumeric();
    NumericClass*V2=els[1]->ReturnNewNumeric();
    V1->Value=pow(V1->Value,V2->Value);
    delete V2;
    return V1;
}
void PowFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
    Variant toV;
    els[1]->ReturnNumeric(toV);
    oV.fltVal=pow(oV.fltVal,toV.fltVal);
}
ExpressionDataType CLngFunction::GetDataType()
{
    return exInt;
}

bool CLngFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数clng要求参数个数为1";
        return false;
    }
    return true;
}

Expression*CLngFunction::Clone()
{
    CLngFunction*newf=new CLngFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string CLngFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "clng(?)";
    return "clng("+els[0]->GetInterpreteString()+")";
}

Expression*CLngFunction::GetValue()
{
    return els[0]->ReturnNewInteger();
}
void CLngFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnInteger(oV);
}
ExpressionDataType CDblFunction::GetDataType()
{
    return exNum;
}

bool CDblFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数cdbl要求参数个数为1";
        return false;
    }
    return true;
}

Expression*CDblFunction::Clone()
{
    CDblFunction*newf=new CDblFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string CDblFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "cdbl(?)";
    return "cdbl("+els[0]->GetInterpreteString()+")";
}

Expression*CDblFunction::GetValue()
{
    return els[0]->ReturnNewNumeric();
}
void CDblFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnNumeric(oV);
}
ExpressionDataType IIFFunction::GetDataType()
{
    if(els[1]==NULL) return exNul;
    ExpressionDataType dType1=els[1]->GetDataType();
    ExpressionDataType dType2=els[2]->GetDataType();
    if((dType1==exInt)&&(dType2==exInt)) return exInt;
    if((dType1==exLog)&&(dType2==exLog)) return exLog;
    if((dType1==exDat)&&(dType2==exDat)) return exLog;
    if((dType1==exStr)||(dType2==exStr)) return exStr;
    if((dType1==exNum)||(dType2==exNum)) return exNum;
    return els[1]->GetDataType();
}

bool IIFFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=3)
    {
        ErrorMsg="函数iif要求参数个数为3";
        return false;
    }
    ExpressionDataType type1=els[0]->GetDataType();
    ExpressionDataType type2=els[1]->GetDataType();
    ExpressionDataType type3=els[2]->GetDataType();
    ErrorMsg="";
    if((type1!=exLog)&&(type1!=exNul))
        ErrorMsg="函数iif要求第一个参数类型为逻辑型,将强制转换数据";
    else if((type2!=type3)&&(type2!=exNul)&&(type3!=exNul))
    {
        ErrorMsg="函数iif要求后面两个参数类型一致,将依据第2个参数类型强制转换数据";
    }
    return true;
}

Expression*IIFFunction::Clone()
{
    IIFFunction*newf=new IIFFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string IIFFunction::GetInterpreteString()
{
    string F="?",M="?",S="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    if(els[1]!=NULL) M=els[1]->GetInterpreteString();
    if(els[2]!=NULL) S=els[2]->GetInterpreteString();
    return "iif("+F+","+M+","+S+")";
}

Expression*IIFFunction::GetValue()
{
    LogicClass*V=els[0]->ReturnNewLogic();
    bool bV=V->Value;
    delete V;
    if(bV)
    {
        return els[1]->GetValue();
    }
    return els[2]->GetValue();
}
void IIFFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnLogic(oV);
    if(oV.boolVal)
    {
        els[1]->ReturnValue(oV);
        return;
    }
    els[2]->ReturnValue(oV);
}
ExpressionDataType CStrFunction::GetDataType()
{
    return exStr;
}

bool CStrFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数cstr要求参数个数为1";
        return false;
    }
    return true;
}

Expression*CStrFunction::Clone()
{
    CStrFunction*newf=new CStrFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string CStrFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "cstr(?)";
    return "cstr("+els[0]->GetInterpreteString()+")";
}

Expression*CStrFunction::GetValue()
{
    StringClass*V=els[0]->ReturnNewString();
    return V;
}
void CStrFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnString(oV);
}
ExpressionDataType LengthFunction::GetDataType()
{
    return exInt;
}

bool LengthFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数length要求参数个数为1";
        return false;
    }
    if((els[0]==NULL)||((els[0]->GetDataType()!=exStr)&&(els[0]->GetDataType()!=exNul)))
    {
        ErrorMsg="函数length参数要求为字符型，将强制转换数据";
    }
    return true;
}

Expression*LengthFunction::Clone()
{
    LengthFunction*newf=new LengthFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string LengthFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "length(?)";
    return "length("+els[0]->GetInterpreteString()+")";
}

Expression*LengthFunction::GetValue()
{
    StringClass*V=els[0]->ReturnNewString();
    int len=V->Value.length();
    delete V;
    IntegerClass*iV=new IntegerClass;
    iV->Value=len;
    return iV;
}
void LengthFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnString(oV);
    string str=(string)(*oV.bstrVal);
    oV.Clear();
    oV.vt=VRT_I4;
    oV.lVal=str.length();
}
ExpressionDataType LCaseFunction::GetDataType()
{
    return exStr;
}

bool LCaseFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数lcase要求参数个数为1";
        return false;
    }
    if((els[0]==NULL)||((els[0]->GetDataType()!=exStr)&&(els[0]->GetDataType()!=exNul)))
    {
        ErrorMsg="函数lcase要求参数为字符型，将强制转换数据";
    }
    return true;
}

Expression*LCaseFunction::Clone()
{
    LCaseFunction*newf=new LCaseFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string LCaseFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "lcase(?)";
    return "lcase("+els[0]->GetInterpreteString()+")";
}

Expression*LCaseFunction::GetValue()
{
    StringClass*sV=els[0]->ReturnNewString();
    StringHelper::MakeLower(sV->Value);
    return sV;
}
void LCaseFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnString(oV);
    string str=(string)(*oV.bstrVal);
    StringHelper::MakeLower(str);
    oV=Variant(str);
}
ExpressionDataType UCaseFunction::GetDataType()
{
    return exStr;
}

bool UCaseFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数ucase要求参数个数为1";
        return false;
    }
    if((els[0]==NULL)||((els[0]->GetDataType()!=exStr)&&(els[0]->GetDataType()!=exNul)))
    {
        ErrorMsg="函数ucase要求参数为字符型，将强制转换数据";
    }
    return true;
}

Expression*UCaseFunction::Clone()
{
    UCaseFunction*newf=new UCaseFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string UCaseFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "ucase(?)";
    return "ucase("+els[0]->GetInterpreteString()+")";
}

Expression*UCaseFunction::GetValue()
{
    StringClass*sV=els[0]->ReturnNewString();
    StringHelper::MakeUpper(sV->Value);
    return sV;
}
void UCaseFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnString(oV);
    string str=(string)(*oV.bstrVal);
    StringHelper::MakeUpper(str);
    oV=Variant(str);
}
ExpressionDataType LTrimFunction::GetDataType()
{
    return exStr;
}

bool LTrimFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数ltrim要求参数个数为1";
        return false;
    }
    if((els[0]==NULL)||((els[0]->GetDataType()!=exStr)&&(els[0]->GetDataType()!=exNul)))
    {
        ErrorMsg="函数ltrim要求参数为字符型，将强制转换数据";
    }
    return true;
}

Expression*LTrimFunction::Clone()
{
    LTrimFunction*newf=new LTrimFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string LTrimFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "ltrim(?)";
    return "ltrim("+els[0]->GetInterpreteString()+")";
}

Expression*LTrimFunction::GetValue()
{
    StringClass*sV=els[0]->ReturnNewString();
    sV->Value=StringHelper::TrimLeft(sV->Value);
    return sV;
}
void LTrimFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnString(oV);
    string str=(string)(*oV.bstrVal);
    str=StringHelper::TrimLeft(str);
    oV=Variant(str);
}
ExpressionDataType RTrimFunction::GetDataType()
{
    return exStr;
}

bool RTrimFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数rtrim要求参数个数为1";
        return false;
    }
    if((els[0]==NULL)||((els[0]->GetDataType()!=exStr)&&(els[0]->GetDataType()!=exNul)))
    {
        ErrorMsg="函数rtrim要求参数为字符型，将强制转换数据";
    }
    return true;
}

Expression*RTrimFunction::Clone()
{
    RTrimFunction*newf=new RTrimFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string RTrimFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "rtrim(?)";
    return "rtrim("+els[0]->GetInterpreteString()+")";
}

Expression*RTrimFunction::GetValue()
{
    StringClass*sV=els[0]->ReturnNewString();
    sV->Value=StringHelper::TrimRight(sV->Value);
    return sV;
}
void RTrimFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnString(oV);
    string str=(string)(*oV.bstrVal);
    str=StringHelper::TrimRight(str);
    oV=Variant(str);
}
ExpressionDataType TrimFunction::GetDataType()
{
    return exStr;
}

bool TrimFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数trim要求参数个数为1";
        return false;
    }
    if((els[0]==NULL)||((els[0]->GetDataType()!=exStr)&&(els[0]->GetDataType()!=exNul)))
    {
        ErrorMsg="函数trim要求参数为字符型，将强制转换数据";
    }
    return true;
}

Expression*TrimFunction::Clone()
{
    TrimFunction*newf=new TrimFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string TrimFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "trim(?)";
    return "trim("+els[0]->GetInterpreteString()+")";
}

Expression*TrimFunction::GetValue()
{
    StringClass*sV=els[0]->ReturnNewString();
    sV->Value=StringHelper::Trim(sV->Value);
    return sV;
}
void TrimFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnString(oV);
    string str=(string)(*oV.bstrVal);
    str=StringHelper::Trim(str);
    oV=Variant(str);
}
ExpressionDataType SpaceFunction::GetDataType()
{
    return exStr;
}

bool SpaceFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数space要求参数个数为1";
        return false;
    }
    if((els[0]==NULL)||((els[0]->GetDataType()!=exInt)&&(els[0]->GetDataType()!=exNul)))
    {
        ErrorMsg="函数space要求参数为整型，将强制转换数据";
    }
    return true;
}

Expression*SpaceFunction::Clone()
{
    SpaceFunction*newf=new SpaceFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string SpaceFunction::GetInterpreteString()
{
    if(els[0]==NULL) return "space(?)";
    return "space("+els[0]->GetInterpreteString()+")";
}

Expression*SpaceFunction::GetValue()
{
    IntegerClass*V=els[0]->ReturnNewInteger();
    StringClass*sV=new StringClass;
    string Info="";
    int num=V->Value;
    delete V;
    if(num>100) num=100;
    for(int k=0;k<num;k++) Info=Info+" ";
    sV->Value=Info;
    return sV;
}
void SpaceFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnInteger(oV);
    int num=oV.lVal;
    if(num>100) num=100;
    string Info="";
    for(int k=0;k<num;k++) Info=Info+" ";
    oV=Variant(Info);
}
ExpressionDataType LeftFunction::GetDataType()
{
    return exStr;
}

bool LeftFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=2)
    {
        ErrorMsg="函数left要求参数个数为2";
        return false;
    }
    bool IsFirstValid=true,IsSecondValid=true;
    if((els[0]==NULL)||((els[0]->GetDataType()!=exStr)&&(els[0]->GetDataType()!=exNul)))
    {
        IsFirstValid=false;
    }
    if((els[1]==NULL)||((els[1]->GetDataType()!=exInt)&&(els[1]->GetDataType()!=exNul)))
    {
        IsSecondValid=false;
    }
    if((!IsFirstValid)&&(!IsSecondValid))
        ErrorMsg="函数left要求第1、2个参数依次为字符型、整数型，将强制转换数据";
    else if(!IsFirstValid)
        ErrorMsg="函数left要求第1个参数为字符型，将强制转换数据";
    else if(!IsSecondValid)
        ErrorMsg="函数left要求第2个参数为整数型，将强制转换数据";
    return true;
}

Expression*LeftFunction::Clone()
{
    LeftFunction*newf=new LeftFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string LeftFunction::GetInterpreteString()
{
    string F="?",S="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    if(els[1]!=NULL) S=els[1]->GetInterpreteString();
    return "left("+F+","+S+")";
}

Expression*LeftFunction::GetValue()
{
    StringClass*V1=els[0]->ReturnNewString();
    IntegerClass*V2=els[1]->ReturnNewInteger();
    V1->Value=V1->Value.substr(0,V2->Value);
    delete V2;
    return V1;
}
void LeftFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnString(oV);
    Variant toV;
    els[1]->ReturnInteger(toV);
    string sV=(string)(*oV.bstrVal);
    sV=sV.substr(0,toV.lVal);
    oV=Variant(sV);
}
ExpressionDataType RightFunction::GetDataType()
{
    return exStr;
}

bool RightFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=2)
    {
        ErrorMsg="函数right要求参数个数为2";
        return false;
    }
    bool IsFirstValid=true,IsSecondValid=true;
    if((els[0]==NULL)||((els[0]->GetDataType()!=exStr)&&(els[0]->GetDataType()!=exNul)))
    {
        IsFirstValid=false;
    }
    if((els[1]==NULL)||((els[1]->GetDataType()!=exInt)&&(els[1]->GetDataType()!=exNul)))
    {
        IsSecondValid=false;
    }
    if((!IsFirstValid)&&(!IsSecondValid))
        ErrorMsg="函数right要求第1、2个参数依次为字符型、整数型，将强制转换数据";
    else if(!IsFirstValid)
        ErrorMsg="函数right要求第1个参数为字符型，将强制转换数据";
    else if(!IsSecondValid)
        ErrorMsg="函数right要求第2个参数为整数型，将强制转换数据";
    return true;
}

Expression*RightFunction::Clone()
{
    RightFunction*newf=new RightFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string RightFunction::GetInterpreteString()
{
    string F="?",S="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    if(els[1]!=NULL) S=els[1]->GetInterpreteString();
    return "right("+F+","+S+")";
}

Expression*RightFunction::GetValue()
{
    StringClass*V1=els[0]->ReturnNewString();
    IntegerClass*V2=els[1]->ReturnNewInteger();
    V1->Value=V1->Value.substr(V1->Value.length()-V2->Value,V2->Value);
    delete V2;
    return V1;
}
void RightFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnString(oV);
    Variant toV;
    els[1]->ReturnInteger(toV);
    string sV=(string)(*oV.bstrVal);
    sV=sV.substr(sV.length()-toV.lVal,toV.lVal);
    oV=Variant(sV);
}
ExpressionDataType InStrFunction::GetDataType()
{
    return exInt;
}

bool InStrFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=3)
    {
        ErrorMsg="函数instr要求参数个数为3";
        return false;
    }
    bool IsFirstValid=true,IsSecondValid=true,IsThirdValid=true;
    if((els[0]==NULL)||((els[0]->GetDataType()!=exInt)&&(els[0]->GetDataType()!=exNul)))
    {
        IsFirstValid=false;
    }
    if((els[1]==NULL)||((els[1]->GetDataType()!=exStr)&&(els[1]->GetDataType()!=exNul)))
    {
        IsSecondValid=false;
    }
    if((els[2]==NULL)||((els[2]->GetDataType()!=exStr)&&(els[2]->GetDataType()!=exNul)))
    {
        IsThirdValid=false;
    }
    if((!IsFirstValid)&&(!IsSecondValid)&&(!IsThirdValid))
        ErrorMsg="函数instr要求第1、2、3个参数依次为整数型、字符型、字符型，将强制转换数据";
    else if((!IsFirstValid)&&(!IsSecondValid))
        ErrorMsg="函数instr要求第1、2个参数为整数型、字符型，将强制转换数据";
    else if((!IsFirstValid)&&(!IsThirdValid))
        ErrorMsg="函数instr要求第1、3个参数为整数型、字符型，将强制转换数据";
    else if((!IsSecondValid)&&(!IsThirdValid))
        ErrorMsg="函数instr要求第2、3个参数为字符型、字符型，将强制转换数据";
    else if(!IsFirstValid)
        ErrorMsg="函数instr要求第1个参数为整数型，将强制转换数据";
    else if(!IsSecondValid)
        ErrorMsg="函数instr要求第2个参数为字符型，将强制转换数据";
    else if(!IsThirdValid)
        ErrorMsg="函数instr要求第3个参数为字符型，将强制转换数据";
    return true;
}

Expression*InStrFunction::Clone()
{
    InStrFunction*newf=new InStrFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string InStrFunction::GetInterpreteString()
{
    string F="?",M="?",S="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    if(els[1]!=NULL) M=els[1]->GetInterpreteString();
    if(els[2]!=NULL) S=els[2]->GetInterpreteString();
    return "instr("+F+","+M+","+S+")";
}

Expression*InStrFunction::GetValue()
{
    IntegerClass*V1=els[0]->ReturnNewInteger();
    StringClass*V2=els[1]->ReturnNewString();
    StringClass*V3=els[2]->ReturnNewString();
    V1->Value=V2->Value.find(V3->Value,V1->Value);
    delete V2;
    delete V3;
    return V1;
}
void InStrFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnInteger(oV);
    Variant toV;
    els[1]->ReturnString(toV);
    string sV=(string)(*oV.bstrVal);
    els[2]->ReturnString(toV);
    oV.lVal=sV.find((string)(*toV.bstrVal),oV.lVal);
}
ExpressionDataType InStrRevFunction::GetDataType()
{
    return exInt;
}

bool InStrRevFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=3)
    {
        ErrorMsg="函数instrrev要求参数个数为3";
        return false;
    }
    bool IsFirstValid=true,IsSecondValid=true,IsThirdValid=true;
    if((els[0]==NULL)||((els[0]->GetDataType()!=exInt)&&(els[0]->GetDataType()!=exNul)))
    {
        IsFirstValid=false;
    }
    if((els[1]==NULL)||((els[1]->GetDataType()!=exStr)&&(els[1]->GetDataType()!=exNul)))
    {
        IsSecondValid=false;
    }
    if((els[2]==NULL)||((els[2]->GetDataType()!=exStr)&&(els[2]->GetDataType()!=exNul)))
    {
        IsThirdValid=false;
    }
    if((!IsFirstValid)&&(!IsSecondValid)&&(!IsThirdValid))
        ErrorMsg="函数instrrev要求第1、2、3个参数依次为整数型、字符型、字符型，将强制转换数据";
    else if((!IsFirstValid)&&(!IsSecondValid))
        ErrorMsg="函数instrrev要求第1、2个参数为整数型、字符型，将强制转换数据";
    else if((!IsFirstValid)&&(!IsThirdValid))
        ErrorMsg="函数instrrev要求第1、3个参数为整数型、字符型，将强制转换数据";
    else if((!IsSecondValid)&&(!IsThirdValid))
        ErrorMsg="函数instrrev要求第2、3个参数为字符型、字符型，将强制转换数据";
    else if(!IsFirstValid)
        ErrorMsg="函数instrrev要求第1个参数为整数型，将强制转换数据";
    else if(!IsSecondValid)
        ErrorMsg="函数instrrev要求第2个参数为字符型，将强制转换数据";
    else if(!IsThirdValid)
        ErrorMsg="函数instrrev要求第3个参数为字符型，将强制转换数据";
    return true;
}

Expression*InStrRevFunction::Clone()
{
    InStrRevFunction*newf=new InStrRevFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string InStrRevFunction::GetInterpreteString()
{
    string F="?",M="?",S="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    if(els[1]!=NULL) M=els[1]->GetInterpreteString();
    if(els[2]!=NULL) S=els[2]->GetInterpreteString();
    return "instrrev("+F+","+M+","+S+")";
}

Expression*InStrRevFunction::GetValue()
{
    IntegerClass*V1=els[0]->ReturnNewInteger();
    StringClass*V2=els[1]->ReturnNewString();
    StringClass*V3=els[2]->ReturnNewString();
    int posi=0,p;
    posi=V2->Value.find(V3->Value,0);
    p=posi;
    if(posi>=0)
    {
        if(posi>V1->Value)
        {
            p=-1;
        }
        else
        {
            while(true)
            {
                posi=V2->Value.find(V3->Value,posi+1);
                if(posi==-1) break;
                if(posi>V1->Value) break;
                p=posi;
            }
        }
    }
    V1->Value=posi;
    delete V2;
    delete V3;
    return V1;
}
void InStrRevFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnInteger(oV);
    Variant toV;
    els[1]->ReturnString(toV);
    string sV=(string)(*oV.bstrVal);
    els[2]->ReturnString(toV);
    string sV1=(string)(*oV.bstrVal);
    int posi=0,p;
    posi=sV.find(sV1,0);
    p=posi;
    if(posi>=0)
    {
        if(posi>oV.lVal)
        {
            p=-1;
        }
        else
        {
            while(true)
            {
                posi=sV.find(sV1,posi+1);
                if(posi==-1) break;
                if(posi>oV.lVal) break;
                p=posi;
            }
        }
    }
    oV.lVal=posi;
}
ExpressionDataType MidFunction::GetDataType()
{
    return exStr;
}

bool MidFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=3)
    {
        ErrorMsg="函数mid要求参数个数为3";
        return false;
    }
    bool IsFirstValid=true,IsSecondValid=true,IsThirdValid=true;
    if((els[0]==NULL)||((els[0]->GetDataType()!=exStr)&&(els[0]->GetDataType()!=exNul)))
    {
        IsFirstValid=false;
    }
    if((els[1]==NULL)||((els[1]->GetDataType()!=exInt)&&(els[1]->GetDataType()!=exNul)))
    {
        IsSecondValid=false;
    }
    if((els[2]==NULL)||((els[2]->GetDataType()!=exInt)&&(els[2]->GetDataType()!=exNul)))
    {
        IsThirdValid=false;
    }
    if((!IsFirstValid)&&(!IsSecondValid)&&(!IsThirdValid))
        ErrorMsg="函数mid要求第1、2、3个参数依次为字符型、整数型、整数型，将强制转换数据";
    else if((!IsFirstValid)&&(!IsSecondValid))
        ErrorMsg="函数mid要求第1、2个参数为字符型、整数型，将强制转换数据";
    else if((!IsFirstValid)&&(!IsThirdValid))
        ErrorMsg="函数mid要求第1、3个参数为字符型、整数型，将强制转换数据";
    else if((!IsSecondValid)&&(!IsThirdValid))
        ErrorMsg="函数mid要求第2、3个参数为整数型、整数型，将强制转换数据";
    else if(!IsFirstValid)
        ErrorMsg="函数mid要求第1个参数为字符型，将强制转换数据";
    else if(!IsSecondValid)
        ErrorMsg="函数mid要求第2个参数为整数型，将强制转换数据";
    else if(!IsThirdValid)
        ErrorMsg="函数mid要求第3个参数为整数型，将强制转换数据";
    return true;
}

Expression*MidFunction::Clone()
{
    MidFunction*newf=new MidFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string MidFunction::GetInterpreteString()
{
    string F="?",M="?",S="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    if(els[1]!=NULL) M=els[1]->GetInterpreteString();
    if(els[2]!=NULL) S=els[2]->GetInterpreteString();
    return "mid("+F+","+M+","+S+")";
}

Expression*MidFunction::GetValue()
{
    StringClass*V1=els[0]->ReturnNewString();
    IntegerClass*V2=els[1]->ReturnNewInteger();
    IntegerClass*V3=els[2]->ReturnNewInteger();
    V1->Value=V1->Value.substr(V2->Value,V3->Value);
    delete V2;
    delete V3;
    return V1;
}
void MidFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnString(oV);
    Variant toV;
    els[1]->ReturnInteger(toV);
    int iV=toV.lVal;
    els[2]->ReturnInteger(toV);
    string sV=(string)(*oV.bstrVal);
    sV=sV.substr(iV,toV.lVal);
    oV=Variant(sV);
}
ExpressionDataType CDateFunction::GetDataType()
{
    return exDat;
}

bool CDateFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数cdate要求参数个数为1";
        return false;
    }
    return true;
}

Expression*CDateFunction::Clone()
{
    CDateFunction*newf=new CDateFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string CDateFunction::GetInterpreteString()
{
    string F="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    return "cdate("+F+")";
}

Expression*CDateFunction::GetValue()
{
    return els[0]->ReturnNewDate();
}
void CDateFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnDate(oV);
}
ExpressionDataType NowFunction::GetDataType()
{
    return exDat;
}

bool NowFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=-0)
    {
        ErrorMsg="函数now要求参数个数为0";
        return false;
    }
    return true;
}

Expression*NowFunction::Clone()
{
    NowFunction*newf=new NowFunction;
    return newf;
}

string NowFunction::GetInterpreteString()
{
    return "now()";
}

Expression*NowFunction::GetValue()
{
    DateTime dt;
    DateClass*dV=new DateClass();
    dV->SetValue(dt.GetYear(),dt.GetMonth(),dt.GetDay());
    return dV;
}
void NowFunction::ReturnValue(Variant&oV)
{
    DateTime dt;
    oV.vt=VRT_DATETIME;
    oV.date=dt.ToDouble();
}
ExpressionDataType DayFunction::GetDataType()
{
    return exInt;
}

bool DayFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数day要求参数个数为1";
        return false;
    }
    if((els[0]==NULL)||((els[0]->GetDataType()!=exDat)&&(els[0]->GetDataType()!=exNul)))
        ErrorMsg="函数day要求参数类型为日期型，将强制转换数据";
    return true;
}

Expression*DayFunction::Clone()
{
    DayFunction*newf=new DayFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string DayFunction::GetInterpreteString()
{
    string F="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    return "day("+F+")";
}

Expression*DayFunction::GetValue()
{
    DateClass*dV=els[0]->ReturnNewDate();
    int day=dV->GetDay();delete dV;
    IntegerClass*iV=new IntegerClass;
    iV->Value=day;
    return iV;
}
void DayFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnDate(oV);
    DateTime dt(oV.date);
    oV.vt=VRT_I4;
    oV.lVal=dt.GetDay();
}
ExpressionDataType MonthFunction::GetDataType()
{
    return exInt;
}

bool MonthFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数month要求参数个数为1";
        return false;
    }
    if((els[0]==NULL)||((els[0]->GetDataType()!=exDat)&&(els[0]->GetDataType()!=exNul)))
        ErrorMsg="函数month要求参数类型为日期型，将强制转换数据";
    return true;
}

Expression*MonthFunction::Clone()
{
    MonthFunction*newf=new MonthFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string MonthFunction::GetInterpreteString()
{
    string F="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    return "month("+F+")";
}

Expression*MonthFunction::GetValue()
{
    DateClass*dV=els[0]->ReturnNewDate();
    int day=dV->GetMonth();delete dV;
    IntegerClass*iV=new IntegerClass;
    iV->Value=day;
    return iV;
}
void MonthFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnDate(oV);
    DateTime dt(oV.date);
    oV.vt=VRT_I4;
    oV.lVal=dt.GetMonth();
}
ExpressionDataType YearFunction::GetDataType()
{
    return exInt;
}

bool YearFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=1)
    {
        ErrorMsg="函数year要求参数个数为1";
        return false;
    }
    if((els[0]==NULL)||((els[0]->GetDataType()!=exDat)&&(els[0]->GetDataType()!=exNul)))
        ErrorMsg="函数year要求参数类型为日期型，将强制转换数据";
    return true;
}

Expression*YearFunction::Clone()
{
    YearFunction*newf=new YearFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string YearFunction::GetInterpreteString()
{
    string F="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    return "year("+F+")";
}

Expression*YearFunction::GetValue()
{
    DateClass*dV=els[0]->ReturnNewDate();
    int day=dV->GetYear();delete dV;
    IntegerClass*iV=new IntegerClass;
    iV->Value=day;
    return iV;
}
void YearFunction::ReturnValue(Variant&oV)
{
    if(els[0]==NULL)
    {
        oV.vt=VRT_EMPTY;
    }
    els[0]->ReturnDate(oV);
    DateTime dt(oV.date);
    oV.vt=VRT_I4;
    oV.lVal=dt.GetYear();
}
ExpressionDataType DateDiffFunction::GetDataType()
{
    return exInt;
}

bool DateDiffFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=2)
    {
        ErrorMsg="函数datediff要求参数个数为2";
        return false;
    }
    bool IsFirstValid=true,IsSecondValid=true;
    if((els[0]==NULL)||((els[0]->GetDataType()!=exDat)&&(els[0]->GetDataType()!=exNul)))
    {
        IsFirstValid=false;
    }
    if((els[1]==NULL)||((els[1]->GetDataType()!=exDat)&&(els[1]->GetDataType()!=exNul)))
    {
        IsSecondValid=false;
    }
    if((!IsFirstValid)&&(!IsSecondValid))
        ErrorMsg="函数datediff要求第1、2个参数为日期型，将强制转换数据";
    else if(!IsFirstValid)
        ErrorMsg="函数datediff要求第1个参数为日期型，将强制转换数据";
    else if(!IsSecondValid)
        ErrorMsg="函数datediff要求第2个参数为日期型，将强制转换数据";
    return true;
}

Expression*DateDiffFunction::Clone()
{
    DateDiffFunction*newf=new DateDiffFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string DateDiffFunction::GetInterpreteString()
{
    string F="?",S="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    if(els[1]!=NULL) S=els[1]->GetInterpreteString();
    return "datediff("+F+","+S+")";
}

Expression*DateDiffFunction::GetValue()
{
    DateClass*V1=els[0]->ReturnNewDate();
    DateClass*V2=els[1]->ReturnNewDate();
    int dif=V1->DiffDays(V2);
    delete V1;delete V2;
    IntegerClass*iV=new IntegerClass;
    iV->Value=dif;
    return iV;
}
void DateDiffFunction::ReturnValue(Variant&oV)
{
    if((els[0]==NULL)||(els[1]==NULL))
    {
        oV.vt=VRT_EMPTY;
        return;
    }
    els[0]->ReturnDate(oV);
    Variant toV;
    els[1]->ReturnDate(toV);
    DateTime dt1(oV.date);
    DateTime dt2(toV.date);
    oV.vt=VRT_I4;
    oV.lVal=(dt2-dt1).TotalDays();
}
ExpressionDataType DateAddFunction::GetDataType()
{
    return exDat;
}

bool DateAddFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=2)
    {
        ErrorMsg="函数dateadd要求参数个数为2";
        return false;
    }
    bool IsFirstValid=true,IsSecondValid=true;
    if((els[0]==NULL)||((els[0]->GetDataType()!=exDat)&&(els[0]->GetDataType()!=exNul)))
    {
        IsFirstValid=false;
    }
    if((els[1]==NULL)||((els[1]->GetDataType()!=exInt)&&(els[1]->GetDataType()!=exNul)))
    {
        IsSecondValid=false;
    }
    if((!IsFirstValid)&&(!IsSecondValid))
        ErrorMsg="函数dateadd要求第1、2个参数为日期型、整数型，将强制转换数据";
    else if(!IsFirstValid)
        ErrorMsg="函数dateadd要求第1个参数为日期型，将强制转换数据";
    else if(!IsSecondValid)
        ErrorMsg="函数dateadd要求第2个参数为整数型，将强制转换数据";
    return true;
}

Expression*DateAddFunction::Clone()
{
    DateAddFunction*newf=new DateAddFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string DateAddFunction::GetInterpreteString()
{
    string F="?",S="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    if(els[1]!=NULL) S=els[1]->GetInterpreteString();
    return "dateadd("+F+","+S+")";
}

Expression*DateAddFunction::GetValue()
{
    DateClass*V1=els[0]->ReturnNewDate();
    IntegerClass*V2=els[1]->ReturnNewInteger();
    V1->DateAdd(V2->Value);
    delete V2;
    return V1;
}
void DateAddFunction::ReturnValue(Variant&oV)
{
    if((els[0]==NULL)||(els[1]==NULL))
    {
        oV.vt=VRT_EMPTY;
        return;
    }

    els[0]->ReturnDate(oV);
    Variant toV;
    els[1]->ReturnInteger(toV);
    DateTime dt(oV.date);
    dt=dt+TimeSpan(toV.lVal,0,0,0);
    oV.vt=VRT_DATETIME;
    oV.date=dt.ToDouble();
}
ExpressionDataType LFillFunction::GetDataType()
{
    return exStr;
}

bool LFillFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=3)
    {
        ErrorMsg="函数lfill要求参数个数为3";
        return false;
    }
    bool IsFirstValid=true,IsSecondValid=true,IsThirdValid=true;
    if((els[0]==NULL)||((els[0]->GetDataType()!=exStr)&&(els[0]->GetDataType()!=exNul)))
    {
        IsFirstValid=false;
    }
    if((els[1]==NULL)||((els[1]->GetDataType()!=exStr)&&(els[1]->GetDataType()!=exNul)))
    {
        IsSecondValid=false;
    }
    if((els[2]==NULL)||((els[2]->GetDataType()!=exInt)&&(els[2]->GetDataType()!=exNul)))
    {
        IsThirdValid=false;
    }
    if((!IsFirstValid)&&(!IsSecondValid)&&(!IsThirdValid))
        ErrorMsg="函数lfill要求第1、2、3个参数依次为字符型、字符型、整数型，将强制转换数据";
    else if((!IsFirstValid)&&(!IsSecondValid))
        ErrorMsg="函数lfill要求第1、2个参数为字符型、字符型，将强制转换数据";
    else if((!IsFirstValid)&&(!IsThirdValid))
        ErrorMsg="函数lfill要求第1、3个参数为字符型、整数型，将强制转换数据";
    else if((!IsSecondValid)&&(!IsThirdValid))
        ErrorMsg="函数lfill要求第2、3个参数为字符型、整数型，将强制转换数据";
    else if(!IsFirstValid)
        ErrorMsg="函数lfill要求第1个参数为字符型，将强制转换数据";
    else if(!IsSecondValid)
        ErrorMsg="函数lfill要求第2个参数为字符型，将强制转换数据";
    else if(!IsThirdValid)
        ErrorMsg="函数lfill要求第3个参数为整数型，将强制转换数据";
    return true;
}

Expression*LFillFunction::Clone()
{
    LFillFunction*newf=new LFillFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string LFillFunction::GetInterpreteString()
{
    string F="?",M="?",S="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    if(els[1]!=NULL) M=els[1]->GetInterpreteString();
    if(els[2]!=NULL) S=els[2]->GetInterpreteString();
    return "lfill("+F+","+M+","+S+")";
}

Expression*LFillFunction::GetValue()
{
    StringClass*V1=els[0]->ReturnNewString();
    StringClass*V2=els[1]->ReturnNewString();
    string str=V1->Value;
    string fills=V2->Value;
    if(!fills.empty())
    {
        IntegerClass*V3=els[2]->ReturnNewInteger();
        int len1=fills.length();
        int filll=V3->Value;
        int len=str.length();
        int pos=len1-1;
        for(int k=len;k<filll;k++)
        {
            str.insert(begin(str),fills[pos]);
            pos--;
            if(pos<0) pos=len1-1;
        }
        delete V3;
    }
    V1->Value=str;
    delete V2;
    return V1;
}
void LFillFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnString(oV);
    Variant toV;
    els[1]->ReturnString(toV);
    string str=(string)(*oV.bstrVal);
    string fills=(string)(*toV.bstrVal);
    if(!fills.empty())
    {
        els[2]->ReturnInteger(toV);
        int len1=fills.length();
        int filll=toV.lVal;
        int len=str.length();
        int pos=len1-1;
        for(int k=len;k<filll;k++)
        {
            str.insert(begin(str),fills[pos]);
            pos--;
            if(pos<0) pos=len1-1;
        }
    }
    oV=Variant(str);
}
ExpressionDataType RFillFunction::GetDataType()
{
    return exStr;
}

bool RFillFunction::IsExpressionValid(string&ErrorMsg)
{
    if(els.size()!=3)
    {
        ErrorMsg="函数rfill要求参数个数为3";
        return false;
    }
    bool IsFirstValid=true,IsSecondValid=true,IsThirdValid=true;
    if((els[0]==NULL)||((els[0]->GetDataType()!=exStr)&&(els[0]->GetDataType()!=exNul)))
    {
        IsFirstValid=false;
    }
    if((els[1]==NULL)||((els[1]->GetDataType()!=exStr)&&(els[1]->GetDataType()!=exNul)))
    {
        IsSecondValid=false;
    }
    if((els[2]==NULL)||((els[2]->GetDataType()!=exInt)&&(els[2]->GetDataType()!=exNul)))
    {
        IsThirdValid=false;
    }
    if((!IsFirstValid)&&(!IsSecondValid)&&(!IsThirdValid))
        ErrorMsg="函数rfill要求第1、2、3个参数依次为字符型、字符型、整数型，将强制转换数据";
    else if((!IsFirstValid)&&(!IsSecondValid))
        ErrorMsg="函数rfill要求第1、2个参数为字符型、字符型，将强制转换数据";
    else if((!IsFirstValid)&&(!IsThirdValid))
        ErrorMsg="函数rfill要求第1、3个参数为字符型、整数型，将强制转换数据";
    else if((!IsSecondValid)&&(!IsThirdValid))
        ErrorMsg="函数rfill要求第2、3个参数为字符型、整数型，将强制转换数据";
    else if(!IsFirstValid)
        ErrorMsg="函数rfill要求第1个参数为字符型，将强制转换数据";
    else if(!IsSecondValid)
        ErrorMsg="函数rfill要求第2个参数为字符型，将强制转换数据";
    else if(!IsThirdValid)
        ErrorMsg="函数rfill要求第3个参数为整数型，将强制转换数据";
    return true;
}

Expression*RFillFunction::Clone()
{
    RFillFunction*newf=new RFillFunction;
    for(int k=0;k<els.size();k++)
    {
        if(els[k]!=NULL)
            newf->AddElement(els[k]->Clone());
        else
            newf->AddElement(NULL);
    }
    return newf;
}

string RFillFunction::GetInterpreteString()
{
    string F="?",M="?",S="?";
    if(els[0]!=NULL) F=els[0]->GetInterpreteString();
    if(els[1]!=NULL) M=els[1]->GetInterpreteString();
    if(els[2]!=NULL) S=els[2]->GetInterpreteString();
    return "rfill("+F+","+M+","+S+")";
}

Expression*RFillFunction::GetValue()
{
    StringClass*V1=els[0]->ReturnNewString();
    StringClass*V2=els[1]->ReturnNewString();
    string str=V1->Value;
    string fills=V2->Value;
    if(!fills.empty())
    {
        IntegerClass*V3=els[2]->ReturnNewInteger();
        int len1=fills.length();
        int filll=V3->Value;
        int len=str.length();
        int pos=0;
        for(int k=len;k<filll;k++)
        {
            str+=fills[pos];
            pos++;
            if(pos>=len1) pos=0;
        }
        delete V3;
    }
    V1->Value=str;
    delete V2;
    return V1;
}
void RFillFunction::ReturnValue(Variant&oV)
{
    els[0]->ReturnString(oV);
    Variant toV;
    els[1]->ReturnString(toV);
    string str=(string)(*oV.bstrVal);
    string fills=(string)(*toV.bstrVal);
    if(!fills.empty())
    {
        els[2]->ReturnInteger(toV);
        int len1=fills.length();
        int filll=toV.lVal;
        int len=str.length();
        int pos=0;
        for(int k=len;k<filll;k++)
        {
            str+=fills[pos];
            pos++;
            if(pos>=len1) pos=0;
        }
    }
    oV=Variant(str);
}


VirtualField::VirtualField()
{
   pVirtualTable=NULL;
   ExpressionKind=7;
   FieldIndex=-1;
   FieldName="";
}
VirtualField::VirtualField(VirtualTable*VirtualTable,string fieldName)
{
   FieldName=fieldName;
   pVirtualTable=VirtualTable;
   ExpressionKind=7;
   if(pVirtualTable!=NULL)
   {
       FieldIndex=pVirtualTable->FindField(FieldName);
   }
}
bool VirtualField::CanSimplified()
{
   return false;
}
ExpressionDataType VirtualField::GetDataType()
{
    if(FieldIndex==-1) return exNul;
    return pVirtualTable->GetFieldType(FieldIndex);
}

bool VirtualField::IsExpressionValid(string&ErrorMsg)
{
    if(FieldIndex==-1)
    {
        string Name=pVirtualTable->GetTipName();
        if(FieldName.empty())
            ErrorMsg="不允许空"+Name;
        else
            ErrorMsg="非法"+Name+":"+FieldName;
        return false;
    }
    if(pVirtualTable==NULL)
    {
        ErrorMsg="非法字段"+FieldName+",当前不存在计算表";
        return false;
    }
    if(els.size()!=0)
    {
        string Name=pVirtualTable->GetTipName();
        if(FieldName.empty())
            ErrorMsg="不允许空"+Name;
        else
            ErrorMsg="非法"+Name+":"+FieldName;
        return false;
    }
    return true;
}

Expression*VirtualField::Clone()
{
    VirtualField*newf=new VirtualField();
    newf->FieldIndex=FieldIndex;
    newf->pVirtualTable=pVirtualTable;
    newf->FieldName=FieldName;
    return newf;
}

string VirtualField::GetInterpreteString()
{
    if(FieldIndex==-1) return "[?]";
    if(pVirtualTable==NULL) return "[?]";
    string AliasName=pVirtualTable->GetFieldAliasName(FieldIndex);
    return "["+AliasName+"]";
}
Expression*VirtualField::GetValue()
{
    ExpressionDataType type=pVirtualTable->GetFieldType(FieldIndex);
    switch(type)
    {
    case exInt:
        {
            IntegerClass*fV=new IntegerClass;
            Variant vt=pVirtualTable->GetFieldValue(FieldIndex);
            fV->Value=vt.lVal;
            return fV;
        }
    case exNum:
        {
            NumericClass*fV=new NumericClass;
            Variant vt=pVirtualTable->GetFieldValue(FieldIndex);
            fV->Value=vt.fltVal;
            return fV;
        }
    case exLog:
        {
            LogicClass*ls=new LogicClass;
            Variant vt=pVirtualTable->GetFieldValue(FieldIndex);
            ls->Value=vt.boolVal;
            return ls;
        }
    case exStr:
        {
            StringClass*sV=new StringClass;
            Variant vt=pVirtualTable->GetFieldValue(FieldIndex);
            sV->Value=(*vt.bstrVal);
            return sV;
        }
    case exDat:
        {
            DateClass*dV=new DateClass;
            Variant vt=pVirtualTable->GetFieldValue(FieldIndex);
            DateTime dt(vt.date);
            dV->SetValue(dt.GetYear(),dt.GetMonth(),dt.GetDay());
            return dV;
        }
    }
    return NULL;
}
void VirtualField::ReturnValue(Variant&oV)
{
    oV=pVirtualTable->GetFieldValue(FieldIndex);
}
Grid::Grid()
{
   pGrids=NULL;
   ExpressionKind=7;
   GridIndex=-1;
   GridName="";
}
Grid::Grid(VirtualGrids*Grids,string Name)
{
   pGrids=Grids;
   ExpressionKind=7;
   GridName=Name;
   if(pGrids!=NULL)
   {
       GridIndex=pGrids->FindGrid(Name);
   }
}
bool Grid::CanSimplified()
{
   return false;
}
ExpressionDataType Grid::GetDataType()
{
    if(GridIndex==-1) return exNul;
    ExpressionDataType type=pGrids->GetGridType(GridIndex);
    return type;
}

bool Grid::IsExpressionValid(string&ErrorMsg)
{
    if(GridIndex==-1)
    {
        string Name=pGrids->GetTipName();
        if(GridName.empty())
            ErrorMsg="不允许空"+Name;
        else
            ErrorMsg="非法"+Name+":"+GridName;
        return false;
    }
    if(pGrids==NULL)
    {
        ErrorMsg="非法栅格,当前不存在栅格集";
        return false;
    }
    if(els.size()!=0)
    {
        string Name=pGrids->GetTipName();
        if(GridName.empty())
            ErrorMsg="不允许空"+Name;
        else
            ErrorMsg="非法"+Name+":"+GridName;
        return false;
    }
    return true;
}

Expression*Grid::Clone()
{
    Grid*newf=new Grid();
    newf->GridIndex=GridIndex;
    newf->pGrids=pGrids;
    newf->GridName=GridName;
    return newf;
}

string Grid::GetInterpreteString()
{
    if(GridIndex==-1) return "[?]";
    if(pGrids==NULL) return "[?]";
    string Alias=pGrids->GetGridAliasName(GridIndex);
    return "["+Alias+"]";
}
Expression*Grid::GetValue()
{
    ExpressionDataType type=pGrids->GetGridType(GridIndex);
    switch(type)
    {
    case exInt:
        {
            IntegerClass*fVs=new IntegerClass;
            FLOAT fV=pGrids->GetGridValue(GridIndex);
            fVs->Value=fV;
            return fVs;
        }
    case exNum:
        {
            NumericClass*fVs=new NumericClass;
            FLOAT fV=pGrids->GetGridValue(GridIndex);
            fVs->Value=fV;
            return fVs;
        }
    }
    return NULL;
}
void Grid::ReturnValue(Variant&oV)
{
    ExpressionDataType type=pGrids->GetGridType(GridIndex);
    switch(type)
    {
    case exInt:
        {
            oV.vt=VRT_I4;
            FLOAT fV=pGrids->GetGridValue(GridIndex);
            oV.lVal=fV;
            return;
        }
    case exNum:
        {
            oV.vt=VRT_R4;
            FLOAT fV=pGrids->GetGridValue(GridIndex);
            oV.fltVal=fV;
            return;
        }
    }
    oV.vt=VRT_EMPTY;
}
PinExpressionClass::~PinExpressionClass()
{
    other=NULL;
}

ExpressionDataType PinExpressionClass::GetDataType()
{
    if(other==NULL) return exNul;
    return other->GetDataType();
}

void PinExpressionClass::SetPin(Expression*exp,string label,string labelinfo)
{
   other=exp;
   Label=label;
   LabelInfo=labelinfo;
}

Expression*PinExpressionClass::Clone()
{
    PinExpressionClass*newf=new PinExpressionClass;
    newf->SetPin(other,Label,LabelInfo);
    return newf;
}

bool PinExpressionClass::CanSimplified()
{
   if(other==NULL) return false;
   Expression*simp=other->GetSimplifyExpression();
   if(simp->GetExpressionKind()<=4)
   {
       delete simp;
       return true;
   }
   delete simp;
   return false;
}
Expression*PinExpressionClass::GetSimplifyExpression()
{
   if(other==NULL) return Clone();
   Expression*simp=other->GetSimplifyExpression();
   if(simp->GetExpressionKind()<=4)
   {
       return simp;
   }
   delete simp;
   return Clone();
}

string PinExpressionClass::GetInterpreteString()
{
    return LabelInfo;
}

Expression*PinExpressionClass::GetValue()
{
    return other->GetValue();
}
void PinExpressionClass::ReturnValue(Variant&oV)
{
    other->ReturnValue(oV);
}

ConstantValue::ConstantValue()
{
   ExpressionKind=9;
   ConstantIndex=-1;
   pConstantCol=NULL;
}
ConstantValue::ConstantValue(ConstantValues*pCol,string Name)
{
   ExpressionKind=9;
   ConstantName=Name;
   pConstantCol=pCol;
   if(pCol!=NULL)
   {
       ConstantIndex=pCol->FindConstant(Name);
   }
}
ExpressionDataType ConstantValue::GetDataType()
{
    if(ConstantIndex==-1) return exNul;
    ExpressionDataType type=pConstantCol->GetConstantType(ConstantIndex);
    return type;
}

bool ConstantValue::IsExpressionValid(string&ErrorMsg)
{
    if(ConstantIndex==-1)
    {
        ErrorMsg="非法常量名"+ConstantName;
        return false;
    }
    if(pConstantCol==NULL)
    {
        ErrorMsg="非法常量名"+ConstantName+"，当前不存在常量列表";
        return false;
    }
    if(els.size()!=0)
    {
        ErrorMsg="非法常量表达式";
        return false;
    }
    return true;
}

Expression*ConstantValue::Clone()
{
    ConstantValue*newf=new ConstantValue();
    newf->ConstantIndex=ConstantIndex;
    newf->pConstantCol=pConstantCol;
    newf->ConstantName=ConstantName;
    return newf;
}

string ConstantValue::GetInterpreteString()
{
    if(ConstantIndex==-1) return "?";
    if(pConstantCol==NULL) return "?";
    string Alias=pConstantCol->GetConstantAliasName(ConstantIndex);
    return Alias;
}
Expression*ConstantValue::GetValue()
{
    if((ConstantIndex==-1)||(pConstantCol==NULL)) return NULL;
    ExpressionDataType type=pConstantCol->GetConstantType(ConstantIndex);
    switch(type)
    {
    case exInt:
        {
            IntegerClass*fV=new IntegerClass;
            Variant vt=pConstantCol->GetConstantValue(ConstantIndex);
            fV->Value=vt.lVal;
            return fV;
        }
    case exNum:
        {
            NumericClass*fV=new NumericClass;
            Variant vt=pConstantCol->GetConstantValue(ConstantIndex);
            fV->Value=vt.fltVal;
            return fV;
        }
    case exLog:
        {
            LogicClass*ls=new LogicClass;
            Variant vt=pConstantCol->GetConstantValue(ConstantIndex);
            ls->Value=vt.boolVal;
            return ls;
        }
    case exStr:
        {
            StringClass*sV=new StringClass;
            Variant vt=pConstantCol->GetConstantValue(ConstantIndex);
            sV->Value=(*vt.bstrVal);
            return sV;
        }
    case exDat:
        {
            DateClass*dV=new DateClass;
            Variant vt=pConstantCol->GetConstantValue(ConstantIndex);
            DateTime dt(vt.date);
            dV->SetValue(dt.GetYear(),dt.GetMonth(),dt.GetDay());
            return dV;
        }
    }
    return NULL;
}
void ConstantValue::ReturnValue(Variant&oV)
{
    if((ConstantIndex==-1)||(pConstantCol==NULL))
    {
        oV.vt=VRT_EMPTY;
        return;
    }
    ExpressionDataType type=pConstantCol->GetConstantType(ConstantIndex);
    switch(type)
    {
    case exInt:
        {
            Variant vt=pConstantCol->GetConstantValue(ConstantIndex);
            oV.vt=VRT_I4;
            oV.lVal=vt.lVal;
            return;
        }
    case exNum:
        {
            Variant vt=pConstantCol->GetConstantValue(ConstantIndex);
            oV.vt=VRT_R4;
            oV.fltVal=vt.fltVal;
            return;
        }
    case exLog:
        {
            Variant vt=pConstantCol->GetConstantValue(ConstantIndex);
            oV.vt=VRT_BOOL;
            oV.boolVal=vt.boolVal;
            return;
        }
    case exStr:
        {
            Variant vt=pConstantCol->GetConstantValue(ConstantIndex);
            oV=vt;
            return;
        }
    case exDat:
        {
            Variant vt=pConstantCol->GetConstantValue(ConstantIndex);
            oV.vt=VRT_DATETIME;
            oV.date=vt.date;
            return;
        }
    }
    oV.vt=VRT_EMPTY;
}


}
