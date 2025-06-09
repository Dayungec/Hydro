#ifndef GEOMETRYTOPOLOGY_H
#define GEOMETRYTOPOLOGY_H

#include "Base/base_globle.h"
#include "Base/autoptr.h"
#include "geometry.h"
#include "Point.h"
#include "segment.h"
#include "envelope.h"
#include "ellipse.h"
#include "points.h"
#include "polyline.h"
#include "polygon.h"
#include "multipolygon.h"
#include "geometrycollection.h"

namespace SGIS{

enum GeometryTopoType
{
    gttIntersect	= 0,
    gttEquals	= 0x1,
    gttDisjoint	= 0x2,
    gttTouches	= 0x3,
    gttCrosses	= 0x4,
    gttWithIn	= 0x5,
    gttContains	= 0x6,
    gttOverlaps	= 0x7
};

class SGIS_EXPORT GeometryTopology
{
public:
    GeometryTopology(AutoPtr<Geometry>geo=nullptr);
    virtual~GeometryTopology();
    AutoPtr<Geometry>GetGeometry();
    void SetGeometry(AutoPtr<Geometry>geo);
    bool Intersects(Geometry*other);
    bool Equals(Geometry*other);
    bool Disjoint(Geometry*other);
    bool Touches(Geometry*other);
    bool Crosses(Geometry*other);
    bool Within(Geometry*other);
    bool Contains(Geometry*other);
    bool Overlaps(Geometry*other);
protected:
    AutoPtr<Geometry>pGeo;
};
}
#endif // GEOMETRYTOPOLOGY_H
