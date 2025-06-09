#include "StringHelper.h"
#include<string.h>
#include <algorithm>
#include "QString"
namespace SGIS{

StringHelper::StringHelper() {

}

StringHelper::~StringHelper() {

}

string StringHelper::FromLocalString(string s){
    QString qs=QString::fromLocal8Bit(s.data());
    return qs.toStdString();
}

string StringHelper::ToLocalString(string s){
    QString qs=QString::fromStdString(s);
    string ss(qs.toLocal8Bit());
    return ss;
}

string& StringHelper::Trim(string &s)
{
    if (s.empty())
    {
        return s;
    }
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

string& StringHelper::TrimLeft(string &s)
{
    if (s.empty())
    {
        return s;
    }
    s.erase(0, s.find_first_not_of(" "));
    return s;
}

string& StringHelper::TrimRight(string &s)
{
    if (s.empty())
    {
        return s;
    }
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

bool StringHelper::EqualsNoCase(const string&s1,const string&s2){
    string ss1=s1;
    string ss2=s2;
    StringHelper::TrimToLower(ss1);
    StringHelper::TrimToLower(ss2);
    return (ss1==ss2);
}

void StringHelper::MakeLower(string &s)
{
    transform(s.begin(), s.end(), s.begin(), ::tolower);
}
void StringHelper::MakeUpper(string &s)
{
    transform(s.begin(), s.end(), s.begin(), ::toupper);
}

void StringHelper::TrimLower(string &s) {
    s = Trim(s);
    MakeLower(s);
}
void StringHelper::TrimUpper(string &s) {
    s = Trim(s);
    MakeUpper(s);
}

void StringHelper::TrimToLower(string&s) {
    if (s.empty()) return;
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    MakeLower(s);
}
void StringHelper::TrimToUpper(string&s) {
    if (s.empty()) return;
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    MakeUpper(s);
}

int StringHelper::ConvertToInt(string stringV)
{
    return atoi(stringV.c_str());
}
double StringHelper::ConvertToDouble(string stringV)
{
    return atof(stringV.c_str());
}

vector<string> StringHelper::Split(const string &s, char seperator){
    vector<string> result;
    int str_size = s.size();
    int formerPos = 0;
    for (int k = 0; k < str_size; k++) {
        if(s[k]==seperator){
            result.push_back(s.substr(formerPos, k - formerPos));
            formerPos = k+1;
        }
    }
    if (formerPos <=str_size) {
        result.push_back(s.substr(formerPos, str_size - formerPos));
    }
    return result;
}

vector<string> StringHelper::Split(const string &s, const string &seperator)
{
    vector<string> result;
    int sep_size = seperator.size();
    if (sep_size == 0) {
        result.push_back(s);
        return result;
    }
    int str_size = s.size();
    int formerPos = 0;
    for (int k = 0; k < str_size; k++) {
        bool valid = false;
        if ((s[k] == seperator[0])&&(str_size-k>= sep_size)) {
            valid = true;
            for (int j = 1; j < sep_size; j++) {
                if (s[k + j] == seperator[j]) {
                    valid = true;
                    break;
                }
            }
        }
        if (valid) {
            result.push_back(s.substr(formerPos, k - formerPos));
            k += sep_size-1;
            formerPos = k+1;
        }
    }
    if (formerPos <=str_size) {
        result.push_back(s.substr(formerPos, str_size - formerPos));
    }
    return result;
}

vector<string> StringHelper::Split(const string &s, char seperator,char contrainerSymbol){
    vector<string> result;
    int str_size = s.size();
    int formerPos = 0;
    int contCount=0;
    for (int k = 0; k < str_size; k++) {
        if(s[k]==seperator){
            if(contCount==0){
                result.push_back(s.substr(formerPos, k - formerPos));
                formerPos = k+1;
            }
        }
        else if(s[k]==contrainerSymbol){
            if(contCount==0)
                contCount=1;
            else if(contCount==1)
                contCount=0;
        }
    }
    if (formerPos <=str_size) {
        result.push_back(s.substr(formerPos, str_size - formerPos));
    }
    return result;
}
vector<string>StringHelper::LineSplit(const string &str){
    int Len=str.length();
    vector<string>subs;
    if(Len==0) return subs;
    int FromPos=0;
    for(int k=0;k<Len;k++){
        char code=str[k];
        if((code==13)||(code==10)){
            if(FromPos<k-1) subs.push_back(str.substr(FromPos,k-FromPos));
            FromPos=k+1;
        }
    }
    if(FromPos<Len) subs.push_back(str.substr(FromPos,Len-FromPos));
    return subs;
}

bool BeChineseFirstCode(unsigned char chData)
{
    return (0xB0 <= chData) && (0xF7 >= chData);	// 汉子第一个字节的范围
}

bool BeChineseSecondCode(unsigned char chData)
{
    return (0xA1 <= chData) && (0xFE >= chData);	// 汉子第二个字节的范围
}

bool BeChinese(unsigned char* pWord)
{
    return BeChineseFirstCode(pWord[0]) && BeChineseSecondCode(pWord[1]);
}

vector<string>StringHelper::VerbatimSplit(const string &strData){
    int nLen = strData.length();
    char space=' ';
    vector<string>strVector;
    unsigned char* pData = (unsigned char*)strData.c_str();
    int i = 0;
    int nBegin = 0;
    int nNum = 0;
    int nCount=1;
    bool bChinese;
    for(i = 0; i < nLen; ++i)
    {
        char code=*(pData + i);
        bChinese=false;
        if(BeChineseFirstCode(*(pData + i)))
        {
            if(BeChineseSecondCode(*(pData + i + 1)))
            {
                bChinese=true;
                ++i;
                ++nNum;
            }
        }
        else
        {
            ++nNum;
        }
        if(bChinese){
            string substr=strData.substr(nBegin, i - nBegin+1);
            strVector.push_back(substr);
            nBegin = i + 1;
            nNum = 0;
        }
        else if((code==space)||(code==10)||(code==13)){
            if(i==nBegin){
                nBegin = i + 1;
                nNum = 0;
                continue;
            }
            string substr=strData.substr(nBegin, i - nBegin);
            bool bValid=true;
            if(substr.length()==1){
                if((substr[0]==10)||(substr[0]==13)||(substr[0]==space)){
                    bValid=false;
                }
            }
            if(bValid) strVector.push_back(substr);
            nBegin = i + 1;
            nNum = 0;
        }
    }
    if(0 != nNum)
    {
        if(strData.length() - nBegin>=1){
            string substr=strData.substr(nBegin, strData.length() - nBegin);
            bool bValid=true;
            if(substr.length()==1){
                if((substr[0]==10)||(substr[0]==13)||(substr[0]==space)){
                    bValid=false;
                }
            }
            if(bValid) strVector.push_back(substr);
        }
    }
    return strVector;
}

vector<string> StringHelper::LetterSplit(const string &strData){
    int nLen = strData.length();
    vector<string>strVector;
    unsigned char* pData = (unsigned char*)strData.c_str();
    int i = 0;
    int nBegin = 0;
    int nNum = 0;
    int nCount=1;
    bool bChinese;
    for(i = 0; i < nLen; ++i)
    {
        char code=*(pData + i);
        bChinese=false;
        if(BeChineseFirstCode(*(pData + i)))
        {
            if(BeChineseSecondCode(*(pData + i + 1)))
            {
                bChinese=true;
                ++i;
                ++nNum;
            }
        }
        else
        {
            ++nNum;
        }
        if(bChinese){
            string substr=strData.substr(nBegin, i - nBegin+1);
            strVector.push_back(substr);
            nBegin = i + 1;
            nNum = 0;
        }
        else{
            string substr=strData.substr(nBegin, i - nBegin+1);
            strVector.push_back(substr);
            nBegin = i + 1;
            nNum = 0;
        }
    }
    if(0 != nNum)
    {
        if(strData.length() - nBegin>=1){
            string substr=strData.substr(nBegin, strData.length() - nBegin);
            strVector.push_back(substr);
        }
    }
    return strVector;
}
bool StringHelper::IsFirstChinease(const string&strData){
    if(strData.length()<2) return false;
    unsigned char* pData = (unsigned char*)strData.c_str();
    if(BeChineseFirstCode(*(pData)))
    {
        if(BeChineseSecondCode(*(pData+1))) return true;
    }
    return false;
}

string StringHelper::Replace(const string &s,const string &substr,const string &newstr){
    if(substr=="") return s;
    string sss=s;
    int sublen=substr.size();
    while(true){
        int npos=sss.find(substr);
        if(npos==-1) break;
        sss=sss.substr(0,npos)+newstr+sss.substr(npos+sublen,sss.size()-npos-sublen);
    }
    return sss;
}
string StringHelper::GetContent(const string& str, char left, char right, int&begin) {
    int pos = str.find(left, begin);
    if (pos < 0) return "";
    int strSize = str.size();
    int leftCount = 1;
    for (int k = pos + 1; k < strSize; k++) {
        char s = str[k];
        if (s == left) {
            leftCount++;
        }
        else if (s == right) {
            leftCount--;
            begin = k + 1;
            if (leftCount == 0) {
                return str.substr(pos + 1, k - pos - 1);
            }
        }
    }
    return "";
}

string StringHelper::GetStringContent(const string& str) {
    string pstr = str;
    pstr = Trim(pstr);
    if (pstr.size() == 0) return "";
    if (pstr.size() < 2) return pstr;
    if (pstr[0] == '"') {
        if (pstr[pstr.size() - 1] == '"')
            pstr = pstr.substr(1, pstr.size() - 2);
    }
    return pstr;
}

string StringHelper::ConvertString(double value, int decimals) {
    if(decimals==0) return to_string((int)value);
    string sFormat = "%." + to_string(decimals) + "f";
    if (decimals <= 0) sFormat = "%d";
    static char formatBuffer[1024];
    size_t i;
    i = snprintf(formatBuffer, 13, sFormat.c_str(), value);
    string svalue;
    svalue.append(formatBuffer);
    return svalue;
}

string StringHelper::ConvertStringByError(double value,double error){
    int iValue=round(value);
    double facvalue=fabs(iValue-value);
    iValue=round(facvalue);
    if(fabs(iValue-facvalue)<=error) return to_string((int)value);
    double fValue=round(facvalue*10.0)/10.0;
    if(fabs(fValue-facvalue)<=error) return ConvertString(value,1);
    fValue=round(facvalue*100.0)/100.0;
    if(fabs(fValue-facvalue)<=error) return ConvertString(value,2);
    fValue=round(facvalue*1000.0)/1000.0;
    if(fabs(fValue-facvalue)<=error) return ConvertString(value,3);
    fValue=round(facvalue*10000.0)/10000.0;
    if(fabs(fValue-facvalue)<=error) return ConvertString(value,4);
    fValue=round(facvalue*100000.0)/100000.0;
    if(fabs(fValue-facvalue)<=error) return ConvertString(value,5);
    return ConvertString(value,6);
}
unsigned char StringHelper::char_to_hex( unsigned char x )
{
    return (unsigned char)(x > 9 ? x + 55: x + 48);
}

int StringHelper::is_alpha_number_char( unsigned char c )
{
    if ( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') )
        return 1;
    return 0;
}

string StringHelper::UrlEncode(string code){
    unsigned char ch;
    int  len = 0;
    string dest;
    while (len < code.size())
    {
        ch = (unsigned char)code[len];
        if (code[len] == ' ')
        {
            code.push_back('+');
        }
        else if (is_alpha_number_char(ch) || strchr("=!~*'()", ch))
        {
            code.push_back(code[len]);
        }
        else
        {
            code.push_back('%');
            code.push_back(char_to_hex( (unsigned char)(ch >> 4) ));
            code.push_back(char_to_hex( (unsigned char)(ch % 16) ));
        }
        ++len;
    }
    return dest;
}

string StringHelper::UrlDecode(string code){
    int j,i;
    char p[2];
    j=0;
    string decd;
    for( i = 0; i < code.size(); i++ )
    {
        memset( p, '/0', 2 );
        if( code[i] != '%' )
        {
            decd.push_back(code[i]);
            continue;
        }
        i++;
        p[0] = code[i];
        i++;
        p[1] = code[i];
        p[0] = p[0] - 48 - ((p[0] >= 'A') ? 7 : 0) - ((p[0] >= 'a') ? 32 : 0);
        p[1] = p[1] - 48 - ((p[1] >= 'A') ? 7 : 0) - ((p[1] >= 'a') ? 32 : 0);
        decd.push_back((unsigned char)(p[0] * 16 + p[1]));
    }
    return decd;
}


}
