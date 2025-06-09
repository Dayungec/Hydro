#ifndef HISTOGRAM_H
#define HISTOGRAM_H
#include "Base/autoptr.h"
#include "Base/classfactory.h"
#include "Base/jsonio.h"
#include  "Base/bufferio.h"

namespace SGIS{

class SGIS_EXPORT Histogram:
        public Object,
        public BufferWritable,
        public JsonWritable
{
public:
    Histogram();
    virtual~Histogram();
    AutoPtr<Histogram>Clone();
    DOUBLE GetMinValue();
    DOUBLE GetMaxValue();
    void SetMinMaxValue(DOUBLE minvalue,DOUBLE maxvalue);
    int GetColumns();
    int&ColumnValue(int nIndex);
    void SetColumns(int columns);
    int GetMaxCount();
    int GetTotalCount();
    void Add(int ColumnIndex,int count);
    vector<double>ComputeEqualNumBreaks(int BreaksNum);
 //BufferWritable:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
 //JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
protected:
    double minValue;
    double maxValue;
    vector<int>counts;
};

}
#endif // HISTOGRAM_H
