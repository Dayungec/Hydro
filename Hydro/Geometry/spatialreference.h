#ifndef SPATIALREFERENCE_H
#define SPATIALREFERENCE_H

#include "Base/base_globle.h"
#include "Base/classfactory.h"
#include "Base/jsonio.h"
#include  "Base/bufferio.h"
#include "Base/databaseconnection.h"
#include "geometry.h"
#include  "Point.h"
#include "envelope.h"
#include "segment.h"
#include "ellipse.h"
#include "polyline.h"
#include "polygon.h"
#include "multipolygon.h"
#include "geometrycollection.h"


class OGRCoordinateTransformation;
namespace SGIS{

enum WellKnownGeogCS
{
    NAD27,
    NAD83,
    WGS72,
    WGS84
};

class GDALSpatialReference;

class SGIS_EXPORT SpatialReference:
        public Object,
        public BufferWritable,
        public JsonWritable
{
public:
    SpatialReference();
    SpatialReference(LONG epsg);
    SpatialReference(string wkt);
    virtual~SpatialReference();
 //BufferWritable:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
 //JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
//SpatialReference:
    bool IsUndefined();
    bool IsGeographic();
    bool IsProjected();
    bool IsSameGeogCS(SpatialReference*other);
    bool IsSame(SpatialReference*other);
    bool ImportFromWkt(string wkt);
    string ExportToWkt();
    string ExportToProj4();
    bool ImportFromESRI(string esri);
    string Name();
    bool ImportFromEPSG(LONG epsg);
    int FindEPSG();
    AutoPtr<SpatialReference>Clone();
    AutoPtr<SpatialReference>CloneGeogCS();
    bool ProjectTo(Geometry*geo,SpatialReference*psp);
    bool ProjectFrom(Geometry*geo,SpatialReference*psp);
    double DistanceTo(Point*centerPoint,double dist,SpatialReference*psp);
    double DistanceFrom(Point*centerPoint,double dist,SpatialReference*psp);
    bool CopyFromOther(SpatialReference*other);
    bool CopyGeoCSFrom(SpatialReference*other);
    bool SetWellKnownGeogCS(enum WellKnownGeogCS wks);
    bool SetAlbers(DOUBLE dfStdP1,DOUBLE dfStdP2,DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //等面积割圆锥投影
    bool SetAzimuthalEquidistant(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //等距方位投影
    bool SetBonne(DOUBLE dfStdP1,DOUBLE dfCentralMeridian,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //彭纳投影--等积伪圆锥投影
    bool SetCylindricalEqualArea(DOUBLE dfStdP1,DOUBLE dfCentralMeridian,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //圆柱等积投影
    bool SetCassiniSoldner(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //卡西尼投影
    bool SetEquidistantConic(DOUBLE dfStdP1,DOUBLE dfStdP2,DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //圆锥等距投影
    bool SetEquirectangular(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //等距柱面投影
    bool SetEquirectangular2(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfPseudoStdParallel1,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //等距柱面投影_2
    bool SetGoodeHomolosine(DOUBLE dfCentralMeridian,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //等积伪圆柱投影
    bool SetGnomonic(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //球心投影
    bool SetIWMPolyconic(DOUBLE dfLat1,DOUBLE dfLat2,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //世界多圆锥投影
    bool SetLambert(DOUBLE dfStdP1,DOUBLE dfStdP2,DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //兰伯特投影
    bool SetMillerCylindrical(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //米勒圆柱投影
    bool SetMercator(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfScale,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //墨卡托投影
    bool SetMollweide(DOUBLE dfCentralMeridian,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //摩尔魏特投影
    bool SetObliqueStereographic(DOUBLE dfOriginLat,DOUBLE dfCMeridian,DOUBLE dfScale,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //斜轴球面投影
    bool SetOrthographic(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //正射投影
    bool SetPolyconic(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //多圆锥投影
    bool SetPolarStereographic(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfScale,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //极射赤平投影
    bool SetRobinson(DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //罗宾森投影
    bool SetSinusoidal(DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //正弦曲线投影
    bool SetStereographic(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfScale,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //赤平投影
    bool SetTransverseMercator(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfScale,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //横轴墨卡托投影
    bool SetTwoPointEquidistant(DOUBLE dfLat1, DOUBLE dfLong1, DOUBLE dfLat2, DOUBLE  dfLong2, DOUBLE  dfFalseEasting, DOUBLE  dfFalseNorthing);
    //两点等距投影
    bool SetVanDerGrinten(DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing);
    //范德格林顿投影
    bool SetUTM(LONG nZone,bool bNorth);
    //通用横轴墨卡托投影
    friend class CoordinateTransformation;
protected:
    GDALSpatialReference*pSpatial;
};

class SGIS_EXPORT CoordinateTransformation{
public:
   CoordinateTransformation(AutoPtr<SpatialReference>fromsp=nullptr,AutoPtr<SpatialReference>tosp=nullptr);
   virtual~CoordinateTransformation();
   AutoPtr<SpatialReference>GetFromSpatialReference();
   AutoPtr<SpatialReference>GetToSpatialReference();
   void SetFromSpatialReference(AutoPtr<SpatialReference>fromsp);
   void SetToSpatialReference(AutoPtr<SpatialReference>tosp);
   bool BeginTransform();
   bool TransformXY(DOUBLE*X,DOUBLE*Y);
   bool TransformPoint(Point*ppt);
   bool TransformEnvelope(Envelope*envi);
   Rect2D<double>TransformEnvelope(Rect2D<double>rect);
   bool TransformEllipse(Ellipse*elli);
   bool TransformArray(double*xs,double*ys,int count);
   bool TransformPoints(Points*points);
   bool TransformPolyline(Polyline*poly);
   bool TransformPolygon(Polygon*poly);
   bool TransformMultiPolygon(MultiPolygon*poly);
   bool TransformGeometryCollection(GeometryCollection*geocol);
   bool TransformGeometry(Geometry*geo);
   bool CanTransform();
protected:
    AutoPtr<SpatialReference>fs;
    AutoPtr<SpatialReference>ts;
    bool IsSame;
    OGRCoordinateTransformation*poCT;
};

class SGIS_EXPORT SpatialReferenceInfo{
public:
    SpatialReferenceInfo();
    virtual~SpatialReferenceInfo();
    int GetGeoCSCount();
    string GetGeoCSName(int nIndex);
    string GetGeoCSId(int nIndex);
    int GetProjCSCount();
    string GetProjCSName(int nIndex);
    string GetProjCSId(int nIndex);
    string GetGeoCSInfo(int nIndex);
    string GetProjCSInfo(int nIndex);
    string GetGeoCSInfo(string epsg);
    string GetProjCSInfo(string epsg);
    string GetSpatialReferenceInfo(string epsg);
protected:
    bool Load();
    int findById(DataTable*dt,string id);
protected:
    AutoPtr<DataTable>datums;
    AutoPtr<DataTable>ellipsoids;
    AutoPtr<DataTable>pcses;
    AutoPtr<DataTable>gcses;
    bool hasLoaded;
};

class SGIS_EXPORT BiCoordinateTransformation{
public:
    BiCoordinateTransformation();
    virtual~BiCoordinateTransformation();
    void SetSpatialReferences(AutoPtr<SpatialReference>psp,AutoPtr<SpatialReference>pspto);
    void SetFromSpatialReference(AutoPtr<SpatialReference>psp);
    void SetToSpatialReference(AutoPtr<SpatialReference>pspto);
    AutoPtr<CoordinateTransformation>GetCoordinateTransformation();
    AutoPtr<CoordinateTransformation>GetCoordinateTransformationRev();
protected:
    AutoPtr<SpatialReference>psp;
    AutoPtr<SpatialReference>pspto;
    AutoPtr<CoordinateTransformation>pTrans;
    AutoPtr<CoordinateTransformation>pTransRev;
};

}

#endif // SPATIALREFERENCE_H
