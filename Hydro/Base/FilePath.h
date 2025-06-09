#ifndef FILEPATH_H
#define FILEPATH_H
#include "base_globle.h"
#include "datetime.h"

namespace SGIS{

class SGIS_EXPORT FileFilterInterp{
public:
    FileFilterInterp(string Filter);
    virtual~FileFilterInterp();
    bool IsFileInFilter(string FileName);
    string GetFilter();
protected:
    string Filter;
    vector<string>exps;
};

class SGIS_EXPORT FileNameFilter{
public:
    FileNameFilter(string filterString="");
    virtual~FileNameFilter();
    void PutFilter(string filterString);
    string GetFilterString();
    bool IsFileInFilter(string fileName);
    int GetSize();
    string GetFilter(int index);
protected:
    vector<FileFilterInterp*>Filters;
    string FilterString;
};

class SGIS_EXPORT FilePath
{
public:
    static string GetDir(string pathName);
    static string GetFileName(string pathName);
    static string GetFileNameNoExa(string pathName);
    static string GetFileExa(string pathName);
    static string GetCurrentDir();
    static string ConvertToStardardPath(string pathName);
    static bool IsFileExisting(string pathName);
    static bool IsDirExisting(string pathName);
    static DateTime GetFileLastWriteTime(string pathName);
    static bool IsFileOrDirExisting(string pathName,bool&bDir);
    static bool CopyFile(string fromPath,string toPath);
    static bool RemoveFile(string pathName);
    static bool ValidFilePath(string&currentpath,string relativePath);
    static bool ValidDirPath(string&currentpath,string relativePath);
};

class SGIS_EXPORT FileDirectory{
public:
    FileDirectory(string directory="");
    virtual~FileDirectory();
    string GetDirectory();
    void SetDirectory(string directory);
    vector<string>FindFiles(string fileFilter);
    vector<string>FindFileNames(string fileFilter);
    vector<string>FindDirectories();
    bool ChangeSubFolderName(string oldDirName,string newDirName);
    bool ChangeSubFileName(string oldFileName,string newFileName);
    bool CreateFolder(bool bCascade=false);
    bool EmptyFolder();
    bool DeleteFolder();
    static vector<string>EnumDrivers();
protected:
    inline bool is_special_dir(const char *path);
    inline bool is_dir(int attrib);
    inline void get_file_path(const char *path, const char *file_name, char *file_path);
    bool DeleteAllFiles(string strPath);
protected:
    string directory;
};

class SGIS_EXPORT DateFileFilter{
public:
    DateFileFilter(string Filter="");
    virtual~DateFileFilter();
    bool SetFilter(string filter);
    bool GetValid();
    vector<string>GetDateFiles(string dir,DateTime fromTime,DateTime endTime);
    bool GetDateFiles(string dir,DateTime fromTime,DateTime endTime,vector<string>&files,vector<DateTime>&dts);
    string GetLastFile(string dir,DateTime fromTime,DateTime endTime);
    string GetLastFileWithLeftNeighbor(string dir,DateTime fromTime,DateTime endTime);
    string GetLastFileWithRightNeighbor(string dir,DateTime fromTime,DateTime endTime);
    string GetLastFileWithDoubleNeighbor(string dir,DateTime fromTime,DateTime endTime);
    string GetFilter();
protected:
    bool GetDateTimeFromFilter(string FileName,DateTime&dateTime,int year=0,int month=0,int day=0,int hour=0);
    void SetInvalid();
    string GetRealFix(string fix,int year);
    string GetRealFix(string fix,int year,int month);
    string GetRealFix(string fix,int year,int month,int day);
protected:
    string Filter;
    vector<string>exps;
    vector<int>lens;
    vector<int>states;
    bool bValid;
    vector<int>dir_states;
    vector<string>dir_prefixs;
    vector<string>dir_suffixs;
    int dir_dateLevel;
    string dir_prefix;
    /*
    {y}/{y02},{M}不用0填充的月份/{M02}0填充的月份,{d}/{d02},{h}/{h02},{m02},{s02},{H02}小时偏移
    {x}/{x0}任意长度字符,{x4}4位任意字符
    目录部分,可以是{y}/{y02}，{M}/{M02}，{d}/{d02},{h}/{h02}，不允许出现"{x"，{m02},{s02}
    目录部分的时间必须从大到小依次出现,允许重复，如{y}/{y}{M02}/
    文件名部分不允许出现非固定长度的时间，如{M},{h}等
    文件名部分非固定长度的{x}/{x0},必须要有一个固定长度的非日期字符串与之配合并能推算位置
    如合法：{x}{M02}_ds,{x}{M02}{d02}_ds,非法{x}{M02}{x}_ds
    以下示例:
    {y}/dw/sj/pl{M}_dw/rw/{d}/{x0}_{h02}_{m02}_{s02}.tif
    {y}/dw/sj/pl{M02}_dw/rw/{d02}/{h02}_{m02}_{s02}{x5}.png
    */
};


}

#endif // FILEPATH_H
