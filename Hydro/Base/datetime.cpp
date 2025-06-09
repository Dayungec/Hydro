#include "datetime.h"
#include "StringHelper.h"
#include "boost/date_time.hpp"
#include <boost/date_time/local_time/local_time.hpp>
#include <chrono>
using namespace boost::gregorian;

namespace SGIS{

const boost::posix_time::ptime initialTime(date(1899,12,30));

TimeSpan::TimeSpan(double seconds) {
    this->hours = seconds/3600.0;
}

TimeSpan::TimeSpan(int days, int hours, int minutes, int seconds) {
    this->hours = days * 24.0 + hours  + minutes / 60.0 + seconds/3600.0;
}

TimeSpan::TimeSpan(int hours, int minutes, int seconds) {
    this->hours = hours  + minutes / 60.0 + seconds/3600.0;
}

double TimeSpan::TotalDays() {
    return round(hours/ 24.0*100000)/100000.0;
}

double TimeSpan::TotalHours() {
    return round(hours*10000.0)/10000.0;
}

double TimeSpan::TotalSeconds() {
    return round(hours*3600.0*100.0)/100.0;
}
TimeSpan& TimeSpan::operator =(const TimeSpan& other) {
    this->hours = other.hours;
    return *this;
}


Date::Date()
{
    boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration duration = currentTime - initialTime;
    days = duration.ticks()/86400000000.0;
}

Date::Date(int year, int month, int day) {
    if((year<1800)||(year>3000)){
        year=1800;
    }
    if((month<1)||(month>12)){
        month=1;
    }
    if((day<1)||(day>31)){
        day=1;
    }
    days=(boost::posix_time::ptime(date(year,month,day))-initialTime).ticks()/86400000000.0;
}

Date::Date(double days){
    this->days=days;
}

Date::Date(const Date&other){
   this->days=other.days;
}

Date::Date(DateTime dateTime){
    this->days=dateTime.ToDouble();
}

Date::~Date(){

}

DateTime Date::ToDateTime(){
    return DateTime(this->days);
}


Date& Date::operator =(const Date& other){
    days=other.days;
    return *this;
}

double Date::ToDouble(){
    return days;
}

void Date::FromDouble(double time){
    this->days=time;
}

string Date::ToFullString(){
    return StringHelper::Format("%04d年%02d月%02d日", GetYear(), GetMonth(), GetDay());
}

string Date::GetHttpTime(){
    boost::posix_time::ptime time=initialTime+boost::posix_time::time_duration(0,0,days*3600.0*24.0);
    boost::local_time::local_time_facet* lf(new boost::local_time::local_time_facet("%a, %d %b %Y %H:%M:%S GMT"));
    std::stringstream ss;
    ss.imbue(std::locale(ss.getloc(), lf));
    ss << time;
    return ss.str();
}

bool Date::FromFullString(string sTime) {
sTime=StringHelper::ToLocalString(sTime);
    string sYear=StringHelper::ToLocalString("年");
    string sMonth=StringHelper::ToLocalString("月");
    string sDay=StringHelper::ToLocalString("日");
    string sHour=StringHelper::ToLocalString("时");
    int yPos = sTime.find(sYear);
    if (yPos == -1) return false;
    int mPos = sTime.find(sMonth);
    if (mPos == -1) return false;;
    int dPos = sTime.find(sDay);
    if (dPos == -1) return false;
    int hPos = sTime.find(":");
    if(hPos==-1){
        hPos=sTime.find(sHour);
    }
    int minPos = -1, sPos = -1;
    if (hPos > 0) {
        minPos= sTime.find(":",hPos+1);
        if (minPos == -1) return false;
    }
    int year = atoi(sTime.substr(0, yPos).c_str());
    int month = atoi(sTime.substr(yPos+2, mPos- yPos-2).c_str());
    int day= atoi(sTime.substr(mPos + 2, dPos - mPos - 2).c_str());
    int hour = 0,minute=0, second = 0;
    if (hPos > 0) {
        hour= atoi(sTime.substr(dPos + 2, hPos - dPos - 2).c_str());
        minute = atoi(sTime.substr(hPos + 1, minPos - hPos - 1).c_str());
        second= atoi(sTime.substr(minPos + 1, sTime.length() - minPos - 1).c_str());
    }
    *this=DateTime(year, month, day, hour, minute, second);
    return true;
}

string Date::ToCompactString() {
    return StringHelper::Format("%04d%02d%02d", GetYear(), GetMonth(), GetDay());
}

bool Date::FromCompactString(string sTime) {
    if (sTime.length() < 8) return false;
    int year = atoi(sTime.substr(0, 4).c_str());
    int month = atoi(sTime.substr(4, 2).c_str());
    int day = atoi(sTime.substr(6, 2).c_str());
    *this=Date(year, month, day);
    return true;
}

string Date::ToString() {
    return StringHelper::Format("%04d-%02d-%02d", GetYear(), GetMonth(), GetDay());
}

bool Date::FromString(string sTime) {
    DateTime dt;
    if(!dt.FromString(sTime)){
        *this=dt;
        return false;
    }
    *this=dt;
    return true;
}

int Date::GetYear(){
    boost::posix_time::ptime time=initialTime+boost::posix_time::time_duration(0,0,round(days*86400.0));
    boost::gregorian::date d = time.date();
    return d.year();
}
int Date::GetMonth(){
    boost::posix_time::ptime time=initialTime+boost::posix_time::time_duration(0,0,round(days*86400.0));
    boost::gregorian::date d = time.date();
    return d.month();
}
int Date::GetDay(){
    boost::posix_time::ptime time=initialTime+boost::posix_time::time_duration(0,0,round(days*86400.0));
    boost::gregorian::date d = time.date();
    return d.day();
}

bool Date::operator > (const Date &t) {
    return days> t.days;
}

bool Date::operator < (const Date &t) {
    return days < t.days;
}

bool Date::operator >= (const Date &t) {
    return days >= t.days;
}

bool Date::operator <= (const Date &t) {
    return days <= t.days;
}

bool Date::operator == (const Date &t) {
    return days == t.days;
}

LONGLONG Date::operator - (const Date &t) {
    return days - t.days;
}

Date Date::Now() {
    return Date();
}

int Date::GetMonthDays(int year,int month) {
    switch (month) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        return 31;
    }
    if (month != 2) return 30;
    DateTime dt1(year, month, 1);
    DateTime dt2(year, month + 1, 1);
    return (dt2 - dt1).TotalDays();
}

