#ifndef STRINGHELPER_H
#define STRINGHELPER_H
#include "base_globle.h"


namespace SGIS{

class SGIS_EXPORT StringHelper
{
public:
    StringHelper();
    virtual~StringHelper();
    static string FromLocalString(string s);
    static string ToLocalString(string s);
    static string& Trim(string &s);
    static string& TrimLeft(string &s);
    static string& TrimRight(string &s);
    static bool EqualsNoCase(const string&s1,const string&s2);
    static void MakeLower(string &s);
    static void MakeUpper(string &s);
    static void TrimLower(string &s);
    static void TrimUpper(string &s);
    static void TrimToLower(string&s);
    static void TrimToUpper(string&s);
    static int ConvertToInt(string stringV);
    static double ConvertToDouble(string stringV);
    static vector<string> Split(const string &s, char seperator);
    static vector<string> Split(const string &s, const string &seperator);
    static vector<string> Split(const string &s, char seperator,char contrainerSymbol);
    static vector<string> LineSplit(const string &s);
    static vector<string> VerbatimSplit(const string &s);
    static vector<string> LetterSplit(const string &s);
    static bool IsFirstChinease(const string&s);
    static string Replace(const string &s,const string &substr,const string &newstr);
    static string GetContent(const string& str, char left, char right, int&begin);
    static string GetStringContent(const string& str);
    static string ConvertString(double value, int decimals);
    static string ConvertStringByError(double value,double error=0.0001);
    template<typename ... Args>
    static string Format(const string& format, Args ... args)
    {
        size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
        char*buf=new char[size];
        snprintf(buf, size, format.c_str(), args ...);
        string result=string(buf, buf + size - 1);
        delete []buf;
        return result;
    };
    static string UrlEncode(string code);
    static string UrlDecode(string code);
protected:
    static unsigned char char_to_hex( unsigned char x );
    static int is_alpha_number_char( unsigned char c );
};

}

#endif // STRINGHELPER_H
