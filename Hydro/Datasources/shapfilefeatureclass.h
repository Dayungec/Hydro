#ifndef SHAPFILEFEATURECLASS_H
#define SHAPFILEFEATURECLASS_H
#include "Base/classfactory.h"
#include "Base/version.h"
#include "dataset.h"
#include "featureclasseditor.h"
class OGRDataSource;
class OGRLayer;
class OGRFeature;
namespace SGIS{

class SGIS_EXPORT ShapefileFeatureClass:
        public BufferWritable,
        public JsonWritable,
        public FeatureClassEdit,
        public FeatureClass,
        public FeatureDataset,
        public FileDataset,
        public Version
{
public:
    ShapefileFeatureClass();
    virtual~ShapefileFeatureClass();
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
    bool SetSpatialReference(AutoPtr<SpatialReference>psp);
    AutoPtr<FeatureClass>Clone();
    void Dispose();
    void FlushCache();
    bool IsFromSameSource(FeatureClass*other);
 //FeatureDataset:
    int GetFeatureClassCount();
    AutoPtr<FeatureClass>GetFeatureClass(int nIndex);
    AutoPtr<FeatureClass>GetDefaultFeatureClass();
    string GetFeatureClassName(int nIndex);
    int FindFeatureClass(string Name);
//FileDataset:
    string GetName();
    bool IsDirectory();
    string GetPathName();
    bool OpenFromFile(string pathName);
    AutoPtr<Workspace>GetWorkspace();
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
public:
//BufferWritable:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
//JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
//public:
    bool AddField(string FieldName,VectorFieldType type,LONG FieldWidth,LONG DecimalCount,CallBack*callBack=nullptr);
    bool DeleteField(LONG FieldIndex,CallBack*callBack=nullptr);
protected:
    bool ResetReading(bool update=false);
    bool MoveTo(LONG RecordIndex);
    bool SaveEdit(string PathName,SpatialReference*psp,CallBack*callBack=nullptr);
protected:
    string dir;
    string fileName;
    OGRDataSource*poDS;
    OGRLayer*poLayer;
    OGRFeature*poFeature;
    int FormerRecord;
    int FeatureCount;
    VersionManager*pVersionManager;
    AutoPtr<FeatureClassEditor>editor;
    int HistoryCount;
    bool update;
    bool isUtf8;
};

class SGIS_EXPORT ShapefileWorkspace:
        public FeatureWorkspace,
        public FileWorkspace
{
public:
    ShapefileWorkspace(string pathName="");
    virtual~ShapefileWorkspace();
 //FeatureWorkspace:
    string GetDescription();
 //FileWorkspace:
    string GetPathName();
    bool IsDirectory();
    vector<string>GetDatasetNames();
    AutoPtr<FileDataset>OpenDataset(string name);
    bool IsDatasetFeatureClass(string name);
    bool CanDeleteDataset(string name);
    bool DeleteDataset(string name);
    bool CanRenameDataset(string name);
    bool RenameDataset(string oldName,string newName);
 //ShapfileWorkspace:
    AutoPtr<ShapefileFeatureClass>OpenFeatureClass(string Name);
    AutoPtr<ShapefileFeatureClass>CreateFeatureClass(string Name,VectorShapeType type,TableDesc*pDesc,SpatialReference*psp);
    AutoPtr<ShapefileFeatureClass>Create3DFeatureClass(string Name,VectorShapeType type,TableDesc*pDesc,SpatialReference*psp);
    static bool RemoveShapefile(string path);
protected:
    string pathName;
};

class SGIS_EXPORT ShapefileWorkspaceFactory:
        public FeatureWorkspaceFactory,
        public FileWorkspaceFactory{
public:
    ShapefileWorkspaceFactory();
    virtual~ShapefileWorkspaceFactory();
//FeatureWorkspaceFactory:
    string GetDescription();
    AutoPtr<WorkspaceFactory>Clone();
//FileWorkspaceFactory:
    vector<string>GetWorkspaceNames(string dir);
    bool IsWorkspaceDirectory();
    AutoPtr<FileWorkspace>OpenFromFile(string pathName);
};

}
#endif // SHAPFILEFEATURECLASS_H