Date Date::operator + (const LONGLONG &d) {
    Date ntime;
    ntime.days = this->days+d;
    return ntime;
}

Date Date::operator - (const LONGLONG &d) {
    Date ntime;
    ntime.days = this->days-d;
    return ntime;
}

Date & Date::operator += (const LONGLONG &d) {
    DateTime ntime;
    this->days+=d;
    return *this;
}

Date & Date::operator -= (const LONGLONG &d) {
    DateTime ntime;
    this->days-=d;
    return *this;
}


DateTime::DateTime()
{
    boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration duration = currentTime - initialTime;
    days = duration.ticks()/86400000000.0;
}

DateTime::DateTime(int year, int month, int day) {
    if((year<1800)||(year>3000)){
        year=1800;
    }
    if((month<1)||(month>12)){
        month=1;
    }
    if((day<1)||(day>31)){
        day=1;
    }
    days=(boost::posix_time::ptime(date(year,month,day))-initialTime).ticks()/86400000000.0;
    //boost::posix_time::ptime initialTime(date(year,month,day));
    //ptime initialTime(date(1800,1,1));
    //time_duration duration = currentTime - initialTime;
    //ticks = duration.ticks();
}

DateTime::DateTime(Date dat,int hour,int minute,int second){
    days=(boost::posix_time::ptime(date(dat.GetYear(),dat.GetMonth(),dat.GetDay()),boost::posix_time::time_duration(hour,minute,second))-initialTime).ticks()/86400000000.0;
}

DateTime::DateTime(int year, int month, int day,int hour,int minute,int second){
    if((year<1800)||(year>3000)){
        year=1800;
    }
    if((month<1)||(month>12)){
        month=1;
    }
    if((day<1)||(day>31)){
        day=1;
    }
    days=(boost::posix_time::ptime(date(year,month,day),boost::posix_time::time_duration(hour,minute,second))-initialTime).ticks()/86400000000.0;
}

DateTime::DateTime(double days){
    this->days=days;
}

DateTime::DateTime(const DateTime&other){
    days=other.days;
}

DateTime::DateTime(Date date){
    days=date.ToDouble();
}

DateTime::~DateTime(){

}

Date DateTime::GetDate(){
    return Date(this->days);
}
DateTime DateTime::Now() {
    return DateTime();
}

