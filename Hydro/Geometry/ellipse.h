#ifndef ELLIPSE_H
#define ELLIPSE_H
#include "geometry.h"
#include "Point.h"
#include "envelope.h"
#include "Base/autoptr.h"
namespace SGIS{

class Points;

class SGIS_EXPORT Ellipse :
        public Geometry
{
public:
    Ellipse();
    Ellipse(double centerX,double centerY,double width,double height);
    Ellipse(const Ellipse&other);
    virtual~Ellipse();
    Ellipse&operator=(const Ellipse&other);
 //IGeometry:
    AutoPtr<Envelope>GetExtent();
    GeometryType GetType();
    AutoPtr<Geometry>Clone();
    bool Equals(Geometry*other);
    bool Intersects(Geometry*other);
    bool Disjoint(Geometry*other);
    bool Touches(Geometry*other);
    bool Crosses(Geometry*other);
    bool Within(Geometry*other);
    bool Contains(Geometry*other);
    bool Overlaps(Geometry*other);
    void AffineTrans(Point*AnchorPoint,DOUBLE RotateAngle,DOUBLE ZoomXRatio,DOUBLE ZoomYRatio,DOUBLE DiffX,DOUBLE DiffY);
    bool Project(CoordinateTransformation*pTrans);
    void Scale(DOUBLE xr,DOUBLE yr,DOUBLE zr=1);
    void Move(DOUBLE difx,DOUBLE dify,DOUBLE difz=0);
 //BufferWritable:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
 //JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
//Ellipse:
    AutoPtr<Points>ReturnPoints(bool highQuality=false);
    AutoPtr<Polygon>ConvertToPolygon(bool highQuality=false);
    void SetCenterPoint(Point*centerPt);
    double&CenterX();
    double&CenterY();
    double&Width();
    double&Height();
    void PutCoord(double centerX,double centerY,double width,double height);
    void GetCoord(double*centerX,double*centerY,double*width,double*height);
protected:
    double centerX;
    double centerY;
    double width;
    double height;
};
}
#endif // ELLIPSE_H
