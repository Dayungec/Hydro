#ifndef DATETIME_H
#define DATETIME_H
#include "base_globle.h"

namespace SGIS{

enum TimeStringStyle {
    tstFull=0,
    tstHour=1,
    tstDay=2
};

class SGIS_EXPORT TimeSpan {
public:
    TimeSpan(double seconds);
    TimeSpan(int days, int hours, int minutes, int seconds);
    TimeSpan(int hours, int minutes, int seconds);
    TimeSpan& operator =(const TimeSpan& other);
    double TotalDays();
    double TotalHours();
    double TotalSeconds();
protected:
    double hours;
};

class Date;
class DateTime;

class SGIS_EXPORT Date{
public:
    Date();
    Date(int year, int month, int day);
    Date(double days);
    Date(const Date&other);
    Date(DateTime dateTime);
    ~Date();
    DateTime ToDateTime();
    static Date Now();
    static int GetMonthDays(int year,int month);
    Date& operator =(const Date& other);
    double ToDouble();
    void FromDouble(double time);
    string ToFullString();
    string GetHttpTime();
    bool FromFullString(string sTime);
    string ToCompactString();
    bool FromCompactString(string sTime);
    string ToString();
    bool FromString(string sTime);
    int GetYear();
    int GetMonth();
    int GetDay();
    bool operator > (const Date &t);
    bool operator < (const Date &t);
    bool operator >= (const Date &t);
    bool operator <= (const Date &t);
    bool operator == (const Date &t);
    LONGLONG operator - (const Date &d);
    Date operator + (const LONGLONG &d);
    Date operator - (const LONGLONG &d);
    Date& operator += (const LONGLONG &d);
    Date& operator -= (const LONGLONG &d);
protected:
    LONGLONG days;
};

class SGIS_EXPORT DateTime
{
public:
    DateTime();
    DateTime(int year, int month, int day);
    DateTime(Date date,int hour,int minute=0,int second=0);
    DateTime(int year, int month, int day,int hour,int minute=0,int second=0);
    DateTime(double days);
    DateTime(const DateTime&other);
    DateTime(Date date);
    ~DateTime();
    Date GetDate();
    static DateTime Now();
    static int GetMonthDays(int year,int month);
    static DateTime TodayBegin();
    static DateTime Yesterday();
    static DateTime YesterdayBegin();
    static DateTime YesterdayLast();
    DateTime& operator =(const DateTime& other);
    double ToDouble();
    void FromDouble(double time);
    string ToFullString(TimeStringStyle style);
    string GetHttpTime();
    bool FromFullString(string sTime);
    string ToCompactString(TimeStringStyle style);
    bool FromCompactString(string sTime);
    string ToString(TimeStringStyle style);
    bool FromString(string sTime);
    int GetYear();
    int GetMonth();
    int GetDay();
    int GetHour();
    int GetMinute();
    int GetSecond();
    DateTime MakeHourBegin();
    DateTime MakeHourEnd();
    string GetTimeStamp();
    bool operator > (const DateTime &t);
    bool operator < (const DateTime &t);
    bool operator >= (const DateTime &t);
    bool operator <= (const DateTime &t);
    bool operator == (const DateTime &t);
    TimeSpan operator - (const DateTime &t);
    DateTime operator + (const TimeSpan &t);
    DateTime operator - (const TimeSpan &t);
    DateTime& operator += (const TimeSpan &t);
    DateTime& operator -= (const TimeSpan &t);
protected:
    double days;
};

}
#endif // DATETIME_H
