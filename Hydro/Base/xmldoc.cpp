#include "xmldoc.h"

#include "xml/tinyxml.h"
#include "StringHelper.h"
#include <fstream>

namespace SGIS{

XmlNode::XmlNode(TiXmlElement*node){
    this->node=node;
    destroyData=false;
}

XmlNode::XmlNode(string nodeName){
    node=new TiXmlElement(nodeName.c_str());
    destroyData=true;
}

XmlNode::XmlNode(string nodeName,string value){
    node=new TiXmlElement(nodeName.c_str());
    if(value!="") SetData(value.c_str());
    destroyData=true;
}

XmlNode::XmlNode(string nodeName,string value,vector<std::pair<string,string>>atts){
    node=new TiXmlElement(nodeName.c_str());
    if(value!="") SetData(value.c_str());
    for(int k=0;k<atts.size();k++){
        string attName=std::get<0>(atts[k]);
        string attValue=std::get<1>(atts[k]);
        node->SetAttribute(attName.c_str(),attValue.c_str());
    }
    destroyData=true;
}

XmlNode::~XmlNode(){
    if(destroyData) delete node;
}

TiXmlElement*XmlNode::DetachNode(){
    if(destroyData) destroyData=false;
    return node;
}

string XmlNode::GetName(){
    return node->Value();
}

string XmlNode::GetData(){
    TiXmlNode*tnode=node->FirstChild();
    if(tnode==nullptr) return "";
    if(tnode->Type()!=TiXmlNode::TINYXML_TEXT) return "";
    return tnode->Value();
}
void XmlNode::SetData(string data){
    TiXmlNode*tNode=node->FirstChild();
    if(tNode==nullptr){
        TiXmlText *text= new TiXmlText(data.c_str());
        node->LinkEndChild(text);
    }
    else if(tNode->Type()!=TiXmlNode::TINYXML_TEXT){
        TiXmlText *text= new TiXmlText(data.c_str());
        node->LinkEndChild(text);
    }
    else{
        TiXmlText *text= (TiXmlText*)tNode;
        text->SetValue(data.c_str());
    }
}
void XmlNode::SetAttribute(string attName,string value){
    node->SetAttribute(attName.c_str(),value.c_str());
}

string XmlNode::GetAttribute(string attName){
    const char*value=node->Attribute(attName.c_str());
    if(value==nullptr) return "";
    string sValue=value;
    return sValue;
}

AutoPtr<XmlNode>XmlNode::GetFirstChildNode(string nodeName){
    if(nodeName==""){
        TiXmlElement*el=node->FirstChildElement();
        if(el==nullptr) return nullptr;
        return AutoPtr<XmlNode>(new XmlNode(el));
    }
    else{
       TiXmlElement*el=node->FirstChildElement(nodeName.c_str());
       if(el==nullptr) return nullptr;
       return AutoPtr<XmlNode>(new XmlNode(el));
    }
}

AutoPtr<XmlNode>XmlNode::GetNextSiblingElement(){
    TiXmlElement*el=node->NextSiblingElement();
    if(el==nullptr) return nullptr;
    return AutoPtr<XmlNode>(new XmlNode(el));
}

vector<AutoPtr<XmlNode>>XmlNode::GetChildren(){
    vector<AutoPtr<XmlNode>>nodes;
    AutoPtr<XmlNode>child=this->GetFirstChildNode();
    while(child!=nullptr){
        nodes.push_back(child);
        child=child->GetNextSiblingElement();
    }
    return nodes;
}

void XmlNode::AppendChild(XmlNode*childnode){
     node->LinkEndChild(childnode->DetachNode());
}

AutoPtr<XmlNode>XmlNode::AppendChild(string name){
    AutoPtr<XmlNode>node(new XmlNode(name));
    AppendChild(node.get());
    return node;
}

AutoPtr<XmlNode>XmlNode::AppendChild(string name,string data){
    AutoPtr<XmlNode>node(new XmlNode(name,data));
    AppendChild(node.get());
    return node;
}

XmlDoc::XmlDoc()
{
    doc=nullptr;
}

XmlDoc::~XmlDoc()
{
    if(doc!=nullptr) delete doc;
}

bool XmlDoc::Load(const string&pathName){
    if(doc!=nullptr) delete doc;
    doc=new TiXmlDocument();
    if (!doc->LoadFile(StringHelper::ToLocalString(pathName).c_str(),TIXML_ENCODING_UNKNOWN)){
        delete doc;
        doc=nullptr;
        return false;
    }
    /*
    if(doc!=nullptr) delete doc;
    doc=new TiXmlDocument();
    if (!doc->LoadFile(StringHelper::ToLocalString(pathName).c_str(),TIXML_ENCODING_UNKNOWN)){
        delete doc;
        doc=nullptr;
        return false;
    }
    */
    return true;
}

void XmlDoc::Create(){
    if(doc!=nullptr) delete doc;
    doc=new TiXmlDocument();
}
bool XmlDoc::Save(const string&pathName){
    if(doc==nullptr) return false;
    TiXmlPrinter printer;
    doc->Accept(&printer);
    string content=printer.CStr();
    //content=StringHelper::content);
    ofstream destFile(StringHelper::ToLocalString(pathName).c_str(), std::ios::binary |ios::out);
    if(!destFile.is_open()) return false;
    destFile.write(content.c_str(),content.size());
    destFile.close();
    return true;
}

string XmlDoc::GetContent(){
    TiXmlPrinter printer;
    doc->Accept(&printer);
    return printer.CStr();
}

AutoPtr<XmlNode>XmlDoc::GetRootNode(){
    TiXmlElement*el=doc->RootElement();
    if(el==nullptr) return nullptr;
    return AutoPtr<XmlNode>(new XmlNode(el));
}

AutoPtr<XmlNode>XmlDoc::GetFirstChildNode(string nodeName){
    if(nodeName==""){
        TiXmlElement*el=doc->FirstChildElement();
        if(el==nullptr) return nullptr;
        return AutoPtr<XmlNode>(new XmlNode(el));
    }
    else{
       TiXmlElement*el=doc->FirstChildElement(nodeName.c_str());
       if(el==nullptr) return nullptr;
       return AutoPtr<XmlNode>(new XmlNode(el));
    }
}

void XmlDoc::AppendChild(XmlNode*node){
    doc->LinkEndChild(node->DetachNode());
}

AutoPtr<XmlNode>XmlDoc::AppendChild(string name){
    AutoPtr<XmlNode>node(new XmlNode(name));
    AppendChild(node.get());
    return node;
}

}