DateTime& DateTime::operator =(const DateTime& other){
    days=other.days;
    return *this;
}

double DateTime::ToDouble(){
    return days;
}

void DateTime::FromDouble(double time){
    days=time;
}

string DateTime::ToFullString(TimeStringStyle style){
    if (style == tstFull)
        return StringHelper::Format("%04d年%02d月%02d日 %02d:%02d:%02d", GetYear(), GetMonth(), GetDay(), GetHour(), GetMinute(), GetSecond());
    else if (style == tstHour)
        return StringHelper::Format("%04d年%02d月%02d日%02d时", GetYear(), GetMonth(), GetDay(), GetHour());
    else
        return StringHelper::Format("%04d年%02d月%02d日", GetYear(), GetMonth(), GetDay());
}

string DateTime::GetHttpTime(){
    boost::posix_time::ptime time=initialTime+boost::posix_time::time_duration(0,0,days*3600.0*24.0);
    boost::local_time::local_time_facet* lf(new boost::local_time::local_time_facet("%a, %d %b %Y %H:%M:%S GMT"));
    std::stringstream ss;
    ss.imbue(std::locale(ss.getloc(), lf));
    ss << time;
    return ss.str();
}

bool DateTime::FromFullString(string sTime) {
    sTime=StringHelper::ToLocalString(sTime);
    string sYear=StringHelper::ToLocalString("年");
    string sMonth=StringHelper::ToLocalString("月");
    string sDay=StringHelper::ToLocalString("日");
    string sHour=StringHelper::ToLocalString("时");
    int yPos = sTime.find(sYear);
    if (yPos == -1) return false;
    int mPos = sTime.find(sMonth);
    if (mPos == -1) return false;;
    int dPos = sTime.find(sDay);
    if (dPos == -1) return false;
    int hPos = sTime.find(":");
    if(hPos==-1){
        hPos=sTime.find(sHour);
    }
    int minPos = -1, sPos = -1;
    if (hPos > 0) {
        minPos= sTime.find(":",hPos+1);
        if (minPos == -1) return false;
    }
    int year = atoi(sTime.substr(0, yPos).c_str());
    int month = atoi(sTime.substr(yPos+2, mPos- yPos-2).c_str());
    int day= atoi(sTime.substr(mPos + 2, dPos - mPos - 2).c_str());
    int hour = 0,minute=0, second = 0;
    if (hPos > 0) {
        hour= atoi(sTime.substr(dPos + 2, hPos - dPos - 2).c_str());
        minute = atoi(sTime.substr(hPos + 1, minPos - hPos - 1).c_str());
        second= atoi(sTime.substr(minPos + 1, sTime.length() - minPos - 1).c_str());
    }
    *this=DateTime(year, month, day, hour, minute, second);
    return true;
}

string DateTime::ToCompactString(TimeStringStyle style) {
    if (style == tstFull)
        return StringHelper::Format("%04d%02d%02d%02d%02d%02d", GetYear(), GetMonth(), GetDay(), GetHour(), GetMinute(), GetSecond());
    else if (style == tstHour)
        return StringHelper::Format("%04d%02d%02d%02d", GetYear(), GetMonth(), GetDay(), GetHour());
    else
        return StringHelper::Format("%04d%02d%02d", GetYear(), GetMonth(), GetDay());
}

bool DateTime::FromCompactString(string sTime) {
    if (sTime.length() < 8) return false;
    int year = atoi(sTime.substr(0, 4).c_str());
    int month = atoi(sTime.substr(4, 2).c_str());
    int day = atoi(sTime.substr(6, 2).c_str());
    int hour = 0, minute = 0, second = 0;
    if (sTime.length() > 8) hour = atoi(sTime.substr(8,2).c_str());
    if (sTime.length() > 10) minute = atoi(sTime.substr(10, 2).c_str());
    if (sTime.length() > 12) second = atoi(sTime.substr(12, 2).c_str());
    *this=DateTime(year, month, day, hour, minute, second);
    return true;
}

string DateTime::ToString(TimeStringStyle style) {
    if (style == tstFull)
        return StringHelper::Format("%04d-%02d-%02d %02d:%02d:%02d", GetYear(), GetMonth(), GetDay(), GetHour(), GetMinute(), GetSecond());
    else if (style == tstHour)
        return StringHelper::Format("%04d-%02d-%02d %02d:00:00", GetYear(), GetMonth(), GetDay(), GetHour());
    else
        return StringHelper::Format("%04d-%02d-%02d", GetYear(), GetMonth(), GetDay());
}

