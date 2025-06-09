#include "colortable.h"
#include "Base/classfactory.h"

namespace SGIS{

REGISTER(ColorTable)

ColorTable::ColorTable()
{
    rPallete.resize(256);
    gPallete.resize(256);
    bPallete.resize(256);
    alphas.resize(256);
    ColorCount=256;
    for(int k=0;k<256;k++)
    {
        rPallete[k]=k;
        gPallete[k]=k;
        bPallete[k]=k;
        alphas[k]=255;
    }
}

ColorTable::~ColorTable()
{

}
UINT ColorTable::GetColors(){
    return ColorCount;
}
void ColorTable::SetColors(int colors){
    if(colors<=0) return;
    if(ColorCount!=colors)
    {
        rPallete.resize(colors);
        gPallete.resize(colors);
        bPallete.resize(colors);
        alphas.resize(colors);
        ColorCount=colors;
    }
    for(int k=0;k<colors;k++)
    {
        rPallete[k]=(k%256);
        gPallete[k]=(k%256);
        bPallete[k]=(k%256);
        alphas[k]=255;
    }
}
void ColorTable::SetColor(int nIndex,BYTE r,BYTE g,BYTE b){
    rPallete[nIndex]=r;
    gPallete[nIndex]=g;
    bPallete[nIndex]=b;
}

void ColorTable::SetColor(int nIndex,BYTE r,BYTE g,BYTE b,BYTE alpha){
    rPallete[nIndex]=r;
    gPallete[nIndex]=g;
    bPallete[nIndex]=b;
    alphas[nIndex]=alpha;
}

void ColorTable::GetColor(int nIndex,BYTE&r,BYTE&g,BYTE&b){
    r=rPallete[nIndex];
    g=gPallete[nIndex];
    b=bPallete[nIndex];
}

void ColorTable::GetColor(int nIndex,BYTE&r,BYTE&g,BYTE&b,BYTE&a){
    r=rPallete[nIndex];
    g=gPallete[nIndex];
    b=bPallete[nIndex];
    a=alphas[nIndex];
}

BYTE&ColorTable::Red(int nIndex){
    return rPallete[nIndex];
}
BYTE&ColorTable::Green(int nIndex){
    return gPallete[nIndex];
}
BYTE&ColorTable::Blue(int nIndex){
    return bPallete[nIndex];
}

BYTE&ColorTable::Alpha(int nIndex){
    return alphas[nIndex];
}

void ColorTable::CopyFromOther(ColorTable*other){
    SetColors(other->ColorCount);
    for(int k=0;k<other->ColorCount;k++){
        rPallete[k]=other->rPallete[k];
        gPallete[k]=other->gPallete[k];
        bPallete[k]=other->bPallete[k];
        alphas[k]=other->alphas[k];
    }
}
AutoPtr<ColorTable>ColorTable::Clone(){
    ColorTable*newc=new ColorTable();
    newc->CopyFromOther(this);
    return newc;
}

//BufferWritable:
bool ColorTable::Read(BufferReader*br){
    ColorCount=br->Read<int>();
    rPallete.resize(ColorCount);
    gPallete.resize(ColorCount);
    bPallete.resize(ColorCount);
    alphas.resize(ColorCount);
    for(int k=0;k<ColorCount;k++){
        rPallete[k]=br->Read<BYTE>();
        gPallete[k]=br->Read<BYTE>();
        bPallete[k]=br->Read<BYTE>();
        alphas[k]=br->Read<BYTE>();
    }
    return true;
}
void ColorTable::Write(BufferWriter*bw){
    bw->Write<int>(ColorCount);
    for(int k=0;k<ColorCount;k++){
        bw->Write<BYTE>(rPallete[k]);
        bw->Write<BYTE>(gPallete[k]);
        bw->Write<BYTE>(bPallete[k]);
        bw->Write<BYTE>(alphas[k]);
    }
}

string ColorTable::GetTypeName(){
    return "ColorTable";
}

//JsonWritable:
bool ColorTable::ReadFromJson(JsonReader*jr,JsonNode node){
    ColorCount=node.size();
    rPallete.resize(ColorCount);
    gPallete.resize(ColorCount);
    bPallete.resize(ColorCount);
    alphas.resize(ColorCount);
    for(int k=0;k<ColorCount;k++){
        JsonNode childNode=node[k];
        rPallete[k]=childNode["red"].asInt();
        gPallete[k]=childNode["green"].asInt();
        bPallete[k]=childNode["blue"].asInt();
        alphas[k]=childNode["alpha"].asInt();
    }
    return true;
}
string ColorTable::ToJson(){
    JsonArrayWriter writer;
    for(int k=0;k<ColorCount;k++){
       JsonObjectWriter subwriter;
       subwriter.Add<int>("red",rPallete[k]);
       subwriter.Add<int>("green",gPallete[k]);
       subwriter.Add<int>("blue",bPallete[k]);
       subwriter.Add<int>("alpha",alphas[k]);
       writer.AddWriter(&subwriter);
    }
    return writer.GetJsonString();
}


}
