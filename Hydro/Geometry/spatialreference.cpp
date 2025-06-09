#include "spatialreference.h"
#include "Base/StringHelper.h"
#include "Base/classfactory.h"
#include "Base/LicenseEngine.h"
#include "Point.h"
#include "points.h"
#include "GDAL/geometryfactory.h"


namespace SGIS{

REGISTER(SpatialReference)

SpatialReference::SpatialReference()
{
    pSpatial=new GDALSpatialReference();
}

SpatialReference::SpatialReference(LONG epsg){
    pSpatial=new GDALSpatialReference();
    pSpatial->ptr()->importFromEPSG(epsg);
}

SpatialReference::SpatialReference(string wkt){
    pSpatial=new GDALSpatialReference();
    pSpatial->ptr()->importFromWkt(wkt.c_str());
}

SpatialReference::~SpatialReference()
{
   delete pSpatial;
}

//BufferWritable:
bool SpatialReference::Read(BufferReader*br){
    string wkt=br->ReadString();
    return ImportFromWkt(wkt);
}

void SpatialReference::Write(BufferWriter*bw){
    string wkt=this->ExportToWkt();
    bw->WriteString(wkt);
}
string SpatialReference::GetTypeName(){
    return "SpatialReference";
}
//JsonWritable:
bool SpatialReference::ReadFromJson(JsonReader*jr,JsonNode node){
    string wkt=node["wkt"].asString();
    return ImportFromWkt(wkt);
}

string SpatialReference::ToJson(){
    string wkt=this->ExportToWkt();
    return "{\"wkt\":\""+wkt+"\"}";
}

bool SpatialReference::IsUndefined(){
    char*pszWKT = NULL;
    OGRErr pErr=pSpatial->ptr()->exportToWkt(&pszWKT);
    if(pErr!=OGRERR_NONE)
    {
        return true;
    }
    string swkt=pszWKT;
    CPLFree(pszWKT);
    return swkt.empty();
}

bool SpatialReference::IsGeographic(){
    if(pSpatial->ptr()->IsGeographic()) return true;
    return false;
}

bool SpatialReference::IsProjected(){
    if(pSpatial->ptr()->IsProjected())return true;
    return false;
}
bool SpatialReference::IsSameGeogCS(SpatialReference*other){
    if(pSpatial->ptr()->IsSameGeogCS(other->pSpatial->ptr()))return true;
    return false;
}
bool SpatialReference::IsSame(SpatialReference*other){
    if(other==nullptr){
        return true;
    }
    if(pSpatial->ptr()->IsSame(other->pSpatial->ptr()))return true;
    return false;
}
bool SpatialReference::ImportFromWkt(string wkt){
    OGRErr pErr=pSpatial->ptr()->importFromWkt(wkt.c_str());
    if(pErr==OGRERR_NONE)return true;
    return false;
}
string SpatialReference::ExportToWkt(){
    char*pszWKT = NULL;
    OGRErr pErr=pSpatial->ptr()->exportToWkt(&pszWKT);
    if(pErr==OGRERR_NONE)
    {
        if(pszWKT!=NULL)
        {
            string sWkt(pszWKT);
            CPLFree(pszWKT);
            return sWkt;
        }
        else
            return "";
    }
    return "";
}

string SpatialReference::ExportToProj4(){
    char*pszWKT = NULL;
    OGRErr pErr=pSpatial->ptr()->exportToProj4(&pszWKT);
    if(pErr==OGRERR_NONE)
    {
        if(pszWKT!=NULL)
        {
            string sWkt(pszWKT);
            CPLFree(pszWKT);
            return sWkt;
        }
        else
            return "";
    }
    return "";
}
bool SpatialReference::ImportFromESRI(string esri){
    char*szWkt =(char *)esri.c_str();
    OGRErr pErr=pSpatial->ptr()->importFromESRI(&szWkt);
    if(pErr==OGRERR_NONE)return true;
    return false;
}
string SpatialReference::Name(){
    string str=ExportToWkt();
    if(str.empty())
        return "Unknown CoordinateSystem";
    else
    {
        int fPos=str.find("\"");
        if(fPos==-1) return "Unknown CoordinateSystem";
        int tPos=str.find("\"",fPos+1);
        if(tPos==-1) return "Unknown CoordinateSystem";
        return str.substr(fPos+1,tPos-fPos-1);
    }
}
bool SpatialReference::ImportFromEPSG(LONG epsg){
    OGRErr pErr=pSpatial->ptr()->importFromEPSG(epsg);
    if(pErr==OGRERR_NONE)return true;
    return false;
}
int SpatialReference::FindEPSG(){
    char*pszWKT = NULL;
    pSpatial->ptr()->exportToWkt(&pszWKT);
    string sWkt="";
    if(pszWKT==NULL) return -1;
    sWkt=pszWKT;
    CPLFree(pszWKT);
    sWkt=StringHelper::Trim(sWkt);
    if(sWkt.empty()) return -1;
    int len=sWkt.length();
    int aPos=sWkt.find_last_of('A');
    if((aPos<=0)||(aPos+21>=len)) return -1;
    if(len-aPos-21>8) return -1;
    if("\"]]"!=sWkt.substr(sWkt.length()-3,3)) return -1;
    string Authority=sWkt.substr(aPos,9);
    if(Authority!="AUTHORITY") return -1;
    string EPSG=sWkt.substr(aPos+10,8);
    if(EPSG!="\"EPSG\",\"") return -1;
    EPSG=sWkt.substr(aPos+18,len-aPos-21);
    return atoi(EPSG.c_str());
}
AutoPtr<SpatialReference>SpatialReference::Clone(){
    SpatialReference*pNew=new SpatialReference();
    char*pszWKT = nullptr;
    if(pSpatial==nullptr){
        return new SpatialReference();
    }
    OGRErr pErr=pSpatial->ptr()->exportToWkt(&pszWKT);
    if(pszWKT!=nullptr)
    {
        pNew->pSpatial->ptr()->importFromWkt(&pszWKT);
    }
    return pNew;
}
AutoPtr<SpatialReference>SpatialReference::CloneGeogCS(){
    SpatialReference*pNew=new SpatialReference();
    char*pszWKT = NULL;
    OGRSpatialReference*psp=pSpatial->ptr()->CloneGeogCS();
    if(psp==NULL){
        delete pNew;
        return NULL;
    }
    OGRErr pErr=psp->exportToWkt(&pszWKT);
    if(pszWKT!=NULL)
    {
        pNew->pSpatial->ptr()->importFromWkt(&pszWKT);
    }
    delete psp;
    return pNew;
}

bool SpatialReference::ProjectTo(Geometry*geo,SpatialReference*psp){
    if(psp==nullptr) return false;
    if(IsSame(psp)) return true;
    AutoPtr<CoordinateTransformation>trans(new CoordinateTransformation());
    trans->SetFromSpatialReference(AutoPtr<SpatialReference>(psp,true));
    trans->SetToSpatialReference(AutoPtr<SpatialReference>(this,true));
    trans->BeginTransform();
    if(trans->CanTransform()) geo->Project(trans.get());
    return true;
}
bool SpatialReference::ProjectFrom(Geometry*geo,SpatialReference*psp){
    if(psp==nullptr) return false;
    if(IsSame(psp)) return true;
    AutoPtr<CoordinateTransformation>trans(new CoordinateTransformation());
    trans->SetFromSpatialReference(AutoPtr<SpatialReference>(this,true));
    trans->SetToSpatialReference(AutoPtr<SpatialReference>(psp,true));
    trans->BeginTransform();
    if(trans->CanTransform()) geo->Project(trans.get());
    return true;
}

double SpatialReference::DistanceTo(Point*centerPoint,double dist,SpatialReference*psp){
    if(psp==nullptr) return dist;
    if(psp->IsSame(this)) return dist;
    AutoPtr<CoordinateTransformation>trans(new CoordinateTransformation());
    trans->SetFromSpatialReference(AutoPtr<SpatialReference>(psp,true));
    trans->SetToSpatialReference(AutoPtr<SpatialReference>(this,true));
    trans->BeginTransform();
    if(!trans->CanTransform()) return dist;
    double x=centerPoint->X();
    double y=centerPoint->Y();
    double lx=x-dist/2.0,ly=y;
    trans->TransformXY(&lx,&ly);
    double rx=x+dist/2.0,ry=y;
    trans->TransformXY(&rx,&ry);
    double xdist=sqrt((lx-rx)*(lx-rx)+(ly-ry)*(ly-ry));
    lx=x;ly=y-dist/2.0;
    trans->TransformXY(&lx,&ly);
    rx=x;ry=y+dist/2.0;
    trans->TransformXY(&rx,&ry);
    double ydist=sqrt((lx-rx)*(lx-rx)+(ly-ry)*(ly-ry));
    return sqrt(xdist*xdist+ydist*ydist);
}
double SpatialReference::DistanceFrom(Point*centerPoint,double dist,SpatialReference*psp){
    if(psp==nullptr) return dist;
    if(psp->IsSame(this)) return dist;
    AutoPtr<CoordinateTransformation>trans(new CoordinateTransformation());
    trans->SetFromSpatialReference(AutoPtr<SpatialReference>(this,true));
    trans->SetToSpatialReference(AutoPtr<SpatialReference>(psp,true));
    trans->BeginTransform();
    if(!trans->CanTransform()) return dist;
    double x=centerPoint->X();
    double y=centerPoint->Y();
    double lx=x-dist/2.0,ly=y;
    trans->TransformXY(&lx,&ly);
    double rx=x+dist/2.0,ry=y;
    trans->TransformXY(&rx,&ry);
    double xdist=sqrt((lx-rx)*(lx-rx)+(ly-ry)*(ly-ry));
    lx=x;ly=y-dist/2.0;
    trans->TransformXY(&lx,&ly);
    rx=x;ry=y+dist/2.0;
    trans->TransformXY(&rx,&ry);
    double ydist=sqrt((lx-rx)*(lx-rx)+(ly-ry)*(ly-ry));
    return sqrt(xdist*xdist+ydist*ydist);
}

bool SpatialReference::CopyFromOther(SpatialReference*psp){
    if(psp==NULL)return false;
    string wkt=psp->ExportToWkt();
    char*cb=(char *)wkt.c_str();
    OGRErr pErr=pSpatial->ptr()->importFromWkt(&cb);
    if(pErr==OGRERR_NONE)return true;
    return false;
}
bool SpatialReference::CopyGeoCSFrom(SpatialReference*other){
    if(other==NULL) return false;
    OGRErr pErr=pSpatial->ptr()->CopyGeogCSFrom(other->pSpatial->ptr());
    if(pErr==OGRERR_NONE)return true;
    return false;
}
bool SpatialReference::SetWellKnownGeogCS(enum WellKnownGeogCS wks){
    OGRErr pErr;
    switch(wks)
    {
    case NAD27:pErr=pSpatial->ptr()->SetWellKnownGeogCS("NAD27");break;
    case NAD83:pErr=pSpatial->ptr()->SetWellKnownGeogCS("NAD83");break;
    case WGS72:pErr=pSpatial->ptr()->SetWellKnownGeogCS("WGS72");break;
    case WGS84:pErr=pSpatial->ptr()->SetWellKnownGeogCS("WGS84");break;
    }
    if(pErr==OGRERR_NONE) return true;
    return false;
}
bool SpatialReference::SetAlbers(DOUBLE dfStdP1,DOUBLE dfStdP2,DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Albers Conic Equal Area");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetACEA(dfStdP1,dfStdP2,dfCenterLat,dfCenterLong,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;

}
//等面积割圆锥投影
bool SpatialReference::SetAzimuthalEquidistant(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Azimuthal Equidistant");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetAE(dfCenterLat,dfCenterLong,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//等距方位投影
bool SpatialReference::SetBonne(DOUBLE dfStdP1,DOUBLE dfCentralMeridian,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Bonne");
    if(pErr!=OGRERR_NONE) return false;
    pErr=pSpatial->ptr()->SetBonne(dfStdP1,dfCentralMeridian,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//彭纳投影--等积伪圆锥投影
bool SpatialReference::SetCylindricalEqualArea(DOUBLE dfStdP1,DOUBLE dfCentralMeridian,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Cylindrical Equal Area");
    if(pErr!=OGRERR_NONE) return false;
    pErr=pSpatial->ptr()->SetCEA(dfStdP1,dfCentralMeridian,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//圆柱等积投影
bool SpatialReference::SetCassiniSoldner(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Cassini-Soldner");
    if(pErr!=OGRERR_NONE) return false;
    pErr=pSpatial->ptr()->SetCS(dfCenterLat,dfCenterLong,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//卡西尼投影
bool SpatialReference::SetEquidistantConic(DOUBLE dfStdP1,DOUBLE dfStdP2,DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Equidistant Conic");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetEC(dfStdP1,dfStdP2,dfCenterLat,dfCenterLong,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//圆锥等距投影
bool SpatialReference::SetEquirectangular(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Equirectangular");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetEquirectangular(dfCenterLat,dfCenterLong,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE)return true;
    return false;
}
//等距柱面投影
bool SpatialReference::SetEquirectangular2(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfPseudoStdParallel1,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Equidistant Conic 2");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetEquirectangular2(dfCenterLat,dfCenterLong,dfPseudoStdParallel1,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE)return true;
    return false;
}
//等距柱面投影_2
bool SpatialReference::SetGoodeHomolosine(DOUBLE dfCentralMeridian,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Goode Homolosine");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetGH(dfCentralMeridian,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//等积伪圆柱投影
bool SpatialReference::SetGnomonic(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Gnomonic");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetGnomonic(dfCenterLat,dfCenterLong,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//球心投影
bool SpatialReference::SetIWMPolyconic(DOUBLE dfLat1,DOUBLE dfLat2,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("International Map of the World Polyconic");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetIWMPolyconic(dfLat1,dfLat2,dfCenterLong,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//世界多圆锥投影
bool SpatialReference::SetLambert(DOUBLE dfLat1,DOUBLE dfLat2,DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Lambert Azimuthal Equal-Area");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetLCC(dfLat1,dfLat2,dfCenterLat,dfCenterLong,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//兰伯特投影
bool SpatialReference::SetMillerCylindrical(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Miller Cylindrical");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetMC(dfCenterLat,dfCenterLong,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE)return true;
    return false;
}
//米勒圆柱投影
bool SpatialReference::SetMercator(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfScale,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Mercator");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetMercator(dfCenterLat,dfCenterLong,dfScale,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//墨卡托投影
bool SpatialReference::SetMollweide(DOUBLE dfCentralMeridian,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Mollweide");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetMollweide(dfCentralMeridian,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE)return true;
    return false;
}
//摩尔魏特投影
bool SpatialReference::SetObliqueStereographic(DOUBLE dfOriginLat,DOUBLE dfCMeridian,DOUBLE dfScale,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Oblique Stereographic");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetOS(dfOriginLat,dfCMeridian,dfScale,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//斜轴球面投影
bool SpatialReference::SetOrthographic(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Orthographic");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetOrthographic(dfCenterLat,dfCenterLong,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//正射投影
bool SpatialReference::SetPolyconic(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Polyconic");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetPolyconic(dfCenterLat,dfCenterLong,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//多圆锥投影
bool SpatialReference::SetPolarStereographic(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfScale,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Polar Stereographic");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetPS(dfCenterLat,dfCenterLong,dfScale,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//极射赤平投影
bool SpatialReference::SetRobinson(DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Robinson");
    if(pErr!=OGRERR_NONE) return false;
    pErr=pSpatial->ptr()->SetRobinson(dfCenterLong,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//罗宾森投影
bool SpatialReference::SetSinusoidal(DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Sinusoidal");
    if(pErr!=OGRERR_NONE) return false;
    pErr=pSpatial->ptr()->SetSinusoidal(dfCenterLong,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//正弦曲线投影
bool SpatialReference::SetStereographic(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfScale,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Polar Stereographic");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetPS(dfCenterLat,dfCenterLong,dfScale,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//赤平投影
bool SpatialReference::SetTransverseMercator(DOUBLE dfCenterLat,DOUBLE dfCenterLong,DOUBLE dfScale,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Polar Stereographic");
    if(pErr!=OGRERR_NONE)return false;
    pErr=pSpatial->ptr()->SetPS(dfCenterLat,dfCenterLong,dfScale,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//横轴墨卡托投影
bool SpatialReference::SetTwoPointEquidistant(DOUBLE dfLat1, DOUBLE dfLong1, DOUBLE dfLat2, DOUBLE  dfLong2, DOUBLE  dfFalseEasting, DOUBLE  dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Two Point Equidistant");
    if(pErr!=OGRERR_NONE) return false;
    pErr=pSpatial->ptr()->SetTPED(dfLat1, dfLong1, dfLat2,dfLong2, dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//两点等距投影
bool SpatialReference::SetVanDerGrinten(DOUBLE dfCenterLong,DOUBLE dfFalseEasting,DOUBLE dfFalseNorthing){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("VanDerGrinten");
    if(pErr!=OGRERR_NONE) return false;
    pErr=pSpatial->ptr()->SetVDG(dfCenterLong,dfFalseEasting,dfFalseNorthing);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
bool SpatialReference::SetUTM(LONG nZone,bool bNorth){
    OGRErr pErr=pSpatial->ptr()->SetProjCS("Universal Transverse Mercator");
    if(pErr!=OGRERR_NONE) return false;
    if(bNorth)
        pErr=pSpatial->ptr()->SetUTM(nZone,TRUE);
    else
        pErr=pSpatial->ptr()->SetUTM(nZone,FALSE);
    if(pErr==OGRERR_NONE) return true;
    return false;
}
//通用横轴墨卡托投影
CoordinateTransformation::CoordinateTransformation(AutoPtr<SpatialReference>fromsp,AutoPtr<SpatialReference>tosp){
    this->fs=fromsp;
    this->ts=tosp;
    poCT=nullptr;
}

CoordinateTransformation::~CoordinateTransformation(){
    if(poCT!=nullptr){
        OGRCoordinateTransformation::DestroyCT(poCT);
        //delete poCT;
    }
}

AutoPtr<SpatialReference>CoordinateTransformation::GetFromSpatialReference(){
    return fs;
}
AutoPtr<SpatialReference>CoordinateTransformation::GetToSpatialReference(){
    return ts;
}
void CoordinateTransformation::SetFromSpatialReference(AutoPtr<SpatialReference>fromsp){
    fs=fromsp;
}
void CoordinateTransformation::SetToSpatialReference(AutoPtr<SpatialReference>tosp){
    ts=tosp;
}

bool CoordinateTransformation::BeginTransform(){
    if(poCT!=nullptr){
        OGRCoordinateTransformation::DestroyCT(poCT);
        //delete poCT;
    }
    poCT=NULL;
    if((fs==nullptr)||(ts==nullptr)) return false;
    if((fs->IsUndefined())||(ts->IsUndefined())) return false;
    poCT=OGRCreateCoordinateTransformation(fs->pSpatial->ptr(),ts->pSpatial->ptr());
    if(poCT!=NULL)
    {
        IsSame=fs->IsSame(ts.get());
        return true;
    }
    return false;
}

bool CoordinateTransformation::TransformXY(DOUBLE*X,DOUBLE*Y){
    if((IsSame)||(poCT==nullptr)) return false;
    return poCT->Transform(1,X,Y);
}

bool CoordinateTransformation::TransformPoint(Point*ppt){
    if((IsSame)||(poCT==nullptr)) return false;
    DOUBLE x,y;
    ppt->GetCoord(&x,&y);
    if(!poCT->Transform(1,&x,&y)) return false;
    ppt->PutCoord(x,y);
    return true;
}

bool CoordinateTransformation::TransformEnvelope(Envelope*envi){
     if((IsSame)||(poCT==nullptr)) return false;
     return envi->Project(this);
}

Rect2D<double>CoordinateTransformation::TransformEnvelope(Rect2D<double>drect){
    double DifX=(drect.Right-drect.Left)/5;
    double DifY=(drect.Top-drect.Bottom)/5;
    Point2D<double>des;
    Rect2D<double> rect;
    bool HasSuc=false;
    bool IsSuc;
    for(int i=0;i<6;i++)
    {
        for(int j=0;j<6;j++)
        {
            des.Y=drect.Top-DifY*i;
            des.X=drect.Left+DifX*j;
            IsSuc=poCT->Transform(1,&des.X,&des.Y);
            if(IsSuc)
            {
                if(HasSuc)
                    rect+=Rect2D<double>(des.X,des.Y,des.X,des.Y);
                else
                    rect=Rect2D<double>(des.X,des.Y,des.X,des.Y);
                HasSuc=true;
            }
        }
    }
    if(HasSuc) return rect;
    return Rect2D<double>(drect.Left,drect.Top,drect.Right,drect.Bottom);
}

bool CoordinateTransformation::TransformEllipse(Ellipse*elli){
    if((IsSame)||(poCT==NULL)) return false;
    return elli->Project(this);
}
bool CoordinateTransformation::TransformArray(double*xs,double*ys,int count){
    if((IsSame)||(poCT==NULL)) return false;
    if(poCT->Transform(count,xs,ys)) return true;
    return false;
}

bool CoordinateTransformation::TransformPoints(Points*points){
    if((IsSame)||(poCT==NULL)) return false;
    return points->Project(this);
}

bool CoordinateTransformation::TransformPolyline(Polyline*poly){
    if((IsSame)||(poCT==NULL)) return false;
    return poly->Project(this);
}

bool CoordinateTransformation::TransformPolygon(Polygon*poly){
    if((IsSame)||(poCT==NULL)) return false;
    return poly->Project(this);
}

bool CoordinateTransformation::TransformMultiPolygon(MultiPolygon*poly){
    if((IsSame)||(poCT==NULL)) return false;
    return poly->Project(this);
}

bool CoordinateTransformation::TransformGeometryCollection(GeometryCollection*geocol){
    if((IsSame)||(poCT==NULL)) return false;
    return geocol->Project(this);
}

bool CoordinateTransformation::TransformGeometry(Geometry*geo){
    if((IsSame)||(poCT==NULL)) return false;
    return geo->Project(this);
}

bool CoordinateTransformation::CanTransform(){
    return ((poCT!=NULL)&&(!IsSame));
}


SpatialReferenceInfo::SpatialReferenceInfo(){
    hasLoaded=false;
}
SpatialReferenceInfo::~SpatialReferenceInfo(){

}

int SpatialReferenceInfo::GetGeoCSCount(){
    Load();
    return gcses->GetRecordCount();
}
string SpatialReferenceInfo::GetGeoCSName(int nIndex){
    Load();
    return gcses->GetFieldValueAsString(nIndex,1);
}
string SpatialReferenceInfo::GetGeoCSId(int nIndex){
    Load();
    return gcses->GetFieldValueAsString(nIndex,0);
}
int SpatialReferenceInfo::GetProjCSCount(){
    Load();
    return pcses->GetRecordCount();
}
string SpatialReferenceInfo::GetProjCSName(int nIndex){
    Load();
    return pcses->GetFieldValueAsString(nIndex,1);
}
string SpatialReferenceInfo::GetProjCSId(int nIndex){
    Load();
    return pcses->GetFieldValueAsString(nIndex,0);
}

int SpatialReferenceInfo::findById(DataTable*dt,string id){
    Load();
    int recordCount=dt->GetRecordCount();
    for(int k=0;k<recordCount;k++){
        string value=dt->GetFieldValueAsString(k,0);
        if(value==id) return k;
    }
    return -1;
}

string SpatialReferenceInfo::GetGeoCSInfo(int index){
    Load();
    string sInfo="坐标名称:"+gcses->GetFieldValueAsString(index,1)+"\r\n";
    sInfo=sInfo+"Datum:";
    string dID=gcses->GetFieldValueAsString(index,2);
    int dindex=findById(datums.get(),dID);
    if(dindex>=0) sInfo=sInfo+datums->GetFieldValueAsString(dindex,1);
    sInfo+="\r\n";
    sInfo+="椭球体:";
    string eID=gcses->GetFieldValueAsString(index,6);
    int eindex=findById(ellipsoids.get(),eID);
    if(eindex>=0) sInfo=sInfo+ellipsoids->GetFieldValueAsString(eindex,1);
    sInfo+="\r\n";
    sInfo+="semi_major_axis:";
    if(eindex>=0) sInfo=sInfo+ellipsoids->GetFieldValueAsString(eindex,2);
    sInfo+="\r\n";
    if(eindex>=0)
    {
        string minor=ellipsoids->GetFieldValueAsString(eindex,5);
        if(minor!="")
        {
            sInfo=sInfo+"semi_minor_axis:"+minor+"\r\n";
        }
    }
    sInfo+="inv_flattening:";
    if(eindex>=0) sInfo=sInfo+ellipsoids->GetFieldValueAsString(eindex,4);
    sInfo+="\r\n";
    return sInfo;
}

string SpatialReferenceInfo::GetProjCSInfo(int index){
    Load();
    string sInfo="坐标名称:"+pcses->GetFieldValueAsString(index,1)+"\r\n";
    string gID=pcses->GetFieldValueAsString(index,3);
    index=findById(gcses.get(),gID);
    if(index==-1) return sInfo;
    sInfo=sInfo+"Datum:";
    string dID=pcses->GetFieldValueAsString(index,2);
    int dindex=findById(datums.get(),dID);
    if(dindex>=0) sInfo=sInfo+datums->GetFieldValueAsString(dindex,1);
    sInfo+="\r\n";
    sInfo+="椭球体:";
    string eID=gcses->GetFieldValueAsString(index,6);
    int eindex=findById(ellipsoids.get(),eID);
    if(eindex>=0) sInfo=sInfo+ellipsoids->GetFieldValueAsString(eindex,1);
    sInfo+="\r\n";
    sInfo+="semi_major_axis:";
    if(eindex>=0) sInfo=sInfo+ellipsoids->GetFieldValueAsString(eindex,2);
    sInfo+="\r\n";
    if(eindex>=0)
    {
        string minor=ellipsoids->GetFieldValueAsString(eindex,5);
        if(minor!="")
        {
            sInfo=sInfo+"semi_minor_axis:"+minor+"\r\n";
        }
    }
    sInfo+="inv_flattening:";
    if(eindex>=0) sInfo=sInfo+ellipsoids->GetFieldValueAsString(eindex,4);
    sInfo+="\r\n";
    return sInfo;
}

string SpatialReferenceInfo::GetGeoCSInfo(string epsg){
    Load();
    int index=findById(gcses.get(),epsg);
    if(index==-1) return "";
    return GetGeoCSInfo(index);
}

string SpatialReferenceInfo::GetProjCSInfo(string epsg){
    Load();
    int index=findById(pcses.get(),epsg);
    if(index==-1) return "";
    return GetProjCSInfo(index);
}

string SpatialReferenceInfo::GetSpatialReferenceInfo(string epsg){
    Load();
    int index=findById(pcses.get(),epsg);
    if(index==-1) return GetGeoCSInfo(epsg);
    return GetProjCSInfo(epsg);
}

bool SpatialReferenceInfo::Load(){
    if(hasLoaded) return true;
    string projDir=LicenseEngine::AssetsPath()+"/proj/";
    pcses=DataTable::LoadFromCsv(projDir+"pcs.csv");
    if(pcses==nullptr) return false;
    gcses=DataTable::LoadFromCsv(projDir+"gcs.csv");
    if(gcses==nullptr) return false;
    datums=DataTable::LoadFromCsv(projDir+"gdal_datum.csv");
    if(datums==nullptr) return false;
    ellipsoids=DataTable::LoadFromCsv(projDir+"ellipsoid.csv");
    if(ellipsoids==nullptr) return false;
    hasLoaded=true;
    return true;
}


BiCoordinateTransformation::BiCoordinateTransformation(){

}

BiCoordinateTransformation::~BiCoordinateTransformation(){

}

void BiCoordinateTransformation::SetSpatialReferences(AutoPtr<SpatialReference>psp,AutoPtr<SpatialReference>pspto){
    if((this->psp==psp.get())&&(this->pspto.get()==pspto.get())) return;
    pTrans=new CoordinateTransformation(psp,pspto);
    pTrans->BeginTransform();
    pTransRev=new CoordinateTransformation(pspto,psp);
    pTransRev->BeginTransform();
}

void BiCoordinateTransformation::SetFromSpatialReference(AutoPtr<SpatialReference>psp){
    if(this->psp.get()==psp.get()) return;
    this->psp=psp;
    if(this->pspto!=nullptr){
        pTrans=new CoordinateTransformation(psp,pspto);
        pTrans->BeginTransform();
        pTransRev=new CoordinateTransformation(pspto,psp);
        pTransRev->BeginTransform();
    }
}
void BiCoordinateTransformation::SetToSpatialReference(AutoPtr<SpatialReference>pspto){
    if(this->pspto.get()==pspto.get()) return;
    this->pspto=pspto;
    if(this->psp!=nullptr){
        pTrans=new CoordinateTransformation(psp,pspto);
        pTrans->BeginTransform();
        pTransRev=new CoordinateTransformation(pspto,psp);
        pTransRev->BeginTransform();
    }
}

AutoPtr<CoordinateTransformation>BiCoordinateTransformation::GetCoordinateTransformation(){
    return pTrans;
}

AutoPtr<CoordinateTransformation>BiCoordinateTransformation::GetCoordinateTransformationRev(){
    return pTransRev;
}


}
