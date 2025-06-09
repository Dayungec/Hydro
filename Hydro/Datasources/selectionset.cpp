#include "selectionset.h"

namespace SGIS{


SelectionSet::SelectionSet()
{

}
SelectionSet::~SelectionSet(){

}
int SelectionSet::GetSize(){
   return pArray.GetSize();
}
int SelectionSet::GetFeatureID(int nIndex){
   return pArray[nIndex];
}
bool SelectionSet::AddFeatureID(int fid){
   int pos=pArray.Add(fid);
   return (pos>=0);
}
bool SelectionSet::RemoveFeatureID(int fid){
   int pos=pArray.Find(fid);
   if(pos<0) return false;
   pArray.Remove(pos);
   return true;
}
void SelectionSet::Remove(int nIndex){
    pArray.Remove(nIndex);
}
void SelectionSet::Clear(){
   pArray.Clear();
}
bool SelectionSet::FindFeatureID(int fid){
   int pos=pArray.Find(fid);
   return (pos>=0);
}

int SelectionSet::FindFeatureIndex(int fid){
    return pArray.Find(fid);
}

AutoPtr<SelectionSet>SelectionSet::Clone(){
   SelectionSet*other=new SelectionSet();
   other->pArray.CopyFromOther(&this->pArray);
   return other;
}
void SelectionSet::CopyFromOther(SelectionSet*other){
   pArray.CopyFromOther(&other->pArray);
}

void SelectionSet::Intersect(SelectionSet*other){
    int size=pArray.GetSize();
    for(int k=size-1;k>=0;k--){
        int fid=pArray[k];
        if(!other->FindFeatureID(fid)){
            pArray.Remove(k);
        }
    }
}

}
