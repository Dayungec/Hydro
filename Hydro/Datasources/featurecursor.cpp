#include "featurecursor.h"

namespace SGIS{


Feature::Feature(){

}

Feature::~Feature(){

}

AutoPtr<Geometry>Feature::GetGeometry(){
    return geo->Clone();
}

Variant Feature::GetFieldValue(int fieldIndex){
    return values[fieldIndex];
}
string Feature::GetFieldValueAsString(int fieldIndex){
    return values[fieldIndex].asString();
}
double Feature::GetFieldValueAsDouble(int fieldIndex){
    return values[fieldIndex].asDouble();
}

LONG Feature::GetFeatureID(){
    return featureID;
}

FeatureCursor::FeatureCursor(){
   desc=nullptr;
   psp=nullptr;
}

FeatureCursor::~FeatureCursor(){

}

AutoPtr<TableDesc>FeatureCursor::GetTableDesc(){
    return desc->Clone();
}

AutoPtr<Feature>FeatureCursor::GetItem(int recordIndex){
    return fs[recordIndex];
}

AutoPtr<Geometry>FeatureCursor::GetFeature(int recordIndex){
    return fs[recordIndex]->GetGeometry();
}
int FeatureCursor::GetFeatureID(int recordIndex){
    return fs[recordIndex]->GetFeatureID();
}

int FeatureCursor::GetFeatureCount(){
    return fs.size();
}

Variant FeatureCursor::GetFieldValue(int recordIndex,int FieldIndex){
    return fs[recordIndex]->GetFieldValue(FieldIndex);
}

double FeatureCursor::GetFieldValueAsDouble(int recordIndex,int FieldIndex){
    return fs[recordIndex]->GetFieldValueAsDouble(FieldIndex);
}
string FeatureCursor::GetFieldValueAsString(int recordIndex,int FieldIndex){
     return fs[recordIndex]->GetFieldValueAsString(FieldIndex);
}
AutoPtr<SpatialReference>FeatureCursor::GetSpatialReference(){
    return psp->Clone();
}

FeatureCursorCreator::FeatureCursorCreator(AutoPtr<TableDesc>desc,AutoPtr<SpatialReference>psp){
    cursor=new FeatureCursor();
    cursor->desc=desc->Clone();
    cursor->psp=(psp==nullptr?nullptr:psp->Clone());
}
FeatureCursorCreator::~FeatureCursorCreator(){

}
void FeatureCursorCreator::AddFeature(int featureID,AutoPtr<Geometry>geo,const vector<Variant>&values){
    int fieldCount=cursor->desc->GetFieldCount();
    Feature*f=new Feature();
    f->geo=geo;
    f->values.resize(fieldCount);
    int size=values.size();
    if(size>fieldCount) size=fieldCount;
    for(int k=0;k<fieldCount;k++){
        Variant value=values[k];
        cursor->desc->ValidFieldValue(k,value);
        f->values[k]=value;
        f->featureID=featureID;
    }
    cursor->fs.push_back(f);
}
AutoPtr<FeatureCursor>FeatureCursorCreator::CreateFeatureCursor(){
    return cursor;
}


}
