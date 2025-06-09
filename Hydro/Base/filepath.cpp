#include "FilePath.h"
#ifdef Q_OS_LINUX
#include "sys/types.h"
#include <iostream>
#include "dirent.h"
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fstream>
#include<vector>
#include <iostream>
#include <cstring>
//#include <experimental/filesystem>
//#include <filesystem>
#else
#include <fstream>
#include <filesystem>
#include <io.h>
#include <direct.h>
#endif
#include "StringHelper.h"
#include "QDir"

namespace SGIS{

string FilePath::GetDir(string pathName){
    int nPos=pathName.find_last_of('\\');
    if(nPos>=0) pathName=FilePath::ConvertToStardardPath(pathName);
    nPos=pathName.find_last_of('/');
    if(nPos==-1) return "";
    return pathName.substr(0,nPos);
}


string FilePath::GetFileName(string pathName){
    int nPos=pathName.find_last_of('\\');
    if(nPos>=0) pathName=FilePath::ConvertToStardardPath(pathName);
    nPos=pathName.find_last_of('/');
    if(nPos==-1) return pathName;
    return pathName.substr(nPos+1,pathName.length()-nPos-1);
}

string FilePath::GetFileNameNoExa(string pathName){
    string fileName=FilePath::GetFileName(pathName);
    int nPos=fileName.find_last_of('.');
    if(nPos==-1) return fileName;
    return fileName.substr(0,nPos);
}

string FilePath::GetFileExa(string pathName){
    string fileName=FilePath::GetFileName(pathName);
    int nPos=fileName.find_last_of('.');
    if(nPos==-1) return "";
    return fileName.substr(nPos+1,fileName.length()-nPos-1);
}

string FilePath::GetCurrentDir(){
    char *buffer;
    if((buffer = getcwd(NULL, 0)) == NULL)
    {
        return "";
    }
    else
    {
        string sDir=buffer;
        free(buffer);
        return StringHelper::FromLocalString(sDir);
    }
}

string FilePath::ConvertToStardardPath(string pathName){
    vector<string>paths;
    string curDir=pathName;
    string buffers="";
    bool firstD=false;
    pathName=StringHelper::Trim(pathName);
    int len=pathName.length();
    for(int k=0;k<len;k++){
        char c=pathName[k];
        if((c=='/')||(c=='\\')){
            if(k==0){
                firstD=true;
            }
            else{
                paths.push_back(buffers);
                buffers="";
            }
        }
        else{
            buffers+=c;
        }
    }
    if(!buffers.empty()){
       paths.push_back(buffers);
    }
    string sPath="";
    if(firstD) sPath+="/";
    for(int k=0;k<paths.size();k++){
        if(k==0)
            sPath+=paths[k];
        else
            sPath+="/"+paths[k];
    }
    return sPath;
}

bool FilePath::IsFileExisting(string pathName){
#ifdef Q_OS_LINUX
    pathName=ConvertToStardardPath(pathName);
    //if(std::filesystem::exists(pathName)){
         //if(std::filesystem::is_regular_file(pathName)) return true;
    //}
    //return false;
    QFileInfo fileInfo(QString::fromStdString(pathName));
    return (fileInfo.exists()&&(fileInfo.isFile()));
#else
    pathName=StringHelper::ToLocalString(pathName);
    pathName=ConvertToStardardPath(pathName);
     struct _finddata_t fileinfo;
    intptr_t f_handle=_findfirst(pathName.c_str(), &fileinfo);
    if(f_handle==-1) return false;
    _findclose(f_handle);
    if (fileinfo.attrib & _A_SUBDIR) return false;
    return true;
#endif
}

bool FilePath::IsDirExisting(string pathName){
#ifdef Q_OS_LINUX
    pathName=ConvertToStardardPath(pathName);
    //if(std::filesystem::exists(pathName)){
         //if(std::filesystem::is_directory(pathName)) return true;
    //}
    //return false;
    QFileInfo fileInfo(QString::fromStdString(pathName));
    return (fileInfo.exists()&&(fileInfo.isDir()));
#else
    pathName=StringHelper::ToLocalString(pathName);
    pathName=ConvertToStardardPath(pathName);
     struct _finddata_t fileinfo;
    intptr_t f_handle=_findfirst(pathName.c_str(), &fileinfo);
    if(f_handle==-1) return false;
    _findclose(f_handle);
    if (!(fileinfo.attrib & _A_SUBDIR)) return false;
    return true;
#endif
}

DateTime FilePath::GetFileLastWriteTime(string pathName){
   #ifdef Q_OS_LINUX
    struct stat fileStat;
        if (stat(pathName.c_str(), &fileStat) == 0) {
            time_t lastModTime = fileStat.st_mtime;
            double systemTime = static_cast<double>(lastModTime);
            DateTime time(1970,1,1,0,0,0);
            time=time+TimeSpan(systemTime);
            return time;
        } else {
            throw std::runtime_error("GetFileLastWriteTime in Filepath.cpp Error");
        }
    #else
    pathName=StringHelper::ToLocalString(pathName);
    auto lstTime = std::filesystem::last_write_time(pathName);
    auto elapse = std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::file_time_type::clock::now().time_since_epoch() - std::chrono::system_clock::now().time_since_epoch()).count();
    auto systemTime = std::chrono::duration_cast<std::chrono::seconds>(lstTime.time_since_epoch()).count() - elapse;
    DateTime time(1970,1,1,0,0,0);
    time=time+TimeSpan(systemTime);
    return time;
    #endif

}

bool FilePath::IsFileOrDirExisting(string pathName,bool&bDir){
    if(IsFileExisting(pathName)){
        bDir=false;
        return true;
    }
    if(IsDirExisting(pathName)){
        bDir=true;
        return true;
    }
    return false;
}

bool FilePath::CopyFile(string fromPath,string toPath){
    fromPath=StringHelper::ToLocalString(fromPath);
    toPath=StringHelper::ToLocalString(toPath);
    std::ifstream in;
    std::ofstream out;
    try
    {
        in.open(fromPath.c_str(), std::ios::binary);//打开源文件
        if (in.fail())//打开源文件失败
        {
            in.close();
            out.close();
            return false;
        }
        out.open(toPath.c_str(), std::ios::binary);//创建目标文件
        if (out.fail())//创建文件失败
        {
            in.close();
            out.close();
            return false;
        }
        else
        {
            out << in.rdbuf();
            out.flush();
            out.close();
            in.close();
            return true;
        }
    }
    catch (std::exception e)
    {
        return false;
    }
}

bool FilePath::RemoveFile(string pathName){
    if(!IsFileExisting(pathName)) return false;
    pathName=StringHelper::ToLocalString(pathName);
    if(remove(pathName.c_str()) == -1) return false;
    return true;
}

bool FilePath::ValidFilePath(string&currentpath,string relativePath){
    if(IsFileExisting(currentpath)) return true;
    vector<string>subpaths;
    string curp=currentpath;
    while(curp!=""){
        string fileName=FilePath::GetFileName(curp);
        string dir=FilePath::GetDir(curp);
        subpaths.push_back(fileName);
        curp=dir;
    }
    int subsize=subpaths.size();
    if(subsize==0) return false;
    curp=subpaths[0];
    if(IsFileExisting(relativePath+"/"+curp)){
        currentpath=relativePath+"/"+curp;
        return true;
    }
    for(int k=1;k<subpaths.size();k++){
        curp=subpaths[k]+"/"+curp;
        if(IsFileExisting(relativePath+"/"+curp)){
            currentpath=relativePath+"/"+curp;
            return true;
        }
    }
    return false;
}


bool FilePath::ValidDirPath(string&currentpath,string relativePath){
    if(IsDirExisting(currentpath)) return true;
    vector<string>subpaths;
    string curp=currentpath;
    while(curp!=""){
        string fileName=FilePath::GetFileName(curp);
        string dir=FilePath::GetDir(curp);
        subpaths.push_back(fileName);
        curp=dir;
    }
    int subsize=subpaths.size();
    if(subsize==0) return false;
    curp=subpaths[0];
    if(IsDirExisting(relativePath+"/"+curp)){
        currentpath=relativePath+"/"+curp;
        return true;
    }
    for(int k=1;k<subpaths.size();k++){
        curp=subpaths[k]+"/"+curp;
        if(IsDirExisting(relativePath+"/"+curp)){
            currentpath=relativePath+"/"+curp;
            return true;
        }
    }
    return false;
}


FileFilterInterp::FileFilterInterp(string Filter){
    this->Filter=Filter;
    int len=Filter.length();
    int Pos=0;
    string subs;
    while(Pos<len){
        int fPos1=Filter.find('?',Pos);
        if(fPos1==-1) fPos1=len;
        int fPos2=Filter.find('*',Pos);
        if(fPos2==-1) fPos2=len;
        int tPos=min(fPos1,fPos2);
        if(tPos==len)
        {
            subs=Filter.substr(Pos,len-Pos);
            StringHelper::MakeUpper(subs);
            exps.push_back(subs);
            Pos=len;
        }
        else if(fPos1<fPos2)
        {
            if(fPos1-Pos>0){
                subs=Filter.substr(Pos,fPos1-Pos);
                StringHelper::MakeUpper(subs);
                exps.push_back(subs);
            }
            exps.push_back("?");
            Pos=fPos1+1;
        }
        else
        {
            if(fPos2-Pos>0){
                subs=Filter.substr(Pos,fPos2-Pos);
                StringHelper::MakeUpper(subs);
                exps.push_back(subs);
            }
            exps.push_back("*");
            Pos=fPos2+1;
        }
    }
}

