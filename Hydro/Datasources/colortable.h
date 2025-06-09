#ifndef COLORTABLE_H
#define COLORTABLE_H
#include "Base/autoptr.h"
#include "Base/jsonio.h"
#include  "Base/bufferio.h"
#include  "Base/classfactory.h"
namespace SGIS{
class SGIS_EXPORT ColorTable:
        public Object,
        public BufferWritable,
        public JsonWritable
{
public:
    ColorTable();
    virtual~ColorTable();
    UINT GetColors();
    void SetColors(int colors);
    void SetColor(int nIndex,BYTE r,BYTE g,BYTE b);
    void SetColor(int nIndex,BYTE r,BYTE g,BYTE b,BYTE alpha);
    void GetColor(int nIndex,BYTE&r,BYTE&g,BYTE&b);
    void GetColor(int nIndex,BYTE&r,BYTE&g,BYTE&b,BYTE&a);
    BYTE&Red(int nIndex);
    BYTE&Green(int nIndex);
    BYTE&Blue(int nIndex);
    BYTE&Alpha(int nIndex);
    void CopyFromOther(ColorTable*other);
    AutoPtr<ColorTable>Clone();
//BufferWritable:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
//JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
protected:
    vector<BYTE>rPallete;//调色板红通道
    vector<BYTE>gPallete;//调色板绿通道
    vector<BYTE>bPallete;//调色板蓝通道
    vector<BYTE>alphas;//调色板蓝通道
    int ColorCount;
};
}
#endif // COLORTABLE_H
