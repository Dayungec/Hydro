#ifndef FEATURECLASSEDITOR_H
#define FEATURECLASSEDITOR_H
#include "Base/autoptr.h"
#include "Base/variant.h"
#include "Base/CallBack.h"
#include "dataset.h"
#include "display.h"

namespace SGIS{

class FieldEl;
class EnRecord;
class OldEnRecord;
class NewEnRecord;
class VirtualRecords;
class FeatureClassVersion;
class FeatureClassEditor;
class FieldEl
{
public:
    FieldEl(SHORT Index,Variant value);
    FieldEl(const FieldEl&el);
    FieldEl();
    virtual~FieldEl();
    int GetProximateSize();
    Variant GetFieldValue();
    string GetFieldValueAsString();
    double GetFieldValueAsDouble();
    SHORT GetFieldIndex();
protected:
    SHORT fieldIndex;
    Variant sValue;
};
class FieldEls
{
public:
    FieldEls();
    ~FieldEls();
    bool DeleteField(int fieldindex);
    void AddField(int fieldindex,Variant value);
    int GetProximateSize(int k);
    int GetProximateSize();
    Variant GetFieldValue(LONG pos);
    string GetFieldValueAsString(LONG pos);
    double GetFieldValueAsDouble(LONG pos);
    void SetFieldValue(int FieldIndex,Variant newValue);
    int FindField(int fieldindex);
    int GetFieldCount();
    int GetFieldIndex(int index);
    AutoPtr<FieldEls>Copy();
protected:
    long FindPlacePosi(int fieldindex);
    void InsertField(int pos,FieldEl&el);
protected:
    FieldEl*fels;
    SHORT fCount;
};
class Records
{
public:
    Records();
    virtual~Records();
    virtual bool IsVirtual()=0;
    virtual int GetRecordNumber()=0;
    virtual int GetProximateSize()=0;
};
class EnRecord :public Records
{
public:
    EnRecord();
    virtual~EnRecord();
    int GetRecordNumber();
    bool IsVirtual();
    virtual bool IsRefEnRecord();
    virtual bool IsOldEnRecord();
    virtual bool IsNewEnRecord();
    virtual LONG GetFeatureID()=0;
    virtual bool GetEnvelope(Envelope*pEnv)=0;
    virtual AutoPtr<Geometry> GetGeometry(bool&bSuc)=0;
    virtual bool GetFieldValue(LONG FieldIndex,Variant&value)=0;
    virtual bool GetFieldValueAsString(LONG FieldIndex,string&value)=0;
    virtual bool GetFieldValueAsDouble(LONG FieldIndex,double&value)=0;
    virtual LONG GetFatherPos()=0;
    virtual bool IsGeometryChanged();
    virtual bool IsFieldValueChanged();
    virtual int GetProximateSize();
    virtual void SetFeature(AutoPtr<Geometry>geo);
    virtual void SetFieldValue(int FieldIndex,Variant newValue);
    virtual int BackwardDestroyInvalidData();
    virtual int BackwardDestroyInvalidData(Geometry*refGeo,FieldEls*pEl);
    virtual int DestroyBackwardInvalidData();
    bool IsContainOther(EnRecord*other);
    friend class FeatureClassVersion;
protected:
    int GetProximateSize(Geometry*pGeo);
protected:
    AutoPtr<Geometry>pGeo;
    AutoPtr<FieldEls>fels;
};
class OldEnRecord :public EnRecord
{
public:
    OldEnRecord(LONG featureID);
    ~OldEnRecord();
    int GetRecordNumber();
    LONG GetFeatureID();
    virtual bool IsOldEnRecord();
    bool GetEnvelope(Envelope*pEnv);
    AutoPtr<Geometry> GetGeometry(bool&bSuc);
    bool GetFieldValue(LONG FieldIndex,Variant&value);
    bool GetFieldValueAsString(LONG FieldIndex,string&value);
    bool GetFieldValueAsDouble(LONG FieldIndex,double&value);
    LONG GetFatherPos();
    friend class FeatureClassVersion;
protected:
    LONG FeatureID;
};
class NewEnRecord :public EnRecord
{
public:
    NewEnRecord(AutoPtr<Geometry>geo,long featureID);
    virtual~NewEnRecord();
    LONG GetFeatureID();
    virtual bool IsNewEnRecord();
    bool GetEnvelope(Envelope*pEnv);
    AutoPtr<Geometry> GetGeometry(bool&bSuc);
     bool GetFieldValue(LONG FieldIndex,Variant&value);
    bool GetFieldValueAsString(LONG FieldIndex,string&value);
    bool GetFieldValueAsDouble(LONG FieldIndex,double&value);
    LONG GetFatherPos();
    bool IsGeometryChanged();
    bool IsFieldValueChanged();
    friend class FeatureClassVersion;
protected:
    LONG FeatureID;
};
class RefEnRecord :public EnRecord
{
public:
    RefEnRecord(AutoPtr<EnRecord>pRecord,LONG fatherIndex);
    ~RefEnRecord();
    bool IsRefEnRecord();
    int GetRecordNumber();
    void SetFeature(AutoPtr<Geometry>geo);
    void SetFieldValue(int FieldIndex,Variant newValue);
    int GetProximateSize();
    LONG GetFeatureID();
    bool GetEnvelope(Envelope*pEnv);
    AutoPtr<Geometry> GetGeometry(bool&bSuc);
    bool GetFieldValue(LONG FieldIndex,Variant&value);
    bool GetFieldValueAsString(LONG FieldIndex,string&value);
    bool GetFieldValueAsDouble(LONG FieldIndex,double&value);
    bool IsGeometryChanged();
    bool IsFieldValueChanged();
    LONG GetFatherPos();
    int BackwardDestroyInvalidData();
    int BackwardDestroyInvalidData(Geometry*refGeo,FieldEls*pEl);
    int DestroyBackwardInvalidData();
    friend class FeatureClassVersion;
protected:
    AutoPtr<EnRecord>refRecord;
    LONG FatherIndex;
};
class VirtualRecords :public Records
{
public:
    VirtualRecords(FeatureClassVersion*pEdit,long fromPos,long toPos);
    ~VirtualRecords();
    bool IsVirtual();
    int GetRecordNumber();
    int GetFromPos();
    int GetToPos();
    int GetProximateSize();
    LONG GetFeatureID(LONG FatherIndex);
    bool GetEnvelope(LONG FatherIndex,Envelope*pEnv);
    AutoPtr<Geometry> GetGeometry(LONG FatherIndex,bool&bSuc);
    bool GetFieldValue(LONG FatherIndex,LONG FieldIndex,Variant&value);
    bool GetFieldValueAsString(LONG FatherIndex,LONG FieldIndex,string&value);
    bool GetFieldValueAsDouble(LONG FatherIndex,LONG FieldIndex,double&value);
    bool CreateBreak(LONG BreakPos,AutoPtr<VirtualRecords>*pFrom,AutoPtr<VirtualRecords>*pTo);
    int BackwardDestroyInvalidData(LONG FatherIndex);
    int BackwardDestroyInvalidData(LONG FatherIndex,Geometry*refGeo,FieldEls*pEl);
    friend class FeatureClassVersion;
public:
    FeatureClassVersion*refEdit;
protected:
    LONG FromPos;
    LONG ToPos;
};
struct FeaturePin
{
    int Pin;
    int DifNumber;
    int RecordNum;
    FeaturePin()
    {
        Pin=0;
        DifNumber=0;
        RecordNum=0;
    };
    FeaturePin(FeaturePin&other)
    {
        Pin=other.Pin;
        DifNumber=other.DifNumber;
        RecordNum=other.RecordNum;
    };
};
class SGIS_EXPORT FeatureClassVersion
{
public:
    FeatureClassVersion(VectorShapeType vType,AutoPtr<Envelope>env,AutoPtr<TableDesc>desc,AutoPtr<SpatialReference>psp);
    ~FeatureClassVersion();
    LONG ComputeReleaseProximateSize();
    AutoPtr<Envelope>GetShapeEnvelope();
    void ReleaseNoUsed();
    LONG GetRecordNumber();
    bool GetEnvelope(LONG RecordIndex,Envelope*pEnv);
    AutoPtr<Geometry> GetGeometry(LONG RecordIndex,bool&bSuc);
    bool GetFieldValue(LONG RecordIndex,LONG FieldIndex,Variant&value);
    bool GetFieldValueAsString(LONG RecordIndex,LONG FieldIndex,string&value);
    bool GetFieldValueAsDouble(LONG RecordIndex,LONG FieldIndex,double&value);
    LONG GetFeatureID(LONG RecordIndex);
    bool IsGeometryChanged(LONG RecordIndex);
    bool IsFieldValueChanged(LONG RecordIndex);
    bool SetFeature(LONG RecordIndex,AutoPtr<Geometry>pGeo);
    bool SetFieldValue(LONG RecordIndex,LONG FieldIndex,Variant&value);
    bool SetFieldValue(LONG RecordIndex,LONG FieldIndex,string&value);
    bool AddFeature(AutoPtr<Geometry>pGeo);
    bool AddFeature(AutoPtr<Geometry>pGeo,const vector<Variant>&newValues);
    bool DeleteFeature(LONG RecordIndex);
    void DeleteAllFeatures();
    void Initialize(LONG recordNumber);
    AutoPtr<FeatureClassVersion>Clone();
    AutoPtr<FeatureClassVersion>Copy();
    string GetName();
    void SetName(string name);
    void SetCurrentPin(FeaturePin featurePin);
public:
    AutoPtr<EnRecord>GetCascadeRecord(int RecordIndex,LONG&FatherIndex);
    AutoPtr<Records>GetItem(int index);
    LONG GetItemCount();
    int BackwardDestroyInvalidData(LONG RecordIndex);
    int BackwardDestroyInvalidData(LONG RecordIndex,Geometry*refGeo,FieldEls*pEl);
protected:
    bool MoveTo(int RecordNum,FeaturePin&featurePin);
    int GetFatherIndex(FeaturePin featurePin);
    void ValidFieldValue(int fieldIndex,Variant&value);
public:
    bool EditChanged;
    LONG ProximateSize;
    LONG RecordNumber;
protected:
    vector<AutoPtr<Records>>pRecords;
    AutoPtr<Envelope>pEnv;
    FeaturePin currentPin;
    LONG ID;
    string Name;
    VectorShapeType shapeType;
    AutoPtr<TableDesc>pDesc;
    AutoPtr<SpatialReference>psp;
};
class SGIS_EXPORT FeatureClassEditor
{
public:
    FeatureClassEditor();
    virtual~FeatureClassEditor();
    void SetHistoryCount(int Count);
    void BeginEdit(FeatureClass*pfc);
    AutoPtr<Envelope>GetShapefileEnvelope();
    LONG GetFeatureID(LONG RecordIndex);
    LONG GetRecordNumber();
    AutoPtr<Geometry>GetGeometry(LONG RecordIndex,bool&bSuc);
    bool GetEnvelope(LONG RecordIndex,Envelope*pEnv);
    bool GetFieldValue(int RecordIndex,int index,Variant&value);
    bool GetFieldValueAsString(int RecordIndex,int index,string&value);
    bool GetFieldValueAsDouble(int RecordIndex,int index,double&value);
    bool AddFeature(AutoPtr<Geometry>pGeo);
    bool AddFeature(AutoPtr<Geometry>pGeo,const vector<Variant>&newValues);
    bool DeleteFeature(LONG RecordIndex);
    bool SetFeature(LONG RecordIndex,AutoPtr<Geometry>pGeo);
    bool SetFieldValue(LONG RecordIndex,LONG FieldIndex,Variant&newValue);
    bool SetFieldValue(LONG RecordIndex,LONG FieldIndex,string&newValue);
    bool DeleteAllFeatures();
    bool DeleteSelFeatures(FeatureClass*pfc,CallBack*callBack);
    bool CalculateField(FeatureClass*pfc,LONG TargetFieldIndex,string Expression,CallBack*callBack);
    bool MoveSelection(FeatureClass*pfc,DOUBLE difx,DOUBLE dify,CallBack*callBack);
    bool MoveSelectionByClientCoord(FeatureClass*pfc,DOUBLE difx,DOUBLE dify,Display*pDisplay,CallBack*callBack);
    int UsableUndoTimes();
    int UsableRedoTimes();
    bool UndoEdit();
    bool RedoEdit();
    bool SaveEdit(FeatureClass*pfc,CallBack*callBack);
    string GetCurrentUndoName();
    string GetCurrentRedoName();
    bool SetCurrentUndoName(string name);
    bool SetCurrentRedoName(string name);
    bool ClipForewardAllRedos();
    bool RemoveOneBackwardUndo();
    void RegisterNewVersion(string Name,bool EditChanged);
    bool GetCurrentEditChanged();
    AutoPtr<FeatureClassEditor>Copy();
    void ResetSelectionSet(SelectionSet*selSet,bool reloading);
protected:
    LONG BackwardDestroyInvalidData();
    AutoPtr<CoordinateTransformation> GetLayerCoordTansform(FeatureClass*pfc,Display*pDisplay);
    AutoPtr<CoordinateTransformation> GetLayersCoordTansform(FeatureClass*pfc,Display*pDisplay);
public:
    AutoPtr<FeatureClassVersion>CurrentEdit;
protected:
    vector<AutoPtr<FeatureClassVersion>>pEdits;
    FeaturePin currentPin;
    int HistoryCount;
    LONG RemainEditsMB;
    bool editChanged;
    int originRecordCount;
};

}
#endif // FEATURECLASSEDITOR_H
