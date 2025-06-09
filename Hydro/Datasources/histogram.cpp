#include "histogram.h"

namespace SGIS{

REGISTER(Histogram)

Histogram::Histogram()
{
    minValue=maxValue=0;
}

Histogram::~Histogram()
{

}

AutoPtr<Histogram>Histogram::Clone(){
    Histogram*pNew=new Histogram();
    pNew->minValue=minValue;
    pNew->maxValue=maxValue;
    int Size=counts.size();
    pNew->counts.resize(Size);
    for(int k=0;k<Size;k++)
    {
        pNew->counts[k]=counts[k];
    }
    return pNew;
}

DOUBLE Histogram::GetMinValue(){
    return minValue;
}
DOUBLE Histogram::GetMaxValue(){
    return maxValue;
}

void Histogram::SetMinMaxValue(DOUBLE minvalue,DOUBLE maxvalue){
    minValue=minvalue;
    maxValue=maxvalue;
    if(minValue>maxValue)
    {
        double temp=minValue;
        minValue=maxValue;
        maxValue=temp;
    }
}

int Histogram::GetColumns(){
     return counts.size();
}

int&Histogram::ColumnValue(int nIndex){
    return counts[nIndex];
}

void Histogram::SetColumns(int columns){
    if(columns<0) columns=0;
    int Size=this->counts.size();
    this->counts.resize(columns);
    if(Size<columns)
    {
        for(int k=Size;k<columns;k++) this->counts[k]=0;
    }
}

int Histogram::GetMaxCount(){
    int maxv=0;
    for(int k=counts.size()-1;k>=0;k--)
    {
        int v=counts[k];
        if(v>maxv) maxv=v;
    }
    return maxv;
}

int Histogram::GetTotalCount(){
    int tb=0;
    for(int k=counts.size()-1;k>=0;k--)
    {
        tb+=counts[k];
    }
    return tb;
}

void Histogram::Add(int columnIndex,int num){
    counts[columnIndex]=num+counts[columnIndex];
}

vector<double>Histogram::ComputeEqualNumBreaks(int BreaksNum){
    LONG Size=counts.size();
    vector<double>breaks;
    if((Size==0)||(BreaksNum==0)) return breaks;
    breaks.resize(BreaksNum);
    DOUBLE Dif=(maxValue-minValue)/Size;
    LONG AllCount=GetTotalCount();
    if(AllCount==0)
    {
        for(int k=0;k<BreaksNum;k++)
        {
            breaks[k]=minValue+Dif*(k+1);
        }
        return breaks;
    }
    DOUBLE DifNum=(float)AllCount/(BreaksNum+1);
    LONG SubCount=0;
    LONG FormerIndex=0;
    DOUBLE FromBreak=minValue;
    for(int k=0;k<Size;k++)
    {
        SubCount+=counts[k];
        LONG CurrentIndex=(SubCount/DifNum+0.0001);
        if(CurrentIndex>BreaksNum) CurrentIndex=BreaksNum;
        DOUBLE ToBreak=minValue+Dif*(k+1);
        if(FormerIndex==CurrentIndex) continue;
        DOUBLE mDif=(ToBreak-FromBreak)/(CurrentIndex-FormerIndex);
        for(int j=FormerIndex;j<CurrentIndex;j++)
        {
            breaks[j]=FromBreak+(j-FormerIndex+1)*mDif;
        }
        FromBreak=ToBreak;
        FormerIndex=CurrentIndex;
        if(FormerIndex==BreaksNum) break;
    }
    if(FormerIndex<BreaksNum)
    {
        DOUBLE ToBreak=maxValue;
        DOUBLE mDif=(ToBreak-FromBreak)/(BreaksNum-FormerIndex);
        for(int j=FormerIndex;j<BreaksNum;j++)
        {
            breaks[j]=FromBreak+(j-FormerIndex+1)*mDif;
        }
    }
    return breaks;
}

//BufferWritable:
bool Histogram::Read(BufferReader*br){
    br->Read(minValue);
    br->Read(maxValue);
    br->ReadVector<int>(counts);
    return true;
}
void Histogram::Write(BufferWriter*bw){
    bw->Write(minValue);
    bw->Write(maxValue);
    bw->WriteVector<int>(counts);
}

string Histogram::GetTypeName(){
   return "Histogram";
}
//JsonWritable:
bool Histogram::ReadFromJson(JsonReader*jr,JsonNode node){
    minValue=node["minValue"].asDouble();
    maxValue=node["maxvalue"].asDouble();
    JsonNode aNode=node["columns"];
    counts.resize(aNode.size());
    for(int k=0;k<counts.size();k++) counts[k]=aNode[k].asInt();
    return true;
}
string Histogram::ToJson(){
    JsonObjectWriter writer;
    writer.Add<double>("minValue",minValue);
    writer.Add<double>("maxValue",maxValue);
    writer.Add<int>("columns",counts);
    return writer.GetJsonString();
}

}
