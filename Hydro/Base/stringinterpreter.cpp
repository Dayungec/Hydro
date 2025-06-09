#include "stringinterpreter.h"
#include "StringHelper.h"

namespace SGIS{


BaseStringInterpreter::BaseStringInterpreter()
{
    exp=NULL;
    ErrorMsg="";
    IsExpressionValid=false;
}
BaseStringInterpreter::~BaseStringInterpreter()
{
    if(exp!=NULL) delete exp;
}
int BaseStringInterpreter::FindMateBracket(int frompursue,string str)
{
    int p=0;
    int len=str.length();
    if(frompursue>len-1) return -1;
    bool times=false;
    for(int k=frompursue;k<len;k++)
    {
        if(str[k]=='\"')
        {
            if(k>0)
            {
                if(str[k-1]=='\\') continue;
            }
            times=(!times);
        }
        else if(times)
            continue;
        if(str[k]=='(') p++;
        if(str[k]==')')
        {
            if(p==0)
                return k;
            else
                p--;
        }
    }
    ErrorMsg="括号不匹配";
    return -1;
}

int BaseStringInterpreter::FindFormerMateBraket(int frompursue,string str)
{
    int p=0;
    int len=str.length();
    if(frompursue<0) return -1;
    bool times=false;
    for(int k=frompursue;k>=0;k--)
    {
        if(str[k]=='\"')
        {
            if(k>0)
            {
                if(str[k-1]=='\\') continue;
            }
            times=(!times);
        }
        else if(times)
            continue;
        if(str[k]==')') p++;
        if(str[k]=='(')
        {
            if(p==0)
                return k;
            else
                p--;
        }
    }
    ErrorMsg="括号不匹配";
    return -1;
}
int BaseStringInterpreter::FindMateSymbol(int frompursue,string str,char sym)
{
    if(sym=='(') return FindMateBracket(frompursue,str);
    if(sym=='"')
    {
        int len=str.length();
        if(frompursue>len-1) return -1;
        for(int k=frompursue;k<len;k++)
        {
            if(str[k]==sym)
            {
                if(k>0)
                {
                    if(str[k-1]=='\\') continue;
                }
                return k;
            }
        }
        ErrorMsg=sym+"不匹配";
    }
    else
    {
        int len=str.length();
        if(frompursue>len-1) return -1;
        bool times=false;
        for(int k=frompursue;k<len;k++)
        {
            if(str[k]=='\"')
            {
                if(k>0)
                {
                    if(str[k-1]=='\\') continue;
                }
                times=(!times);
            }
            else if(times)
                continue;
            if((str[k]==sym)&&(!times)) return k;
        }
        ErrorMsg=sym+"不匹配";
    }
    return -1;
}

ExpressionDataType BaseStringInterpreter::GetExpressionDataType()
{
   if(exp==NULL) return exNul;
   return exp->GetDataType();
}

bool BaseStringInterpreter::GetSubString(string&str1,int fromposi,int toposi,string&str2)
{
   if(fromposi-toposi==1)
   {
       str2="";
       return true;
   }
   int len=str1.length();
   if((fromposi<0)||(fromposi>=len)||(toposi<0)||(toposi>=len)) return false;
   if(fromposi>toposi)
   {
       int temp=fromposi;
       fromposi=toposi;
       toposi=temp;
   }
   str2=str1.substr(str1.length()-len+fromposi,len-fromposi);
   toposi=toposi-fromposi;
   str2=str2.substr(0,toposi+1);
   return true;
}

int BaseStringInterpreter::GetExpressionKindByString(string&str)
{
    str=StringHelper::Trim(str);
    if(str.empty())
    {
        ErrorMsg="非法表达式";
        return -1;
    }
    StringClass V;V.Value=str;
    bool IsInt=V.IsInterger();
    bool IsNum=V.IsNumeric();
    if(IsInt) return 0;
    if(IsNum) return 1;
    string temp=str;
    if((temp=="t")||(temp=="f")||(temp=="true")||(temp=="false")) return 2;
    if(str.length()<1) return -1;
    if(str.length()>=2)
    {
        if((str[0]=='"')&&(str[str.length()-1]=='"'))
        {
            str.erase(begin(str)+str.length()-1);
            str.erase(begin(str));
            return 3;
        }
        if((str[0]=='#')&&(str[str.length()-1]=='#'))
        {
            str.erase(begin(str)+str.length()-1);
            str.erase(begin(str));
            DateClass dV;
            if(!dV.ValidDateString(str))
            {
               ErrorMsg="非法日期"+V.Value;
               return -1;
            }
            return 4;
        }
    }
    int kind=QueryOtherExpressionKindByString(str);
    if((kind==-1)&&(ErrorMsg==""))
        ErrorMsg="不能识别的字符:"+str+",如果是字符串请加上双引号,如果是日期请再两侧加上#";
    return kind;
}

bool BaseStringInterpreter::AddOperation(string str,ExpressionClass*newops)
{
    int kind=GetExpressionKindByString(str);
    if(kind==-1)
    {
        return false;
    }
    if(kind==0)
    {
        IntegerClass*intV=new IntegerClass;
        intV->Value=atoi(str.c_str());
        newops->AddElement(intV);
        return true;
    }
    else if(kind==1)
    {
        NumericClass*fV=new NumericClass;
        fV->Value=atof(str.c_str());
        newops->AddElement(fV);
        return true;
    }
    else if(kind==2)
    {
        LogicClass*lV=new LogicClass;
        str=StringHelper::Trim(str);

        if((str=="t")||(str=="true"))
          lV->Value=true;
        else
          lV->Value=false;
        newops->AddElement(lV);
        return true;
    }
    else if(kind==3)
    {
        StringClass*sV=new StringClass;
        sV->Value=str;
        sV->InterpreteString();
        newops->AddElement(sV);
        return true;
    }
    else if(kind==4)
    {
        DateClass*sV=new DateClass;
        sV->SetValue(str);
        newops->AddElement(sV);
        return true;
    }
    else if(kind!=-1)
    {
        if(AddOtherOperation(kind,str,newops)) return true;
    }
    ErrorMsg="不能识别的字符"+str;
    return false;
}

FunctionClass*BaseStringInterpreter::GetFunctionKind(string str,int&kind)
{
   string func=str;
   StringHelper::MakeLower(str);
   if(str=="abs")
   {
       kind=1;
       AbsFunction*newop=new AbsFunction;
       return newop;
   }
   if(str=="sqr")
   {
       kind=2;
       SqrFunction*newop=new SqrFunction;
       return newop;
   }
   if(str=="round")
   {
       kind=3;
       RoundFunction*newop=new RoundFunction;
       return newop;
   }
   if(str=="exp")
   {
       kind=4;
       ExpFunction*newop=new ExpFunction;
       return newop;
   }
   if(str=="lg")
   {
       kind=5;
       LogFunction*newop=new LogFunction;
       return newop;
   }
   if(str=="ln")
   {
       kind=6;
       LnFunction*newop=new LnFunction;
       return newop;
   }
   if(str=="sin")
   {
       kind=7;
       SinFunction*newop=new SinFunction;
       return newop;
   }
   if(str=="cos")
   {
       kind=8;
       CosFunction*newop=new CosFunction;
       return newop;
   }
   if(str=="tan")
   {
       kind=9;
       TanFunction*newop=new TanFunction;
       return newop;
   }
   if(str=="asin")
   {
       kind=10;
       AsinFunction*newop=new AsinFunction;
       return newop;
   }
   if(str=="acos")
   {
       kind=11;
       AcosFunction*newop=new AcosFunction;
       return newop;
   }
   if(str=="atan")
   {
       kind=12;
       AtanFunction*newop=new AtanFunction;
       return newop;
   }
   if(str=="trunc")
   {
       kind=13;
       TruncFunction*newop=new TruncFunction;
       return newop;
   }
   if(str=="min")
   {
       kind=14;
       MinFunction*newop=new MinFunction;
       return newop;
   }
   if(str=="max")
   {
       kind=15;
       MaxFunction*newop=new MaxFunction;
       return newop;
   }
   if(str=="mean")
   {
       kind=16;
       MeanFunction*newop=new MeanFunction;
       return newop;
   }
   if(str=="sum")
   {
       kind=17;
       SumFunction*newop=new SumFunction;
       return newop;
   }
   if(str=="fmod")
   {
       kind=18;
       FmodFunction*newop=new FmodFunction;
       return newop;
   }
   if(str=="pow")
   {
       kind=19;
       PowFunction*newop=new PowFunction;
       return newop;
   }
   if(str=="cstr")
   {
       kind=20;
       CStrFunction*newop=new CStrFunction;
       return newop;
   }
   if(str=="length")
   {
       kind=21;
       LengthFunction*newop=new LengthFunction;
       return newop;
   }
   if(str=="clng")
   {
       kind=22;
       CLngFunction*newop=new CLngFunction;
       return newop;
   }
   if(str=="cdbl")
   {
       kind=23;
       CDblFunction*newop=new CDblFunction;
       return newop;
   }
   if(str=="lcase")
   {
       kind=24;
       LCaseFunction*newop=new LCaseFunction;
       return newop;
   }
   if(str=="ucase")
   {
       kind=25;
       UCaseFunction*newop=new UCaseFunction;
       return newop;
   }
   if(str=="ltrim")
   {
       kind=26;
       LTrimFunction*newop=new LTrimFunction;
       return newop;
   }
   if(str=="rtrim")
   {
       kind=27;
       RTrimFunction*newop=new RTrimFunction;
       return newop;
   }
   if(str=="space")
   {
       kind=28;
       SpaceFunction*newop=new SpaceFunction;
       return newop;
   }
   if(str=="left")
   {
       kind=29;
       LeftFunction*newop=new LeftFunction;
       return newop;
   }
   if(str=="right")
   {
       kind=30;
       RightFunction*newop=new RightFunction;
       return newop;
   }
   if(str=="instr")
   {
       kind=31;
       InStrFunction*newop=new InStrFunction;
       return newop;
   }
   if(str=="instrrev")
   {
       kind=32;
       InStrRevFunction*newop=new InStrRevFunction;
       return newop;
   }
   if(str=="mid")
   {
       kind=33;
       MidFunction*newop=new MidFunction;
       return newop;
   }
   if(str=="now")
   {
       kind=34;
       NowFunction*newop=new NowFunction;
       return newop;
   }
   if(str=="cdate")
   {
       kind=35;
       CDateFunction*newop=new CDateFunction;
       return newop;
   }
   if(str=="day")
   {
       kind=36;
       DayFunction*newop=new DayFunction;
       return newop;
   }
   if(str=="month")
   {
       kind=37;
       MonthFunction*newop=new MonthFunction;
       return newop;
   }
   if(str=="year")
   {
       kind=38;
       YearFunction*newop=new YearFunction;
       return newop;
   }
   if(str=="datediff")
   {
       kind=39;
       DateDiffFunction*newop=new DateDiffFunction;
       return newop;
   }
   if(str=="dateadd")
   {
       kind=40;
       DateAddFunction*newop=new DateAddFunction;
       return newop;
   }
   if(str=="iif")
   {
       kind=41;
       IIFFunction*newop=new IIFFunction;
       return newop;
   }
   if(str=="lfill")
   {
       kind=42;
       LFillFunction*newop=new LFillFunction;
       return newop;
   }
   if(str=="rfill")
   {
       kind=43;
       RFillFunction*newop=new RFillFunction;
       return newop;
   }
   FunctionClass*f=QueryOtherFunctionKind(str,kind);
   if(f==NULL)
   {
       if(ErrorMsg=="") ErrorMsg="不能识别的函数:"+func;
       return NULL;
   }
   return f;
}

bool BaseStringInterpreter::InterpreteFunctionContent(string str,FunctionClass*newops)
{
    int len=str.length();
    char ca;
    int from;
    from=0;
    string sub;
    for(int k=0;k<len;k++)
    {
       ca=str[k];
       if(ca=='(')
       {
           int temp=FindMateBracket(k+1,str);
           if(temp==-1)
           {
               return false;
           }
           k=temp;
       }
       else if(ca==',')
       {
           if(GetSubString(str,from,k-1,sub)==false)
           {
               ErrorMsg="函数非法!";
               return false;
           }
           ExpressionClass*newop=new ExpressionClass;
           if(GetExpressionByString(sub,newop)==false)
           {
               ErrorMsg="非法的函数参数,请书写正确参数或确保参数之间用逗号分隔!";
               delete newop;
               return false;
           }
           newops->AddElement(newop);
           from=k+1;
       }
       if(k==len-1)
       {
           if(GetSubString(str,from,k,sub)==false)
           {
               ErrorMsg="函数非法!";
               return false;
           }
           ExpressionClass*newop=new ExpressionClass;
           if(GetExpressionByString(sub,newop)==false)
           {
               delete newop;
               return false;
           }
           newops->AddElement(newop);
       }
    }
    if(newops->IsExpressionValid(ErrorMsg)) return true;
    return false;
}

FunctionClass*BaseStringInterpreter::GetFunction(string str,int leftbracketposi)
{
    string func,content;
    if(GetSubString(str,0,leftbracketposi,func)==false) return NULL;
    if(GetSubString(str,leftbracketposi+2,str.length()-2,content)==false) return NULL;
    int kind=0;
    FunctionClass*newops=GetFunctionKind(func,kind);
    if(newops==NULL) return NULL;
    bool IsValid;
    IsValid=InterpreteFunctionContent(content,newops);
    if(IsValid)
    {
        return newops;
    }
    if(ErrorMsg.length()>2)
    {
        string Info;
        GetSubString(ErrorMsg,0,4,Info);
        if(Info=="函数")
        {
            ErrorMsg=func+ErrorMsg;
        }
    }
    delete newops;
    return NULL;
}

bool BaseStringInterpreter::GetExpressionByString(string str,ExpressionClass*newops)
{
    int from=0;
    int len=str.length();
    if(len==0) return false;
    for(int k=0;k<len;k++)
    {
        char ca=str[k];
        if(ca==' ')
        {
            if(from==k) from=k+1;
            continue;
        }
        else if((ca=='+')||(ca=='-'))
        {
            string numr;
            if(GetSubString(str,from,k-1,numr)==false) return false;
            numr=StringHelper::Trim(numr);
            if((from==k)&&(k!=0))
            {
               ErrorMsg="非法的+或-!";
               return false;
            }
            if(numr!=")")
            {
                if(numr=="")
                {
                    IntegerClass*V=new IntegerClass;
                    V->Value=0;
                    newops->AddElement(V);
                }
                else
                {
                   if(AddOperation(numr,newops)==false) return false;
                }
            }
            if(ca=='+')
                newops->AddOps(opAdd);
            else
                newops->AddOps(opMinus);
            from=k+1;
        }
        else if((ca=='*')||(ca=='/')||(ca=='%'))
        {
            if(k-from==0)
            {
                ErrorMsg="非法运算符*、/、%";
                return false;
            }
            string numr;
            if(GetSubString(str,from,k-1,numr)==false) return false;
            numr=StringHelper::Trim(numr);
            if(numr!=")")
            {
                if(AddOperation(numr,newops)==false) return false;
            }
            if(ca=='*')
                newops->AddOps(opMultiply);
            else if(ca=='/')
                newops->AddOps(opDivide);
            else
                newops->AddOps(opMod);
            from=k+1;
        }
        else if(ca=='&')
        {
            if((k-from>=2)&&(k<str.length()-1)&&((str[k+1]==' ')||(str[k+1]=='(')))
            {
              string numr;
              if(GetSubString(str,from,k,numr)==false) return false;
              numr=StringHelper::Trim(numr);
              string right=numr.substr(numr.length()-2,2);
              if(right=="&&")
              {
                numr=numr.substr(0,numr.length()-2);
                numr=StringHelper::Trim(numr);
                if(numr!=")")
                {
                  if(AddOperation(numr,newops)==false) return false;
                }
                newops->AddOps(opAnd);
                from=k+1;
              }
            }
        }
        else if(ca=='|')
        {
            if((k-from>=2)&&(k<str.length()-1)&&((str[k+1]==' ')||(str[k+1]=='(')))
            {
              string numr;
              if(GetSubString(str,from,k,numr)==false) return false;

              numr=StringHelper::Trim(numr);
              string right=numr.substr(numr.length()-2,2);
              if(right=="||")
              {
                numr=numr.substr(0,numr.length()-2);
                numr=StringHelper::Trim(numr);
                if(numr!=")")
                {
                  if(AddOperation(numr,newops)==false) return false;
                }
                newops->AddOps(opOr);
                from=k+1;
              }
            }
        }
        else if(ca=='>')
        {
            bool CanEqual=false;
            if(k>=str.length()-1)
            {
                ErrorMsg="非法字符>";
                return false;
            }
            if(str[k+1]=='=')
            {
               CanEqual=true;
            }
            string numr;
            if(GetSubString(str,from,k-1,numr)==false) return false;
            numr=StringHelper::Trim(numr);
            if(numr!=")")
            {
                if(AddOperation(numr,newops)==false) return false;
            }
            if(CanEqual)
                newops->AddOps(opBigOrEqual);
            else
                newops->AddOps(opBig);
            if(CanEqual) k++;
            from=k+1;
        }
        else if(ca=='<')
        {
            bool CanEqual=false;
            if(k>=str.length()-1)
            {
                ErrorMsg="非法字符<";
                return false;
            }
            if(str[k+1]=='=')
            {
               CanEqual=true;
            }
            string numr;
            if(GetSubString(str,from,k-1,numr)==false) return false;
            numr=StringHelper::Trim(numr);
            if(numr!=")")
            {
                if(AddOperation(numr,newops)==false) return false;
            }
            if(CanEqual)
                newops->AddOps(opSmallOrEqual);
            else
                newops->AddOps(opSmall);
            if(CanEqual) k++;
            from=k+1;
        }
        else if(ca=='=')
        {
            bool CanEqual=true;
            if(k>=str.length()-1)
            {
                ErrorMsg="非法字符=";
                return false;
            }
            if(k==from)
            {
                ErrorMsg="非法字符=";
                return false;
            }
            int pp=2;
            if(str[k-1]=='!')
            {
               CanEqual=false;
            }
            else if(str[k+1]=='=')
            {
               CanEqual=true;
               pp=1;
            }
            else
            {
                ErrorMsg="非法字符=";
                return false;
            }
            string numr;
            if(GetSubString(str,from,k-pp,numr)==false) return false;
            numr=StringHelper::Trim(numr);
            if(numr!=")")
            {
                if(AddOperation(numr,newops)==false) return false;
            }
            if(CanEqual)
                newops->AddOps(opEqual);
            else
                newops->AddOps(opNotEqual);
            from=k+(3-pp);
            if(pp==1) k++;
        }
        else if(ca=='(')
        {
            int mateposi=FindMateBracket(k+1,str);
            if(mateposi==-1)
            {
                ErrorMsg="括号不匹配";
                return false;
            }
            if(k-from>0)
            {
                string func;
                if(GetSubString(str,from,mateposi,func)==false) return false;
                FunctionClass*newoper=GetFunction(func,k-from-1);
                if(newoper==NULL)
                {
                    return false;
                }
                newops->AddElement(newoper);
                k=from=mateposi;
            }
            else if(k-from==0)
            {
                string sub;
                if(GetSubString(str,from+1,mateposi-1,sub)==false) return false;
                ExpressionClass*newoper=new ExpressionClass;
                if(GetExpressionByString(sub,newoper)==false)
                {
                    delete newoper;
                    return false;
                }
                newops->AddElement(newoper);
                k=from=mateposi;
            }
        }
        else if(ca=='"')
        {
            int mateposi=FindMateSymbol(k+1,str,'"');
            if(mateposi==-1)
            {
                ErrorMsg="引号不匹配";
                return false;
            }
            k=mateposi;
        }
        else if(ca=='#')
        {
            int mateposi=FindMateSymbol(k+1,str,'#');
            if(mateposi==-1)
            {
                 ErrorMsg="#号不匹配";
                return false;
            }
            k=mateposi;
        }
        else if(ca=='!')
        {
            if(k<str.length()-1)
            {
                if((str[k+1]==' ')||(str[k+1]=='('))
                {
                  string substr;
                  if(GetSubString(str,from,k,substr)==false) return false;
                  substr=StringHelper::Trim(substr);
                  if(substr=="!")
                  {
                    newops->AddOps(opNot);
                    from=k+1;
                  }
                }
            }
        }
        else if((ca=='l')||(ca=='L'))
        {
            if((k<str.length()-5)&&(k>=2))
            {
                string subString=str.substr(k-1,6);
                StringHelper::MakeLower(subString);
                if(subString==" like ")
                {
                    string numr;
                    if(GetSubString(str,from,k-2,numr)==false) return false;
                    numr=StringHelper::Trim(numr);
                    if(numr!=")")
                    {
                        if(AddOperation(numr,newops)==false) return false;
                    }
                    newops->AddOps(opLike);
                    k+=4;
                    from=k;
                }
            }
        }
        if(QueryOtherSymbolInString(k,str)==false)
        {
            return false;
        }
        if(k==len-1)
        {
            string numr;
            if(GetSubString(str,from,k,numr)==false) return false;
            numr=StringHelper::Trim(numr);
            if(numr!=")")
               if(AddOperation(numr,newops)==false) return false;
        }
    }
    if(ResetOperation(newops)==false) return false;
    return true;
}

bool BaseStringInterpreter::ResetOperation(ExpressionClass*newops)
{
   return newops->OrderExpression(ErrorMsg);
}


bool BaseStringInterpreter::InputExpressionString(string Exp)
{
    ExpressionString=Exp;
    ErrorMsg="";
    Exp=StringHelper::Trim(Exp);
    if(Exp=="")
    {
        ErrorMsg="表达式为空!";
        IsExpressionValid=false;
        return false;
    }
    if(exp!=NULL)
    {
        delete exp;
        exp=NULL;
    }
    ExpressionClass*newexp=new ExpressionClass;
    if(GetExpressionByString(Exp,newexp)==false)
    {
        if(ErrorMsg=="") ErrorMsg="非法表达式";
        delete newexp;
        IsExpressionValid=false;
        return false;
    }
    exp=newexp;
    IsExpressionValid=true;
    return true;
}


bool BaseStringInterpreter::InputBoolExpressionString(string Exp)
{
    ExpressionString=Exp;
    ErrorMsg="";
    Exp=StringHelper::Trim(Exp);
    if(Exp=="")
    {
        ErrorMsg="表达式为空!";
        IsExpressionValid=false;
        return false;
    }
    if(exp!=NULL)
    {
        delete exp;
        exp=NULL;
    }
    ExpressionClass*newexp=new ExpressionClass;
    if(GetExpressionByString(Exp,newexp)==false)
    {

        if(ErrorMsg=="") ErrorMsg="非法表达式";
        delete newexp;
        IsExpressionValid=false;
        return false;
    }
    if(newexp->GetDataType()!=exLog)
    {
       if(ErrorMsg=="") ErrorMsg="需要逻辑表达式";
       delete newexp;
       IsExpressionValid=false;
       return false;
    }
    exp=newexp;
    IsExpressionValid=true;
    return true;
}

void BaseStringInterpreter::SimplifyExpresion()
{
    if(exp==NULL) return;
    Expression*newexp=exp->GetSimplifyExpression();
    if(newexp!=NULL)
    {
        delete exp;
        exp=newexp;
    }
}

string BaseStringInterpreter::GetInterpreterString()
{
    if(exp==NULL) return "?";
    return exp->GetInterpreteString();
}

AutoPtr<Expression>BaseStringInterpreter::GetValue()
{
    return AutoPtr<Expression>(exp->GetValue());
}
void BaseStringInterpreter::ReturnValue(Variant&oV)
{
    if(exp==NULL)
        oV.vt=VRT_EMPTY;
    else
        exp->ReturnValue(oV);
}

StringInterpreter::StringInterpreter(AutoPtr<VirtualTable>Table,AutoPtr<ConstantValues>ConstantCol)
{
    pVirtualTable=Table;
    pConstantCol=ConstantCol;
}

StringInterpreter::~StringInterpreter(){

}

AutoPtr<VirtualTable>StringInterpreter::GetVirtualTable(){
    return pVirtualTable;
}

AutoPtr<ConstantValues>StringInterpreter::GetConstantCol(){
    return pConstantCol;
}
void StringInterpreter::SetVirtualTable(AutoPtr<VirtualTable>Table)
{
    pVirtualTable=Table;
}
void StringInterpreter::SetConstantCol(AutoPtr<ConstantValues>pCol)
{
    pConstantCol=pCol;
}
bool StringInterpreter::InputExpressionString(string Exp)
{
    //if(pVirtualTable!=NULL) pVirtualTable->LabelAllFieldsUnused();
    if(BaseStringInterpreter::InputExpressionString(Exp))
    {
        SimplifyExpresion();
        return true;
    }
    return false;
}
bool StringInterpreter::InputExpressionStringNoMsg(string Exp)
{
    //if(pVirtualTable!=NULL) pVirtualTable->LabelAllFieldsUnused();
    if(BaseStringInterpreter::InputExpressionString(Exp))
    {
        SimplifyExpresion();
        return true;
    }
    return false;
}

bool StringInterpreter::InputBoolExpressionString(string Exp)
{
    //if(pVirtualTable!=NULL) pVirtualTable->LabelAllFieldsUnused();
    if(BaseStringInterpreter::InputBoolExpressionString(Exp))
    {
        SimplifyExpresion();
        return true;
    }
    return false;
}
bool StringInterpreter::InputBoolExpressionStringNoMsg(string Exp)
{
    //if(pVirtualTable!=NULL) pVirtualTable->LabelAllFieldsUnused();
    if(BaseStringInterpreter::InputBoolExpressionString(Exp))
    {
        SimplifyExpresion();
        return true;
    }
    return false;
}

FunctionClass*StringInterpreter::QueryOtherFunctionKind(string str,int&kind)
{
    return NULL;
}

int StringInterpreter::QueryOtherExpressionKindByString(string&str)
{
    if(str.length()>=2)
    {
        if((str[0]=='[')&&(str[str.length()-1]==']'))
        {
            if(pVirtualTable==nullptr)
            {
                ErrorMsg="非法字段"+str+"，当前无计算表";
                return -1;
            }
            str.erase(begin(str)+str.length()-1);
            str.erase(begin(str));
            str=StringHelper::Trim(str);
            string t=str;
            LONG FieldIndex=pVirtualTable->FindField(t);
            if(FieldIndex==-1)
            {
                string TipName=pVirtualTable->GetTipName();
                ErrorMsg="非法"+TipName+"["+str+"]";
                return -1;
            }
            else
               pVirtualTable->LabelFieldUsing(FieldIndex);
            return 5;
        }
    }
    if(pConstantCol!=nullptr)
    {
        LONG nIndex=pConstantCol->FindConstant(str);
        if(nIndex>=0)
        {
            return 6;
        }
    }
    if(pVirtualTable!=NULL)
    {
        string TipName=pVirtualTable->GetTipName();
        ErrorMsg="不能识别的字符:"+str+",字符串、日期、"+TipName+"请分别用\"\"、##、[]括起来";
    }
    else
        ErrorMsg="不能识别的字符:"+str+",字符串、日期请分别用\"\"、##括起来";
    return -1;
}
bool StringInterpreter::AddOtherOperation(int kind,string str,ExpressionClass*newops)
{
   if(kind==5)
   {
        VirtualField*fV=new VirtualField(pVirtualTable.get(),str);
        newops->AddElement(fV);
        return true;
   }
   else if(kind==6)
   {
       ConstantValue*fV=new ConstantValue(pConstantCol.get(),str);
       newops->AddElement(fV);
       return true;
   }
   return false;
}
bool StringInterpreter::QueryOtherSymbolInString(int&NowPin,string str)
{
   char ca;
   ca=str[NowPin];
   if(ca=='[')
   {
        int mateposi=FindMateSymbol(NowPin+1,str,']');
        if(mateposi==-1)
        {
            return false;
        }
        NowPin=mateposi;
    }
    return true;
}
AutoPtr<Expression>StringInterpreter::GetValue()
{
   return AutoPtr<Expression>(exp->GetValue());
}
void StringInterpreter::ReturnValue(Variant&oV)
{
    exp->ReturnValue(oV);
}

AutoPtr<VirtualGrids>GridInterpreter::GetGrids(){
    return pGrids;
}
AutoPtr<ConstantValues>GridInterpreter::GetConstantCol(){
    return pConstantCol;
}

void GridInterpreter::SetGrids(AutoPtr<VirtualGrids>Grids)
{
    pGrids=Grids;
}
void GridInterpreter::SetConstantCol(AutoPtr<ConstantValues>pCol)
{
    pConstantCol=pCol;
}

GridInterpreter::GridInterpreter(AutoPtr<VirtualGrids>Grids,AutoPtr<ConstantValues>ConstantCol)
{
    pGrids=Grids;
    pConstantCol=ConstantCol;
}

GridInterpreter::~GridInterpreter(){

}
bool GridInterpreter::InputExpressionString(string Exp)
{
    if(pGrids!=NULL) pGrids->LabelAllGridsUnused();
    if(BaseStringInterpreter::InputExpressionString(Exp))
    {
        SimplifyExpresion();
        return true;
    }
    return false;
}

bool GridInterpreter::InputBoolExpressionString(string Exp)
{
    if(pGrids!=NULL) pGrids->LabelAllGridsUnused();
    if(BaseStringInterpreter::InputBoolExpressionString(Exp))
    {
        SimplifyExpresion();
        return true;
    }
    return false;
}

FunctionClass*GridInterpreter::QueryOtherFunctionKind(string str,int&kind)
{
    return NULL;
}

int GridInterpreter::QueryOtherExpressionKindByString(string&str)
{
    if(str.length()>=2)
    {
        if((str[0]=='[')&&(str[str.length()-1]==']'))
        {
            if(pGrids==NULL)
            {
                ErrorMsg="非法字段["+str+"]，当前无栅格集";
                return -1;
            }
            str.erase(begin(str)+str.length()-1);
            str.erase(begin(str));
            str=StringHelper::Trim(str);
            string t=str;
            LONG GridIndex=pGrids->FindGrid(t);
            if(GridIndex==-1)
            {
                string name=pGrids->GetTipName();
                ErrorMsg="非法"+name+"["+str+"]";
                return -1;
            }
            else
               pGrids->LabelGridUsing(GridIndex);
            return 5;
        }
    }
    if(pConstantCol!=NULL)
    {
        LONG nIndex=pConstantCol->FindConstant(str);
        if(nIndex>=0)
        {
            return 6;
        }
    }
    if(pGrids!=NULL)
    {
        string name=pGrids->GetTipName();
        ErrorMsg="不能识别的字符:"+str+",字符串、日期、"+name+"请分别用\"\"、##、[]括起来";
    }
    else
        ErrorMsg="不能识别的字符:"+str+",字符串、日期请分别用\"\"、##括起来";
    return -1;
}
bool GridInterpreter::AddOtherOperation(int kind,string str,ExpressionClass*newops)
{
   if(kind==5)
   {
        Grid*fV=new Grid(pGrids.get(),str);
        newops->AddElement(fV);
        return true;
   }
   else if(kind==6)
   {
       ConstantValue*fV=new ConstantValue(pConstantCol.get(),str);
       newops->AddElement(fV);
       return true;
   }
   return false;
}
bool GridInterpreter::QueryOtherSymbolInString(int&NowPin,string str)
{
   char ca;
   ca=str[NowPin];
   if(ca=='[')
   {
        int mateposi=FindMateSymbol(NowPin+1,str,']');
        if(mateposi==-1)
        {
            return false;
        }
        NowPin=mateposi;
    }
    return true;
}
AutoPtr<Expression>GridInterpreter::GetValue()
{
   return AutoPtr<Expression>(exp->GetValue());
}
void GridInterpreter::ReturnValue(Variant&oV)
{
    exp->ReturnValue(oV);
}

}
