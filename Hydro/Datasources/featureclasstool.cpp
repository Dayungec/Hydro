#include "featureclasstool.h"
#include "Base/stringinterpreter.h"
#include "Geometry/geometrytopology.h"
#include "Base/BaseArray.h"
#include "Base/StringHelper.h"
#include "memfeatureclass.h"

namespace SGIS{

FeatureClassTool::FeatureClassTool(FeatureClass*pfc){
    this->pfc=pfc;
}
FeatureClassTool::~FeatureClassTool(){

}

int FeatureClassTool::FindFeatureID(FeatureClass*pfc,int FeatureID){
    LONG num=pfc->GetFeatureCount();
    if (num == 0) return -1;
    bool needSearch=false;
    AutoPtr<FeatureClassEdit>pEdit=AutoPtr<FeatureClass>(pfc,true);
    if(pEdit!=nullptr){
        bool IsEditing=pEdit->IsEditing();
        if(IsEditing) needSearch=true;
    }
    if(!needSearch){
        AutoPtr<MemFeatureClass>memfc=AutoPtr<FeatureClass>(pfc,true);
        if(memfc!=nullptr){
            if(memfc->HasFeatureIds()) needSearch=true;
        }
    }
    if(!needSearch){
        if((FeatureID<0)||(FeatureID>=num))
            return -1;
        else
            return FeatureID;
    }
    long last=num-1;
    long lastvalue=pfc->GetFeatureID(last);
    if(FeatureID>lastvalue) return -1;
    long first=0;
    long firstvalue=pfc->GetFeatureID(0);
    if(FeatureID<firstvalue) return -1;
    if(FeatureID==firstvalue)
       return 0;
    else if(FeatureID==lastvalue)
        return num-1;
    long temp,tempvalue;
    while(true)
    {
        temp=(first+last)/2;
        if(temp==first) return -1;
        tempvalue=pfc->GetFeatureID(temp);
        if(FeatureID<tempvalue)
        {
            last=temp;
            lastvalue=tempvalue;
        }
        else if(FeatureID>tempvalue)
        {
            first=temp;
            firstvalue=tempvalue;
        }
        else
        {
            return temp;
        }
    }
    return -1;
}

int FeatureClassTool::FindFeatureID(int FeatureID){
    return FindFeatureID(pfc,FeatureID);
}

AutoPtr<IndexedArray<string>>FeatureClassTool::GetUniqueValues(vector<int>fieldIndices,string fieldSplit,int maxSize){
    AutoPtr<IndexedArray<string>>UniqueValues(new IndexedArray<string>());
    LONG FeatureCount=pfc->GetFeatureCount();
    LONG Size=fieldIndices.size();
    int num=0;
    if(Size==1)
    {
        AutoPtr<TableDesc>pDesc=pfc->GetTableDesc();
        VectorFieldType ftype=pDesc->GetFieldType(fieldIndices[0]);
        if(ftype==vftInteger){
            AutoPtr<OrderArray<int>>pArray(new OrderArray<int>(false));
            string sV;
            for(int k=0;k<FeatureCount;k++)
            {
                DOUBLE dV=pfc->GetFieldValueAsDouble(k,fieldIndices[0]);
                LONG index=pArray->Add(dV);
                if(index>=0)  {
                    sV=StringHelper::Format("%d",dV);
                    UniqueValues->Insert(index,sV);
                    num++;
                    if((num>=maxSize)&&(maxSize>0)) break;
                }
            }
        }
        else if(ftype==vftReal)
        {
            AutoPtr<OrderArray<double>>pArray(new OrderArray<double>(false));
            string sV;
            for(int k=0;k<FeatureCount;k++)
            {
                DOUBLE dV=pfc->GetFieldValueAsDouble(k,fieldIndices[0]);
                string bsV=pfc->GetFieldValueAsString(k,fieldIndices[0]);
                if(UniqueValues->FindValueIndex(bsV)>=0) continue;
                LONG index=pArray->Add(dV);
                if(index>=0)
                {
                    UniqueValues->Insert(index,bsV);
                    num++;
                    if((num>=maxSize)&&(maxSize>0)) break;
                }
            }
        }
        else
        {
            AutoPtr<OrderArray<string>>pArray(new OrderArray<string>(false));
            for(int k=0;k<FeatureCount;k++)
            {
                string bsV=pfc->GetFieldValueAsString(k,fieldIndices[0]);
                LONG index=pArray->Add(bsV);
                if(index>=0) UniqueValues->Insert(index,bsV);
                if(index>=0)
                {
                    num++;
                    if((num>=maxSize)&&(maxSize>0)) break;
                }
            }
        }
    }
    else
    {
        AutoPtr<OrderArray<string>>pArray(new OrderArray<string>(false));
        string sV;
        for(int k=0;k<FeatureCount;k++)
        {
            for(int j=0;j<Size;j++)
            {
                string bstr=pfc->GetFieldValueAsString(k,fieldIndices[j]);
                if(j==0)
                    sV=bstr;
                else
                    sV=sV+fieldSplit+bstr;
            }
            LONG index=pArray->Add(sV);
            if(index>=0) UniqueValues->Insert(index,sV);
            if(index>=0)
            {
                num++;
                if((num>=maxSize)&&(maxSize>0)) break;
            }
        }
    }
    return UniqueValues;
}
AutoPtr<Histogram>FeatureClassTool::ComputeHistogram(string sFieldName,DOUBLE minValue,DOUBLE maxValue,LONG ColumnsNum){
    if(ColumnsNum<=0) return nullptr;
    int index=-1;
    AutoPtr<TableDesc>pDesc=pfc->GetTableDesc();
    if(!sFieldName.empty())
    {
        index=pDesc->FindField(sFieldName);
    }
    if(index==-1) return nullptr;
    VectorFieldType fty=pDesc->GetFieldType(index);
    if((fty!=vftInteger)&&(fty!=vftReal)) return nullptr;
    Histogram*pHist=new Histogram();
    pHist->SetColumns(ColumnsNum);
    pHist->SetMinMaxValue(minValue,maxValue);
    int iDif;
    DOUBLE Dif=(maxValue-minValue)/ColumnsNum;
    LONG RecordCount=pfc->GetFeatureCount();
    for(int k=0;k<RecordCount;k++)
    {
        double sValue=pfc->GetFieldValueAsDouble(k,index);
        iDif=(sValue-minValue)/Dif;
        if((iDif<0)||(iDif>=ColumnsNum)) continue;
        pHist->Add(iDif,1);
    }
    return pHist;
}

bool FeatureClassTool::ComputeStatistics(string sFieldName,DOUBLE*min,DOUBLE*max,DOUBLE*mean,DOUBLE*stddev){
    *min=*max=*mean=*stddev=0;
    LONG index=-1;
    AutoPtr<TableDesc>pDesc=pfc->GetTableDesc();
    if(!sFieldName.empty())
    {
        index=pDesc->FindField(sFieldName);
    }
    if(index==-1) return false;
    VectorFieldType fty=pDesc->GetFieldType(index);
    if((fty!=vftInteger)&&(fty!=vftReal)) return false;
    AutoPtr<FeatureClassEdit>pEdit=AutoPtr<FeatureClass>(pfc,true);
    bool IsEditing=false;
    StatisHistory&pStaHist=pfc->GetStatisHistory();
    if(pEdit!=nullptr)
    {
        IsEditing=pEdit->IsEditing();
    }
    if(!IsEditing)
    {
        if(pStaHist.GetStatis(sFieldName,*min,*max,*mean,*stddev))
        {
            return true;
        }
    }
    double MinValue,MaxValue;
    double Sum,Sum2;
    Sum=Sum2=0;
    int Count=0;
    long k=-1;
    LONG FeatureCount=pfc->GetFeatureCount();
    double sValue;
    for(int k=0;k<FeatureCount;k++)
    {
        sValue=pfc->GetFieldValueAsDouble(k,index);
        if(k==0)
        {
            MinValue=MaxValue=sValue;
        }
        else
        {
            if(sValue<MinValue) MinValue=sValue;
            if(sValue>MaxValue) MaxValue=sValue;
        }
        Sum+=sValue;
        Sum2+=sValue*sValue;
        Count++;
    }
    if(Count==0) return true;
    *min=MinValue;
    *max=MaxValue;
    *mean=Sum/Count;
    *stddev=sqrt((Sum2-Count*pow(Sum/Count,2))/Count);
    if(!IsEditing) pStaHist.AddStatisHist(sFieldName,*min,*max,*mean,*stddev);
    return true;
}


AutoPtr<FeatureCursor>FeatureClassTool::Select(FeatureClass*pfc,QueryFilter*pFilter,CallBack*callBack){
    AutoPtr<SpatialFilter>psFilter=AutoPtr<QueryFilter>(pFilter,true);
    LONG SubFieldCount=pFilter->GetFieldCount();
    AutoPtr<TableDesc>pDesc=pfc->GetTableDesc();
    vector<int>SubFieldIndexs;
    if(SubFieldCount>0)
    {
        for(int k=0;k<SubFieldCount;k++)
        {
            string Field=pFilter->GetField(k);
            LONG index=pDesc->FindField(Field);
            if(index>=0) SubFieldIndexs.push_back(index);
        }
    }
    else
    {
        SubFieldCount=pDesc->GetFieldCount();
        for(int k=0;k<SubFieldCount;k++)
        {
            SubFieldIndexs.push_back(k);
        }
    }
    string wc=pFilter->WhereClause();
    AutoPtr<VirtualTable>pvTable(new VirtualTable());
    LONG FieldCount=pDesc->GetFieldCount();
    bool IsOk;
    pvTable->AddField("FID",exInt);
    for(int k=0;k<FieldCount;k++)
    {
        string FieldName=pDesc->GetFieldName(k);
        VectorFieldType ft=pDesc->GetFieldType(k);
        ExpressionDataType dt=FieldTypeHelper::ToExpressionDataType(ft);
        pvTable->AddField(FieldName,dt);
    }
    AutoPtr<StringInterpreter>pInterpreter(new StringInterpreter);
    bool NeedValidString=false;
    if(!wc.empty())
    {
        pInterpreter->SetVirtualTable(pvTable);
        if(!pInterpreter->InputBoolExpressionString(wc)) return nullptr;
        NeedValidString=true;
    }
    vector<int>UsableFieldIndexs;
    IsOk=pvTable->GetFieldUsing(0);
    if(IsOk) UsableFieldIndexs.push_back(-1);
    for(int k=0;k<FieldCount;k++)
    {
        IsOk=pvTable->GetFieldUsing(k+1);
        if(IsOk) UsableFieldIndexs.push_back(k);
    }
    AutoPtr<Geometry>pvGeo=nullptr;
    GeometryTopoType gtType=gttIntersect;
    if(psFilter!=nullptr)
    {
        pvGeo=psFilter->GetGeometry();
        if(pvGeo!=nullptr) gtType=psFilter->SpatialRel();
    }
    LONG FeatureCount=pfc->GetFeatureCount();
    Variant oV;
    AutoPtr<GeometryTopology>pTopo(new GeometryTopology(pvGeo));
    if(pvGeo!=nullptr) pTopo->SetGeometry(pvGeo);
    AutoPtr<TableDescCreator>pDescCreator(new TableDescCreator());
    int TargetFieldCount=SubFieldIndexs.size();
    for(int k=0;k<TargetFieldCount;k++) pDescCreator->AddField((*pDesc)[SubFieldIndexs[k]]);
    AutoPtr<TableDesc>newDesc=pDescCreator->CreateTableDesc();
    AutoPtr<SpatialReference>psp=pfc->GetSpatialReference();
    AutoPtr<FeatureCursorCreator>pCreator(new FeatureCursorCreator(newDesc,psp));
    CallBackHelper::BeginProgress(callBack,"查询");
    for(int k=0;k<FeatureCount;k++)
    {
        CallBackHelper::SendProgress(callBack,(float)k/FeatureCount*100);
        if(NeedValidString)
        {
            for(int j=UsableFieldIndexs.size()-1;j>=0;j--)
            {
                if(UsableFieldIndexs[j]==-1){
                    pvTable->SetFieldValue(0,pfc->GetFeatureID(k));
                }
                else{
                   oV=pfc->GetFieldValue(k,UsableFieldIndexs[j]);
                   pvTable->SetFieldValue(UsableFieldIndexs[j]+1,oV);
                }
            }
            pInterpreter->ReturnValue(oV);
            oV.ChangeType(VRT_BOOL);
            if(!oV.boolVal) continue;
        }
        AutoPtr<Geometry>refGeo=pfc->GetFeature(k);
        if(refGeo==nullptr) continue;
        if(pvGeo!=nullptr)
        {
            switch(gtType)
            {
            case gttIntersect:
                IsOk=pTopo->Intersects(refGeo.get());break;
            case gttEquals:
                IsOk=pTopo->Equals(refGeo.get());break;
            case gttDisjoint:
                IsOk=pTopo->Disjoint(refGeo.get());break;
            case gttTouches:
                IsOk=pTopo->Touches(refGeo.get());break;
            case gttCrosses:
                IsOk=pTopo->Crosses(refGeo.get());break;
            case gttWithIn:
                IsOk=pTopo->Within(refGeo.get());break;
            case gttContains:
                IsOk=pTopo->Contains(refGeo.get());break;
            case gttOverlaps:
                IsOk=pTopo->Overlaps(refGeo.get());break;
            }
            if(!IsOk)
            {
                continue;
            }
        }
        vector<Variant>values;
        values.resize(TargetFieldCount);
        for(int j=0;j<TargetFieldCount;j++)
        {
            Variant vt=pfc->GetFieldValueAsString(k,SubFieldIndexs[j]);
            values[j]=vt;
        }
        pCreator->AddFeature(pfc->GetFeatureID(k),refGeo,values);
    }
    return pCreator->CreateFeatureCursor();
}

AutoPtr<FeatureCursor>FeatureClassTool::SelectByPoint(FeatureClass*pfc,Point*centerPoint, double toler, CallBack*callBack){
    LONG FeatureCount=pfc->GetFeatureCount();
    AutoPtr<TableDesc>pDesc=pfc->GetTableDesc();
    LONG TargetFieldCount=pDesc->GetFieldCount();
    AutoPtr<SpatialReference>psp=pfc->GetSpatialReference();
    AutoPtr<FeatureCursorCreator>pCreator(new FeatureCursorCreator(pDesc,psp));
    CallBackHelper::BeginProgress(callBack,"查询");
    DOUBLE X,Y;
    centerPoint->GetCoord(&X,&Y);
    DOUBLE Left,Top,Right,Bottom;
    Left=X-toler/2;
    Top=Y+toler/2;
    Right=Left+toler;
    Bottom=Top-toler;
    AutoPtr<Envelope>pEnv(new Envelope());
    DOUBLE X1,Y1,X2,Y2;
    DOUBLE dist;
    bool IsOk;
    for(int k=0;k<FeatureCount;k++)
    {
        CallBackHelper::SendProgress(callBack,(float)k/FeatureCount*100);
        IsOk=pfc->GetFeatureEnvelope(k,pEnv.get());
        if(!IsOk) continue;
        pEnv->GetCoord(&X1,&Y2,&X2,&Y1);
        if((Left>X2)||(Right<X1)||(Top<Y1)||(Bottom>Y2)) continue;
        AutoPtr<Geometry>refGeo=pfc->GetFeature(k);
        if(refGeo==nullptr) continue;
        dist=centerPoint->DistanceTo(refGeo.get());
        if(dist>toler) continue;
        vector<Variant>values;
        values.resize(TargetFieldCount);
        for(int j=0;j<TargetFieldCount;j++)
        {
            values[j]=pfc->GetFieldValue(k,j);
        }
        pCreator->AddFeature(pfc->GetFeatureID(k),refGeo,values);
    }
    return pCreator->CreateFeatureCursor();
}

AutoPtr<FeatureCursor>FeatureClassTool::SelectByPointEx(FeatureClass*pfc,Point*centerPoint,double toler,const vector<string>&fields,CallBack*callBack){
    AutoPtr<TableDesc>pDesc=pfc->GetTableDesc();
    AutoPtr<TableDescCreator>pCreator(new TableDescCreator());
    vector<int>fieldIndices;
    if(fields.size()>0){
        for(int k=0;k<fields.size();k++){
            int fIndex=pDesc->FindField(fields[k]);
            if(fIndex==-1) continue;
            fieldIndices.push_back(fIndex);
        }
    }else{
        int fieldCount=pDesc->GetFieldCount();
        for(int k=0;k<fieldCount;k++){
            fieldIndices.push_back(k);
        }
    }
    LONG TargetFieldCount=fieldIndices.size();
    for(int k=0;k<TargetFieldCount;k++){
        pCreator->AddField(FieldDesp(pDesc->GetFieldName(fieldIndices[k]),pDesc->GetFieldType(fieldIndices[k]),pDesc->GetFieldWidth(fieldIndices[k]),pDesc->GetFieldPrecision(fieldIndices[k])));
    }
    AutoPtr<TableDesc>newDesc=pCreator->CreateTableDesc();
    LONG FeatureCount=pfc->GetFeatureCount();
    AutoPtr<SpatialReference>psp=pfc->GetSpatialReference();
    AutoPtr<FeatureCursorCreator>pfCreator(new FeatureCursorCreator(newDesc,pfc->GetSpatialReference()));
    CallBackHelper::BeginProgress(callBack,"查询");
    DOUBLE X,Y;
    centerPoint->GetCoord(&X,&Y);
    DOUBLE Left,Top,Right,Bottom;
    Left=X-toler/2;
    Top=Y+toler/2;
    Right=Left+toler;
    Bottom=Top-toler;
    AutoPtr<Envelope>pEnv(new Envelope());
    DOUBLE X1,Y1,X2,Y2;
    DOUBLE dist;
    bool IsOk;
    for(int k=0;k<FeatureCount;k++)
    {
        CallBackHelper::SendProgress(callBack,(float)k/FeatureCount*100);
        IsOk=pfc->GetFeatureEnvelope(k,pEnv.get());
        if(!IsOk) continue;
        pEnv->GetCoord(&X1,&Y2,&X2,&Y1);
        if((Left>X2)||(Right<X1)||(Top<Y1)||(Bottom>Y2)) continue;
        AutoPtr<Geometry>refGeo=pfc->GetFeature(k);
        if(refGeo==nullptr) continue;
        dist=centerPoint->DistanceTo(refGeo.get());
        if(dist>toler) continue;
        vector<Variant>values;
        values.resize(TargetFieldCount);
        for(int j=0;j<TargetFieldCount;j++)
        {
            values[j]=pfc->GetFieldValue(k,fieldIndices[j]);
        }
        pfCreator->AddFeature(pfc->GetFeatureID(k),refGeo,values);
    }
    return pfCreator->CreateFeatureCursor();
}


AutoPtr<FeatureCursor>FeatureClassTool::Select(QueryFilter*pFilter,CallBack*callBack){
    return Select(pfc,pFilter,callBack);
}

AutoPtr<FeatureCursor>FeatureClassTool::SelectByPoint(Point*centerPoint, double toler, CallBack*callBack){
    return SelectByPoint(pfc,centerPoint,toler,callBack);
}

AutoPtr<FeatureCursor>FeatureClassTool::SelectByPointEx(Point*centerPoint,double toler,const vector<string>&fields,CallBack*callBack){
    return SelectByPointEx(pfc,centerPoint,toler,fields,callBack);
}

bool FeatureClassTool::Query(FeatureClass*pfc,QueryFilter*pFilter,SelectionSelMode mode,CallBack*callBack){
    AutoPtr<SpatialFilter>psFilter=AutoPtr<QueryFilter>(pFilter,true);
    LONG SubFieldCount=pFilter->GetFieldCount();
    AutoPtr<TableDesc>pDesc=pfc->GetTableDesc();
    vector<int>SubFieldIndexs;
    if(SubFieldCount>0)
    {
        for(int k=0;k<SubFieldCount;k++)
        {
            string Field=pFilter->GetField(k);
            LONG index=pDesc->FindField(Field);
            if(index>=0) SubFieldIndexs.push_back(index);
        }
    }
    else
    {
        SubFieldCount=pDesc->GetFieldCount();
        for(int k=0;k<SubFieldCount;k++)
        {
            SubFieldIndexs.push_back(k);
        }
    }
    string wc=pFilter->WhereClause();
    AutoPtr<VirtualTable>pvTable(new VirtualTable());
    pvTable->AddField("FID",exInt);
    LONG FieldCount=pDesc->GetFieldCount();
    bool IsOk;
    for(int k=0;k<FieldCount;k++)
    {
        string FieldName=pDesc->GetFieldName(k);
        VectorFieldType ft=pDesc->GetFieldType(k);
        ExpressionDataType dt=FieldTypeHelper::ToExpressionDataType(ft);
        pvTable->AddField(FieldName,dt);
    }
    AutoPtr<StringInterpreter>pInterpreter(new StringInterpreter);
    bool NeedValidString=false;
    if(!wc.empty())
    {
        pInterpreter->SetVirtualTable(pvTable);
        if(!pInterpreter->InputBoolExpressionString(wc)) return false;
        NeedValidString=true;
    }
    vector<int>UsableFieldIndexs;
    IsOk=pvTable->GetFieldUsing(0);
    if(IsOk) UsableFieldIndexs.push_back(-1);
    for(int k=0;k<FieldCount;k++)
    {
        IsOk=pvTable->GetFieldUsing(k+1);
        if(IsOk) UsableFieldIndexs.push_back(k);
    }
    AutoPtr<Geometry>pvGeo=nullptr;
    GeometryTopoType gtType=gttIntersect;
    if(psFilter!=nullptr)
    {
        pvGeo=psFilter->GetGeometry();
        if(pvGeo!=nullptr) gtType=psFilter->SpatialRel();
    }
    LONG FeatureCount=pfc->GetFeatureCount();
    Variant oV;
    AutoPtr<GeometryTopology>pTopo(new GeometryTopology(pvGeo));
    if(pvGeo!=nullptr) pTopo->SetGeometry(pvGeo);
    CallBackHelper::BeginProgress(callBack,"查询");
    AutoPtr<SelectionSet>pSelectionSet=pfc->GetSelectionSet();
    if(mode==SelectionSelMode::ssmNew)
        pSelectionSet->Clear();
    for(int k=0;k<FeatureCount;k++)
    {
        CallBackHelper::SendProgress(callBack,(float)k/FeatureCount*100);
        if(NeedValidString)
        {
            for(int j=UsableFieldIndexs.size()-1;j>=0;j--)
            {
                if(UsableFieldIndexs[j]==-1){
                    pvTable->SetFieldValue(0,pfc->GetFeatureID(k));
                }
                else{
                   oV=pfc->GetFieldValue(k,UsableFieldIndexs[j]);
                   pvTable->SetFieldValue(UsableFieldIndexs[j]+1,oV);
                }
            }
            pInterpreter->ReturnValue(oV);
            oV.ChangeType(VRT_BOOL);
            if(!oV.boolVal) continue;
        }
        if(pvGeo!=nullptr)
        {
            AutoPtr<Geometry>refGeo=pfc->GetFeature(k);
            if(refGeo==nullptr) continue;
            switch(gtType)
            {
            case gttIntersect:
                IsOk=pTopo->Intersects(refGeo.get());break;
            case gttEquals:
                IsOk=pTopo->Equals(refGeo.get());break;
            case gttDisjoint:
                IsOk=pTopo->Disjoint(refGeo.get());break;
            case gttTouches:
                IsOk=pTopo->Touches(refGeo.get());break;
            case gttCrosses:
                IsOk=pTopo->Crosses(refGeo.get());break;
            case gttWithIn:
                IsOk=pTopo->Within(refGeo.get());break;
            case gttContains:
                IsOk=pTopo->Contains(refGeo.get());break;
            case gttOverlaps:
                IsOk=pTopo->Overlaps(refGeo.get());break;
            }
            if(!IsOk)
            {
                continue;
            }
        }
        LONG featureID=pfc->GetFeatureID(k);
        if(mode==SelectionSelMode::ssmRemove)
            pSelectionSet->RemoveFeatureID(featureID);
        else
            pSelectionSet->AddFeatureID(featureID);
    }
    CallBackHelper::EndProgress(callBack);
    return true;
}

void FeatureClassTool::QueryByPoint(FeatureClass*pfc,Point*centerPoint,double toler,SelectionSelMode mode,CallBack*callBack){
    LONG FeatureCount=pfc->GetFeatureCount();
    AutoPtr<TableDesc>pDesc=pfc->GetTableDesc();
    AutoPtr<SpatialReference>psp=pfc->GetSpatialReference();
    CallBackHelper::BeginProgress(callBack,"查询");
    DOUBLE X,Y;
    centerPoint->GetCoord(&X,&Y);
    DOUBLE Left,Top,Right,Bottom;
    Left=X-toler/2;
    Top=Y+toler/2;
    Right=Left+toler;
    Bottom=Top-toler;
    AutoPtr<Envelope>pEnv(new Envelope());
    DOUBLE X1,Y1,X2,Y2;
    DOUBLE dist;
    bool IsOk;
    AutoPtr<SelectionSet>pSelectionSet=pfc->GetSelectionSet();
    for(int k=0;k<FeatureCount;k++)
    {
        CallBackHelper::SendProgress(callBack,(float)k/FeatureCount*100);
        IsOk=pfc->GetFeatureEnvelope(k,pEnv.get());
        if(!IsOk) continue;
        pEnv->GetCoord(&X1,&Y2,&X2,&Y1);
        if((Left>X2)||(Right<X1)||(Top<Y1)||(Bottom>Y2)) continue;
        AutoPtr<Geometry>refGeo=pfc->GetFeature(k);
        if(refGeo==nullptr) continue;
        dist=centerPoint->DistanceTo(refGeo.get());
        if(dist>toler) continue;
        LONG featureID=pfc->GetFeatureID(k);
        if(mode==SelectionSelMode::ssmRemove)
            pSelectionSet->RemoveFeatureID(featureID);
        else
            pSelectionSet->AddFeatureID(featureID);
    }
    CallBackHelper::EndProgress(callBack);
}

bool FeatureClassTool::Query(QueryFilter*pFilter,SelectionSelMode mode,CallBack*callBack){
    return Query(pfc,pFilter,mode,callBack);
}

void FeatureClassTool::QueryByPoint(Point*centerPoint,double toler,SelectionSelMode mode,CallBack*callBack){
    QueryByPoint(pfc,centerPoint,toler,mode,callBack);
}

int FeatureClassTool::CaptureNearestFeatureID(Point*centerPoint,double toler,SelectionSet*filter,DOUBLE*minDist){
    *minDist=0;
    LONG selCount=0;
    if(filter!=nullptr) selCount=filter->GetSize();
    DOUBLE x,y;
    centerPoint->GetCoord(&x,&y);
    LONG Count=pfc->GetFeatureCount();
    double Left=x-toler/2;
    double Top=y+toler/2;
    double Right=x+toler/2;
    double Bottom=y-toler/2;
    DOUBLE X1,Y1,X2,Y2;
    double dist,mindist=0;
    AutoPtr<Envelope>pEnv(new Envelope());
    LONG targetID=-1;
    bool IsOk;
    for(int k=0;k<Count;k++)
    {
        IsOk=pfc->GetFeatureEnvelope(k,pEnv.get());
        if(!IsOk) continue;
        pEnv->GetCoord(&X1,&Y1,&X2,&Y2);
        if((Left>X2+toler)||(Right<X1- toler)||(Top>Y1+ toler)||(Bottom<Y2- toler)) continue;
        LONG featureID=pfc->GetFeatureID(k);
        if(selCount>0)
        {
            IsOk=filter->FindFeatureID(featureID);
            if(!IsOk) continue;
        }
        AutoPtr<Geometry>pGeo=pfc->GetFeature(k);
        dist=centerPoint->DistanceTo(pGeo.get());
        if(dist>toler) continue;
        if(targetID==-1)
        {
            targetID=featureID;
            mindist=dist;
        }
        else if(dist<mindist)
        {
            targetID=featureID;
            mindist=dist;
            if(dist==0) break;
        }
    }
    *minDist=mindist;
    return targetID;
}
bool FeatureClassTool::TestCaptureNearestFeature(Point*centerPoint,double toler,SelectionSet*filter){
    LONG selCount=0;
    if(filter!=NULL) selCount=filter->GetSize();
    DOUBLE x,y;
    centerPoint->GetCoord(&x,&y);
    LONG Count=pfc->GetFeatureCount();
    double Left=x-toler/2;
    double Top=y+toler/2;
    double Right=x+toler/2;
    double Bottom=y-toler/2;
    DOUBLE X1,Y1,X2,Y2;
    double dist;
    AutoPtr<Envelope>pEnv(new Envelope);
    bool IsOk;
    for(int k=0;k<Count;k++)
    {
        IsOk=pfc->GetFeatureEnvelope(k,pEnv.get());
        if(!IsOk) continue;
        pEnv->GetCoord(&X1,&Y1,&X2,&Y2);
        if ((Left > X2 + toler) || (Right < X1 - toler) || (Top > Y1 + toler) || (Bottom < Y2 - toler)) continue;
        LONG featureID=pfc->GetFeatureID(k);
        if(selCount>0)
        {
            IsOk=filter->FindFeatureID(featureID);
            if(!IsOk) continue;
        }
        AutoPtr<Geometry>pGeo=pfc->GetFeature(k);
        dist=centerPoint->DistanceTo(pGeo.get());
        if(dist<toler)
        {
            return true;
        }
    }
    return false;
}

vector<GeometryBorderCaptureData>FeatureClassTool::CaptureFeaturesVertex(Point*centerPoint,double toler,SelectionSet*pSel,bool needCaptureSeg){
    vector<GeometryBorderCaptureData>datas;
    LONG selCount=pSel->GetSize();
    LONG featureCount=pfc->GetFeatureCount();
    DOUBLE x,y;
    centerPoint->GetCoord(&x,&y);
    DOUBLE Left=x-toler;
    DOUBLE Right=x+toler;
    DOUBLE Top=y+toler;
    DOUBLE Bottom=y-toler;
    AutoPtr<Envelope>pEnv(new Envelope);
    bool IsOk;
    DOUBLE X1,Y1,X2,Y2;
    AutoPtr<GeometryBorderCapture>pCapture(new GeometryBorderCapture());
    pCapture->SetPosition(centerPoint);
    for(int k=0;k<featureCount;k++)
    {
        IsOk=pfc->GetFeatureEnvelope(k,pEnv.get());
        if(!IsOk) continue;
        pEnv->GetCoord(&X1,&Y1,&X2,&Y2);
        if((Left>X2)||(Right<X1)||(Top<Y2)||(Bottom>Y1)) continue;
        LONG featureID=pfc->GetFeatureID(k);
        if(selCount>0)
        {
            IsOk=pSel->FindFeatureID(featureID);
            if(!IsOk) continue;
        }
        AutoPtr<Geometry>pGeo=pfc->GetFeature(k);
        if(pGeo==NULL) continue;
        GeometryBorderCaptureData cd=pCapture->CaptureVertex(pGeo.get(),toler);
        if(cd.PointIndex==-1)
        {
            if(needCaptureSeg)
            {
                cd=pCapture->CaptureSegment(pGeo.get(),toler);
            }
            if(cd.PointIndex==-1) continue;
        }
        LONG FeatureID=pfc->GetFeatureID(k);
        cd.FeatureID=FeatureID;
        datas.push_back(cd);
    }
    return datas;
}

bool FeatureClassTool::CalculateField(string field,string wc,CallBack*callBack){
    AutoPtr<TableDesc>pDesc=pfc->GetTableDesc();
    int fieldIndex=pDesc->FindField(field);
    if(fieldIndex==-1) return false;
    AutoPtr<FeatureClassEdit>pEdit=AutoPtr<FeatureClass>(pfc,true);
    if(pEdit==nullptr) return false;
    AutoPtr<VirtualTable>pvTable(new VirtualTable());
    pvTable->AddField("FID",exInt);
    LONG FieldCount=pDesc->GetFieldCount();
    bool IsOk;
    for(int k=0;k<FieldCount;k++)
    {
        string FieldName=pDesc->GetFieldName(k);
        VectorFieldType ft=pDesc->GetFieldType(k);
        ExpressionDataType dt=FieldTypeHelper::ToExpressionDataType(ft);
        pvTable->AddField(FieldName,dt);
    }
    AutoPtr<StringInterpreter>pInterpreter(new StringInterpreter);
    if(!wc.empty())
    {
        pInterpreter->SetVirtualTable(pvTable);
        if(!pInterpreter->InputExpressionString(wc)) return false;
    }
    vector<int>UsableFieldIndexs;
    IsOk=pvTable->GetFieldUsing(0);
    if(IsOk) UsableFieldIndexs.push_back(-1);
    for(int k=0;k<FieldCount;k++)
    {
        IsOk=pvTable->GetFieldUsing(k+1);
        if(IsOk) UsableFieldIndexs.push_back(k);
    }
    LONG FeatureCount=pfc->GetFeatureCount();
    Variant oV;
    CallBackHelper::BeginProgress(callBack,"计算字段");
    AutoPtr<SelectionSet>pSelectionSet=pfc->GetSelectionSet();
    LONG selCount=pSelectionSet->GetSize();
    if(selCount==0){
        for(int k=0;k<FeatureCount;k++)
        {
            CallBackHelper::SendProgress(callBack,(float)k/FeatureCount*100);
            for(int j=UsableFieldIndexs.size()-1;j>=0;j--)
            {
                if(UsableFieldIndexs[j]==-1){
                    pvTable->SetFieldValue(0,pfc->GetFeatureID(k));
                }
                else{
                    oV=pfc->GetFieldValue(k,UsableFieldIndexs[j]);
                    pvTable->SetFieldValue(UsableFieldIndexs[j]+1,oV);
                }
            }
            pInterpreter->ReturnValue(oV);
            pEdit->SetFieldValue(k,fieldIndex,oV);
        }
    }
    else{
        for(int k=0;k<selCount;k++)
        {
            CallBackHelper::SendProgress(callBack,(float)k/selCount*100);
            int recordIndex=this->FindFeatureID(pfc,pSelectionSet->GetFeatureID(k));
            for(int j=UsableFieldIndexs.size()-1;j>=0;j--)
            {
                if(UsableFieldIndexs[j]==-1){
                    pvTable->SetFieldValue(0,pfc->GetFeatureID(recordIndex));
                }
                else{
                    oV=pfc->GetFieldValue(recordIndex,UsableFieldIndexs[j]);
                    pvTable->SetFieldValue(UsableFieldIndexs[j]+1,oV);
                }
            }
            pInterpreter->ReturnValue(oV);
            pEdit->SetFieldValue(recordIndex,fieldIndex,oV);
        }
    }
    CallBackHelper::EndProgress(callBack);
    return true;
}

}