FileFilterInterp::~FileFilterInterp(){

}

bool FileFilterInterp::IsFileInFilter(string FileName){
    int Size=exps.size();
    if(Size==0) return false;
    int len=FileName.length();
    if(len==0) return false;
    int Pin=0;
    bool IsAny=false;
    StringHelper::MakeUpper(FileName);
    int lestCount=0;
    for(int k=0;k<Size;k++)
    {
        string exp=exps[k];
        if(exp=="*")
        {
            IsAny=true;
            Pin-=lestCount;
            continue;
        }
        else if(exp=="?")
        {
            if(Pin>=len)
                return false;
            else if(k==Size-1)
            {
                if(Pin+1!=len) return false;
            }
            if(!IsAny) Pin++;
            lestCount++;
            continue;
        }
        int expsize=exp.length();
        if(Pin+expsize>len) return false;
        if(!IsAny)
        {
            string str=FileName.substr(Pin,expsize);
            if(str!=exp) return false;
            if(k==Size-1)
            {
                if(exp!="*")
                {
                    if(Pin+exp.length()!=len) return false;
                }
            }
            Pin+=exp.length();
        }
        else
        {
            int fPin=FileName.find(exp,Pin);
            if(fPin==-1) return false;
            if(fPin-Pin<lestCount) return false;
            Pin=fPin;
            if(k==Size-1)
            {
                if(exp!="*")
                {
                    if(Pin+exp.length()!=len) return false;
                }
            }
            Pin=Pin+expsize;
        }
        IsAny=false;
        lestCount=0;
    }
    return true;
}

string FileFilterInterp::GetFilter(){
   return Filter;
}

FileNameFilter::FileNameFilter(string filterString){
    this->FilterString=filterString;
    if(this->FilterString.empty()) return;
    PutFilter(filterString);
}
FileNameFilter::~FileNameFilter(){
    for(int k=Filters.size()-1;k>=0;k--) delete Filters[k];
    Filters.clear();
}

void FileNameFilter::PutFilter(string filterString){
    for(int k=Filters.size()-1;k>=0;k--) delete Filters[k];
    Filters.clear();
    vector<string>subfilters=StringHelper::Split(filterString,";");
    for(int k=0;k<subfilters.size();k++){
        FileFilterInterp*newinter=new FileFilterInterp(subfilters[k]);
        Filters.push_back(newinter);
    }
    FilterString=filterString;
}

string FileNameFilter::GetFilterString(){
    return FilterString;
}

int FileNameFilter::GetSize(){
    return Filters.size();
}

string FileNameFilter::GetFilter(int index){
    return Filters[index]->GetFilter();
}

bool FileNameFilter::IsFileInFilter(string FileName){
    int Size=Filters.size();
    if(Size==0) return true;
    for(int k=0;k<Filters.size();k++)
    {
        if(Filters[k]->IsFileInFilter(FileName)) return true;
    }
    return false;
}


FileDirectory::FileDirectory(string directory){
    this->directory=directory;
}

FileDirectory::~FileDirectory(){

}

string FileDirectory::GetDirectory(){
    return directory;
}

void FileDirectory::SetDirectory(string directory){
    this->directory=directory;
}

vector<string>FileDirectory::FindFiles(string fileFilter){
    vector<string>files;
    string p;
#ifdef Q_OS_LINUX
    /*
    DIR* pDir = opendir(directory.c_str());
    if(pDir==nullptr)
        return {};
    struct dirent* ptr;
    string last=directory.substr(directory.length()-1,1);
    if(fileFilter==""){
        while((ptr = readdir(pDir))!=0) {
            if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0){
                unsigned char dType=ptr->d_type;
                if(dType!=8) continue;
                string subfileName="";
                string last=directory.substr(directory.length()-1,1);
                if((last=="/")||(last=="\\"))
                    subfileName=p.assign(directory).append(ptr->d_name);
                else
                    subfileName=p.assign(directory).append("/").append(ptr->d_name);
                files.push_back(subfileName);
            }
        }
    }
    else{
        FileNameFilter fnf(fileFilter);
        while((ptr = readdir(pDir))!=nullptr) {
            if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0){
                unsigned char dType=ptr->d_type;
                if(dType!=8) continue;
                if(fnf.IsFileInFilter(ptr->d_name)){
                    string subfileName="";
                    string last=directory.substr(directory.length()-1,1);
                    if((last=="/")||(last=="\\"))
                        subfileName=p.assign(directory).append(ptr->d_name);
                    else
                        subfileName=p.assign(directory).append("/").append(ptr->d_name);
                    files.push_back(subfileName);
                }
            }
        }
    }
    closedir(pDir);
    */
    QDir dir(QString::fromStdString(directory));
    string sDir=StringHelper::TrimRight(directory);
    if(sDir.length()>0){
        if((sDir[sDir.length()-1]=='/')||(sDir[sDir.length()-1]=='\\')) sDir=sDir.substr(0,sDir.length()-1);
    }
    if(fileFilter==""){
        QStringList qfiles = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QString &file : qfiles) {
             files.push_back(sDir+"/"+file.toStdString());
        }
    }
    else{
        QStringList qfiles = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        FileNameFilter fnf(fileFilter);
        for (const QString &file : qfiles) {
             string sfile=file.toStdString();
             if(fnf.IsFileInFilter(sfile))
                files.push_back(sDir+"/"+sfile);
        }
    }
 #else
    // 文件句柄
    intptr_t hFile = 0;
    // 文件信息
    struct _finddata_t fileinfo;
    string dire=StringHelper::ToLocalString(directory);
    if(fileFilter==""){
        if ((hFile = _findfirst(p.assign(dire).append("/*").c_str(), &fileinfo)) != -1) {
            do {
                // 保存文件的全路径
                if ((fileinfo.attrib & _A_SUBDIR)) continue;
                string subfileName="";
                string last=directory.substr(directory.length()-1,1);
                if((last=="/")||(last=="\\"))
                    subfileName=p.assign(dire).append(fileinfo.name);
                else
                    subfileName=p.assign(dire).append("/").append(fileinfo.name);
                files.push_back(StringHelper::FromLocalString(subfileName));
            } while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1
            _findclose(hFile);
        }
    }
    else{
        FileNameFilter fnf(fileFilter);
        if ((hFile = _findfirst(p.assign(dire).append("/*").c_str(), &fileinfo)) != -1) {
            do {
                // 保存文件的全路径
                if ((fileinfo.attrib & _A_SUBDIR)) continue;
                string fileinfoName=fileinfo.name;
                if(fnf.IsFileInFilter(fileinfoName)){
                    string subfileName="";
                    string last=dire.substr(dire.length()-1,1);
                    if((last=="/")||(last=="\\"))
                        subfileName=p.assign(dire).append(fileinfoName);
                    else
                        subfileName=p.assign(dire).append("/").append(fileinfoName);
                    files.push_back(StringHelper::FromLocalString(subfileName));
                }
            } while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1
            _findclose(hFile);
        }
    }
#endif
    return files;
}

vector<string>FileDirectory::FindFileNames(string fileFilter){
    vector<string>files;
     string p;
#ifdef Q_OS_LINUX
    /*
    DIR *pDir;
    struct dirent* ptr;
    pDir = opendir(directory.c_str());
    if(pDir==nullptr)
        return {};
    string last=directory.substr(directory.length()-1,1);
    if(fileFilter==""){
        while((ptr = readdir(pDir))!=0) {
            if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0){
                unsigned char dType=ptr->d_type;
                if(dType!=8) continue;
                string subfileName=ptr->d_name;
                files.push_back(subfileName);
            }
        }
    }
    else{
        FileNameFilter fnf(fileFilter);
        while((ptr = readdir(pDir))!=0) {
            if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0){
                unsigned char dType=ptr->d_type;
                if(dType!=8) continue;
                if(fnf.IsFileInFilter(ptr->d_name)){
                    string subfileName=ptr->d_name;
                    files.push_back(subfileName);
                }
            }
        }
    }
    closedir(pDir);
    */
     QDir dir(QString::fromStdString(directory));
     if(fileFilter==""){
         QStringList qfiles = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
         for (const QString &file : qfiles) {
              files.push_back(file.toStdString());
         }
     }
     else{
         QStringList qfiles = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
         FileNameFilter fnf(fileFilter);
         for (const QString &file : qfiles) {
              string sfile=file.toStdString();
              if(fnf.IsFileInFilter(sfile))
                 files.push_back(sfile);
         }
     }
#else
    // 文件句柄
    intptr_t hFile = 0;
    // 文件信息
    struct _finddata_t fileinfo;
    string dire=StringHelper::ToLocalString(directory);
    if(fileFilter==""){
        if ((hFile = _findfirst(p.assign(dire).append("/*").c_str(), &fileinfo)) != -1) {
            do {
                // 保存文件的全路径
                if ((fileinfo.attrib & _A_SUBDIR)){
                    if(_findnext(hFile, &fileinfo)!=0) break;
                    continue;
                }
                string name=StringHelper::FromLocalString(fileinfo.name);
                files.push_back(name);
            } while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1
            _findclose(hFile);
        }
    }
    else{
        FileNameFilter fnf(fileFilter);
        if ((hFile = _findfirst(p.assign(dire).append("/*").c_str(), &fileinfo)) != -1) {
            do {
                // 保存文件的全路径
                if ((fileinfo.attrib & _A_SUBDIR)){
                    continue;
                }
                if((fileinfo.attrib & _A_HIDDEN)||(fileinfo.attrib & _A_SYSTEM)){
                    continue;
                }
                string name=StringHelper::FromLocalString(fileinfo.name);
                if(fnf.IsFileInFilter(name)){
                   files.push_back(name);
                }
            } while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1
            _findclose(hFile);
        }
    }
#endif
    return files;
}

