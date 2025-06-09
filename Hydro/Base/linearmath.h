#ifndef LINEARMATH_H
#define LINEARMATH_H
#include "autoptr.h"
#include "bufferio.h"
#include "jsonio.h"

namespace SGIS{

template<typename T>
class SGIS_EXPORT LinearMath{
public:
    static double CorrelationCoefficient(const vector<T>&vx,const vector<T>&vy){
        double sxx=0, syy=0, sxy=0, sx=0, sy=0;
        int size=vx.size();
        if(vy.size()!=size) return 0;
        for (int j = 0; j < size; j++) {
            sxx += vx[j] * vx[j];
            syy += vy[j] * vy[j];
            sx += vx[j];
            sy += vy[j];
            sxy += vx[j] * vy[j];
        }
        double a1 = sxx - sx * sx / size;
        double a2 = syy - sy * sy / size;
        if((a1==0)&&(a2==0)) return 0;
        double aa1=sqrt(a1*a2);
        double bb1=sx * sy / size/aa1;
        return (sxy/aa1 - bb1);
    };
};

class SGIS_EXPORT Matrix4x4:
        public BufferWritable,
        public JsonWritable
{
public:
    Matrix4x4();
    Matrix4x4(double*rawData);
    Matrix4x4(const vector<double>&data);
    virtual~Matrix4x4();
    double*GetRawData();
    double&operator [](int nIndex);
    double&Value(int row,int col);
    void SetRow(int rowIndex,const vector<double>&values);
    void GetRow(int rowIndex,vector<double>&values);
    void SetCol(int colIndex,const vector<double>&values);
    void GetCol(int colIndex,vector<double>&values);
    void Identity();
    void CopyFromOther(Matrix4x4*other);
    AutoPtr<Matrix4x4>Inverse();
    void Transpose();
    AutoPtr<Matrix4x4>Multiply(Matrix4x4*other);
    AutoPtr<Matrix4x4>Clone();
//BufferWritable:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
//JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
protected:
    double*rawData;
};

};

#endif // LINEARMATH_H
