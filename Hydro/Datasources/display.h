#ifndef DISPLAY_H
#define DISPLAY_H
#include "Base/autoptr.h"
#include "Base/classfactory.h"
#include "Geometry/geometry2d.h"
#include "Geometry/spatialreference.h"

namespace SGIS{
class SGIS_EXPORT Display:
        public Object,
        public BufferWritable,
        public JsonWritable
{
public:
    Display();
    virtual~Display();
    static int GetDefaultDPI();
    AutoPtr<SpatialReference>GetSpatialReference();
    void SetSpatialReference(AutoPtr<SpatialReference>psp);
    double ToMapXDistance(double dist);
    double ToMapYDistance(double dist);
    void ToMapPoint(DOUBLE*X,DOUBLE*Y);
    void ToMapPoints(vector<double>&XYs);
    void ToMapPoints(DOUBLE*XYs,int count);
    void ToMapGeometry(Geometry*geo);
    double FromMapXDistance(double dist);
    double FromMapYDistance(double dist);
    void FromMapPoint(DOUBLE*X,DOUBLE*Y);
    void FromMapPoints(vector<double>&XYs);
    void FromMapPoints(DOUBLE*XYs,int count);
    void FromMapGeometry(Geometry*geo);
    AutoPtr<Envelope>GetClientExtent();
    AutoPtr<Envelope>GetMapExtent();
    void SetExtent(Envelope*client,Envelope*map);
    void SetExtentDirectly(Envelope*client,Envelope*map);
    void ZoomInMap(double ratio);
    void ZoomOutMap(double ratio);
    void SizeMap(Envelope*client);
    void PanMap(Point*orginMap,Point*targetMap);
    void CenterMap(Point*centerPoint);
    void ZoomToRect(Envelope*extent);
    double GetAngle();
    void SetAngle(double ang);
    bool ProjectFromDisplay(Geometry*geo,SpatialReference*psp);
    bool ProjectToDisplay(Geometry*geo,SpatialReference*psp);
    double DistanceFromDisplay(Point*centerpoint,double dist,SpatialReference*psp);
    double DistanceToDisplay(Point*centerpoint,double dist,SpatialReference*psp);
    AutoPtr<Display>Clone();
    bool ClipFromMapGeometry(Geometry*geo);
    bool ClipFromMapGeometry(Geometry*geo,SpatialReference*psp);
    bool ClipFromClientGeometry(Geometry*geo);

    AutoPtr<Geometry>CopyFromMapGeometry(Geometry*geo);
    AutoPtr<Geometry>CopyFromMapGeometry(Geometry*geo,SpatialReference*psp);
    AutoPtr<Geometry>CopyFromClientGeometry(Geometry*geo);
    float GetCurrentScale(float zoomratio=1.0);

    void ToMapPoint(Point*pGeo);
    void ToMapPoints(Points*pGeo);
    void ToMapSegment(Segment*pGeo);
    void ToMapEllipse(Ellipse*pGeo);
    void ToMapEnvelope(Envelope*pGeo);
    void ToMapPolyline(Polyline*pGeo);
    void ToMapPolygon(Polygon*pGeo);
    void ToMapMultiPolygon(MultiPolygon*pGeo);
    void ToMapCollection(GeometryCollection*pGeo);

    void FromMapPoint(Point*pGeo);
    void FromMapPoints(Points*pGeo);
    void FromMapSegment(Segment*pGeo);
    void FromMapEllipse(Ellipse*pGeo);
    void FromMapEnvelope(Envelope*pGeo);
    void FromMapPolyline(Polyline*pGeo);
    void FromMapPolygon(Polygon*pGeo);
    void FromMapMultiPolygon(MultiPolygon*pGeo);
    void FromMapCollection(GeometryCollection*pGeo);

    bool&Valid();
 protected:
    void ComputeTransform();
    int ClipFromPoints(Points*points,bool isRing,bool bClient);
public:
//BufferWritable:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
//JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
protected:
    Rect2D<int>ClientRect;
    Rect2D<double>MapRect;
    double ax,bx;
    double ay,by;
    DOUBLE angle;//单位度
    DOUBLE cosa;
    DOUBLE sina;
    DOUBLE MapCenterX;
    DOUBLE MapCenterY;
    AutoPtr<SpatialReference>pSpatial;
    int neighborCodes[9];
    bool valid;
};

}
#endif // DISPLAY_H
