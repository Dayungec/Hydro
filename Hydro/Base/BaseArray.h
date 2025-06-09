#ifndef BASEARRAY_H
#define BASEARRAY_H
#include "base_globle.h"
#include "autoptr.h"
#include "jsonio.h"

namespace SGIS{

template<typename T>
class SGIS_EXPORT OrderArray
{
public:
    OrderArray(bool AllowIdentical = false,int maxSize=0) {
        this->AllowIdentical = AllowIdentical;
        this->maxSize=maxSize;
    };
    virtual ~OrderArray() {

    };
    T operator[] (int index) {
        return elements[index];
    };
    int Add(T value) {
        if(maxSize>0){
            if(elements.size()>=maxSize){
                if(value>=elements[elements.size()-1]) return -1;
            }
        }
        if (AllowIdentical) {
            long index = innerfindplaceposiOfidentical(value);
            elements.insert(std::begin(elements) + index, value);
            if(maxSize>0){
               if(elements.size()>=maxSize){
                   elements.erase(begin(elements)+elements.size()-1);
               }
            }
            return index;
        }
        else {
            long index = innerfindplacepos(value);
            if (index == -1) return index;
            elements.insert(std::begin(elements) + index, value);
            if(maxSize>0){
               if(elements.size()>=maxSize){
                   elements.erase(begin(elements)+elements.size()-1);
               }
            }
            return index;
        }
    };
    bool SetValue(int nIndex,T value){
        if(elements.size()==1){
            elements[nIndex]=value;
            return true;
        }
        else if(!AllowIdentical){
            if(nIndex==0){
                if(value>=elements[1]) return false;
            }
            else if(nIndex==elements.size()-1){
                if(value<=elements[elements.size()-2]) return false;
            }
            else{
                if(value>=elements[nIndex-1]) return false;
                if(value<=elements[nIndex+1]) return false;
            }
            elements[nIndex]=value;
            return true;
        }else{
            if(nIndex==0){
                if(value>elements[1]) return false;
            }
            else if(nIndex==elements.size()-1){
                if(value<elements[elements.size()-2]) return false;
            }
            else{
                if(value>elements[nIndex-1]) return false;
                if(value<elements[nIndex+1]) return false;
            }
            elements[nIndex]=value;
            return true;
        }
        return false;
    };
    bool GetAllowIdentical(){
        return AllowIdentical;
    };
    long GetSize() {
        return elements.size();
    };
    void Trim(int count){
        if(maxSize>0){
            if(count>maxSize) return;
        }
        if(elements.size()>count) elements.resize(count);
    };
    bool TrimToOther(int count,OrderArray<T>&other){
        if(maxSize>0){
            if(count>maxSize) return false;
        }
        if(elements.size()<count) count=elements.size();
        other.elements.resize(count);
        memcpy(other.elements.data(),elements.data(),sizeof(T)*count);
        return true;
    };
    void Clear() {
        elements.clear();
    };
    void Remove(int index) {
        elements.erase(std::begin(elements) + index);
    };
    int Find(T value) {
        return (AllowIdentical ? innerfindOfidentical(value) : innerfind(value));
    };
    int FindPlacePos(T value) {
        return (AllowIdentical ? innerfindplaceposiOfidentical(value) : innerfindplacepos(value));
    };
    int FindOrder(T value) {
        return innerfindplaceposiOfidentical(value);
    };
    void CopyFromOther(OrderArray<T>*other){
        this->AllowIdentical=other->AllowIdentical;
        this->maxSize=other->maxSize;
        elements.resize(other->elements.size());
        memcpy(elements.data(),other->elements.data(),sizeof(T)*other->elements.size());
    };
    void CopyTo(int count,vector<T>&other){
        if(elements.size()<count) count=elements.size();
        other.resize(count);
        if(count==0) return;
        other.resize(count);
        memcpy(other.data(),elements.data(),sizeof(T)*count);
    };
protected:
    int innerfind(T Value) {
        long num = elements.size();
        if (num <= 15)
        {
            for (int k = 0; k < num; k++)
            {
                if (Value == elements[k])
                    return k;
                else if (Value < elements[k])
                    return -1;
            }
            return -1;
        }
        long last = num - 1;
        T lastvalue = elements[last];
        if (Value > lastvalue) return -1;
        long first = 0;
        T firstvalue = elements[first];
        if (Value < firstvalue) return -1;
        if (Value == firstvalue)
            return 0;
        else if (Value == lastvalue)
            return num - 1;
        long temp;
        T tempvalue;
        while (true)
        {
            temp = (first + last) / 2;
            if (temp == first) return -1;
            tempvalue = elements[temp];
            if (Value < tempvalue)
            {
                last = temp;
                lastvalue = tempvalue;
            }
            else if (Value > tempvalue)
            {
                first = temp;
                firstvalue = tempvalue;
            }
            else
            {
                return temp;
            }
        }
    };
    int innerfindOfidentical(T Value) {
        long num = elements.size();
        if (num <= 15)
        {
            for (int k = 0; k < num; k++)
            {
                if (Value == elements[k])
                    return k;
                else if (Value < elements[k])
                    return -1;
            }
            return -1;
        }
        long last = num - 1;
        T lastvalue = elements[last];
        if (Value > lastvalue) return -1;
        long first = 0;
        T firstvalue = elements[first];
        if (Value < firstvalue) return -1;
        if (Value == firstvalue) return 0;
        long temp;
        T tempvalue;
        while (true)
        {
            temp = (first + last) / 2;
            tempvalue = elements[temp];
            if (temp == first)
            {
                if (Value == tempvalue) return temp;
                if(temp<last){
                    if (Value == elements[temp+1]) return temp;
                }
                return -1;
            }
            if (Value <= tempvalue)
            {
                last = temp;
                lastvalue = tempvalue;
            }
            else if (Value > tempvalue)
            {
                first = temp;
                firstvalue = tempvalue;
            }
        }
        return -1;
    };
    long innerfindplacepos(T value) {
        long num = elements.size();
        if (num <= 15)
        {
            for (int k = 0; k < num; k++)
            {
                if (value == elements[k])
                    return -1;
                else if (value < elements[k])
                    return k;
            }
            return num;
        }
        long last = num - 1;
        T lastvalue = elements[last];
        if (value > lastvalue) return num;
        long first = 0;
        T firstvalue = elements[first];
        if (value < firstvalue) return 0;
        if ((value == firstvalue) || (value == lastvalue))
            return -1;
        long temp;
        T tempvalue;
        while (true)
        {
            temp = (first + last) / 2;
            if (temp == first) return first + 1;
            tempvalue = elements[temp];
            if (value < tempvalue)
            {
                last = temp;
                lastvalue = tempvalue;
            }
            else if (value > tempvalue)
            {
                first = temp;
                firstvalue = tempvalue;
            }
            else
            {
                return -1;
            }
        }
    };
    long innerfindplaceposiOfidentical(T value) {
        long num = elements.size();
        if (num <= 15)
        {
            for (int k = 0; k < num; k++)
            {
                if (value <= elements[k])
                    return k;
            }
            return num;
        }
        long last = num - 1;
        T lastvalue = elements[last];
        if (value > lastvalue) return num;
        long first = 0;
        T firstvalue = elements[first];
        if (value <= firstvalue) return 0;
        long temp;
        T tempvalue;
        while (true)
        {
            temp = (first + last) / 2;
            if (temp == first){
                if(first==last) return first;
                if(value>elements[first])
                    return first+1;
                return first;
            }
            tempvalue = elements[temp];
            if (value <= tempvalue)
            {
                last = temp;
                lastvalue = tempvalue;
            }
            else if (value > tempvalue)
            {
                first = temp;
                firstvalue = tempvalue;
            }
        }
        return -1;
    };
public:
    vector<T>elements;
    bool AllowIdentical;
    int maxSize;
};

template<typename T>
class SGIS_EXPORT IndexedArray{
public:
    IndexedArray(bool AllowIdentical = false)
        :elements(AllowIdentical){
        this->AllowIdentical = AllowIdentical;
    };
    virtual~IndexedArray(){

    };
    int FindValueIndex(T value){
        int nIndex=elements.Find(value);
        if(nIndex==-1) return -1;
        return Indexes[nIndex];
    };
    bool Add(T value){
        long nIndex=elements.FindPlacePos(value);
        if(nIndex==-1) return false;
        for(int k=orders.size()-1;k>=0;k--){
            if(orders[k]>=nIndex) orders[k]=orders[k]+1;
        }
        elements.Add(value);
        Indexes.insert(std::begin(Indexes)+nIndex,elements.GetSize()-1);
        orders.push_back(nIndex);
        return true;
    };
    bool Insert(int nIndex,T value){
        long iItem=elements.FindPlacePos(value);
        if(iItem==-1) return false;
        elements.Add(value);
        for(int k=Indexes.size()-1;k>=0;k--)
        {
            if(Indexes[k]>=iItem) Indexes[k]=Indexes[k]+1;
        }
        for(int k=orders.size()-1;k>=0;k--)
        {
            if(orders[k]>=nIndex) orders[k]=orders[k]+1;
        }
        Indexes.insert(std::begin(Indexes)+nIndex,iItem);
        orders.insert(std::begin(orders)+iItem,nIndex);
    };
    int GetSize(){
        return Indexes.size();
    };
    T operator[] (int nIndex) {
        return elements[orders[nIndex]];
    };
    void RemoveAt(int nIndex){
        int pIndex=orders[nIndex];
        elements.Remove(pIndex);
        Indexes.erase(std::begin(Indexes)+pIndex);
        orders.erase(std::begin(orders)+nIndex);
        for(int k=Indexes.size()-1;k>=0;k--)
        {
            int pos=Indexes[k];
            if(pos>nIndex) Indexes[k]=pos-1;
        }
        for(int k=orders.size()-1;k>=0;k--)
        {
            int pos=orders[k];
            if(pos>pIndex) orders[k]=pos-1;
        }
    };
    void Clear(){
        elements.Clear();
        Indexes.clear();
        orders.clear();
    };
    void CopyFromOther(IndexedArray<T>*other){
        elements.CopyFromOther(&other->elements);
        Indexes.resize(other->Indexes.size());
        orders.resize(other->orders.size());
        int size=other->Indexes.size();
        for(int k=0;k<size;k++){
            Indexes[k]=other->Indexes[k];
            orders[k]=other->orders[k];
        }
        AllowIdentical=other->AllowIdentical;
    };
protected:
    OrderArray<T>elements;
    vector<int>Indexes;
    vector<int>orders;
    bool AllowIdentical;
};

class SGIS_EXPORT TwoValueArray{
public:
    TwoValueArray(int size=0);
    virtual~TwoValueArray();
    int GetSize();
    bool SetSize(int size,bool defaultValue=false);
    void CopyFrom(unsigned char*buffer,int bytelen);
    void Attach(unsigned char*buffer,int bytelen);
    void SetDefaultValue(bool defaultValue);
    int GetCombineValue(int nPos,int len);
    void SetCombineValue(int nPos,int len,int value);
    bool operator[] (int nIndex);
    void SetValue(int nIndex,bool value);
    unsigned char*GetBits();
protected:
    int size;
    unsigned char*bits;
    bool attached;
};

class SGIS_EXPORT FourValueArray{
public:
    FourValueArray(int size);
    virtual~FourValueArray();
    int GetSize();
    bool SetSize(int size,unsigned char defaultValue=0);
    void SetDefaultValue(unsigned char defaultValue);
    unsigned char operator[] (int nIndex);
    void SetValue(int nIndex,unsigned char value);
protected:
    int size;
    unsigned char*bits;
};

class SGIS_EXPORT AutoBuffer{
public:
    AutoBuffer(int initialBlocks=1,int sizeofBlock=1024);
    virtual~AutoBuffer();
    char*GetBuffer(int len);
protected:
    vector<char>buffer;
    int sizeofBlock;
    int blocks;
};

class SGIS_EXPORT FixBuffer{
public:
   FixBuffer(ULONGLONG size=0);
   FixBuffer(char*buffer,ULONGLONG size);
   virtual~FixBuffer();
   void Attach(FixBuffer*buffer);
   void Attach(char*buffer,int len);
   AutoPtr<FixBuffer>Clone();
   ULONGLONG GetSize();
   char*GetBuffer();
   string ConvertString();
   AutoPtr<JsonNode>ConvertJson();
protected:
   char*buffer;
   ULONGLONG size;
   bool needDestroy;
};

template<typename T>
class SGIS_EXPORT AutoArray{
public:
    AutoArray(){

    };
    AutoArray(int size){
        elements.resize(size);
    };
    virtual~AutoArray(){

    };
    void Add(T el)
    {
        elements.push_back(el);
    };
    T&operator[](int nIndex){
        return elements[nIndex];
    };
    T GetItem(int nIndex)
    {
        return elements[nIndex];
    };
    int GetSize()
    {
        return elements.size();
    };
    T*GetData(){
        return elements.data();
    };
    vector<T>*GetArray(){
        return &elements;
    };
    bool Find(T el)
    {
        for(int k=elements.size()-1;k>=0;k--)
        {
            if(elements[k]==el) return true;
        }
        return false;
    };
    void Clear()
    {
        elements.clear();
    };
    void InitialFromArray(vector<T>&other,bool Reverse)
    {
        int Size=other.size();
        if(Size==0)
        {
            Clear();
            return;
        }
        elements.resize(other.size());
        if(Reverse)
        {
            for(int k=0;k<Size;k++) elements[k]=other[Size-k-1];
        }
        else
            memcpy(elements.data(),other.data(),sizeof(T)*Size);
    };
protected:
    vector<T>elements;
};

template<typename T>
struct NamedItem{
    string name;
    T value;
    NamedItem(){
        name="";
    };
    NamedItem(string name,T value){
        this->name=name;
        this->value=value;
    };
    NamedItem(const NamedItem&other){
        this->name=other.name;
        this->value=other.value;
    };
    NamedItem&operator=(const NamedItem&other){
        this->name=other.name;
        this->value=other.value;
        return *this;
    };
};

template<typename T>
class SGIS_EXPORT NamedMap{
public:
    NamedMap(){

    };
    NamedMap(vector<NamedItem<T>>&items){
        this->items.resize(items.size());
        for(int k=0;k<this->items.size();k++){
            this->items[k]=items[k];
        }
    };
    virtual~NamedMap(){

    };
    int GetSize(){
       return items.size();
    };
    NamedItem<T>&operator[](int nIndex){
        return items[nIndex];
    };
    NamedItem<T>operator[](string name){
        int size=items.size();
        for(int k=0;k<size;k++){
            if(name==items[k].name) return items[k];
        }
        return NamedItem<T>();
    };
    void Add(NamedItem<T>item){
        items.push_back(item);
    };
    void Add(string name,T Value){
        items.push_back({name,Value});
    };
    void Clear(){
        items.clear();
    };
    void Remove(int nIndex){
        items.erase(begin(items)+nIndex);
    };
    bool Remove(string name){
        int size=items.size();
        bool hasRemoved=false;
        for(int k=size-1;k>=0;k--){
            if(name==items[k].name){
                items.erase(begin(items)+k);
                hasRemoved=true;
            }
        }
        return hasRemoved;
    };
protected:
    vector<NamedItem<T>>items;
};

}

#endif // BASEARRAY_H
