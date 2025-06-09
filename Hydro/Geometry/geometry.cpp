#include "geometry.h"
#include "polyline.h"
#include "polygon.h"

namespace SGIS{
Geometry::Geometry()
{

}
Geometry::~Geometry()
{

}

AutoPtr<Polyline>Geometry::ConvertToPolyline(Polyline*polyline)
{
    AutoPtr<CurvePolyline>cp=AutoPtr<Polyline>(polyline,true);
    if(cp!=nullptr)
        return cp->ConvertToDefaultPolyline(true);
    else
    {
        return AutoPtr<Polyline>(polyline,true);
    }
}
AutoPtr<Polygon>Geometry::ConvertToPolygon(Polygon*polygon)
{
    AutoPtr<CurvePolygon>cp=AutoPtr<Polygon>(polygon,true);
    if(cp!=nullptr)
        return cp->ConvertToDefaultPolygon(true);
    else
    {
        return AutoPtr<Polygon>(polygon,true);
    }
}

Geometry3D::Geometry3D()
{

}
Geometry3D::~Geometry3D(){

}

void Geometry3D::innerRotate(double sina,double cosa,DOUBLE x,double y,DOUBLE z,DOUBLE&X,DOUBLE&Y,DOUBLE&Z)
{
    float cosa1=(1-cosa);
    double tx=(cosa+x*x*cosa1)*X+(x*y*cosa1-z*sina)*Y+(x*z*cosa1+y*sina)*Z;
    double ty=(x*y*cosa1+z*sina)*X+(cosa+y*y*cosa1)*Y+(y*z*cosa1-x*sina)*Z;
    double tz=(x*z*cosa1-y*sina)*X+(y*z*cosa1+x*sina)*Y+(cosa+z*z*cosa1)*Z;
    X=tx;
    Y=ty;
    Z=tz;
}

}