vector<string>FileDirectory::FindDirectories(){
    vector<string>dirs;
    string p;
#ifdef Q_OS_LINUX
    /*
    DIR *pDir;
    struct dirent* ptr;
    pDir = opendir(directory.c_str());
    if(pDir==nullptr)
        return {};
    string last=directory.substr(directory.length()-1,1);
    while((ptr = readdir(pDir))!=0) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0){
            unsigned char dType=ptr->d_type;
            if(dType!=4) continue;
            string subfileName="";
            string last=directory.substr(directory.length()-1,1);
            if((last=="/")||(last=="\\"))
                subfileName=p.assign(directory).append(ptr->d_name);
            else
                subfileName=p.assign(directory).append("/").append(ptr->d_name);
            dirs.push_back(subfileName);
        }
    }
    */
    QDir dir(QString::fromStdString(directory));
    string sDir=StringHelper::TrimRight(directory);
    if(sDir.length()>0){
        if((sDir[sDir.length()-1]=='/')||(sDir[sDir.length()-1]=='\\')) sDir=sDir.substr(0,sDir.length()-1);
    }
    QStringList qfiles = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &file : qfiles) {
        dirs.push_back(sDir+"/"+file.toStdString());
    }
#else
    // 文件句柄
    intptr_t hFile = 0;
    // 文件信息

    struct _finddata_t fileinfo;
    string dire=StringHelper::ToLocalString(directory);

    if ((hFile = _findfirst(p.assign(dire).append("/*").c_str(), &fileinfo)) != -1) {
        do {
            // 保存文件的全路径
            if (!(fileinfo.attrib & _A_SUBDIR)){
                continue;
            }
            if((fileinfo.attrib & _A_HIDDEN)||(fileinfo.attrib & _A_SYSTEM)){
                continue;
            }
            string fn=StringHelper::FromLocalString(fileinfo.name);
            if((fn!=".")&&(fn!="..")){
                string subfileName="";
                string last=directory.substr(directory.length()-1,1);
                if((last=="/")||(last=="\\"))
                    subfileName=p.assign(dire).append(fileinfo.name);
                else
                    subfileName=p.assign(dire).append("/").append(fileinfo.name);
                dirs.push_back(StringHelper::FromLocalString(subfileName));
            }
        } while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1
        _findclose(hFile);
    }
#endif
    return dirs;
}

bool FileDirectory::ChangeSubFolderName(string oldDirName,string newDirName){
    string dir=FilePath::GetDir(oldDirName);
    if(dir!="") return false;
    dir=FilePath::GetDir(newDirName);
    if(dir!="") return false;
    if(!FilePath::IsDirExisting(directory+"/"+oldDirName)) return false;
    string n1=oldDirName;
    string n2=newDirName;
    StringHelper::TrimToLower(n1);
    StringHelper::TrimToLower(n2);
    if(n1==n2) return false;
    n1=directory+"/"+oldDirName;
    n2=directory+"/"+newDirName;
    return (::rename(StringHelper::ToLocalString(n1).c_str(),StringHelper::ToLocalString(n2).c_str())==0);
}

bool FileDirectory::ChangeSubFileName(string oldFileName,string newFileName){
    string dir=FilePath::GetDir(oldFileName);
    if(dir!="") return false;
    dir=FilePath::GetDir(newFileName);
    if(dir!="") return false;
    if(!FilePath::IsFileExisting(directory+"/"+oldFileName)) return false;
    string n1=oldFileName;
    string n2=newFileName;
    StringHelper::TrimToLower(n1);
    StringHelper::TrimToLower(n2);
    if(n1==n2) return false;
    n1=directory+"/"+oldFileName;
    n2=directory+"/"+newFileName;
    return (::rename(StringHelper::ToLocalString(n1).c_str(),StringHelper::ToLocalString(n2).c_str())==0);
}

bool FileDirectory::CreateFolder(bool bCascade){
#ifdef Q_OS_LINUX
    if(bCascade){
        vector<string>paths;
        string curDir=directory;
        bool bLinux=false;
        while(true){
            string fileName=FilePath::GetFileName(curDir);
            if(fileName!="") paths.push_back(fileName);
            curDir=FilePath::GetDir(curDir);
            if(!fileName.empty()) bLinux=true;
            if(curDir=="") break;
        }
        if(paths.size()==0) return false;
        curDir="";
        if(bLinux) curDir="/";
        for(int k=paths.size()-1;k>=0;k--){
            if(curDir=="")
                curDir+=paths[k];
            else{
                string last=curDir.substr(curDir.length()-1,1);
                if((last=="/")||(last=="\\"))
                    curDir+=paths[k];
                else
                    curDir+="/"+paths[k];
            }

            QString str=QString::fromStdString(curDir);
            QDir dir(str);
            if(!dir.exists()){
                if(!dir.mkdir(str)) return false;
            }
            /*
            if (0 != access(curDir.c_str(), 0))
            {
                int state=mkdir(curDir.c_str(),S_IRWXU); // 返回0 表示成功， 返回 -1表示错误，并且会设置errno值。
                if (state!=0) return false;
            }
            */
        }
        return true;
    }
    else{
        string folderPath = directory;
        QString str=QString::fromStdString(folderPath);
        QDir dir(str);
        if(!dir.exists()){
            if(!dir.mkdir(str)) return false;
        }
        return true;
        /*
        if (0 != access(folderPath.c_str(), 0))
        {
            int state=mkdir(folderPath.c_str(),S_IRWXU);   // 返回0 表示成功， 返回 -1表示错误，并且会设置errno值。
            return (state==0);
        }
        else
            return true;
        */
    }
    return false;
#else
    if(bCascade){
        vector<string>paths;
        string curDir=directory;
        while(true){
            string fileName=FilePath::GetFileName(curDir);
            if(fileName!="") paths.push_back(fileName);
            curDir=FilePath::GetDir(curDir);
            if(curDir=="") break;
        }
        if(paths.size()==0) return false;
        curDir="";
        for(int k=paths.size()-1;k>=0;k--){
            if(curDir=="")
               curDir+=paths[k];
            else{
                string last=curDir.substr(curDir.length()-1,1);
                if((last=="/")||(last=="\\"))
                    curDir+=paths[k];
                else
                    curDir+="/"+paths[k];
            }
            if (0 != access(StringHelper::ToLocalString(curDir).c_str(), 0))
            {
                int state=mkdir(StringHelper::ToLocalString(curDir).c_str());
                if (state!=0) return false;
            }
        }
        return true;
    }
    else{
        string folderPath = StringHelper::ToLocalString(directory);
        if (0 != access(folderPath.c_str(), 0))
        {
            // if this folder not exist, create a new one.
            int state=mkdir(folderPath.c_str());   // 返回 0 表示创建成功，-1 表示失败
            return (state==0);
        }
        else
            return true;
    }
    return false;
#endif
}

bool FileDirectory::is_special_dir(const char *path)
{
    return strcmp(path, "..") == 0 || strcmp(path, ".") == 0;
}

bool FileDirectory::is_dir(int attrib)
{
    return attrib == 16 || attrib == 18 || attrib == 20;
}

inline void FileDirectory::get_file_path(const char *path, const char *file_name, char *file_path)
{
#ifdef Q_OS_LINUX
    strncpy(file_path,file_name,sizeof(char) * _MAX_PATH);
    file_path[strlen(file_path) - 1] = '\0';
    int length=strlen(file_name);
    strncat(file_path,file_name,length);
    strncat(file_path,"/*",2);
#else
    strcpy_s(file_path, sizeof(char) * _MAX_PATH, path);
    file_path[strlen(file_path) - 1] = '\0';
    strcat_s(file_path, sizeof(char) * _MAX_PATH, file_name);
    strcat_s(file_path, sizeof(char) * _MAX_PATH, "/*");
#endif
}

