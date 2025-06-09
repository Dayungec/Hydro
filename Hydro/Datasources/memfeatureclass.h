#ifndef MEMFEATURECLASS_H
#define MEMFEATURECLASS_H
#include "Base/version.h"
#include "dataset.h"
#include "featureclasseditor.h"

namespace SGIS{
class MemField;
class MemVectors;
class MemFeatureClass;
class MemFeatureWorkspace;
class MemFeatureWorkspaceFactory;

class SGIS_EXPORT MemField
{
public:
    MemField(string Name,VectorFieldType type);
    virtual~MemField();
    void AddRecord(Variant value);
    void SetValue(int nIndex,Variant value);
    void Remove(int nIndex);
    void Clear();
    string GetFieldValueAsString(int nIndex);
    Variant GetFieldValue(int nIndex);
    DOUBLE GetFieldValueAsDouble(int nIndex);
    void SetSize(LONG Size);
protected:
    void ValidValue(Variant&value);
protected:
    string FieldName;
    VectorFieldType FieldType;
    vector<Variant>FieldValues;
};

class SGIS_EXPORT MemVectors
{
public:
    MemVectors(AutoPtr<TableDesc>desc,VectorShapeType type);
    virtual~MemVectors();
    void DeleteField(int index);
    int GetFeatureID(int nIndex);
    int FindField(string FieldName);
    bool AddField(string FieldName,VectorFieldType type,LONG FieldWidth,LONG Precision);
    bool AddRecord(AutoPtr<Geometry>geo,const vector<Variant>&newValues);
    bool AddRecord(AutoPtr<Geometry>geo);
    bool AddRecord(int fid,AutoPtr<Geometry>geo,const vector<Variant>&newValues);
    bool AddRecord(int fid,AutoPtr<Geometry>geo);
    void DeleteRecord(int index);
    void RecomputeEnvelope();
    AutoPtr<TableDesc>GetTableDesc();
    LONG GetRecordNumber();
    AutoPtr<Geometry>GetGeometry(int index);
    AutoPtr<Geometry>GetGeometryRef(int index);
    bool GetEnvelope(int index,Envelope*env);
    void SetGeometry(int index,AutoPtr<Geometry>geo);
    Variant GetFieldValue(int index,int findex);
    string GetFieldValueAsString(int index,int findex);
    double GetFieldValueAsDouble(int index,int findex);
    void SetFieldValue(int index,int findex,Variant Value);
    AutoPtr<Envelope>GetShapeEnvelope();
    void RemoveAllRecords();
    VectorShapeType GetShapeType();
    AutoPtr<MemVectors>Copy();
    bool HasFeatureIds();
protected:
    LONG RecordNumber;
    AutoPtr<TableDesc>pDesc;
    vector<AutoPtr<MemField>>fields;
    vector<AutoPtr<Geometry>>geos;
    vector<int>ids;
    AutoPtr<Envelope>Extent;
    VectorShapeType ShapeType;
};


class SGIS_EXPORT MemFeatureClass:
        public FeatureClass,
        public FeatureDataset,
        public FeatureClassEdit,
        public Version
{
protected:
    MemFeatureClass();
public:
    virtual~MemFeatureClass();
    AutoPtr<MemVectors>GetMemVectors();
    bool HasFeatureIds();
//FeatureClass:
    string GetDescription();
    string GetFeatureClassName();
    AutoPtr<FeatureDataset>GetFeatureDataset();
    AutoPtr<Envelope>GetExtent();
    int GetFeatureCount();
    AutoPtr<Geometry>GetFeature(int recordIndex);
    bool GetFeatureEnvelope(int recordIndex,Envelope*env);
    int GetFeatureID(int recordIndex);
    Variant GetFieldValue(int recordIndex,int FieldIndex);
    double GetFieldValueAsDouble(int recordIndex,int FieldIndex);
    string GetFieldValueAsString(int recordIndex,int FieldIndex);
    bool IsValid();
    AutoPtr<FeatureClass>Clone();
    void Dispose();
    bool AddFeature(int fid,AutoPtr<Geometry>geo);
    bool AddFeatureEx(int fid,AutoPtr<Geometry>geo,const vector<Variant>&values);
    bool Save(string pathName);
    bool IsFromSameSource(FeatureClass*other);
//FeatureDataset:
    string GetName();
    string GetTypeName();
    AutoPtr<Workspace>GetWorkspace();
    int GetFeatureClassCount();
    AutoPtr<FeatureClass>GetFeatureClass(int nIndex);
    AutoPtr<FeatureClass>GetDefaultFeatureClass();
    string GetFeatureClassName(int nIndex);
    int FindFeatureClass(string Name);
    bool TemporaryDataset();
protected:
//FeatureClassEdit:
    bool IsEditing();
    bool StartEditing();
    bool SaveEdit(CallBack*callBack=nullptr);
    bool Transaction(CallBack*callBack=nullptr);
    bool StopEdits(bool SaveEdit,CallBack*callBack=nullptr);
    bool SetFeature(LONG RecordIndex,AutoPtr<Geometry>geo);
    bool SetFieldValue(int RecordIndex,int FieldIndex,Variant newVal);
    bool SetFieldValueWithString(int RecordIndex,int FieldIndex,string newVal);
    bool AddFeature(AutoPtr<Geometry>geo);
    bool AddFeatureEx(AutoPtr<Geometry>geo,const vector<Variant>&values);
    bool DeleteFeature(int recordIndex);
    bool DeleteAllFeatures(int recordIndex);
    bool DeleteSelFeatures(CallBack*callBack=nullptr);
    bool CalculateField(int TargetFieldIndex,string Expression,CallBack*callBack=nullptr);
    bool MoveSelection(DOUBLE difx,DOUBLE dify,CallBack*callBack=nullptr);
    bool MoveSelectionByClientCoord(DOUBLE difx,DOUBLE dify,Display*pDisplay,CallBack*callBack=nullptr);
//Version:
    int GetHistoryMaxItems();
    void SetHistoryMaxItems(int maxItems);
    int GetUsableUndoTimes();
    int GetUsableRedoTimes();
    bool UndoEdit();
    bool RedoEdit();
    bool BeginNewVersion(string name,bool EditChanged=true);
    bool GetCurrentEditChanged();
    string GetCurrentUndoName();
    string GetCurrentRedoName();
    bool SetCurrentUndoName(string name);
    bool SetCurrentRedoName(string name);
    bool ClipForewardAllRedos();
    bool RemoveOneBackwardUndo();
    VersionManager* GetVersionManager();
    void SetVersionManager(VersionManager*pManager);
    bool AddField(string FieldName,VectorFieldType type,LONG FieldWidth,LONG DecimalCount,CallBack*callBack=nullptr);
    bool DeleteField(LONG FieldIndex,CallBack*callBack=nullptr);
protected:
    void InitialData(string name,VectorShapeType shapeType,TableDesc*desc,SpatialReference*pSpatial,bool is3d);
protected:
    VersionManager*pVersionManager;
    AutoPtr<FeatureClassEditor>editor;
    int HistoryCount;
    AutoPtr<MemVectors>pVectors;
    int CurrentPin;
    friend class MemFeatureWorkspace;
};

class SGIS_EXPORT MemFeatureWorkspace:
        public FeatureWorkspace
{
public:
    MemFeatureWorkspace();
    virtual~MemFeatureWorkspace();
//FeatureWorkspace:
    string GetDescription();
    bool IsDatasetFeatureClass(string name);
//MemFeatureWorkspace:
    AutoPtr<MemFeatureClass>CreateFeatureClass(string name,VectorShapeType type,TableDesc*pDesc,SpatialReference*psp);
    AutoPtr<MemFeatureClass>Create3DFeatureClass(string name,VectorShapeType type,TableDesc*pDesc,SpatialReference*psp);
};

class SGIS_EXPORT MemFeatureWorkspaceFactory:
        public FeatureWorkspaceFactory{
public:
    MemFeatureWorkspaceFactory();
    virtual~MemFeatureWorkspaceFactory();
//FeatureWorkspaceFactory:
    string GetDescription();
    AutoPtr<WorkspaceFactory>Clone();
//MemFeatureWorkspaceFactory:
    AutoPtr<MemFeatureWorkspace>CreateMemFeatureWorkspace();
};

}

#endif // MEMFEATURECLASS_H