bool DateTime::FromString(string sTime) {
    sTime=StringHelper::Trim(sTime);
    string dates="";
    string times="";
    int nPos=sTime.find(' ');
    if(nPos>0){
        dates=sTime.substr(0,nPos);
        times=sTime.substr(nPos+1,sTime.length()-nPos-1);
    }
    else{
        nPos=sTime.find('.');
        if(nPos>0){
            double dV=atof(sTime.c_str());
            *this=DateTime(dV);
            return true;
        }
        else{
            nPos=sTime.length();
            if(nPos==19){
                dates=sTime.substr(0,10);
                times=sTime.substr(11,8);
            }
            else if(nPos==14){
                dates=sTime.substr(0,8);
                times=sTime.substr(8,6);
            }
            else if(nPos==8){
                int dV=atoi(sTime.c_str());
                string sdV=to_string(dV);
                if(sdV==sTime){
                    int year=dV/10000;
                    int month=dV/100-year*100;
                    int day=dV%100;
                    *this=DateTime(year,month,day);
                    return true;
                }
            }
            else{
                int dV=atoi(sTime.c_str());
                string sdV=to_string(dV);
                if(sdV==sTime){
                    *this=DateTime(dV);
                    return true;
                }
                else{
                    dates=sTime;
                    times="";
                }
            }
        }
    }
    int year=0,month=0,day=0,hour=0,minute = 0, second = 0;
    vector<string>sdates=StringHelper::Split(dates,"-");
    if(sdates.size()==3){
        year=atoi(sdates[0].c_str());
        month=atoi(sdates[1].c_str());
        day=atoi(sdates[2].c_str());
    }
    else{
        vector<string>sdates=StringHelper::Split(dates,"/");
        if(sdates.size()==3){
            year=atoi(sdates[0].c_str());
            month=atoi(sdates[1].c_str());
            day=atoi(sdates[2].c_str());
        }
        else{
            int yPos = sTime.find("年");
            if (yPos>0){
                int mPos = sTime.find("月");
                if (mPos>0){
                    int dPos = sTime.find("日");
                    if (dPos>0){
                        year = atoi(sTime.substr(0, yPos).c_str());
                        month = atoi(sTime.substr(yPos+2, mPos- yPos-2).c_str());
                        day= atoi(sTime.substr(mPos + 2, dPos - mPos - 2).c_str());
                    }
                }
            }
            if((day==0)||(month==0)){
                nPos=dates.length();
                if(nPos==10){
                    year = atoi(sTime.substr(0, 4).c_str());
                    month = atoi(sTime.substr(5, 2).c_str());
                    day = atoi(sTime.substr(8, 2).c_str());
                }
                else if(nPos==8){
                    year = atoi(sTime.substr(0, 4).c_str());
                    month = atoi(sTime.substr(4, 2).c_str());
                    day = atoi(sTime.substr(6, 2).c_str());
                }
                else{
                   *this=DateTime(0);
                   return false;
                }
            }
        }
    }
    nPos=times.length();
    if(nPos==8){
        hour = atoi(times.substr(0, 2).c_str());
        minute = atoi(times.substr(3, 2).c_str());
        second = atoi(times.substr(6, 2).c_str());
    }
    else if(nPos==6){
        hour = atoi(times.substr(0, 2).c_str());
        minute = atoi(times.substr(2, 2).c_str());
        second = atoi(times.substr(4, 2).c_str());
    }
    else if(nPos>0){
        nPos=times.find(' ');
        int dif=0;
        if(nPos>0){
            string right=times.substr(nPos+1,times.length()-nPos-1);
            StringHelper::TrimLower(right);
            if(right=="pm") dif=12;
            times=times.substr(0,nPos);
        }
        vector<string>stimes=StringHelper::Split(times,":");
        hour=atoi(stimes[0].c_str());
        if(hour<12) hour+=dif;
        nPos=stimes.size();
        if(nPos>1) minute=atoi(stimes[1].c_str());
        if(nPos>2) second=atoi(stimes[2].c_str());
    }
    *this=DateTime(year, month, day, hour, minute, second);
    return true;
}

