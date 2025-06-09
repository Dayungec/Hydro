#include "queryfilter.h"
#include "Base/StringHelper.h"

namespace SGIS{

QueryFilter::QueryFilter(){

}
QueryFilter::~QueryFilter(){

}
string&QueryFilter::WhereClause(){
    return whereClause;
}
string QueryFilter::GetSubFields(){
    int Count=subFields.GetSize();
    string sValue="";
    for(int k=0;k<Count;k++)
    {
        string ss=subFields[k];
        if(k==0)
            sValue=ss;
        else
            sValue+=","+ss;
    }
    return sValue;
}
void QueryFilter::SetSubFields(string sfields){
   vector<string>fields=StringHelper::Split(sfields,",");
   for(int k=0;k<fields.size();k++){
       AddField(fields[k]);
   }
}
bool QueryFilter::AddField(string name){
   string sName=name;
   StringHelper::TrimToLower(sName);
   if(sName.empty()) return false;
   for(int k=0;k<subFields.GetSize();k++){
       string other=subFields[k];
       StringHelper::TrimToLower(other);
       if(other==sName) return false;
   }
   subFields.Add(name);
   return true;
}
int QueryFilter::GetFieldCount(){
   return subFields.GetSize();
}
string QueryFilter::GetField(int nIndex){
   return subFields[nIndex];
}

SpatialFilter::SpatialFilter(){
    gType=GeometryTopoType::gttIntersect;
}
SpatialFilter::~SpatialFilter(){

}
GeometryTopoType&SpatialFilter::SpatialRel(){
    return gType;
}
AutoPtr<Geometry>SpatialFilter::GetGeometry(){
    return refGeo;
}
void SpatialFilter::SetGeometry(AutoPtr<Geometry>geo){
    refGeo=geo;
}


}
