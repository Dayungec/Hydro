#include "featureclasseditor.h"
#include "Base/stringinterpreter.h"
#include "featureclasstool.h"
#include <QString>
namespace SGIS{

FieldEl::FieldEl(SHORT Index,Variant value)
{
    fieldIndex=Index;
    sValue=value;
}
FieldEl::FieldEl(const FieldEl&el)
{
   fieldIndex=el.fieldIndex;
   sValue=el.sValue;
}
FieldEl::FieldEl()
{
}
FieldEl::~FieldEl()
{

}
int FieldEl::GetProximateSize()
{
    switch(sValue.vt)
    {
    case VRT_BSTR:
        return (*sValue.bstrVal).length()+16;
    default:
        return 16;
    }
    return 16;
}

Variant FieldEl::GetFieldValue(){
    return sValue;
}

string FieldEl::GetFieldValueAsString()
{
    return sValue.asString();
}
double FieldEl::GetFieldValueAsDouble()
{
    return sValue.asDouble();
}
SHORT FieldEl::GetFieldIndex()
{
    return fieldIndex;
}

FieldEls::FieldEls()
{
    fels=nullptr;
    fCount=0;
};
FieldEls::~FieldEls()
{
    if(fels!=nullptr) delete []fels;
}
bool FieldEls::DeleteField(int fieldindex)
{
    int pos=FindField(fieldindex);
    if(pos<0) return false;
    if(fCount==1)
    {
        delete []fels;
        fels=NULL;
        fCount=0;
        return true;
    }
    FieldEl*els=new FieldEl[fCount-1];
    for(int k=0;k<pos;k++)
    {
       els[k]=fels[k];
    }
    for(int k=pos+1;k<fCount;k++)
    {
       els[k-1]=fels[k];
    }
    fCount--;
    delete []fels;
    fels=els;
    return true;
}

void FieldEls::AddField(int fieldindex,Variant value)
{
    int pos=FindField(fieldindex);
    if(pos>=0)
    {
        FieldEl el(fieldindex,value);
        fels[pos]=el;
    }
    else
    {
        pos=FindPlacePosi(fieldindex);
        FieldEl el(fieldindex,value);
        InsertField(pos,el);
    }
}

int FieldEls::GetProximateSize(int k)
{
    return fels[k].GetProximateSize();
}

int FieldEls::GetProximateSize()
{
    int Size=6;
    for(int k=0;k<fCount;k++)
    {
        Size+=fels[k].GetProximateSize();
    }
    return Size;
}

long FieldEls::FindPlacePosi(int fieldindex)
{
    long num=fCount;
    if(num<=15)
    {
        for(int k=0;k<num;k++)
        {
            if(fieldindex==fels[k].GetFieldIndex())
                return -1;
            else if(fieldindex<fels[k].GetFieldIndex())
                return k;
        }
        return num;
    }
    long last=num-1;
    long lastvalue=fels[last].GetFieldIndex();
    if(fieldindex>lastvalue) return num;
    long first=0;
    long firstvalue=fels[first].GetFieldIndex();
    if(fieldindex<firstvalue) return 0;
    if((fieldindex==firstvalue)||(fieldindex==lastvalue))
        return -1;
    long temp,tempvalue;
    while(true)
    {
       temp=(first+last)/2;
       if(temp==first) return first+1;
       tempvalue=fels[temp].GetFieldIndex();
       if(fieldindex<tempvalue)
       {
           last=temp;
           lastvalue=tempvalue;
       }
       else if(fieldindex>tempvalue)
       {
           first=temp;
           firstvalue=tempvalue;
       }
       else
       {
          return -1;
       }
    }
}
int FieldEls::FindField(int fieldindex)
{
    long num=fCount;
    if(num<=15)
    {
        for(int k=0;k<num;k++)
        {
            if(fieldindex==fels[k].GetFieldIndex())
                return k;
            else if(fieldindex<fels[k].GetFieldIndex())
                return -1;
        }
        return -1;
    }
    long last=num-1;
    long lastvalue=fels[last].GetFieldIndex();
    if(fieldindex>lastvalue) return -1;
    long first=0;
    long firstvalue=fels[first].GetFieldIndex();
    if(fieldindex<firstvalue) return -1;
    if(fieldindex==firstvalue)
        return 0;
    else if(fieldindex==lastvalue)
        return num-1;
    long temp,tempvalue;
    while(true)
    {
       temp=(first+last)/2;
       if(temp==first) return -1;
       tempvalue=fels[temp].GetFieldIndex();
       if(fieldindex<tempvalue)
       {
           last=temp;
           lastvalue=tempvalue;
       }
       else if(fieldindex>tempvalue)
       {
           first=temp;
           firstvalue=tempvalue;
       }
       else
       {
          return temp;
       }
    }
}
void FieldEls::InsertField(int pos,FieldEl&el)
{
    if(fCount==0)
    {
        fels=new FieldEl[1];
        fels[0]=el;
        fCount++;
    }
    else
    {
       FieldEl*els=new FieldEl[fCount+1];
       for(int k=0;k<pos;k++)
       {
           els[k]=fels[k];
       }
       els[pos]=el;
       for(int k=pos;k<fCount;k++)
       {
           els[k+1]=fels[k];
       }
       fCount++;
       delete []fels;
       fels=els;
    }
}

Variant FieldEls::GetFieldValue(LONG pos){
     return fels[pos].GetFieldValue();
}
string FieldEls::GetFieldValueAsString(LONG pos)
{
    return fels[pos].GetFieldValueAsString();
}
double FieldEls::GetFieldValueAsDouble(LONG pos)
{
    return fels[pos].GetFieldValueAsDouble();
}
void FieldEls::SetFieldValue(int FieldIndex,Variant newValue)
{
    AddField(FieldIndex,newValue);
}
int FieldEls::GetFieldCount()
{
    return fCount;
}
int FieldEls::GetFieldIndex(int index)
{
    return fels[index].GetFieldIndex();
}
AutoPtr<FieldEls>FieldEls::Copy()
{
    FieldEls*pNew=new FieldEls();
    pNew->fCount=fCount;
    if(fCount>0)
    {
        pNew->fels=new FieldEl[fCount];
        for(int k=0;k<fCount;k++) pNew->fels[k]=fels[k];
    }
    return pNew;
}

Records::Records()
{

}
Records::~Records()
{

}

EnRecord::EnRecord()
{
    pGeo=nullptr;
    fels=nullptr;
};
EnRecord::~EnRecord()
{

}
int EnRecord::GetRecordNumber()
{
    return 1;
}
bool EnRecord::IsVirtual()
{
    return false;
}
bool EnRecord::IsRefEnRecord()
{
    return false;
}
bool EnRecord::IsOldEnRecord()
{
    return false;
}
bool EnRecord::IsNewEnRecord()
{
    return false;
}

bool EnRecord::IsGeometryChanged()
{
    return (pGeo!=nullptr);
}
bool EnRecord::IsFieldValueChanged()
{
    return (fels!=nullptr);
}
int EnRecord::GetProximateSize(Geometry*pGeo){
    GeometryType gType=pGeo->GetType();
    switch(gType){
    case gtPoint:return 24;
    case gtPoints:{
        AutoPtr<Points>pts=AutoPtr<Geometry>(pGeo,true);
        int size=pts->GetSize();
        return 24*size;
    }
    case gtEnvelope:
    case gtSegment:
        return 48;
    case gtEllipse:
        return 32;
    case gtPolyline:
    {
        AutoPtr<Polyline>poly=AutoPtr<Geometry>(pGeo,true);
        int count=poly->GetSize();
        int msize=0;
        for(int k=0;k<count;k++){
            AutoPtr<Points>path=poly->GetItem(k);
            msize+=path->GetSize()*24;
        }
        return msize;
    }
    case gtPolygon:
    {
        AutoPtr<Polygon>poly=AutoPtr<Geometry>(pGeo,true);
        int count=poly->GetSize();
        int msize=0;
        for(int k=0;k<count;k++){
            AutoPtr<Points>path=poly->GetItem(k);
            msize+=path->GetSize()*24;
        }
        return msize;
    }
    case gtMultiPolygon:
    {
        AutoPtr<MultiPolygon>mpoly=AutoPtr<Geometry>(pGeo,true);
        int count=mpoly->GetSize();
        int msize=0;
        for(int k=0;k<count;k++){
            AutoPtr<Polygon>poly=mpoly->GetItem(k);
            int scount=poly->GetSize();
            for(int p=0;p<scount;p++){
                AutoPtr<Points>path=poly->GetItem(p);
                msize+=path->GetSize()*24;
            }
        }
        return msize;
    }
    case gtCollection:{
        AutoPtr<GeometryCollection>geos=AutoPtr<Geometry>(pGeo,true);
        int count=geos->GetSize();
        int msize=0;
        for(int k=0;k<count;k++){
            AutoPtr<Geometry>subgeo=geos->GetItem(k);
            msize+=GetProximateSize(subgeo.get());
        }
        return msize;
    }
    }
    return 0;
}
int EnRecord::GetProximateSize()
{
    int Size=0;
    if(pGeo!=nullptr)
    {
        Size+=GetProximateSize(pGeo.get());
    }
    if(fels!=NULL) Size+=fels->GetProximateSize();
    return Size+20;
}
void EnRecord::SetFeature(AutoPtr<Geometry>geo)
{
     pGeo=geo;
}
void EnRecord::SetFieldValue(int FieldIndex,Variant newValue)
{
    if(fels==NULL)
    {
         fels=new FieldEls();
         fels->AddField(FieldIndex,newValue);
     }
     else
        fels->AddField(FieldIndex,newValue);
}
int EnRecord::BackwardDestroyInvalidData()
{
    int Size=0;
    if(pGeo!=NULL)
    {
        Size+=GetProximateSize(pGeo.get());
        pGeo=nullptr;
    }
    if(fels!=nullptr)
    {
        Size+=fels->GetProximateSize();
    }
    fels=nullptr;
    return Size;
}
int EnRecord::BackwardDestroyInvalidData(Geometry*refGeo,FieldEls*pEl)
{
    int Size=0;
    if((pGeo!=refGeo)&&(refGeo!=nullptr))
    {
        if(pGeo!=NULL)
        {
            Size+=fels->GetProximateSize();
            pGeo=nullptr;
        }
    }
    if((pEl==NULL)||(fels==nullptr)) return Size;
    for(int k=pEl->GetFieldCount()-1;k>=0;k--)
    {
        int index=pEl->GetFieldIndex(k);
        if(fels->FindField(index)>=0)
        {
            Size+=fels->GetProximateSize(k);
            fels->DeleteField(index);
        }
    }
    if(fels->GetFieldCount()==0)
    {
        fels=nullptr;
        Size+=6;
    }
    return Size;
}
int EnRecord::DestroyBackwardInvalidData()
{
    return 0;
}
bool EnRecord::IsContainOther(EnRecord*other)
{
    if(pGeo==NULL)
    {
        if(other->pGeo!=nullptr) return false;
    }
    AutoPtr<FieldEls>otherEls=other->fels;
    if(fels==NULL)
    {
        if(otherEls==nullptr) return true;
        return false;
    }
    if(fels!=NULL)
    {
        if(otherEls==nullptr) return true;
        for(int k=otherEls->GetFieldCount()-1;k>=0;k--)
        {
            int index=otherEls->GetFieldIndex(k);
            if(fels->FindField(index)==-1) return false;
        }
    }
    return true;
}

OldEnRecord::OldEnRecord(LONG featureID)
{
    FeatureID=featureID;
}
OldEnRecord::~OldEnRecord()
{

}
int OldEnRecord::GetRecordNumber()
{
    return 1;
}
LONG OldEnRecord::GetFeatureID()
{
    return FeatureID;
}
bool OldEnRecord::IsOldEnRecord()
{
    return true;
}

bool OldEnRecord::GetEnvelope(Envelope*pEnv)
{
     if(pGeo==NULL)
     {
         return false;
     }
     else
     {
         AutoPtr<Envelope>env=pGeo->GetExtent();
         pEnv->CopyFromOther(env.get());
     }
     return true;
}

LONG OldEnRecord::GetFatherPos()
{
    return FeatureID;
}

AutoPtr<Geometry>OldEnRecord::GetGeometry(bool&bSuc){
    if(pGeo==nullptr)
    {
        bSuc=false;
        return nullptr;
    }
    else
    {
        bSuc=true;
        return pGeo->Clone();
    }
}
bool OldEnRecord::GetFieldValue(LONG FieldIndex,Variant&value)
{
    if(fels==NULL) return false;
    LONG nPos=fels->FindField(FieldIndex);
    if(nPos>=0)
    {
        value=fels->GetFieldValue(nPos);
        return true;
    }
    return false;
}
bool OldEnRecord::GetFieldValueAsString(LONG FieldIndex,string&value)
{
    if(fels==NULL) return false;
    LONG nPos=fels->FindField(FieldIndex);
    if(nPos>=0)
    {
        value=fels->GetFieldValueAsString(nPos);
        return true;
    }
    return false;
}
bool OldEnRecord::GetFieldValueAsDouble(LONG FieldIndex,double&value)
{
    if(fels==NULL) return false;
    LONG nPos=fels->FindField(FieldIndex);
    if(nPos>=0)
    {
        value=fels->GetFieldValueAsDouble(nPos);
        return true;
    }
    return false;
}

NewEnRecord::NewEnRecord(AutoPtr<Geometry>geo,long featureID)
{
    pGeo=geo;
    FeatureID=featureID;
}
NewEnRecord::~NewEnRecord()
{

}
LONG NewEnRecord::GetFeatureID()
{
    return FeatureID;
}
bool NewEnRecord::IsNewEnRecord()
{
    return true;
}
LONG NewEnRecord::GetFatherPos()
{
    return -1;
}
bool NewEnRecord::IsGeometryChanged()
{
    return true;
}
bool NewEnRecord::IsFieldValueChanged()
{
    return true;
}

bool NewEnRecord::GetEnvelope(Envelope*pEnv)
{
     AutoPtr<Envelope>env=pGeo->GetExtent();
     pEnv->CopyFromOther(env.get());
     return true;
}

AutoPtr<Geometry>NewEnRecord::GetGeometry(bool&bSuc){
    if(pGeo==nullptr)
    {
        bSuc=false;
        return nullptr;
    }
    else
    {
        bSuc=true;
        return pGeo->Clone();
    }
}
bool NewEnRecord::GetFieldValue(LONG FieldIndex,Variant&value)
{
    if(fels==nullptr)
    {
        value.Clear();
        return true;
    }
    LONG nPos=fels->FindField(FieldIndex);
    if(nPos>=0)
    {
        value=fels->GetFieldValue(nPos);
        return true;
    }
    value.Clear();
    return true;
}

bool NewEnRecord::GetFieldValueAsString(LONG FieldIndex,string&value)
{
    if(fels==NULL)
    {
        value="";
        return true;
    }
    LONG nPos=fels->FindField(FieldIndex);
    if(nPos>=0)
    {
        value=fels->GetFieldValueAsString(nPos);
        return true;
    }
    value="";
    return true;
}
bool NewEnRecord::GetFieldValueAsDouble(LONG FieldIndex,double&value)
{
    if(fels==NULL)
    {
        value= 0;
        return true;
    }
    LONG nPos=fels->FindField(FieldIndex);
    if(nPos>=0)
        value=fels->GetFieldValueAsDouble(nPos);
    else
        value=0;
    return true;
}

RefEnRecord::RefEnRecord(AutoPtr<EnRecord>pRecord,LONG fatherIndex)
{
    refRecord=pRecord;
    FatherIndex=fatherIndex;
}
RefEnRecord::~RefEnRecord()
{

}

bool RefEnRecord::IsRefEnRecord()
{
    return true;
}
int RefEnRecord::GetRecordNumber()
{
    return 1;
}

int RefEnRecord::GetProximateSize()
{
    int Size=0;
    if(pGeo!=NULL)
    {
        Size+=EnRecord::GetProximateSize(pGeo.get());
    }
    if(fels!=NULL) Size+=fels->GetProximateSize();
    return Size+24;
}

bool RefEnRecord::IsGeometryChanged()
{
    if(pGeo!=NULL) return true;
    return refRecord->IsGeometryChanged();
}
bool RefEnRecord::IsFieldValueChanged()
{
    if(fels!=NULL) return true;
    return refRecord->IsFieldValueChanged();
}
LONG RefEnRecord::GetFatherPos()
{
    return FatherIndex;
}

void RefEnRecord::SetFeature(AutoPtr<Geometry>geo)
{
    EnRecord::SetFeature(geo);
    if(!IsContainOther(refRecord.get())) return;
    AutoPtr<EnRecord>ref(refRecord);
    while(ref->IsRefEnRecord())
    {
        AutoPtr<RefEnRecord>r(ref);
        if(r->IsContainOther(r->refRecord.get()))
        {
            ref=r->refRecord;
        }
        else
            break;
    }
    if(ref==refRecord) return;
    refRecord=ref;
}
void RefEnRecord::SetFieldValue(int FieldIndex,Variant newValue)
{
    EnRecord::SetFieldValue(FieldIndex,newValue);
    if(!IsContainOther(refRecord.get())) return;
    AutoPtr<EnRecord>ref=refRecord;
    while(ref->IsRefEnRecord())
    {
        AutoPtr<RefEnRecord>r(ref);
        if(r->IsContainOther(r->refRecord.get()))
        {
            ref=r->refRecord;
        }
        else
            break;
    }
    if(ref==refRecord) return;
    refRecord=ref;
}
LONG RefEnRecord::GetFeatureID()
{
    return refRecord->GetFeatureID();
}
bool RefEnRecord::GetEnvelope(Envelope*pEnv)
{
     if(pGeo==nullptr)
     {
         return refRecord->GetEnvelope(pEnv);
     }
     else
     {
         AutoPtr<Envelope>env=pGeo->GetExtent();
         pEnv->CopyFromOther(env.get());
     }
     return true;
}

AutoPtr<Geometry>RefEnRecord::GetGeometry(bool&bSuc){
    if(pGeo==nullptr)
    {
         return refRecord->GetGeometry(bSuc);
    }
    else
    {
        bSuc=true;
        return pGeo->Clone();
    }
}
bool RefEnRecord::GetFieldValue(LONG FieldIndex,Variant&value)
{
    if(fels==NULL) return refRecord->GetFieldValue(FieldIndex,value);
    LONG nPos=fels->FindField(FieldIndex);
    if(nPos>=0)
    {
        value=fels->GetFieldValue(nPos);
        return true;
    }
    return refRecord->GetFieldValue(FieldIndex,value);
}
bool RefEnRecord::GetFieldValueAsString(LONG FieldIndex,string&value)
{
    if(fels==NULL) return refRecord->GetFieldValueAsString(FieldIndex,value);
    LONG nPos=fels->FindField(FieldIndex);
    if(nPos>=0)
    {
        value=fels->GetFieldValueAsString(nPos);
        return true;
    }
    return refRecord->GetFieldValueAsString(FieldIndex,value);
}
bool RefEnRecord::GetFieldValueAsDouble(LONG FieldIndex,double&value)
{
    if(fels==NULL) return refRecord->GetFieldValueAsDouble(FieldIndex,value);
    LONG nPos=fels->FindField(FieldIndex);
    if(nPos>=0)
    {
        value=fels->GetFieldValueAsDouble(nPos);
        return true;
    }
    return refRecord->GetFieldValueAsDouble(FieldIndex,value);
}
int RefEnRecord::BackwardDestroyInvalidData()
{
    int Size=EnRecord::BackwardDestroyInvalidData();
    Size+=refRecord->BackwardDestroyInvalidData();
    return Size;
}
int RefEnRecord::BackwardDestroyInvalidData(Geometry*refGeo,FieldEls*pEl)
{
    int Size=EnRecord::BackwardDestroyInvalidData(refGeo,pEl);
    Size+=refRecord->BackwardDestroyInvalidData(refGeo,pEl);
    return Size;
}
int RefEnRecord::DestroyBackwardInvalidData()
{
    return refRecord->BackwardDestroyInvalidData(pGeo.get(),fels.get());
}
VirtualRecords::VirtualRecords(FeatureClassVersion*pEdit,long fromPos,long toPos)
{
    FromPos=fromPos;
    ToPos=toPos;
    refEdit=pEdit;
}
VirtualRecords::~VirtualRecords()
{

}

bool VirtualRecords::IsVirtual()
{
    return true;
}
int VirtualRecords::GetRecordNumber()
{
    return ToPos-FromPos+1;
}
int VirtualRecords::GetFromPos()
{
    return FromPos;
}
int VirtualRecords::GetToPos()
{
    return ToPos;
}
int VirtualRecords::GetProximateSize()
{
    return 24;
}

LONG VirtualRecords::GetFeatureID(LONG FatherIndex)
{
    if(refEdit==NULL) return FatherIndex;
    return refEdit->GetFeatureID(FatherIndex);
}
bool VirtualRecords::GetEnvelope(LONG FatherIndex,Envelope*pEnv)
{
    if(refEdit==NULL) return false;
    return refEdit->GetEnvelope(FatherIndex,pEnv);
}
AutoPtr<Geometry>VirtualRecords::GetGeometry(LONG FatherIndex,bool&bSuc){
    if(refEdit==nullptr){
        bSuc=false;
        return nullptr;
    }
    return refEdit->GetGeometry(FatherIndex,bSuc);
}
bool VirtualRecords::GetFieldValue(LONG FatherIndex,LONG FieldIndex,Variant&value)
{
    if(refEdit==nullptr) return false;
    return refEdit->GetFieldValue(FatherIndex,FieldIndex,value);
}
bool VirtualRecords::GetFieldValueAsString(LONG FatherIndex,LONG FieldIndex,string&value)
{
    if(refEdit==NULL) return false;
    return refEdit->GetFieldValueAsString(FatherIndex,FieldIndex,value);
}
bool VirtualRecords::GetFieldValueAsDouble(LONG FatherIndex,LONG FieldIndex,double&value)
{
    if(refEdit==NULL) return false;
    return refEdit->GetFieldValueAsDouble(FatherIndex,FieldIndex,value);
}
bool VirtualRecords::CreateBreak(LONG BreakPos,AutoPtr<VirtualRecords>*pFrom,AutoPtr<VirtualRecords>*pTo)
{
   *pFrom=nullptr;
   *pTo=nullptr;
   if((BreakPos<FromPos)||(BreakPos>ToPos))
   {
       return false;
   }
   if(FromPos==ToPos) return false;
   if(BreakPos>FromPos)
   {
        *pFrom=new VirtualRecords(refEdit,FromPos,BreakPos-1);
   }
   if(BreakPos<ToPos)
   {
        *pTo=new VirtualRecords(refEdit,BreakPos+1,ToPos);
   }
   return true;
}
int VirtualRecords::BackwardDestroyInvalidData(LONG FatherIndex)
{
    if(refEdit!=NULL) return refEdit->BackwardDestroyInvalidData(FatherIndex);
    return 0;
}
int VirtualRecords::BackwardDestroyInvalidData(LONG FatherIndex,Geometry*refGeo,FieldEls*pEl)
{
    if(refEdit!=NULL) return refEdit->BackwardDestroyInvalidData(FatherIndex,refGeo,pEl);
    return 0;
}
FeatureClassVersion::FeatureClassVersion(VectorShapeType vType,AutoPtr<Envelope>env,AutoPtr<TableDesc>desc,AutoPtr<SpatialReference>psp)
{
    ProximateSize=0;
    RecordNumber=0;
    currentPin.Pin=-1;
    currentPin.RecordNum=-1;
    currentPin.DifNumber=0;
    pEnv=env->Clone();
    EditChanged=false;
    shapeType=vType;
    pDesc=desc;
    this->psp=psp;
}

FeatureClassVersion::~FeatureClassVersion()
{
    for(int k=pRecords.size()-1;k>=0;k--)
    {
        pRecords[k]=nullptr;
    }
}

LONG FeatureClassVersion::ComputeReleaseProximateSize()
{
    LONG Size=0;
    for(int k=pRecords.size()-1;k>=0;k--)
    {
        if(pRecords[k]==nullptr) continue;
        if(pRecords[k].user_count()==1) Size+=pRecords[k]->GetProximateSize();
    }
    Size+=pRecords.size()*4;
    return Size;
}
AutoPtr<Envelope>FeatureClassVersion::GetShapeEnvelope()
{
    return pEnv;
}
void FeatureClassVersion::ReleaseNoUsed()
{
    for(int k=pRecords.size()-1;k>=0;k--)
    {
        pRecords[k]=nullptr;
    }
}
LONG FeatureClassVersion::GetRecordNumber()
{
    return RecordNumber;
}

int FeatureClassVersion::GetFatherIndex(FeaturePin currentPin)
{
    AutoPtr<Records>pRecord=pRecords[currentPin.Pin];
    if(pRecord->IsVirtual())
    {
        AutoPtr<VirtualRecords>pv(pRecord);
        return pv->GetFromPos()+currentPin.DifNumber-1;
    }
    else
    {
       AutoPtr<EnRecord>pr(pRecord);
       return pr->GetFatherPos();
    }
}
bool FeatureClassVersion::MoveTo(int RecordNum,FeaturePin&featurePin)
{
    if(currentPin.RecordNum==RecordNum)
    {
        featurePin=currentPin;
        return true;
    }
    if((RecordNum<0)||(RecordNum>=RecordNumber))
    {
        return false;
    }
    if(RecordNumber==pRecords.size())
    {
        currentPin.Pin=RecordNum;
        currentPin.DifNumber=1;
        currentPin.RecordNum=RecordNum;
        featurePin=currentPin;
        return true;
    }
    if(currentPin.Pin==-1)
    {
        currentPin.Pin=0;
        currentPin.DifNumber=1;
        currentPin.RecordNum=0;
    }
    if(RecordNum==0)
    {
        currentPin.Pin=0;
        currentPin.DifNumber=1;
        currentPin.RecordNum=0;
        featurePin=currentPin;
        return true;
    }
    else if(RecordNum==RecordNumber-1)
    {
        currentPin.Pin=pRecords.size()-1;
        currentPin.DifNumber=pRecords[currentPin.Pin]->GetRecordNumber();
        currentPin.RecordNum=RecordNum;
        featurePin=currentPin;
        return true;
    }
    if(currentPin.RecordNum<RecordNum)
    {
        LONG num;
        while(true)
        {
            num=pRecords[currentPin.Pin]->GetRecordNumber()-currentPin.DifNumber;
            if(num==0)
            {
                currentPin.Pin++;
                currentPin.DifNumber=0;
                continue;
            }
            if(currentPin.RecordNum+num<RecordNum)
            {
                currentPin.Pin++;
                currentPin.DifNumber=0;
            }
            else
            {
                currentPin.DifNumber=RecordNum-currentPin.RecordNum+currentPin.DifNumber;
                currentPin.RecordNum=RecordNum;
                break;
            }
            currentPin.RecordNum+=num;
        }
    }
    else
    {
        LONG num;
        while(true)
        {
            num=currentPin.DifNumber;
            if(currentPin.RecordNum-num>=RecordNum)
            {
                currentPin.Pin--;
                currentPin.DifNumber=pRecords[currentPin.Pin]->GetRecordNumber();
            }
            else
            {
                currentPin.DifNumber=currentPin.DifNumber+RecordNum-currentPin.RecordNum;
                currentPin.RecordNum=RecordNum;
                break;
            }
            currentPin.RecordNum-=num;
        }
    }
    featurePin=currentPin;
    return true;
}
int FeatureClassVersion::BackwardDestroyInvalidData(LONG RecordIndex)
{
    FeaturePin featurePin;
    if(!MoveTo(RecordIndex,featurePin)) return 0;
    AutoPtr<Records>pRecord=pRecords[featurePin.Pin];
    bool IsVirtual=pRecord->IsVirtual();
    if(IsVirtual)
    {
        AutoPtr<VirtualRecords>r(pRecord);
        return r->BackwardDestroyInvalidData(r->GetFromPos()+featurePin.DifNumber-1);
    }
    else
    {
        AutoPtr<EnRecord>r(pRecord);
        return r->BackwardDestroyInvalidData();
    }
}
int FeatureClassVersion::BackwardDestroyInvalidData(LONG RecordIndex,Geometry*refGeo,FieldEls*pEl)
{
    FeaturePin currentPin;
    if(!MoveTo(RecordIndex,currentPin)) return 0;
    AutoPtr<Records>pRecord=pRecords[currentPin.Pin];
    bool IsVirtual=pRecord->IsVirtual();
    if(IsVirtual)
    {
        AutoPtr<VirtualRecords>r(pRecord);
        return r->BackwardDestroyInvalidData(r->GetFromPos()+currentPin.DifNumber-1,refGeo,pEl);
    }
    else
    {
        AutoPtr<EnRecord>r(pRecord);
        return r->BackwardDestroyInvalidData(refGeo,pEl);
    }
}
bool FeatureClassVersion::GetEnvelope(LONG RecordIndex,Envelope*pEnv)
{
    FeaturePin currentPin;
    if(!MoveTo(RecordIndex,currentPin)) return false;
    AutoPtr<Records>pRecord=pRecords[currentPin.Pin];
    while(true)
    {
        bool IsVirtual=pRecord->IsVirtual();
        if(IsVirtual)
        {
            AutoPtr<VirtualRecords>r(pRecord);
            if(r->refEdit==NULL) return false;
            RecordIndex=r->GetFromPos()+currentPin.DifNumber-1;
            if(!r->refEdit->MoveTo(RecordIndex,currentPin)) return false;
            pRecord=r->refEdit->pRecords[currentPin.Pin];
            continue;
        }
        else
        {
            AutoPtr<EnRecord>r(pRecord);
            if(r->IsOldEnRecord())
            {
                AutoPtr<OldEnRecord>er(r);
                if(er->pGeo==NULL)
                {
                    return false;
                }
                else
                {
                    AutoPtr<Envelope>env=er->pGeo->GetExtent();
                    pEnv->CopyFromOther(env.get());
                }
                return true;
            }
            else if(r->IsNewEnRecord())
            {
                AutoPtr<Envelope>env= r->pGeo->GetExtent();
                pEnv->CopyFromOther(env.get());
                return true;
            }
            else
            {
                AutoPtr<RefEnRecord>er(r);
                if(er->pGeo==NULL)
                {
                    pRecord=er->refRecord;
                    continue;
                 }
                 else
                 {
                     AutoPtr<Envelope>env= er->pGeo->GetExtent();
                     pEnv->CopyFromOther(env.get());
                 }
                 return true;
            }
        }
    }
    return false;
}

AutoPtr<Geometry>FeatureClassVersion::GetGeometry(LONG RecordIndex,bool&bSuc){
    FeaturePin currentPin;
    if(!MoveTo(RecordIndex,currentPin)){
        bSuc=false;
        return nullptr;
    }
    AutoPtr<Records>pRecord=pRecords[currentPin.Pin];
    while(true)
    {
        bool IsVirtual=pRecord->IsVirtual();
        if(IsVirtual)
        {
            AutoPtr<VirtualRecords>r(pRecord);
            if(r->refEdit==nullptr){
                bSuc=false;
                return nullptr;
            }
            RecordIndex=r->GetFromPos()+currentPin.DifNumber-1;
            if(!r->refEdit->MoveTo(RecordIndex,currentPin)){
                bSuc=false;
                return nullptr;
            }
            pRecord=r->refEdit->pRecords[currentPin.Pin];
            continue;
        }
        else
        {
            AutoPtr<EnRecord>r(pRecord);
            if(r->pGeo!=nullptr)
            {
                bSuc=true;
                return r->pGeo->Clone();
            }
            if(r->IsRefEnRecord())
            {
                 AutoPtr<RefEnRecord>rf(r);
                 pRecord=rf->refRecord;
                 bSuc=true;
                 continue;
            }
            else if(r->IsNewEnRecord())
            {
                bSuc=true;
                return nullptr;
            }
            bSuc=false;
            return nullptr;
        }
    }
}

bool FeatureClassVersion::GetFieldValue(LONG RecordIndex,LONG FieldIndex,Variant&value){
    FeaturePin currentPin;
    if(!MoveTo(RecordIndex,currentPin)) return false;
    AutoPtr<Records>pRecord=pRecords[currentPin.Pin];
    while(true)
    {
        bool IsVirtual=pRecord->IsVirtual();
        if(IsVirtual)
        {
            AutoPtr<VirtualRecords>r(pRecord);
            if(r->refEdit==NULL) return false;
            RecordIndex=r->GetFromPos()+currentPin.DifNumber-1;
            if(!r->refEdit->MoveTo(RecordIndex,currentPin)) return false;
            pRecord=r->refEdit->pRecords[currentPin.Pin];
            continue;
        }
        else
        {
            AutoPtr<EnRecord>r(pRecord);
            if(r->IsOldEnRecord())
            {
                AutoPtr<OldEnRecord>er(r);
                if(er->fels==NULL) return false;
                LONG nPos=er->fels->FindField(FieldIndex);
                if(nPos>=0)
                {
                    value=er->fels->GetFieldValue(nPos);
                    return true;
                }
                return false;
            }
            else if(r->IsNewEnRecord())
            {
                AutoPtr<NewEnRecord>er(r);
                if(er->fels==NULL)
                {
                    value.Clear();
                    return true;
                }
                LONG nPos=er->fels->FindField(FieldIndex);
                if(nPos>=0)
                {
                    value=er->fels->GetFieldValue(nPos);
                    return true;
                }
                value.Clear();
                return true;
            }
            else
            {
                AutoPtr<RefEnRecord>er(r);
                if(er->fels==NULL)
                {
                    pRecord=er->refRecord;
                    continue;
                }
                LONG nPos=er->fels->FindField(FieldIndex);
                if(nPos>=0)
                {
                    value=er->fels->GetFieldValue(nPos);
                    return true;
                }
                pRecord=er->refRecord;
                continue;
            }
        }
    }
}
bool FeatureClassVersion::GetFieldValueAsString(LONG RecordIndex,LONG FieldIndex,string&value)
{
    FeaturePin currentPin;
    if(!MoveTo(RecordIndex,currentPin)) return false;
    AutoPtr<Records>pRecord=pRecords[currentPin.Pin];
    VectorFieldType vType=this->pDesc->GetFieldType(FieldIndex);
    while(true)
    {
        bool IsVirtual=pRecord->IsVirtual();
        if(IsVirtual)
        {
            AutoPtr<VirtualRecords>r(pRecord);
            if(r->refEdit==NULL) return false;
            RecordIndex=r->GetFromPos()+currentPin.DifNumber-1;
            if(!r->refEdit->MoveTo(RecordIndex,currentPin)) return false;
            pRecord=r->refEdit->pRecords[currentPin.Pin];
            continue;
        }
        else
        {
            AutoPtr<EnRecord>r(pRecord);
            if(r->IsOldEnRecord())
            {
                AutoPtr<OldEnRecord>er(r);
                if(er->fels==NULL) return false;
                LONG nPos=er->fels->FindField(FieldIndex);
                if(nPos>=0)
                {
                    value=er->fels->GetFieldValueAsString(nPos);
                    if(value==""){
                        if(vType!=vftString) value="0";
                    }
                    return true;
                }
                return false;
            }
            else if(r->IsNewEnRecord())
            {
                AutoPtr<NewEnRecord>er(r);
                if(er->fels==NULL)
                {
                    if(vType!=vftString)
                        value="0";
                    else
                        value="";
                    return true;
                }
                LONG nPos=er->fels->FindField(FieldIndex);
                if(nPos>=0)
                {
                    value=er->fels->GetFieldValueAsString(nPos);
                    if(value==""){
                        if(vType!=vftString) value="0";
                    }
                    return true;
                }
                value="";
                return true;
            }
            else
            {
                AutoPtr<RefEnRecord>er(r);
                if(er->fels==NULL)
                {
                    pRecord=er->refRecord;
                    continue;
                }
                LONG nPos=er->fels->FindField(FieldIndex);
                if(nPos>=0)
                {
                    value=er->fels->GetFieldValueAsString(nPos);
                    if(value==""){
                        if(vType!=vftString) value="0";
                    }
                    return true;
                }
                pRecord=er->refRecord;
                continue;
            }
        }
    }
}
bool FeatureClassVersion::GetFieldValueAsDouble(LONG RecordIndex,LONG FieldIndex,double&value)
{
    FeaturePin currentPin;
    if(!MoveTo(RecordIndex,currentPin)) return false;
    AutoPtr<Records>pRecord=pRecords[currentPin.Pin];
    while(true)
    {
        bool IsVirtual=pRecord->IsVirtual();
        if(IsVirtual)
        {
            AutoPtr<VirtualRecords>r(pRecord);
            if(r->refEdit==NULL) return false;
            RecordIndex=r->GetFromPos()+currentPin.DifNumber-1;
            if(!r->refEdit->MoveTo(RecordIndex,currentPin)) return false;
            pRecord=r->refEdit->pRecords[currentPin.Pin];
            continue;
        }
        else
        {
            AutoPtr<EnRecord>r(pRecord);
            if(r->IsOldEnRecord())
            {
                AutoPtr<OldEnRecord>er(r);
                if(er->fels==NULL) return false;
                LONG nPos=er->fels->FindField(FieldIndex);
                if(nPos>=0)
                {
                    value=er->fels->GetFieldValueAsDouble(nPos);
                    return true;
                }
                return false;
            }
            else if(r->IsNewEnRecord())
            {
                AutoPtr<NewEnRecord>er(r);
                if(er->fels==NULL)
                {
                    value=0;
                    return true;
                }
                LONG nPos=er->fels->FindField(FieldIndex);
                if(nPos>=0)
                {
                    value=er->fels->GetFieldValueAsDouble(nPos);
                    return true;
                }
                value=0;
                return true;
            }
            else
            {
                AutoPtr<RefEnRecord>er(r);
                if(er->fels==NULL)
                {
                    pRecord=er->refRecord;
                    continue;
                }
                LONG nPos=er->fels->FindField(FieldIndex);
                if(nPos>=0)
                {
                    value=er->fels->GetFieldValueAsDouble(nPos);
                    return true;
                }
                pRecord=er->refRecord;
                continue;
            }
        }
    }
}
LONG FeatureClassVersion::GetFeatureID(LONG RecordIndex)
{
    FeaturePin currentPin;
    if(!MoveTo(RecordIndex,currentPin)) return 0;
    AutoPtr<Records>pRecord=pRecords[currentPin.Pin];
    while(true)
    {
        bool IsVirtual=pRecord->IsVirtual();
        if(IsVirtual)
        {
            AutoPtr<VirtualRecords>r(pRecord);
            RecordIndex=r->GetFromPos()+currentPin.DifNumber-1;
            if(r->refEdit==NULL) return RecordIndex;
            if(!r->refEdit->MoveTo(RecordIndex,currentPin)) return false;
            pRecord=r->refEdit->pRecords[currentPin.Pin];
            continue;
        }
        else
        {
            AutoPtr<EnRecord>r(pRecord);
            if(r->IsRefEnRecord())
            {
                 AutoPtr<RefEnRecord>rf(r);
                 if(rf->refRecord==NULL) return false;
                 pRecord=rf->refRecord;
                 continue;
            }
            else if(r->IsOldEnRecord())
            {
                AutoPtr<OldEnRecord>er(r);
                return er->FeatureID;
            }
            else
            {
                AutoPtr<NewEnRecord>er(r);
                return er->FeatureID;
            }
        }
    }
}
bool FeatureClassVersion::IsGeometryChanged(LONG RecordIndex)
{
    FeaturePin currentPin;
    if(!MoveTo(RecordIndex,currentPin)) return false;
    AutoPtr<Records>pRecord=pRecords[currentPin.Pin];
    while(true)
    {
        bool IsVirtual=pRecord->IsVirtual();
        if(IsVirtual)
        {
            AutoPtr<VirtualRecords>r(pRecord);
            if(r->refEdit==NULL) return false;
            pRecord = GetCascadeRecord(RecordIndex,RecordIndex);
            if(pRecord==NULL) return false;
            continue;
        }
        else
        {
            AutoPtr<EnRecord>r(pRecord);
            if(r->IsRefEnRecord())
            {
                 AutoPtr<RefEnRecord>rf(r);
                 if(rf->pGeo!=NULL) return true;
                 pRecord=rf->refRecord;
                 continue;
            }
            else if(r->IsOldEnRecord())
            {
                AutoPtr<OldEnRecord>er(r);
                return (er->pGeo!=NULL);
            }
            else
            {
                return true;
            }
        }
    }
}
bool FeatureClassVersion::IsFieldValueChanged(LONG RecordIndex)
{
    FeaturePin currentPin;
    if(!MoveTo(RecordIndex,currentPin)) return false;
    AutoPtr<Records>pRecord=pRecords[currentPin.Pin];
    while(true)
    {
        bool IsVirtual=pRecord->IsVirtual();
        if(IsVirtual)
        {
            AutoPtr<VirtualRecords>r(pRecord);
            if(r->refEdit==NULL) return false;
            pRecord = GetCascadeRecord(RecordIndex,RecordIndex);
            if(pRecord==NULL) return false;
            continue;
        }
        else
        {
            AutoPtr<EnRecord>r(pRecord);
            if(r->IsRefEnRecord())
            {
                 AutoPtr<RefEnRecord>rf(r);
                 if(rf->fels!=NULL) return true;
                 pRecord=rf->refRecord;
                 continue;
            }
            else if(r->IsOldEnRecord())
            {
                AutoPtr<OldEnRecord>er(r);
                return (er->fels!=NULL);
            }
            else
            {
                return true;
            }
        }
    }
    return false;
}
bool FeatureClassVersion::SetFeature(LONG RecordIndex,AutoPtr<Geometry>pGeo)
{
    FeaturePin featurePin;
    if(!MoveTo(RecordIndex,featurePin)) return false;
    LONG FatherIndex=GetFatherIndex(featurePin);
    AutoPtr<Records>pRecord=pRecords[featurePin.Pin];
    if(pRecord->IsVirtual())
    {
        AutoPtr<VirtualRecords>old(pRecord);
        AutoPtr<VirtualRecords>pLeft,pRight;
        old->CreateBreak(FatherIndex,&pLeft,&pRight);
        if(pLeft!=nullptr)
        {
            pRecords[featurePin.Pin]=pLeft;
            LONG FatherID=GetFeatureID(RecordIndex);
            AutoPtr<EnRecord>pRefRecord=nullptr;
            if(old->refEdit!=nullptr)	pRefRecord=old->refEdit->GetCascadeRecord(FatherIndex,FatherID);
            AutoPtr<EnRecord>pNew;
            if(pRefRecord==nullptr)
                pNew=new OldEnRecord(FatherID);
            else
                pNew=new RefEnRecord(pRefRecord,FatherIndex);
            pNew->SetFeature(pGeo);
            pRecords.insert(begin(pRecords)+featurePin.Pin+1,pNew);
            ProximateSize+=pNew->GetProximateSize()+4;
            if(pRight!=NULL)
            {
                ProximateSize+=pRight->GetProximateSize()+4;
                pRecords.insert(begin(pRecords)+featurePin.Pin+2,pRight);
            }
            featurePin.Pin++;
            featurePin.DifNumber=1;
            featurePin.RecordNum=RecordIndex;
        }
        else
        {
            ProximateSize-=old->GetProximateSize();
            LONG FatherID=GetFeatureID(RecordIndex);
            AutoPtr<EnRecord>pRefRecord=nullptr;
            if(old->refEdit!=nullptr)	pRefRecord=old->refEdit->GetCascadeRecord(FatherIndex,FatherID);
            AutoPtr<EnRecord>pNew;
            if(pRefRecord==nullptr)
                pNew=new OldEnRecord(FatherID);
            else
                pNew=new RefEnRecord(pRefRecord,FatherIndex);
            pNew->SetFeature(pGeo);
            ProximateSize+=pNew->GetProximateSize();
            pRecords[featurePin.Pin]=pNew;
            if(pRight!=NULL)
            {
                ProximateSize+=pRight->GetProximateSize()+4;
                pRecords.insert(begin(pRecords)+featurePin.Pin+1,pRight);
            }
            featurePin.DifNumber=1;
            featurePin.RecordNum=RecordIndex;
        }
        SetCurrentPin(featurePin);
    }
    else
    {
        AutoPtr<EnRecord>rRecord(pRecord);
        LONG Size=pRecord->GetProximateSize();
        rRecord->SetFeature(pGeo);
        LONG Size1=pRecord->GetProximateSize();
        ProximateSize+=Size1-Size;
    }
    return true;
}
void FeatureClassVersion::ValidFieldValue(int fieldIndex,Variant&value){
    pDesc->ValidFieldValue(fieldIndex,value);
}
bool FeatureClassVersion::SetFieldValue(LONG RecordIndex,LONG FieldIndex,string&newValue)
{
    Variant vt(newValue);
    ValidFieldValue(FieldIndex,vt);
    return SetFieldValue(RecordIndex,FieldIndex,vt);
}
bool FeatureClassVersion::SetFieldValue(LONG RecordIndex,LONG FieldIndex,Variant&newValue)
{
    FeaturePin featurePin;
    if(!MoveTo(RecordIndex,featurePin)) return false;
    ValidFieldValue(FieldIndex,newValue);
    LONG FatherIndex=GetFatherIndex(featurePin);
    AutoPtr<Records>pRecord=pRecords[featurePin.Pin];
    if(pRecord->IsVirtual())
    {
        AutoPtr<VirtualRecords>old(pRecord);
        AutoPtr<VirtualRecords>pLeft,pRight;
        old->CreateBreak(FatherIndex,&pLeft,&pRight);
        if(pLeft!=nullptr)
        {
            pRecords[featurePin.Pin]=pLeft;
            LONG FatherID=GetFeatureID(RecordIndex);
            AutoPtr<EnRecord>pRefRecord=nullptr;
            if(old->refEdit!=nullptr)	pRefRecord=old->refEdit->GetCascadeRecord(FatherIndex,FatherID);
            AutoPtr<EnRecord>pNew;
            if(pRefRecord==nullptr)
                pNew=new OldEnRecord(FatherID);
            else
                pNew=new RefEnRecord(pRefRecord,FatherIndex);
            pNew->SetFieldValue(FieldIndex,newValue);
            pRecords.insert(begin(pRecords)+featurePin.Pin+1,pNew);
            ProximateSize+=pNew->GetProximateSize()+4;
            if(pRight!=NULL)
            {
                ProximateSize+=pRight->GetProximateSize()+4;
                pRecords.insert(begin(pRecords)+featurePin.Pin+2,pRight);
            }
            featurePin.Pin++;
            featurePin.DifNumber=1;
            featurePin.RecordNum=RecordIndex;
        }
        else
        {
            ProximateSize-=old->GetProximateSize();
            LONG FatherID=GetFeatureID(RecordIndex);
            AutoPtr<EnRecord>pRefRecord=nullptr;
            if(old->refEdit!=NULL)	pRefRecord=old->refEdit->GetCascadeRecord(FatherIndex,FatherID);
            AutoPtr<EnRecord>pNew;
            if(pRefRecord==NULL)
                pNew=new OldEnRecord(FatherID);
            else
                pNew=new RefEnRecord(pRefRecord,FatherIndex);
            pNew->SetFieldValue(FieldIndex,newValue);
            ProximateSize+=pNew->GetProximateSize();
            pRecords[featurePin.Pin]=pNew;
            if(pRight!=NULL)
            {
                ProximateSize+=pRight->GetProximateSize()+4;
                pRecords.insert(begin(pRecords)+featurePin.Pin+1,pRight);
            }
            featurePin.DifNumber=1;
            featurePin.RecordNum=RecordIndex;
        }
        SetCurrentPin(featurePin);
    }
    else
    {
        AutoPtr<EnRecord>rRecord(pRecord);
        LONG Size=pRecord->GetProximateSize();
        rRecord->SetFieldValue(FieldIndex,newValue);
        LONG Size1=pRecord->GetProximateSize();
        ProximateSize+=Size1-Size;
    }
    return true;
}
bool FeatureClassVersion::AddFeature(AutoPtr<Geometry>pGeo)
{
    GeometryType gType=pGeo->GetType();
    if(shapeType==vstPoint)
    {
        if(gType!=gtPoint) return false;
    }
    else if(shapeType==vstPolyline)
    {
        if(gType!=gtPolyline) return false;
    }
    else if(shapeType==vstPolygon)
    {
        if((gType!=gtPolygon)&&(gType!=gtMultiPolygon)) return false;
    }
    AutoPtr<Envelope>pNew=pGeo->GetExtent();
    pEnv->UnionOther(pNew.get());
    AutoPtr<NewEnRecord>pNewEl(new NewEnRecord(pGeo,ID++));
    ProximateSize+=pNewEl->GetProximateSize()+4;
    pRecords.push_back(pNewEl);
    RecordNumber++;
    return true;
}

string FeatureClassVersion::GetName()
{
    return Name;
}
void FeatureClassVersion::SetName(string name)
{
    Name=name;
}
void FeatureClassVersion::SetCurrentPin(FeaturePin featurePin)
{
    currentPin=featurePin;
}
AutoPtr<EnRecord>FeatureClassVersion::GetCascadeRecord(int RecordIndex,LONG&FatherIndex)
{
    FeaturePin currentPin;
    MoveTo(RecordIndex,currentPin);
    AutoPtr<Records>pRecord=pRecords[currentPin.Pin];
    if(pRecord->IsVirtual())
    {
        AutoPtr<VirtualRecords>r(pRecord);
        FatherIndex=r->GetFromPos()+currentPin.DifNumber-1;
        if(r->refEdit==NULL) return nullptr;
        return r->refEdit->GetCascadeRecord(FatherIndex,FatherIndex);
    }
    else
        return pRecord;
}
AutoPtr<Records>FeatureClassVersion::GetItem(int index)
{
    return pRecords[index];
}
LONG FeatureClassVersion::GetItemCount()
{
    return pRecords.size();
}
bool FeatureClassVersion::AddFeature(AutoPtr<Geometry>pGeo,const vector<Variant>&newValues)
{
    GeometryType gType=pGeo->GetType();
    if(shapeType==vstPoint)
    {
        if(gType!=gtPoint) return false;
    }
    else if(shapeType==vstPolyline)
    {
        if(gType!=gtPolyline) return false;
    }
    else if(shapeType==vstPolygon)
    {
        if((gType!=gtPolygon)&&(gType!=gtMultiPolygon)) return false;
    }
    AutoPtr<Envelope>pNew=pGeo->GetExtent();
    pEnv->UnionOther(pNew.get());
    AutoPtr<NewEnRecord>pNewEl(new NewEnRecord(pGeo,ID++));
    int Size=newValues.size();
    for(int k=0;k<Size;k++)
    {
        Variant vt(newValues[k]);
        ValidFieldValue(k,vt);
        pNewEl->SetFieldValue(k,vt);
    }
    ProximateSize+=pNewEl->GetProximateSize()+4;
    pRecords.push_back(pNewEl);
    RecordNumber++;
    return true;
}
bool FeatureClassVersion::DeleteFeature(LONG RecordIndex)
{
    FeaturePin featurePin;
    if(!MoveTo(RecordIndex,featurePin)) return false;
    AutoPtr<Records>pRecord=pRecords[featurePin.Pin];
    if(pRecord->IsVirtual())
    {
        AutoPtr<VirtualRecords>old(pRecord);
        AutoPtr<VirtualRecords>pLeft,pRight;
        int breakPos=old->GetFromPos()+featurePin.DifNumber -1;
        old->CreateBreak(breakPos,&pLeft,&pRight);
        if(pLeft!=nullptr)
        {
            pRecords[featurePin.Pin]=pLeft;
            if(pRight!=NULL)
            {
                ProximateSize+=pRight->GetProximateSize()+4;
                pRecords.insert(begin(pRecords)+featurePin.Pin+1,pRight);
                featurePin.Pin++;
                featurePin.DifNumber=1;
                featurePin.RecordNum=RecordIndex;
            }
            else
            {
                if(RecordIndex>=RecordNumber-1)
                {
                    featurePin.DifNumber=pLeft->GetRecordNumber();
                    featurePin.RecordNum=RecordIndex-1;
                }
                else
                {
                    featurePin.Pin++;
                    featurePin.DifNumber=1;
                    featurePin.RecordNum=RecordIndex;
                }
            }
        }
        else
        {
            ProximateSize-=old->GetProximateSize();
            if(pRight!=NULL)
            {
                ProximateSize+=pRight->GetProximateSize();
                pRecords[featurePin.Pin]=pRight;
                featurePin.DifNumber=1;
                featurePin.RecordNum=RecordIndex;
            }
            else
            {
                pRecords.erase(begin(pRecords)+featurePin.Pin);
                ProximateSize-=4;
                if(RecordIndex<RecordNumber-1)
                {
                    featurePin.DifNumber=1;
                    featurePin.RecordNum=RecordIndex;
                }
                else if(pRecords.size()>0)
                {
                    featurePin.Pin--;
                    featurePin.DifNumber=pRecords[featurePin.Pin]->GetRecordNumber();
                    featurePin.RecordNum=RecordIndex-1;
                }
                else
                {
                    featurePin.Pin=-1;
                    featurePin.DifNumber=0;
                    featurePin.RecordNum=-1;
                }
            }
        }
    }
    else
    {
        pRecords.erase(begin(pRecords)+featurePin.Pin);
        ProximateSize-=4;
        if(RecordIndex<RecordNumber-1)
        {
            featurePin.DifNumber=1;
            featurePin.RecordNum=RecordIndex;
        }
        else if(pRecords.size()>0)
        {
            featurePin.Pin--;
            featurePin.DifNumber=pRecords[featurePin.Pin]->GetRecordNumber();
            featurePin.RecordNum=RecordIndex-1;
        }
        else
        {
            featurePin.Pin=-1;
            featurePin.DifNumber=0;
            featurePin.RecordNum=-1;
        }
    }
    SetCurrentPin(featurePin);
    RecordNumber--;
    return true;
}
void FeatureClassVersion::DeleteAllFeatures()
{
    RecordNumber=0;
    for(int k=pRecords.size()-1;k>=0;k--) pRecords[k]=nullptr;
    pRecords.clear();
    currentPin.Pin=-1;
    currentPin.DifNumber=0;
    currentPin.RecordNum=-1;
    ProximateSize=0;
}
void FeatureClassVersion::Initialize(LONG recordNumber)
{
    for(int k=pRecords.size()-1;k>=0;k--) pRecords[k]=nullptr;
    pRecords.clear();
    RecordNumber=recordNumber;
    if(RecordNumber>0)
    {
        AutoPtr<VirtualRecords>pRecord(new VirtualRecords(NULL,0,RecordNumber-1));
        ProximateSize=pRecord->GetProximateSize()+4;
        pRecords.push_back(pRecord);
    }
    ID=RecordNumber;
}
AutoPtr<FeatureClassVersion>FeatureClassVersion::Clone()
{
     FeatureClassVersion*pEdit=new FeatureClassVersion(shapeType,pEnv,pDesc,psp);
     pEdit->RecordNumber=RecordNumber;
     if(RecordNumber>0)
     {
         AutoPtr<VirtualRecords>pRecord(new VirtualRecords(this,0,RecordNumber-1));
         pEdit->ProximateSize=pRecord->GetProximateSize()+4;
         pEdit->pRecords.push_back(pRecord);
     }
     pEdit->ID=ID;
     return pEdit;
}
AutoPtr<FeatureClassVersion>FeatureClassVersion::Copy()
{
    FeatureClassVersion*pEdit=new FeatureClassVersion(shapeType,pEnv,pDesc,psp);
    int recSize=pRecords.size();
    for(int k=0;k<recSize;k++)
    {
        pEdit->pRecords.push_back(pRecords[k]);
    }
    pEdit->currentPin.Pin=currentPin.Pin;
    pEdit->currentPin.RecordNum=currentPin.RecordNum;
    pEdit->currentPin.DifNumber=currentPin.DifNumber;
    pEdit->ID=ID;
    pEdit->Name=Name;
    pEdit->EditChanged=EditChanged;
    pEdit->ProximateSize=ProximateSize;
    pEdit->RecordNumber=RecordNumber;
    return pEdit;
}
FeatureClassEditor::FeatureClassEditor()
{
     currentPin.Pin=-1;
     HistoryCount=500;
     RemainEditsMB=0;
     editChanged=false;
     CurrentEdit=nullptr;
}
FeatureClassEditor::~FeatureClassEditor()
{
     for(int k=pEdits.size()-1;k>=0;k--)
     {
         pEdits[k]=nullptr;
     }
     pEdits.clear();
}
void FeatureClassEditor::SetHistoryCount(int Count)
{
    if(Count<2) Count=2;
    if(Count>1000) Count=1000;
    HistoryCount=Count;
}

LONG FeatureClassEditor::GetFeatureID(LONG RecordIndex)
{
    if(CurrentEdit==NULL) return -1;
    return CurrentEdit->GetFeatureID(RecordIndex);
}
LONG FeatureClassEditor::GetRecordNumber()
{
    if(CurrentEdit==NULL) return -1;
    return CurrentEdit->GetRecordNumber();
}
AutoPtr<Geometry>FeatureClassEditor::GetGeometry(LONG RecordIndex,bool&bSuc)
{
    if(CurrentEdit==NULL){
        bSuc=false;
        return nullptr;
    }
    return CurrentEdit->GetGeometry(RecordIndex,bSuc);
}
bool FeatureClassEditor::GetEnvelope(LONG RecordIndex,Envelope*pEnv)
{
     if(CurrentEdit==nullptr) return false;
     return CurrentEdit->GetEnvelope(RecordIndex,pEnv);
}
bool FeatureClassEditor::GetFieldValue(int RecordIndex,int index,Variant&value)
{
    if(CurrentEdit==nullptr) return false;
    return CurrentEdit->GetFieldValue(RecordIndex,index,value);
}
bool FeatureClassEditor::GetFieldValueAsString(int RecordIndex,int index,string&value)
{
    if(CurrentEdit==nullptr) return false;
    return CurrentEdit->GetFieldValueAsString(RecordIndex,index,value);
}
bool FeatureClassEditor::GetFieldValueAsDouble(int RecordIndex,int index,double&value)
{
    if(CurrentEdit==nullptr) return false;
    return CurrentEdit->GetFieldValueAsDouble(RecordIndex,index,value);
}
LONG FeatureClassEditor::BackwardDestroyInvalidData()
{
    if(pEdits.size()<2) return 0;
    AutoPtr<FeatureClassVersion>pEdit=pEdits[1];
    AutoPtr<FeatureClassVersion>pEdit0=pEdits[0];
    LONG RecordNum0=pEdit0->GetRecordNumber();
    int Pos=0;
    int Size=pEdit->GetItemCount();
    int MSize=0;
    for(int k=0;k<Size;k++)
    {
        AutoPtr<Records>pRecord=pEdit->GetItem(k);
        bool IsVirtual=pRecord->IsVirtual();
        if(IsVirtual)
        {
            AutoPtr<VirtualRecords>r(pRecord);
            int fromPos=r->GetFromPos();
            int toPos=r->GetToPos();
            for(int j=Pos;j<fromPos;j++) MSize+=pEdit->BackwardDestroyInvalidData(j);
            Pos=toPos+1;
        }
        else
        {
            AutoPtr<EnRecord>r(pRecord);
            int fromPos=r->GetFatherPos();
            if(fromPos==-1) break;
            for(int j=Pos;j<fromPos;j++) MSize+=pEdit->BackwardDestroyInvalidData(j);
            MSize+=r->DestroyBackwardInvalidData();
            Pos=fromPos+1;
        }
    }
    for(int k=Pos;k<RecordNum0;k++)
    {
        MSize+=pEdit->BackwardDestroyInvalidData(k);
    }
    return MSize;
}
void FeatureClassEditor::BeginEdit(FeatureClass*pfc)
{
    for(int k=pEdits.size()-1;k>=0;k--)
     {
         pEdits[k]=nullptr;
     }
     pEdits.clear();
     VectorShapeType vType=pfc->GetShapeType();
     AutoPtr<Envelope>pEnv=pfc->GetExtent();
     if(pEnv==nullptr) pEnv=AutoPtr<Envelope>(new Envelope());
     AutoPtr<TableDesc>pDesc=pfc->GetTableDesc();
     AutoPtr<TableDesc>newDesc=pDesc->Clone();
     AutoPtr<SpatialReference>sp=pfc->GetSpatialReference();
     AutoPtr<SpatialReference>psp;
     if(sp==nullptr)
         psp=AutoPtr<SpatialReference>(new SpatialReference());
     else
     {
         psp=sp->Clone();
     }
     AutoPtr<FeatureClassVersion>pEdit(new FeatureClassVersion(vType,pEnv,newDesc,psp));
     LONG Count=pfc->GetFeatureCount();
     originRecordCount=Count;
     pEdit->Initialize(Count);
     pEdits.push_back(pEdit);
     currentPin.Pin=0;
     CurrentEdit=pEdit;
     RemainEditsMB=0;
}
AutoPtr<Envelope>FeatureClassEditor::GetShapefileEnvelope()
{
    if(CurrentEdit==NULL) return nullptr;
    return CurrentEdit->GetShapeEnvelope();
}
bool FeatureClassEditor::AddFeature(AutoPtr<Geometry>pGeo)
{
    if(CurrentEdit==NULL) return false;
    if(currentPin.Pin<pEdits.size()-1) RegisterNewVersion("增加特征",true);
    return CurrentEdit->AddFeature(pGeo);
}

bool FeatureClassEditor::AddFeature(AutoPtr<Geometry>pGeo,const vector<Variant>&newValues)
{
    if(CurrentEdit==nullptr) return false;
    if(currentPin.Pin<pEdits.size()-1) RegisterNewVersion("增加特征",true);
    return CurrentEdit->AddFeature(pGeo,newValues);
}
bool FeatureClassEditor::DeleteFeature(LONG RecordIndex)
{
    if(CurrentEdit==NULL) return false;
    if(currentPin.Pin<pEdits.size()-1) RegisterNewVersion("删除特征",true);
    return CurrentEdit->DeleteFeature(RecordIndex);
}
bool FeatureClassEditor::SetFeature(LONG RecordIndex,AutoPtr<Geometry>pGeo)
{
    if(CurrentEdit==NULL) return false;
    if(currentPin.Pin<pEdits.size()-1) RegisterNewVersion("修改特征",true);
    return CurrentEdit->SetFeature(RecordIndex,pGeo);
}
bool FeatureClassEditor::SetFieldValue(LONG RecordIndex,LONG FieldIndex,Variant&newValue){
    if(CurrentEdit==NULL) return false;
    if(currentPin.Pin<pEdits.size()-1) RegisterNewVersion("设置字段值",true);
    return CurrentEdit->SetFieldValue(RecordIndex,FieldIndex,newValue);
}
bool FeatureClassEditor::SetFieldValue(LONG RecordIndex,LONG FieldIndex,string&newValue)
{
    if(CurrentEdit==NULL) return false;
    if(currentPin.Pin<pEdits.size()-1) RegisterNewVersion("设置字段值",true);
    return CurrentEdit->SetFieldValue(RecordIndex,FieldIndex,newValue);
}
bool FeatureClassEditor::DeleteAllFeatures()
{
    if(CurrentEdit==NULL) return false;
    if(currentPin.Pin<pEdits.size()-1) RegisterNewVersion("删除所有记录",true);
    CurrentEdit->DeleteAllFeatures();
    return true;
}
bool FeatureClassEditor::DeleteSelFeatures(FeatureClass*pfc,CallBack*callBack)
{
    AutoPtr<SelectionSet>pSelectionSet=pfc->GetSelectionSet();
    LONG Count=pSelectionSet->GetSize();
    if(Count==0) return false;
    LONG FromIndex=0;
    LONG FeatureCount=GetRecordNumber();
    LONG CurrentFID=pSelectionSet->GetFeatureID(FromIndex);
    vector<LONG>FIDs;
    AutoPtr<Progress>pro=AutoPtr<CallBack>(callBack,true);
    CallBackHelper::BeginProgress(callBack,"删除选择集");
    for(int k=0;k<FeatureCount;k++)
    {
        CallBackHelper::SendProgress(callBack,(float)k/FeatureCount*100);
        LONG FID=GetFeatureID(k);
        if(FID<CurrentFID) continue;
        if(FID==CurrentFID) FIDs.push_back(k);
        FromIndex++;
        if(FromIndex>=Count) continue;
        CurrentFID=pSelectionSet->GetFeatureID(FromIndex);
    }
    pSelectionSet->Clear();
    LONG Size=FIDs.size();
    for(int k=Size-1;k>=0;k--) DeleteFeature(FIDs[k]);
    CallBackHelper::EndProgress(callBack);
    return true;
}
bool FeatureClassEditor::CalculateField(FeatureClass*pfc,LONG TargetFieldIndex,string Expression,CallBack*callBack)
{
    AutoPtr<TableDesc>pDesc=pfc->GetTableDesc();
    AutoPtr<VirtualTable>pvTable(new VirtualTable());
    LONG FieldCount=pDesc->GetFieldCount();
    Variant oV;
    for(int k=0;k<FieldCount;k++)
    {
        string FieldName=pDesc->GetFieldName(k);
        VectorFieldType ft=pDesc->GetFieldType(k);
        ExpressionDataType dt;
        switch(ft)
        {
        case vftInteger:
            dt=exInt;
            break;
        case vftReal:
            dt=exNum;
            break;
        case vftString:
        case vftBinary:
            dt=exStr;
            break;
        case vftDate:
        case vftTime:
        case vftDateTime:
            dt=exDat;
            break;
        }
        pvTable->AddField(FieldName,dt);
    }
    AutoPtr<StringInterpreter>pInterpreter(new StringInterpreter());
    pInterpreter->SetVirtualTable(pvTable);
    bool IsOk=pInterpreter->InputExpressionString(Expression);
    if(!IsOk) return false;
    vector<int>UsableFieldIndexs;
    for(int k=0;k<FieldCount;k++)
    {
        IsOk=pvTable->GetFieldUsing(k);
        if(IsOk) UsableFieldIndexs.push_back(k);
    }
    LONG FeatureCount=GetRecordNumber();
    AutoPtr<SelectionSet>pSelectionSet=pfc->GetSelectionSet();
    CallBackHelper::BeginProgress(callBack,"字段计算");
    bool OnlySel=true;
    LONG SelCount=pSelectionSet->GetSize();
    if(SelCount==0) OnlySel=false;
    for(int k=0;k<FeatureCount;k++)
    {
        CallBackHelper::SendProgress(callBack,(float)k/FeatureCount*100);
        if(OnlySel)
        {
            LONG FID=this->GetFeatureID(k);
            IsOk=pSelectionSet->FindFeatureID(FID);
            if(!IsOk) continue;
        }
        for(int j=UsableFieldIndexs.size()-1;j>=0;j--)
        {
            oV=pfc->GetFieldValue(k,UsableFieldIndexs[j]);
            pvTable->SetFieldValue(UsableFieldIndexs[j],oV);
        }
        pInterpreter->ReturnValue(oV);
        if(oV.vt==VRT_EMPTY) continue;
        SetFieldValue(k,TargetFieldIndex,oV);
    }
    return true;
}
string FeatureClassEditor::GetCurrentUndoName()
{
    return CurrentEdit->GetName();
}
string FeatureClassEditor::GetCurrentRedoName()
{
    if(currentPin.Pin<pEdits.size()-1)
        return pEdits[currentPin.Pin+1]->GetName();
    else
        return "";
}
bool FeatureClassEditor::SetCurrentUndoName(string name){
    if(currentPin.Pin<pEdits.size()-1){
        pEdits[currentPin.Pin+1]->SetName(name);
        return true;
    }
    return false;
}
bool FeatureClassEditor::SetCurrentRedoName(string name){
    if(currentPin.Pin<pEdits.size()-1){
        pEdits[currentPin.Pin+1]->SetName(name);
        return true;
    }
    return false;
}
bool FeatureClassEditor::ClipForewardAllRedos()
{
    int Size=pEdits.size();
    if(currentPin.Pin>=Size-1) return false;
    for(int k=Size-1;k>currentPin.Pin;k--)
    {
        pEdits[k]=nullptr;
        pEdits.erase(begin(pEdits)+k);
    }
    return true;
}

AutoPtr<CoordinateTransformation>FeatureClassEditor::GetLayersCoordTansform(FeatureClass*pfc,Display*pDisplay)
{
    AutoPtr<SpatialReference>psplys=pDisplay->GetSpatialReference();
    AutoPtr<SpatialReference>psply=pfc->GetSpatialReference();
    CoordinateTransformation*pCoordTrans=new CoordinateTransformation();
    pCoordTrans->SetFromSpatialReference(psply);
    pCoordTrans->SetToSpatialReference(psplys);
    pCoordTrans->BeginTransform();
    return pCoordTrans;
}
AutoPtr<CoordinateTransformation>FeatureClassEditor::GetLayerCoordTansform(FeatureClass*pfc,Display*pDisplay)
{
    AutoPtr<SpatialReference>psplys=pDisplay->GetSpatialReference();
    AutoPtr<SpatialReference>psply=pfc->GetSpatialReference();
    CoordinateTransformation*pCoordTrans=new CoordinateTransformation();
    pCoordTrans->SetFromSpatialReference(psplys);
    pCoordTrans->SetToSpatialReference(psply);
    pCoordTrans->BeginTransform();
    return pCoordTrans;
}

bool FeatureClassEditor::MoveSelection(FeatureClass*pfc,DOUBLE difx,DOUBLE dify,CallBack*callBack)
{
    AutoPtr<SelectionSet>pSet=pfc->GetSelectionSet();
    LONG selCount=pSet->GetSize();
    if(selCount==0) return false;
    bool IsSuc=false;
    CallBackHelper::BeginProgress(callBack,"移动选择要素");
    FeatureClassTool pTool(pfc);
    for(int k=0;k<selCount;k++)
    {
        LONG selID=pSet->GetFeatureID(k);
        LONG index=pTool.FindFeatureID(selID);
        if(index==-1) continue;
        bool bSuc;
        AutoPtr<Geometry>pGeo=GetGeometry(index,bSuc);
        pGeo->Move(difx,dify);
        if(SetFeature(index,pGeo)) IsSuc=true;
        CallBackHelper::SendProgress(callBack,(FLOAT)k*100.0/selCount);
    }
    CallBackHelper::EndProgress(callBack);
    return IsSuc;
}
bool FeatureClassEditor::MoveSelectionByClientCoord(FeatureClass*pfc,DOUBLE difx,DOUBLE dify,Display*pDisplay,CallBack*callBack)
{
    AutoPtr<SelectionSet>pSet=pfc->GetSelectionSet();
    LONG selCount=pSet->GetSize();
    if(selCount==0) return false;
    AutoPtr<CoordinateTransformation>pTrans=GetLayersCoordTansform(pfc,pDisplay);
    AutoPtr<CoordinateTransformation>pTrans2=GetLayerCoordTansform(pfc,pDisplay);
    bool IsOk;
    bool IsSuc=false;
    CallBackHelper::BeginProgress(callBack,"移动选择要素");
    FeatureClassTool pTool(pfc);
    for(int k=0;k<selCount;k++)
    {
        LONG selID=pSet->GetFeatureID(k);
        LONG index=pTool.FindFeatureID(selID);
        if(index==-1) continue;
        AutoPtr<Geometry>pGeo=GetGeometry(index,IsSuc);
        pGeo->Project(pTrans.get());
        pDisplay->FromMapGeometry(pGeo.get());
        pGeo->Move(difx,dify);
        pDisplay->ToMapGeometry(pGeo.get());
        IsSuc=pGeo->Project(pTrans2.get());
        if(SetFeature(index,pGeo)) IsSuc=true;
        CallBackHelper::SendProgress(callBack,(FLOAT)k*100.0/selCount);
    }
    CallBackHelper::EndProgress(callBack);
    return IsSuc;
}
int FeatureClassEditor::UsableUndoTimes()
{
    return currentPin.Pin;
}
int FeatureClassEditor::UsableRedoTimes()
{
    return pEdits.size()-1-currentPin.Pin;
}
bool FeatureClassEditor::UndoEdit()
{
    if(currentPin.Pin<=0) return false;
    currentPin.Pin--;
    CurrentEdit=pEdits[currentPin.Pin];
    return true;
}
bool FeatureClassEditor::RedoEdit()
{
    if(currentPin.Pin>=pEdits.size()-1) return false;
    currentPin.Pin++;
    CurrentEdit=pEdits[currentPin.Pin];
    return true;
}
bool FeatureClassEditor::SaveEdit(FeatureClass*pfc,CallBack*callBack)
{
    AutoPtr<FeatureClassEdit>pEdit=AutoPtr<FeatureClass>(pfc,true);
    if(pEdit==nullptr) return false;
    CallBackHelper::BeginProgress(callBack,"保存编辑");
    AutoPtr<TableDesc>pDesc=pfc->GetTableDesc();
    LONG fieldCount=pDesc->GetFieldCount();
    AutoPtr<FeatureClassVersion>pCurEdit=CurrentEdit;
    LONG recordNum=pCurEdit->GetRecordNumber();
    CurrentEdit=nullptr;
    LONG oldFeatureCount=pfc->GetFeatureCount();
    LONG addedFeatureCount=oldFeatureCount;
    bool IsOk;
    int formper=0;
    for(int k=0;k<recordNum;k++)
    {
        LONG featureID=pCurEdit->GetFeatureID(k);
        bool IsGeometryChanged=pCurEdit->IsGeometryChanged(k);
        bool IsFieldValueChanged=pCurEdit->IsFieldValueChanged(k);
        if(featureID>=addedFeatureCount)
        {
            bool bSuc;
            AutoPtr<Geometry>geo=pCurEdit->GetGeometry(k,bSuc);
            if(geo==nullptr) continue;
            if(!pEdit->AddFeature(geo))
            {
                CallBackHelper::EndProgress(callBack);
                BeginEdit(pfc);
                return false;
            }
            string sValue;
            for(int j=0;j<fieldCount;j++)
            {
                if(pCurEdit->GetFieldValueAsString(k,j,sValue))
                {
                    if(!pEdit->SetFieldValue(addedFeatureCount,j,sValue))
                    {
                        CallBackHelper::EndProgress(callBack);
                        BeginEdit(pfc);
                        return false;
                    }
                }
            }
            addedFeatureCount++;
        }
        else
        {
            bool bSuc;
            AutoPtr<Geometry>geo=pCurEdit->GetGeometry(k,bSuc);
            if(geo!=NULL)
            {
                if(!pEdit->SetFeature(featureID,geo))
                {
                    CallBackHelper::EndProgress(callBack);
                    BeginEdit(pfc);
                    return false;
                }
            }
            if(IsFieldValueChanged)
            {
                string sValue;
                for(int j=0;j<fieldCount;j++)
                {
                    if(pCurEdit->GetFieldValueAsString(k,j,sValue))
                    {
                        IsOk=pEdit->SetFieldValue(featureID,j,sValue);
                        if(!IsOk)
                        {
                             CallBackHelper::EndProgress(callBack);
                            BeginEdit(pfc);
                            return false;
                        }
                    }
                }
            }
        }
        if(callBack!=nullptr)
        {
            int per=(addedFeatureCount-oldFeatureCount)*100.0/(recordNum+oldFeatureCount);
            if(formper!=per)
            {
               CallBackHelper::SendProgress(callBack,per);
               formper=per;
            }
        }
    }
    LONG curfid=-1;
    int difNum=0;
    for(int k=0;k<recordNum;k++)
    {
        LONG featureID=pCurEdit->GetFeatureID(k);
        if(featureID>=oldFeatureCount) break;
        if(featureID>curfid+1)
        {
            for(int p=curfid+1;p<featureID;p++)
            {
                IsOk=pEdit->DeleteFeature(p-difNum);
                if(!IsOk)
                {
                    CallBackHelper::EndProgress(callBack);
                    BeginEdit(pfc);
                    return false;
                }
                difNum++;
            }
        }
        curfid=featureID;
        if(callBack!=nullptr)
        {
            int per=(difNum+addedFeatureCount-oldFeatureCount)*100.0/(recordNum+oldFeatureCount);
            if(formper!=per)
            {
                CallBackHelper::SendProgress(callBack,per);
               formper=per;
            }
        }
    }
    for(int p=curfid+1;p<oldFeatureCount;p++)
    {
        IsOk=pEdit->DeleteFeature(p-difNum);
        if(!IsOk)
        {
            CallBackHelper::EndProgress(callBack);
            BeginEdit(pfc);
            return false;
        }
        difNum++;
        if(callBack!=nullptr)
        {
            int per=(difNum+addedFeatureCount-oldFeatureCount)*100.0/(recordNum+oldFeatureCount);
            if(formper!=per)
            {
                CallBackHelper::SendProgress(callBack,per);
                formper=per;
            }
        }
    }
    CallBackHelper::EndProgress(callBack);
    BeginEdit(pfc);
    return true;
}
bool FeatureClassEditor::RemoveOneBackwardUndo()
{
    if(currentPin.Pin<=0) return false;
    if(pEdits.size()==0) return false;
    if(!editChanged) editChanged=GetCurrentEditChanged();
    LONG ReleaseSize=BackwardDestroyInvalidData();
    LONG Size=pEdits[0]->ProximateSize;
    LONG Size1=pEdits[0]->ComputeReleaseProximateSize();
    RemainEditsMB+=Size-Size1-ReleaseSize;
    pEdits[0]->ReleaseNoUsed();
    pEdits.erase(begin(pEdits)+0);
    currentPin.Pin--;
    return true;
}
void FeatureClassEditor::RegisterNewVersion(string Name,bool EditChanged)
{
    int Size=pEdits.size();
    for(int k=Size-1;k>currentPin.Pin;k--)
    {
        pEdits[k]=nullptr;
        pEdits.erase(begin(pEdits)+k);
    }
    while(pEdits.size()>2)
    {
        LONG Proximize=RemainEditsMB;
        for(int k=pEdits.size()-1;k>=0;k--)
        {
            Proximize+=pEdits[k]->ProximateSize;
        }
        float Mb=Proximize/1024.0/1024.0;
        if(Mb>=40.0)
        {
            RemoveOneBackwardUndo();
        }
        else
            break;
    }
    if(currentPin.Pin>=HistoryCount-1)
    {
        RemoveOneBackwardUndo();
    }
    AutoPtr<FeatureClassVersion>pEdit=CurrentEdit->Clone();
    pEdit->SetName(Name);
    pEdit->EditChanged=EditChanged;
    pEdits.push_back(pEdit);
    currentPin.Pin++;
    CurrentEdit=pEdit;
}
bool FeatureClassEditor::GetCurrentEditChanged()
{
    if(editChanged) return editChanged;
    if(currentPin.Pin<=0) return false;
    int pin=currentPin.Pin;
    AutoPtr<FeatureClassVersion>ce=pEdits[pin];
    int recordNumber=originRecordCount;
    while(ce->EditChanged)
    {
        int itemCount=ce->GetItemCount();
        if(itemCount==0) return (recordNumber!=0);
        if(itemCount!=1) return true;
        AutoPtr<Records>pRecords=ce->GetItem(0);
        if(!pRecords->IsVirtual()) return true;
        int rn=pRecords->GetRecordNumber();
        if(recordNumber==-1)
            rn=recordNumber;
        else if(rn!=recordNumber)
            return true;
        pin--;
        if(pin==0) return false;
        ce=pEdits[pin];
    }
    return false;
}
AutoPtr<FeatureClassEditor>FeatureClassEditor::Copy()
{
    FeatureClassEditor*pCol=new FeatureClassEditor();
    if(currentPin.Pin>=0)
    {
        pCol->pEdits.push_back(CurrentEdit->Copy());
        pCol->currentPin.Pin=0;
        pCol->CurrentEdit=pCol->pEdits[0];
    }
    pCol->HistoryCount=HistoryCount;
    pCol->RemainEditsMB=RemainEditsMB;
    pCol->editChanged=editChanged;
    return pCol;
}

void FeatureClassEditor::ResetSelectionSet(SelectionSet*pSelectionSet,bool reloading){
    LONG Count=pSelectionSet->GetSize();
    if(Count==0) return;
    LONG FromIndex=0;
    LONG FeatureCount=GetRecordNumber();
    LONG CurrentFID=pSelectionSet->GetFeatureID(FromIndex);
    if(reloading)
    {
        vector<LONG>FIDs;
        for(int k=0;k<FeatureCount;k++)
        {
            LONG FID=GetFeatureID(k);
            while(FID>=CurrentFID)
            {
                if(FID==CurrentFID) FIDs.push_back(k);
                FromIndex++;
                if(FromIndex>=Count) break;
                CurrentFID=pSelectionSet->GetFeatureID(FromIndex);
            }
            if(FromIndex>=Count) break;
        }
        pSelectionSet->Clear();
        int selCount=FIDs.size();
        for(int k=0;k<selCount;k++) pSelectionSet->AddFeatureID(FIDs[k]);
    }
    else
    {
        for(int k=0;k<FeatureCount;k++)
        {
            LONG FID=GetFeatureID(k);
            while(FID>=CurrentFID)
            {
                if(FID>CurrentFID)
                {
                    pSelectionSet->Remove(FromIndex);
                    Count--;
                }
                else
                    FromIndex++;
                if(FromIndex>=Count) break;
                CurrentFID=pSelectionSet->GetFeatureID(FromIndex);
            }
            if(FromIndex>=Count) break;
        }
        for(int k=FromIndex;k<Count;k++)
        {
            pSelectionSet->Remove(FromIndex);
        }
    }
}


}