bool FileDirectory::DeleteAllFiles(string strPath)
{
#ifdef Q_OS_LINUX
    DIR *pDir = NULL;
        struct dirent *dmsg;
        char szFileName[128];
        char szFolderName[128];
        strcpy(szFolderName, strPath.c_str());
        strcat(szFolderName, "/%s");
        if ((pDir = opendir(strPath.c_str())) != NULL)
        {
            // 遍历目录并删除文件
            while ((dmsg = readdir(pDir)) != NULL)
            {
                if (strcmp(dmsg->d_name, ".") != 0 && strcmp(dmsg->d_name, "..") != 0)
                {
                    sprintf(szFileName, szFolderName, dmsg->d_name);

                    string tmp = szFileName;
                    //如果是文件夹，名称中不包含"."
                    if (tmp.find(".") == -1){
                        DeleteAllFiles(szFileName);
                    }
                    remove(szFileName);
                }
            }
            closedir(pDir);
            return true;
        }//关闭打开的文件句柄，并释放关联资源，否则无法删除空目录
    return false;
#else
    _finddata_t dir_info;  // 文件夹信息
    _finddata_t file_info;  // 文件信息
    intptr_t f_handle;

    char tmp_path[_MAX_PATH];
    string folderPath = StringHelper::ToLocalString(strPath);
    if((f_handle = _findfirst(folderPath.c_str(),&dir_info)) != -1)
    {
        while ((_findnext(f_handle,&file_info)) == 0)
        {
            if (is_special_dir(file_info.name)) continue;
            if(is_dir(file_info.attrib))   //如果是目录，生成完整的路径
            {
                get_file_path(strPath.c_str(), file_info.name, tmp_path);
                DeleteAllFiles(tmp_path);    //开始递归删除目录中的内容
                tmp_path[strlen(tmp_path) - 2] = '\0';
                if(file_info.attrib != 20){
                    // 删除空目录,必须在递归返回前调用 _findclose, 否则无法删除目录
                    if (_rmdir(tmp_path) == -1){
                        _findclose(f_handle);
                        return false;
                    }
                }
            }
            else
            {
                strcpy_s(tmp_path,folderPath.c_str());
                tmp_path[strlen(tmp_path) - 1] = '\0';
                strcat_s(tmp_path,file_info.name);  // 生成完整文件路径

                if(remove(tmp_path) == -1)
                {
                    _findclose(f_handle);
                    return false;
                }
            }
        }
        _findclose(f_handle);//关闭打开的文件句柄，并释放关联资源，否则无法删除空目录
        return true;
    }
    return false;
#endif
}

bool FileDirectory::EmptyFolder(){
    return DeleteAllFiles(directory+"/*");
}

bool FileDirectory::DeleteFolder(){
    if(!EmptyFolder()) return false;
 #ifdef Q_OS_LINUX
     if(rmdir(directory.c_str()) == -1){
         return false;
     }
     return true;
 #else
     if(_rmdir(directory.c_str()) == -1){
         return false;
     }
     return true;
 #endif
}

vector<string>FileDirectory::EnumDrivers(){
    vector<string>drivers;
    foreach(QFileInfo info,QDir::drives()){
        drivers.push_back(info.absoluteFilePath().toStdString());
    }
    return drivers;
}


DateFileFilter::DateFileFilter(string Filter){
    this->Filter=Filter;
    dir_prefix="";
    bValid=false;
    if(!Filter.empty()) SetFilter(Filter);
}

DateFileFilter::~DateFileFilter(){

}

bool DateFileFilter::SetFilter(string Filter){
    this->Filter=Filter;
    dir_prefix="";
    SetInvalid();
    int len=Filter.length();
    int Pos=0;
    for(int k=0;k<len;k++){
        if(Filter[k]=='{'){
            int tPos=Filter.find('}',k+1);
            if(tPos==-1) continue;
            string sub=Filter.substr(k,tPos-k+1);
            int dlen=0;
            int state=0;
            if(sub=="{y}"){
                dlen=4;
                state=1;
            }
            else if(sub=="{y02}"){
                dlen=2;
                state=1;
            }
            else if(sub=="{M}"){
                dlen=2;
                state=-2;
            }
            else if(sub=="{M02}"){
                dlen=2;
                state=2;
            }
            else if(sub=="{d}"){
                dlen=2;
                state=-3;
            }
            else if(sub=="{d02}"){
                dlen=2;
                state=3;
            }
            else if(sub=="{h}"){
                dlen=2;
                state=-4;
            }
            else if(sub=="{h02}"){
                dlen=2;
                state=4;
            }
            else if(sub=="{m02}"){
                dlen=2;
                state=5;
            }
            else if(sub=="{s02}"){
                dlen=2;
                state=6;
            }
            else if(sub=="{H01}"){
                dlen=1;
                state=7;
            }
            else if(sub=="{H02}"){
                dlen=2;
                state=7;
            }
            else if(sub=="{H03}"){
                dlen=3;
                state=7;
            }
            else if((sub=="{x}")||(sub=="{x0}")){
                dlen=0;
                state=50;
            }
            else{
                if(sub.substr(0,2)=="{x"){
                    string ss=sub.substr(2,sub.length()-3);
                    dlen=atoi(ss.c_str());
                    if(dlen<=0){
                        SetInvalid();
                        return false;
                    }
                    state=50;
                }
            }
            if(abs(state)>0){
                if(k-Pos>0){
                    exps.push_back(Filter.substr(Pos,k-Pos));
                    lens.push_back(k-Pos);
                    states.push_back(0);
                }
                exps.push_back(sub);
                Pos=k+sub.length();
                lens.push_back(dlen);
                states.push_back(state);
            }
        }
        else if((Filter[k]=='/')||(Filter[k]=='\\')){
            if(k>0){
                if((Filter[k-1]=='/')||(Filter[k-1]=='\\')){
                    Pos++;
                    continue;
                }
            }
            if(k-Pos>0){
                exps.push_back(Filter.substr(Pos,k-Pos));
                lens.push_back(k-Pos);
                states.push_back(0);
            }
            exps.push_back("/");
            Pos=k+1;
            lens.push_back(1);
            states.push_back(0);
        }
    }
    if(Pos<len){
        exps.push_back(Filter.substr(Pos,len-Pos));
        lens.push_back(len-Pos);
        states.push_back(0);
    }
    /*
    if(exps.size()>0){
        if(exps[0]=="/"){
            exps.erase(begin(exps));
            lens.erase(begin(lens));
            states.erase(begin(states));
        }
    }
    */
    int lastPin=-1;
    for(int k=0;k<exps.size();k++){
        if(exps[k]=="/"){
            lastPin=k;
        }
    }
    if(lastPin==exps.size()-1){
        SetInvalid();
        return false;
    }
    dir_dateLevel=0;
    vector<int>pins;
    vector<int>p_states;
    for(int k=0;k<lastPin;k++){
        if(abs(states[k])==dir_dateLevel+1){
            pins.push_back(k);
            if(states[k]==1){
                if(lens[k]==2)
                    p_states.push_back(-1);
                else
                    p_states.push_back(1);
            }
            else
                p_states.push_back(states[k]);
            dir_dateLevel++;
        }
        else if(abs(states[k])==50){
            SetInvalid();
            return false;
        }
        else if(abs(states[k])>dir_dateLevel){
            SetInvalid();
            return false;
        }
    }
    if(dir_dateLevel>4){
        SetInvalid();
        return false;
    }
    if(dir_dateLevel==0){
        dir_prefix="";
        for(int k=0;k<lastPin;k++){
            dir_prefix+=exps[k];
        }
    }
    vector<int>fromPins;
    int pin=0;
    for(int k=0;k<pins.size();k++){
        if(k==0){
            fromPins.push_back(0);
            continue;
        }
        pin=pins[k-1]+1;
        for(int l=pins[k]-1;l>pins[k-1];l--){
            if(exps[l]=="/"){
                pin=l;
                break;
            }
        }
        fromPins.push_back(pin);
    }
    for(int k=0;k<pins.size();k++){
        string prefix="",suffix="";
        int from=fromPins[k];
        for(int l=from;l<pins[k];l++){
            prefix+=exps[l];
        }
        int toPin;
        if(k==pins.size()-1)
            toPin=lastPin;
        else
            toPin=fromPins[k+1];
        for(int l=pins[k]+1;l<toPin;l++){
            suffix+=exps[l];
        }
        dir_states.push_back(p_states[k]);
        /*
        if(prefix.length()>0){
            if(prefix[0]=='/') prefix=prefix.substr(1,prefix.length()-1);
        }
        if(suffix.length()>0){
            if(suffix[suffix.length()-1]=='/') suffix=suffix.substr(0,suffix.length()-1);
        }
        */
        dir_prefixs.push_back(prefix);
        dir_suffixs.push_back(suffix);
    }
    for(int k=0;k<=lastPin;k++){
        exps.erase(begin(exps));
        lens.erase(begin(lens));
        states.erase(begin(states));
    }
    for(int k=states.size()-1;k>=0;k--){
        if(lens[k]==0){
            if(k>0){
                if(lens[k-1]==0){
                    exps.erase(begin(exps)+k);
                    lens.erase(begin(lens)+k);
                    states.erase(begin(states)+k);
                }
            }
        }
    }
    vector<bool>sds(7);
    for(int s=0;s<6;s++) sds[s]=false;
    for(int k=0;k<dir_states.size();k++){
        int s=abs(dir_states[k]);
        if((s>=1)&&(s<=6)) sds[s-1]=true;
    }
    for(int k=0;k<states.size();k++){
        int s=abs(states[k]);
        if((s>=1)&&(s<=6)) sds[s-1]=true;
        StringHelper::MakeUpper(exps[k]);
    }
    if((!sds[0])||(!sds[1])||(!sds[2])){
        SetInvalid();
        return false;
    }
    if((sds[5])&&((!sds[4])||(!sds[3]))){
        SetInvalid();
        return false;
    }
    if((sds[4])&&(!sds[3])){
        SetInvalid();
        return false;
    }
    for(int k=0;k<states.size();k++){
        if(lens[k]==0){
            bool hasReference=false;
            if(k==states.size()-1)
                hasReference=true;
            else if(k==states.size()-2){
                if(lens[k+1]>0) hasReference=true;
            }
            else{
                hasReference=true;
                for(int l=k+1;l<states.size();l++){
                    if(lens[l]==0){
                        hasReference=false;
                        break;
                    }
                    else if(states[l]==0){
                        break;
                    }
                }
            }
            if(!hasReference){
                SetInvalid();
                return false;
            }
        }
    }
    bValid=true;
    return true;
}

