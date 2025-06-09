#include "geometrytopology.h"
#include "GDAL/geometryfactory.h"

namespace SGIS{
GeometryTopology::GeometryTopology(AutoPtr<Geometry>geo){
    this->pGeo=geo;
}
GeometryTopology::~GeometryTopology(){

}
AutoPtr<Geometry>GeometryTopology::GetGeometry(){
    return pGeo;
}
void GeometryTopology::SetGeometry(AutoPtr<Geometry>geo){
    this->pGeo=geo;
}
bool GeometryTopology::Intersects(Geometry*other){
    if((pGeo==nullptr)||(other==nullptr)) return false;
    return pGeo->Intersects(other);
}
bool GeometryTopology::Equals(Geometry*other){
    if((pGeo==nullptr)||(other==nullptr)) return false;
     return pGeo->Equals(other);
}
bool GeometryTopology::Disjoint(Geometry*other){
    if((pGeo==nullptr)||(other==nullptr)) return false;
     return pGeo->Disjoint(other);
}
bool GeometryTopology::Touches(Geometry*other){
    if((pGeo==nullptr)||(other==nullptr)) return false;
    return pGeo->Touches(other);
}
bool GeometryTopology::Crosses(Geometry*other){
    if((pGeo==nullptr)||(other==nullptr)) return false;
    return pGeo->Crosses(other);
}
bool GeometryTopology::Within(Geometry*other){
    if((pGeo==nullptr)||(other==nullptr)) return false;
    return pGeo->Within(other);
}
bool GeometryTopology::Contains(Geometry*other){
    if((pGeo==nullptr)||(other==nullptr)) return false;
    return pGeo->Contains(other);
}
bool GeometryTopology::Overlaps(Geometry*other){
    if((pGeo==nullptr)||(other==nullptr)) return false;
    return pGeo->Overlaps(other);
}

}