int DateTime::GetYear(){
    boost::posix_time::ptime time=initialTime+boost::posix_time::time_duration(0,0,round(days*86400.0));
    boost::gregorian::date d = time.date();
    return d.year();
}
int DateTime::GetMonth(){
    boost::posix_time::ptime time=initialTime+boost::posix_time::time_duration(0,0,round(days*86400.0));
    boost::gregorian::date d = time.date();
    return d.month();
}
int DateTime::GetDay(){
    boost::posix_time::ptime time=initialTime+boost::posix_time::time_duration(0,0,round(days*86400.0));
    boost::gregorian::date d = time.date();
    return d.day();
}
int DateTime::GetHour(){
    boost::posix_time::ptime time=initialTime+boost::posix_time::time_duration(0,0,round(days*86400.0));
    boost::posix_time::time_duration time_of_day = time.time_of_day();
    return time_of_day.hours();
}
int DateTime::GetMinute(){
    boost::posix_time::ptime time=initialTime+boost::posix_time::time_duration(0,0,round(days*86400.0));
    boost::posix_time::time_duration time_of_day = time.time_of_day();
    return time_of_day.minutes();
}
int DateTime::GetSecond(){
    boost::posix_time::ptime time=initialTime+boost::posix_time::time_duration(0,0,round(days*86400.0));
    boost::posix_time::time_duration time_of_day = time.time_of_day();
    return time_of_day.seconds();
}

bool DateTime::operator > (const DateTime &t) {
    return days> t.days;
}

bool DateTime::operator < (const DateTime &t) {
    return days < t.days;
}

bool DateTime::operator >= (const DateTime &t) {
    return days >= t.days;
}

bool DateTime::operator <= (const DateTime &t) {
    return days <= t.days;
}

bool DateTime::operator == (const DateTime &t) {
    return days == t.days;
}

TimeSpan DateTime::operator - (const DateTime &t) {
    double seconds = (days - t.days)*86400.0;
    return TimeSpan(seconds);
}

DateTime DateTime::operator + (const TimeSpan &t) {
    double seconds = ((TimeSpan)t).TotalSeconds();
    DateTime ntime;
    ntime.days = this->days+seconds/86400.0;
    return ntime;
}

DateTime DateTime::operator - (const TimeSpan &t) {
    double seconds = ((TimeSpan)t).TotalSeconds();
    DateTime ntime;
    ntime.days = this->days-seconds/86400.0;
    return ntime;
}

DateTime& DateTime::operator += (const TimeSpan &t) {
    double seconds = ((TimeSpan)t).TotalSeconds();
    DateTime ntime;
    this->days+=seconds/86400.0;
    return *this;
}

DateTime& DateTime::operator -= (const TimeSpan &t) {
    double seconds = ((TimeSpan)t).TotalSeconds();
    DateTime ntime;
    this->days-=seconds/86400.0;
    return *this;
}

DateTime DateTime::MakeHourBegin() {
    DateTime dt(GetYear(),GetMonth(),GetDay(),GetHour(),0,0);
    return dt;
}

DateTime DateTime::MakeHourEnd() {
    DateTime dt(GetYear(),GetMonth(),GetDay(),GetHour(),59,59);
    return dt;
}

DateTime DateTime::TodayBegin(){
    DateTime now = DateTime::Now();
    return DateTime(now.GetYear(), now.GetMonth(), now.GetDay(), 0, 0, 0);
}
DateTime DateTime::Yesterday() {
    DateTime now = DateTime::Now();
    return now - TimeSpan(1, 0, 0, 0);
}
DateTime DateTime::YesterdayBegin() {
    DateTime now = DateTime::TodayBegin();
    return now - TimeSpan(1, 0, 0, 0);
}
DateTime DateTime::YesterdayLast() {
    DateTime now = DateTime::Now();
    return DateTime(now.GetYear(), now.GetMonth(), now.GetDay(),23, 59, 59)-TimeSpan(1,0,0,0);
}

int DateTime::GetMonthDays(int year,int month) {
    switch (month) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        return 31;
    }
    if (month != 2) return 30;
    DateTime dt1(year, month, 1);
    DateTime dt2(year, month + 1, 1);
    return (dt2 - dt1).TotalDays();
}

string DateTime::GetTimeStamp() {
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());//获取当前时间点
    //std::time_t timestamp = tp.time_since_epoch().count(); //计算距离1970-1-1,00:00的时间长度
    std::time_t timestamp = tp.time_since_epoch().count();
    return to_string(timestamp);
}

}