bool DateFileFilter::GetValid(){
    return bValid;
}

void DateFileFilter::SetInvalid(){
    bValid=false;
    exps.clear();
    lens.clear();
    states.clear();
    dir_states.clear();
    dir_prefixs.clear();
    dir_suffixs.clear();
}

string DateFileFilter::GetRealFix(string fix,int year){
    //{y}/{y02}，{M}/{M02}，{d}/{d02},{h}/{h02}
    int nPos=fix.find("{y02}");
    if(nPos>=0){
        string y02=to_string(year-((int)(year/100))*100);
        if(y02.length()==1) y02="0"+y02;
        while(true){
            fix=fix.replace(nPos,5,y02);
            nPos=fix.find("{y02}");
            if(nPos<0) break;
        }
    }
    nPos=fix.find("{y}");
    if(nPos>=0){
        string y=to_string(year);
        while(true){
            fix=fix.replace(nPos,3,y);
            nPos=fix.find("{y}");
            if(nPos<0) break;
        }
    }
    return fix;
}
string DateFileFilter::GetRealFix(string fix,int year,int month){
    fix=GetRealFix(fix,year);
    int nPos=fix.find("{M02}");
    if(nPos>=0){
        string m02=to_string(month);
        if(m02.length()==1) m02="0"+m02;
        while(true){
            fix=fix.replace(nPos,5,m02);
            nPos=fix.find("{M02}");
            if(nPos<0) break;
        }
    }
    nPos=fix.find("{m}");
    if(nPos>=0){
        string m=to_string(month);
        while(true){
            fix=fix.replace(nPos,3,m);
            nPos=fix.find("{m}");
            if(nPos<0) break;
        }
    }
    return fix;
}
string DateFileFilter::GetRealFix(string fix,int year,int month,int day){
    fix=GetRealFix(fix,year,month);
    int nPos=fix.find("{d02}");
    if(nPos>=0){
        string d02=to_string(day);
        if(d02.length()==1) d02="0"+d02;
        while(true){
            fix=fix.replace(nPos,5,d02);
            nPos=fix.find("{d02}");
            if(nPos<0) break;
        }
    }
    nPos=fix.find("{d}");
    if(nPos>=0){
        string d=to_string(day);
        while(true){
            fix=fix.replace(nPos,3,d);
            nPos=fix.find("{d}");
            if(nPos<0) break;
        }
    }
    return fix;
}

bool DateFileFilter::GetDateTimeFromFilter(string FileName,DateTime&dateTime,int m_year,int m_month,int m_day,int m_hour){
    if(!bValid) return false;
    int Size=exps.size();
    if(Size==0) return false;
    int len=FileName.length();
    if(len==0) return false;
    int year=m_year;
    int month=m_month;
    int day=m_day;
    int hour=m_hour;
    int minute=0;
    int second=0;
    int hourDif=0;
    string sub;
    int Pin=0;
    StringHelper::MakeUpper(FileName);
    for(int k=0;k<Size;k++){
        if(lens[k]==0){
            if(k==Size-1)
                break;
            else if(k==Size-2){
                Pin=len-lens[k+1];
            }
            else{
                int dlen=0;
                for(int l=k+1;l<states.size();l++){
                    if(states[l]==0){
                        int nPos=FileName.find(exps[l],dlen);
                        if(nPos<0) return false;
                        Pin=nPos-dlen;
                        break;
                    }
                    if(l==states.size()-1){
                        dlen+=lens[l];
                        Pin=len-dlen;
                        break;
                    }
                    dlen+=lens[l];
                }
            }
            continue;
        }
        switch(states[k]){
        case 0:
        {
            sub=FileName.substr(Pin,lens[k]);
            if(sub!=exps[k]) return false;
            break;
        }
        case 1:{
            sub=FileName.substr(Pin,lens[k]);
            year=atoi(sub.c_str());
            if(lens[k]==2) year=2000+year;
            if(year<=0) return false;
            break;
        }
        case 2:{
            sub=FileName.substr(Pin,lens[k]);
            month=atoi(sub.c_str());
            if((month<=0)||(month>12)) return false;
            break;
        }
        case 3:{
            sub=FileName.substr(Pin,lens[k]);
            day=atoi(sub.c_str());
            if((day<=0)||(day>31)) return false;
            break;
        }
        case 4:{
            sub=FileName.substr(Pin,lens[k]);
            hour=atoi(sub.c_str());
            if((hour<0)||(hour>59)) return false;
            break;
        }
        case 5:{
            sub=FileName.substr(Pin,lens[k]);
            minute=atoi(sub.c_str());
            if((minute<0)||(minute>59)) return false;
            break;
        }
        case 6:{
            sub=FileName.substr(Pin,lens[k]);
            second=atoi(sub.c_str());
            if((second<0)||(second>59)) return false;
            break;
        }
        case 7:{
            sub=FileName.substr(Pin,lens[k]);
            hourDif=atoi(sub.c_str());
            if(hourDif<0) return false;
            break;
        }
        }
        Pin+=lens[k];
    }
    if(Size>0){
        if(lens[Size-1]!=0){
            if(Pin!=len) return false;
        }
    }
    if((year<1889)||(month<0)||(month>12)||(day<0)||(day>31)||(hour>23)||(minute>59)||(second>59)) return false;
    dateTime=DateTime(year,month,day,hour,minute,second);
    if(hourDif>0){
        dateTime+=TimeSpan(hourDif,0,0);
    }
    return true;
}

