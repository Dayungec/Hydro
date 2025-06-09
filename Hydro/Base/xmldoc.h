#ifndef XMLDOC_H
#define XMLDOC_H
#include "base_globle.h"
#include "autoptr.h"
class TiXmlElement;
class TiXmlDocument;

namespace SGIS{

class SGIS_EXPORT XmlNode{
public:
    XmlNode(TiXmlElement*node);
    XmlNode(std::string nodeName);
    XmlNode(std::string nodeName,std::string value);
    XmlNode(std::string nodeName,std::string value,vector<std::pair<std::string,std::string>>atts);
    virtual~XmlNode();
    std::string GetName();
    std::string GetData();
    void SetData(std::string data);
    void SetAttribute(std::string attName,std::string value);
    std::string GetAttribute(std::string attName);
    TiXmlElement*DetachNode();
    AutoPtr<XmlNode>GetFirstChildNode(std::string nodeName="");
    AutoPtr<XmlNode>GetNextSiblingElement();
    vector<AutoPtr<XmlNode>>GetChildren();
    void AppendChild(XmlNode*childnode);
    AutoPtr<XmlNode>AppendChild(std::string name);
    AutoPtr<XmlNode>AppendChild(std::string name,std::string data);
protected:
    TiXmlElement*node;
    bool destroyData;
};

class SGIS_EXPORT XmlDoc
{
public:
    XmlDoc();
    virtual~XmlDoc();
    bool Load(const std::string&pathName);
    void Create();
    bool Save(const std::string&pathName);
    AutoPtr<XmlNode>GetRootNode();
    AutoPtr<XmlNode>GetFirstChildNode(std::string nodeName="");
    void AppendChild(XmlNode*node);
    AutoPtr<XmlNode>AppendChild(std::string name);
    std::string GetContent();
protected:
   TiXmlDocument* doc;
};

}

#endif // XMLDOC_H