vector<string>DateFileFilter::GetDateFiles(string dir,DateTime fromTime,DateTime endTime){
    if(!bValid) return {};
    int dirs=dir_dateLevel;
    string smDir=dir;
    if(!smDir.empty()){
        if((smDir[smDir.length()-1]!='/')||(smDir[smDir.length()-1]!='\\')){
            smDir+="/";
        }
    }
    if(!dir_prefix.empty()) smDir+=dir_prefix;
    vector<string>results;
    string searchFilter="";
    if(this->states.size()>0){
        if(states[0]==0) searchFilter=exps[0];
        searchFilter+="*";
        if(states[states.size()-1]==0) searchFilter+=exps[states.size()-1];
    }
    if(dirs==0){
        FileDirectory fd(smDir);
        vector<string>files=fd.FindFiles(searchFilter);
        DateTime dt;
        for(int k=0;k<files.size();k++){
            string fileName=FilePath::GetFileName(files[k]);
            if(this->GetDateTimeFromFilter(fileName,dt)){
                 if((dt>=fromTime)&&(dt<=endTime)) results.push_back(files[k]);
            }
        }
    }
    else if(dirs==1){
        int fromYear=fromTime.GetYear();
        int toYear=endTime.GetYear();
        string sYear;
        string sDir;
        for(int l=fromYear;l<=toYear;l++){
            sYear=to_string(l);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sDir=smDir+dir_prefixs[0]+sYear+dir_suffixs[0];
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,l)){
                    if((dt>=fromTime)&&(dt<=endTime)) results.push_back(files[k]);
                }
            }
        }
    }
    else if(dirs==2){
        int year=fromTime.GetYear();
        int month=fromTime.GetMonth();
        int monthes=(endTime.GetYear()-fromTime.GetYear())*12+endTime.GetMonth()-fromTime.GetMonth()+1;
        string sYear,sMonth;
        string sDir;
        for(int l=0;l<monthes;l++){
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDir=smDir+sYear+sMonth;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,year,month)){
                    if((dt>=fromTime)&&(dt<=endTime)) results.push_back(files[k]);
                }
            }
            month+=1;
            if(month>12){
                month=1;
                year++;
            }
        }
    }
    else if(dirs==3){
        int days=(endTime-fromTime).TotalDays()+1;
        Date curTime(fromTime.GetYear(),fromTime.GetMonth(),fromTime.GetDay());
        string sYear,sMonth,sDay;
        string sDir;
        for(int l=0;l<days;l++){
            int year=curTime.GetYear();
            int month=curTime.GetMonth();
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDay=to_string(curTime.GetDay());
            if((dir_states[2]==3)&&(sDay.length()==1)) sDay="0"+sDay;
            sDay=GetRealFix(dir_prefixs[2],year,month)+sDay+GetRealFix(dir_suffixs[2],year,month);
            sDir=smDir+sYear+sMonth+sDay;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,curTime.GetYear(),curTime.GetMonth(),curTime.GetDay())){
                    if((dt>=fromTime)&&(dt<=endTime)) results.push_back(files[k]);
                }
            }
            curTime+=1;
        }
    }
    else if(dirs==4){
        int hours=(endTime-fromTime).TotalHours()+1;
        DateTime curTime(fromTime.GetYear(),fromTime.GetMonth(),fromTime.GetDay(),fromTime.GetHour());
        string sYear,sMonth,sDay,sHour;
        string sDir;
        for(int l=0;l<hours;l++){
            int year=curTime.GetYear();
            int month=curTime.GetMonth();
            int day=curTime.GetDay();
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDay=to_string(day);
            if((dir_states[2]==3)&&(sDay.length()==1)) sDay="0"+sDay;
            sDay=GetRealFix(dir_prefixs[2],year,month)+sDay+GetRealFix(dir_suffixs[2],year,month);
            sHour=to_string(curTime.GetHour());
            if((dir_states[3]==4)&&(sHour.length()==1)) sHour="0"+sHour;
            sHour=GetRealFix(dir_prefixs[3],year,month,day)+sHour+GetRealFix(dir_suffixs[3],year,month,day);
            sDir=smDir+sYear+sMonth+sDay+sHour;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,curTime.GetYear(),curTime.GetMonth(),curTime.GetDay(),curTime.GetHour())){
                    if((dt>=fromTime)&&(dt<=endTime)) results.push_back(files[k]);
                }
            }
            curTime+=TimeSpan(1,0,0);
        }
    }
    return results;
}

bool DateFileFilter::GetDateFiles(string dir,DateTime fromTime,DateTime endTime,vector<string>&dfiles,vector<DateTime>&dts){
    if(!bValid) return false;
    int dirs=dir_dateLevel;
    string smDir=dir;
    if(!smDir.empty()){
        if((smDir[smDir.length()-1]!='/')||(smDir[smDir.length()-1]!='\\')){
            smDir+="/";
        }
    }
    if(!dir_prefix.empty()) smDir+=dir_prefix;
    string searchFilter="";
    if(this->states.size()>0){
        if(states[0]==0) searchFilter=exps[0];
        searchFilter+="*";
        if(states[states.size()-1]==0) searchFilter+=exps[states.size()-1];
    }
    if(dirs==0){
        FileDirectory fd(smDir);
        vector<string>files=fd.FindFiles(searchFilter);
        DateTime dt;
        for(int k=0;k<files.size();k++){
            string fileName=FilePath::GetFileName(files[k]);
            if(this->GetDateTimeFromFilter(fileName,dt)){
                if((dt>=fromTime)&&(dt<=endTime)){
                    dfiles.push_back(files[k]);
                    dts.push_back(dt);
                }
            }
        }
    }
    else if(dirs==1){
        int fromYear=fromTime.GetYear();
        int toYear=endTime.GetYear();
        string sYear;
        string sDir;
        for(int l=fromYear;l<=toYear;l++){
            sYear=to_string(l);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sDir=smDir+dir_prefixs[0]+sYear+dir_suffixs[0];
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,l)){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        dfiles.push_back(files[k]);
                        dts.push_back(dt);
                    }
                }
            }
        }
    }
    else if(dirs==2){
        int year=fromTime.GetYear();
        int month=fromTime.GetMonth();
        int monthes=(endTime.GetYear()-fromTime.GetYear())*12+endTime.GetMonth()-fromTime.GetMonth()+1;
        string sYear,sMonth;
        string sDir;
        for(int l=0;l<monthes;l++){
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDir=smDir+sYear+sMonth;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,year,month)){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        dfiles.push_back(files[k]);
                        dts.push_back(dt);
                    }
                }
            }
            month+=1;
            if(month>12){
                month=1;
                year++;
            }
        }
    }
    else if(dirs==3){
        int days=(endTime-fromTime).TotalDays()+1;
        Date curTime(fromTime.GetYear(),fromTime.GetMonth(),fromTime.GetDay());
        string sYear,sMonth,sDay;
        string sDir;
        for(int l=0;l<days;l++){
            int year=curTime.GetYear();
            int month=curTime.GetMonth();
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDay=to_string(curTime.GetDay());
            if((dir_states[2]==3)&&(sDay.length()==1)) sDay="0"+sDay;
            sDay=GetRealFix(dir_prefixs[2],year,month)+sDay+GetRealFix(dir_suffixs[2],year,month);
            sDir=smDir+sYear+sMonth+sDay;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,curTime.GetYear(),curTime.GetMonth(),curTime.GetDay())){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        dfiles.push_back(files[k]);
                        dts.push_back(dt);
                    }
                }
            }
            curTime+=1;
        }
    }
    else if(dirs==4){
        int hours=(endTime-fromTime).TotalHours()+1;
        DateTime curTime(fromTime.GetYear(),fromTime.GetMonth(),fromTime.GetDay(),fromTime.GetHour());
        string sYear,sMonth,sDay,sHour;
        string sDir;
        for(int l=0;l<hours;l++){
            int year=curTime.GetYear();
            int month=curTime.GetMonth();
            int day=curTime.GetDay();
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDay=to_string(day);
            if((dir_states[2]==3)&&(sDay.length()==1)) sDay="0"+sDay;
            sDay=GetRealFix(dir_prefixs[2],year,month)+sDay+GetRealFix(dir_suffixs[2],year,month);
            sHour=to_string(curTime.GetHour());
            if((dir_states[3]==4)&&(sHour.length()==1)) sHour="0"+sHour;
            sHour=GetRealFix(dir_prefixs[3],year,month,day)+sHour+GetRealFix(dir_suffixs[3],year,month,day);
            sDir=smDir+sYear+sMonth+sDay+sHour;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,curTime.GetYear(),curTime.GetMonth(),curTime.GetDay(),curTime.GetHour())){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        dfiles.push_back(files[k]);
                        dts.push_back(dt);
                    }
                }
            }
            curTime+=TimeSpan(1,0,0);
        }
    }
    return true;
}

string DateFileFilter::GetLastFile(string dir,DateTime fromTime,DateTime endTime){
    if(!bValid) return "";
    int dirs=dir_states.size();
    string smDir=dir;
    if(!smDir.empty()){
        if((smDir[smDir.length()-1]!='/')||(smDir[smDir.length()-1]!='\\')){
            smDir+="/";
        }
    }
    if(!dir_prefix.empty()) smDir+=dir_prefix;
    DateTime nowt=fromTime;
    string result="";
    string searchFilter="";
    if(this->states.size()>0){
        if(states[0]==0) searchFilter=exps[0];
        searchFilter+="*";
        if(states[states.size()-1]==0) searchFilter+=exps[states.size()-1];
    }
    if(dirs==0){
        FileDirectory fd(smDir);
        vector<string>files=fd.FindFiles(searchFilter);
        DateTime dt;
        for(int k=0;k<files.size();k++){
            string fileName=FilePath::GetFileName(files[k]);
            if(this->GetDateTimeFromFilter(fileName,dt)){
                if((dt>=fromTime)&&(dt<=endTime)){
                    if((dt>=nowt)||(result.empty())){
                        nowt=dt;
                        result=files[k];
                    }
                }
            }
        }
    }
    else if(dirs==1){
        int fromYear=fromTime.GetYear();
        int toYear=endTime.GetYear();
        string sYear;
        string sDir;
        for(int l=fromYear;l<=toYear;l++){
            sYear=to_string(l);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sDir=smDir+dir_prefixs[0]+sYear+dir_suffixs[0];
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,l)){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                        }
                    }
                }
            }
        }
    }
    else if(dirs==2){
        int year=fromTime.GetYear();
        int month=fromTime.GetMonth();
        int monthes=(endTime.GetYear()-fromTime.GetYear())*12+endTime.GetMonth()-fromTime.GetMonth()+1;
        string sYear,sMonth;
        string sDir;
        for(int l=0;l<monthes;l++){
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDir=smDir+sYear+sMonth;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,year,month)){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                        }
                    }
                }
            }
            month+=1;
            if(month>12){
                month=1;
                year++;
            }
        }
    }
    else if(dirs==3){
        int days=(endTime-fromTime).TotalDays()+1;
        Date curTime(fromTime.GetYear(),fromTime.GetMonth(),fromTime.GetDay());
        string sYear,sMonth,sDay;
        string sDir;
        for(int l=0;l<days;l++){
            int year=curTime.GetYear();
            int month=curTime.GetMonth();
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDay=to_string(curTime.GetDay());
            if((dir_states[2]==3)&&(sDay.length()==1)) sDay="0"+sDay;
            sDay=GetRealFix(dir_prefixs[2],year,month)+sDay+GetRealFix(dir_suffixs[2],year,month);
            sDir=smDir+sYear+sMonth+sDay;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,curTime.GetYear(),curTime.GetMonth(),curTime.GetDay())){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                        }
                    }
                }
            }
            curTime+=1;
        }
    }
    else if(dirs==4){
        int hours=(endTime-fromTime).TotalHours()+1;
        DateTime curTime(fromTime.GetYear(),fromTime.GetMonth(),fromTime.GetDay(),fromTime.GetHour());
        string sYear,sMonth,sDay,sHour;
        string sDir;
        for(int l=0;l<hours;l++){
            int year=curTime.GetYear();
            int month=curTime.GetMonth();
            int day=curTime.GetDay();
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDay=to_string(day);
            if((dir_states[2]==3)&&(sDay.length()==1)) sDay="0"+sDay;
            sDay=GetRealFix(dir_prefixs[2],year,month)+sDay+GetRealFix(dir_suffixs[2],year,month);
            sHour=to_string(curTime.GetHour());
            if((dir_states[3]==4)&&(sHour.length()==1)) sHour="0"+sHour;
            sHour=GetRealFix(dir_prefixs[3],year,month,day)+sHour+GetRealFix(dir_suffixs[3],year,month,day);
            sDir=smDir+sYear+sMonth+sDay+sHour;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,curTime.GetYear(),curTime.GetMonth(),curTime.GetDay(),curTime.GetHour())){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                        }
                    }
                }
            }
            curTime+=TimeSpan(1,0,0);
        }
    }
    return result;
}

string DateFileFilter::GetLastFileWithLeftNeighbor(string dir,DateTime fromTime,DateTime endTime){
    if(!bValid) return "";
    int dirs=dir_states.size();
    string smDir=dir;
    if(!smDir.empty()){
        if((smDir[smDir.length()-1]!='/')||(smDir[smDir.length()-1]!='\\')){
            smDir+="/";
        }
    }
    if(!dir_prefix.empty()) smDir+=dir_prefix;
    DateTime nowt=fromTime;
    string result="";
    string searchFilter="";
    if(this->states.size()>0){
        if(states[0]==0) searchFilter=exps[0];
        searchFilter+="*";
        if(states[states.size()-1]==0) searchFilter+=exps[states.size()-1];
    }
    if(dirs==0){
        FileDirectory fd(smDir);
        vector<string>files=fd.FindFiles(searchFilter);
        DateTime dt;
        for(int k=0;k<files.size();k++){
            string fileName=FilePath::GetFileName(files[k]);
            if(this->GetDateTimeFromFilter(fileName,dt)){
                if((dt>=fromTime)&&(dt<=endTime)){
                    if((dt>=nowt)||(result.empty())){
                        nowt=dt;
                        result=files[k];
                    }
                }
            }
        }
    }
    else if(dirs==1){
        int fromYear=fromTime.GetYear();
        int toYear=endTime.GetYear();
        fromYear--;
        string sYear;
        string sDir;
        for(int l=toYear;l>=fromYear;l--){
            sYear=to_string(l);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sDir=smDir+dir_prefixs[0]+sYear+dir_suffixs[0];
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            bool hasSet=false;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,l)){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                            hasSet=true;
                        }
                    }
                }
            }
            if(hasSet) return result;
        }
    }
    else if(dirs==2){
        DateTime ft=fromTime;
        if(ft.GetMonth()>=1)
            ft=DateTime(ft.GetYear(),ft.GetMonth()-1,ft.GetDay());
        else
            ft=DateTime(ft.GetYear()-1,12,ft.GetDay());
        DateTime tt=endTime;
        int year=tt.GetYear();
        int month=tt.GetMonth();
        int monthes=(tt.GetYear()-ft.GetYear())*12+tt.GetMonth()-ft.GetMonth()+1;
        string sYear,sMonth;
        string sDir;
        for(int l=monthes-1;l>=0;l--){
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDir=smDir+sYear+sMonth;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            bool hasSet=false;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,year,month)){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                            hasSet=true;
                        }
                    }
                }
            }
            if(hasSet) return result;
            month--;
            if(month<0){
                month=12;
                year--;
            }
        }
    }
    else if(dirs==3){
        int days=(endTime-fromTime).TotalDays()+2;
        Date curTime(endTime.GetYear(),endTime.GetMonth(),endTime.GetDay());
        string sYear,sMonth,sDay;
        string sDir;
        for(int l=days-1;l>=0;l--){
            int year=curTime.GetYear();
            int month=curTime.GetMonth();
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDay=to_string(curTime.GetDay());
            if((dir_states[2]==3)&&(sDay.length()==1)) sDay="0"+sDay;
            sDay=GetRealFix(dir_prefixs[2],year,month)+sDay+GetRealFix(dir_suffixs[2],year,month);
            sDir=smDir+sYear+sMonth+sDay;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            bool hasSet=false;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,curTime.GetYear(),curTime.GetMonth(),curTime.GetDay())){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                            hasSet=true;
                        }
                    }
                }
            }
            if(hasSet) return result;
            curTime-=1;
        }
    }
    else if(dirs==4){
        int hours=(endTime-fromTime).TotalHours()+2;
        DateTime curTime(endTime.GetYear(),endTime.GetMonth(),endTime.GetDay(),endTime.GetHour());
        string sYear,sMonth,sDay,sHour;
        string sDir;
        for(int l=hours-1;l>=0;l--){
            int year=curTime.GetYear();
            int month=curTime.GetMonth();
            int day=curTime.GetDay();
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDay=to_string(day);
            if((dir_states[2]==3)&&(sDay.length()==1)) sDay="0"+sDay;
            sDay=GetRealFix(dir_prefixs[2],year,month)+sDay+GetRealFix(dir_suffixs[2],year,month);
            sHour=to_string(curTime.GetHour());
            if((dir_states[3]==4)&&(sHour.length()==1)) sHour="0"+sHour;
            sHour=GetRealFix(dir_prefixs[3],year,month,day)+sHour+GetRealFix(dir_suffixs[3],year,month,day);
            sDir=smDir+sYear+sMonth+sDay+sHour;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            bool hasSet=false;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,curTime.GetYear(),curTime.GetMonth(),curTime.GetDay(),curTime.GetHour())){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                        }
                    }
                }
            }
            if(hasSet) return result;
            curTime-=TimeSpan(1,0,0);
        }
    }
    return result;
}

string DateFileFilter::GetLastFileWithRightNeighbor(string dir,DateTime fromTime,DateTime endTime){
    if(!bValid) return "";
    int dirs=dir_states.size();
    string smDir=dir;
    if(!smDir.empty()){
        if((smDir[smDir.length()-1]!='/')||(smDir[smDir.length()-1]!='\\')){
            smDir+="/";
        }
    }
    if(!dir_prefix.empty()) smDir+=dir_prefix;
    DateTime nowt=fromTime;
    string result="";
    string searchFilter="";
    if(this->states.size()>0){
        if(states[0]==0) searchFilter=exps[0];
        searchFilter+="*";
        if(states[states.size()-1]==0) searchFilter+=exps[states.size()-1];
    }
    if(dirs==0){
        FileDirectory fd(smDir);
        vector<string>files=fd.FindFiles(searchFilter);
        DateTime dt;
        for(int k=0;k<files.size();k++){
            string fileName=FilePath::GetFileName(files[k]);
            if(this->GetDateTimeFromFilter(fileName,dt)){
                if((dt>=fromTime)&&(dt<=endTime)){
                    if((dt>=nowt)||(result.empty())){
                        nowt=dt;
                        result=files[k];
                    }
                }
            }
        }
    }
    else if(dirs==1){
        int fromYear=fromTime.GetYear();
        int toYear=endTime.GetYear();
        toYear++;
        string sYear;
        string sDir;
        for(int l=toYear;l>=fromYear;l--){
            sYear=to_string(l);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sDir=smDir+dir_prefixs[0]+sYear+dir_suffixs[0];
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            bool hasSet=false;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,l)){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                            hasSet=true;
                        }
                    }
                }
            }
            if(hasSet) return result;
        }
    }
    else if(dirs==2){
        DateTime ft=fromTime;
        DateTime tt=endTime;
        if(tt.GetMonth()<=11)
            tt=DateTime(tt.GetYear(),tt.GetMonth()+1,tt.GetDay());
        else
            tt=DateTime(tt.GetYear()+1,1,tt.GetDay());
        int year=tt.GetYear();
        int month=tt.GetMonth();
        int monthes=(tt.GetYear()-ft.GetYear())*12+tt.GetMonth()-ft.GetMonth()+1;
        string sYear,sMonth;
        string sDir;
        for(int l=monthes-1;l>=0;l--){
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDir=smDir+sYear+sMonth;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            bool hasSet=false;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,year,month)){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                            hasSet=true;
                        }
                    }
                }
            }
            if(hasSet) return result;
            month--;
            if(month<0){
                month=12;
                year--;
            }
        }
    }
    else if(dirs==3){
        int days=(endTime-fromTime).TotalDays()+2;
        Date curTime(endTime.GetYear(),endTime.GetMonth(),endTime.GetDay());
        curTime=curTime+1;
        string sYear,sMonth,sDay;
        string sDir;
        for(int l=days-1;l>=0;l--){
            int year=curTime.GetYear();
            int month=curTime.GetMonth();
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDay=to_string(curTime.GetDay());
            if((dir_states[2]==3)&&(sDay.length()==1)) sDay="0"+sDay;
            sDay=GetRealFix(dir_prefixs[2],year,month)+sDay+GetRealFix(dir_suffixs[2],year,month);
            sDir=smDir+sYear+sMonth+sDay;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            bool hasSet=false;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,curTime.GetYear(),curTime.GetMonth(),curTime.GetDay())){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                            hasSet=true;
                        }
                    }
                }
            }
            if(hasSet) return result;
            curTime-=1;
        }
    }
    else if(dirs==4){
        int hours=(endTime-fromTime).TotalHours()+2;
        DateTime curTime(endTime.GetYear(),endTime.GetMonth(),endTime.GetDay(),endTime.GetHour());
        curTime+=TimeSpan(1,0,0);
        string sYear,sMonth,sDay,sHour;
        string sDir;
        for(int l=hours-1;l>=0;l--){
            int year=curTime.GetYear();
            int month=curTime.GetMonth();
            int day=curTime.GetDay();
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDay=to_string(day);
            if((dir_states[2]==3)&&(sDay.length()==1)) sDay="0"+sDay;
            sDay=GetRealFix(dir_prefixs[2],year,month)+sDay+GetRealFix(dir_suffixs[2],year,month);
            sHour=to_string(curTime.GetHour());
            if((dir_states[3]==4)&&(sHour.length()==1)) sHour="0"+sHour;
            sHour=GetRealFix(dir_prefixs[3],year,month,day)+sHour+GetRealFix(dir_suffixs[3],year,month,day);
            sDir=smDir+sYear+sMonth+sDay+sHour;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            bool hasSet=false;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,curTime.GetYear(),curTime.GetMonth(),curTime.GetDay(),curTime.GetHour())){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                        }
                    }
                }
            }
            if(hasSet) return result;
            curTime-=TimeSpan(1,0,0);
        }
    }
    return result;
}

string DateFileFilter::GetLastFileWithDoubleNeighbor(string dir,DateTime fromTime,DateTime endTime){
    if(!bValid) return "";
    int dirs=dir_states.size();
    string smDir=dir;
    if(!smDir.empty()){
        if((smDir[smDir.length()-1]!='/')||(smDir[smDir.length()-1]!='\\')){
            smDir+="/";
        }
    }
    if(!dir_prefix.empty()) smDir+=dir_prefix;
    DateTime nowt=fromTime;
    string result="";
    string searchFilter="";
    if(this->states.size()>0){
        if(states[0]==0) searchFilter=exps[0];
        searchFilter+="*";
        if(states[states.size()-1]==0) searchFilter+=exps[states.size()-1];
    }
    if(dirs==0){
        FileDirectory fd(smDir);
        vector<string>files=fd.FindFiles(searchFilter);
        DateTime dt;
        for(int k=0;k<files.size();k++){
            string fileName=FilePath::GetFileName(files[k]);
            if(this->GetDateTimeFromFilter(fileName,dt)){
                if((dt>=fromTime)&&(dt<=endTime)){
                    if((dt>=nowt)||(result.empty())){
                        nowt=dt;
                        result=files[k];
                    }
                }
            }
        }
    }
    else if(dirs==1){
        int fromYear=fromTime.GetYear();
        int toYear=endTime.GetYear();
        fromYear--;
        toYear++;
        string sYear;
        string sDir;
        for(int l=toYear;l>=fromYear;l--){
            sYear=to_string(l);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sDir=smDir+dir_prefixs[0]+sYear+dir_suffixs[0];
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            bool hasSet=false;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,l)){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                            hasSet=true;
                        }
                    }
                }
            }
            if(hasSet) return result;
        }
    }
    else if(dirs==2){
        DateTime ft=fromTime;
        if(ft.GetMonth()>=1)
            ft=DateTime(ft.GetYear(),ft.GetMonth()-1,ft.GetDay());
        else
            ft=DateTime(ft.GetYear()-1,12,ft.GetDay());
        DateTime tt=endTime;
        if(tt.GetMonth()<=11)
            tt=DateTime(tt.GetYear(),tt.GetMonth()+1,tt.GetDay());
        else
            tt=DateTime(tt.GetYear()+1,1,tt.GetDay());
        int year=tt.GetYear();
        int month=tt.GetMonth();
        int monthes=(tt.GetYear()-ft.GetYear())*12+tt.GetMonth()-ft.GetMonth()+1;
        string sYear,sMonth;
        string sDir;
        for(int l=monthes-1;l>=0;l--){
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDir=smDir+sYear+sMonth;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            bool hasSet=false;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,year,month)){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                            hasSet=true;
                        }
                    }
                }
            }
            if(hasSet) return result;
            month--;
            if(month<0){
                month=12;
                year--;
            }
        }
    }
    else if(dirs==3){
        int days=(endTime-fromTime).TotalDays()+3;
        Date curTime(endTime.GetYear(),endTime.GetMonth(),endTime.GetDay());
        curTime=curTime+1;
        string sYear,sMonth,sDay;
        string sDir;
        for(int l=days-1;l>=0;l--){
            int year=curTime.GetYear();
            int month=curTime.GetMonth();
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDay=to_string(curTime.GetDay());
            if((dir_states[2]==3)&&(sDay.length()==1)) sDay="0"+sDay;
            sDay=GetRealFix(dir_prefixs[2],year,month)+sDay+GetRealFix(dir_suffixs[2],year,month);
            sDir=smDir+sYear+sMonth+sDay;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            bool hasSet=false;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,curTime.GetYear(),curTime.GetMonth(),curTime.GetDay())){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                            hasSet=true;
                        }
                    }
                }
            }
            if(hasSet) return result;
            curTime-=1;
        }
    }
    else if(dirs==4){
        int hours=(endTime-fromTime).TotalHours()+3;
        DateTime curTime(endTime.GetYear(),endTime.GetMonth(),endTime.GetDay(),endTime.GetHour());
        curTime+=TimeSpan(1,0,0);
        string sYear,sMonth,sDay,sHour;
        string sDir;
        for(int l=hours-1;l>=0;l--){
            int year=curTime.GetYear();
            int month=curTime.GetMonth();
            int day=curTime.GetDay();
            sYear=to_string(year);
            if(dir_states[0]==-1){
                if(sYear.length()>2)
                    sYear=sYear.substr(0,2);
                else if(sYear.length()==1)
                    sYear="0"+sYear;
            }
            sYear=dir_prefixs[0]+sYear+dir_suffixs[0];
            sMonth=to_string(month);
            if((dir_states[1]==2)&&(sMonth.length()==1)) sMonth="0"+sMonth;
            sMonth=GetRealFix(dir_prefixs[1],year)+sMonth+GetRealFix(dir_suffixs[1],year);
            sDay=to_string(day);
            if((dir_states[2]==3)&&(sDay.length()==1)) sDay="0"+sDay;
            sDay=GetRealFix(dir_prefixs[2],year,month)+sDay+GetRealFix(dir_suffixs[2],year,month);
            sHour=to_string(curTime.GetHour());
            if((dir_states[3]==4)&&(sHour.length()==1)) sHour="0"+sHour;
            sHour=GetRealFix(dir_prefixs[3],year,month,day)+sHour+GetRealFix(dir_suffixs[3],year,month,day);
            sDir=smDir+sYear+sMonth+sDay+sHour;
            FileDirectory fd(sDir);
            vector<string>files=fd.FindFiles(searchFilter);
            DateTime dt;
            bool hasSet=false;
            for(int k=0;k<files.size();k++){
                string fileName=FilePath::GetFileName(files[k]);
                if(this->GetDateTimeFromFilter(fileName,dt,curTime.GetYear(),curTime.GetMonth(),curTime.GetDay(),curTime.GetHour())){
                    if((dt>=fromTime)&&(dt<=endTime)){
                        if((dt>=nowt)||(result.empty())){
                            nowt=dt;
                            result=files[k];
                        }
                    }
                }
            }
            if(hasSet) return result;
            curTime-=TimeSpan(1,0,0);
        }
    }
    return result;
}

string DateFileFilter::GetFilter(){
    return Filter;
}


}
