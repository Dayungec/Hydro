#include "variant.h"
#include "datetime.h"
namespace SGIS{

Variant::Variant(){
    vt=VRT_EMPTY;
    bstrVal=nullptr;
}
Variant::Variant(const ULONGLONG&v){
    vt=VRT_UI8;
    ullVal=v;
}
Variant::Variant(const LONGLONG&v){
    vt=VRT_I8;
    llVal=v;
}
Variant::Variant(const ULONG&v){
    vt=VRT_UI4;
    ulVal=v;
}
Variant::Variant(const LONG&v){
    vt=VRT_I4;
    lVal=v;
}
Variant::Variant(const int&v){
    vt=VRT_I4;
    lVal=v;
}
Variant::Variant(const USHORT&v){
    vt=VRT_UI2;
    uiVal=v;
}
Variant::Variant(const SHORT&v){
    vt=VRT_I2;
    iVal=v;
}
Variant::Variant(const BYTE&v){
    vt=VRT_UI1;
    bVal=v;
}

Variant::Variant(const CHAR&v){
    vt=VRT_I1;
    cVal=v;
}

Variant::Variant(const bool&v){
    vt=VRT_BOOL;
    boolVal=v;
}
Variant::Variant(const FLOAT&v){
    vt=VRT_R4;
    fltVal=v;
}
Variant::Variant(const DOUBLE&v){
    vt=VRT_R8;
    dblVal=v;
}
Variant::Variant(const string&v){
    vt=VRT_BSTR;
    bstrVal=new string(v);
}

Variant::Variant(Date dt){
    vt=VRT_DATE;
    date=dt.ToDouble();
}

Variant::Variant(DateTime dt){
    vt=VRT_DATETIME;
    date=dt.ToDouble();
}

Variant::Variant(void*other){
    vt=VRT_PIN;
    pData=other;
}

Variant::Variant(const Variant&other){
    //Clear();
    this->vt=other.vt;
    switch(other.vt){
    case VRT_BOOL:
        this->boolVal=other.boolVal;
        break;
    case VRT_I1:
        this->cVal=other.cVal;
        break;
    case VRT_UI1:
        this->bVal=other.bVal;
        break;
    case VRT_UI2:
        this->uiVal=other.uiVal;
        break;
    case VRT_I2:
        this->iVal=other.iVal;
        break;
    case VRT_UI4:
        this->ulVal=other.ulVal;
        break;
    case VRT_I4:
        this->lVal=other.lVal;
        break;
    case VRT_UI8:
        this->ullVal=other.ullVal;
        break;
    case VRT_I8:
        this->llVal=other.llVal;
        break;
    case VRT_R4:
        this->fltVal=other.fltVal;
        break;
    case VRT_R8:
        this->dblVal=other.dblVal;
        break;
    case VRT_DATETIME:
    case VRT_DATE:
        this->date=other.date;
        break;
    case VRT_BSTR:
        this->bstrVal=new string(*other.bstrVal);
        break;
    case VRT_PIN:
        this->pData=other.pData;
        break;
    }
}

Variant::~Variant(){
   Clear();
}

void Variant::Clear(){
    if(vt==VRT_BSTR){
        if(bstrVal!=nullptr)
            delete bstrVal;
    }
    vt=VRT_EMPTY;
    bstrVal=NULL;
}

Variant& Variant::operator =(const ULONGLONG& other){
    if(vt==VRT_PIN){
        ULONGLONG*value=(ULONGLONG*)pData;
        *value=other;
        return *this;
    }
    Clear();
    vt=VRT_UI8;
    ullVal=other;
    return *this;
}
Variant& Variant::operator =(const LONGLONG& other){
    if(vt==VRT_PIN){
        LONGLONG*value=(LONGLONG*)pData;
        *value=other;
        return *this;
    }
    Clear();
    vt=VRT_I8;
    llVal=other;
    return *this;
}
Variant& Variant::operator =(const ULONG& other){
    if(vt==VRT_PIN){
        ULONG*value=(ULONG*)pData;
        *value=other;
        return *this;
    }
    Clear();
    vt=VRT_UI4;
    ulVal=other;
    return *this;
}
Variant& Variant::operator =(const LONG& other){
    if(vt==VRT_PIN){
        LONG*value=(LONG*)pData;
        *value=other;
        return *this;
    }
    Clear();
    vt=VRT_I4;
    lVal=other;
    return *this;
}
Variant& Variant::operator =(const int& other){
    if(vt==VRT_PIN){
        int*value=(int*)pData;
        *value=other;
        return *this;
    }
    Clear();
    vt=VRT_I4;
    lVal=other;
    return *this;
}
Variant& Variant::operator =(const USHORT& other){
    if(vt==VRT_PIN){
        USHORT*value=(USHORT*)pData;
        *value=other;
        return *this;
    }
    Clear();
    vt=VRT_UI2;
    uiVal=other;
    return *this;
}
Variant& Variant::operator =(const SHORT& other){
    if(vt==VRT_PIN){
        SHORT*value=(SHORT*)pData;
        *value=other;
        return *this;
    }
    Clear();
    vt=VRT_I2;
    iVal=other;
    return *this;
}
Variant& Variant::operator =(const BYTE& other){
    if(vt==VRT_PIN){
        BYTE*value=(BYTE*)pData;
        *value=other;
        return *this;
    }
    Clear();
    vt=VRT_UI1;
    bVal=other;
    return *this;
}
Variant& Variant::operator =(const CHAR& other){
    if(vt==VRT_PIN){
        CHAR*value=(CHAR*)pData;
        *value=other;
        return *this;
    }
    Clear();
    vt=VRT_I1;
    cVal=other;
    return *this;
}
Variant& Variant::operator =(const bool& other){
    if(vt==VRT_PIN){
        bool*value=(bool*)pData;
        *value=other;
        return *this;
    }
    Clear();
    vt=VRT_BOOL;
    boolVal=other;
    return *this;
}
Variant& Variant::operator =(const FLOAT& other){
    if(vt==VRT_PIN){
        FLOAT*value=(FLOAT*)pData;
        *value=other;
        return *this;
    }
    Clear();
    vt=VRT_R4;
    fltVal=other;
    return *this;
}
Variant& Variant::operator =(const DOUBLE& other){
    if(vt==VRT_PIN){
        DOUBLE*value=(DOUBLE*)pData;
        *value=other;
        return *this;
    }
    Clear();
    vt=VRT_R8;
    dblVal=other;
    return *this;
}
Variant& Variant::operator =(const string& other){
    if(vt==VRT_PIN){
        string*value=(string*)pData;
        *value=other;
        return *this;
    }
    Clear();
    vt=VRT_BSTR;
    bstrVal=new string(other);
    return *this;
}
Variant& Variant::operator =(DateTime other){
    if(vt==VRT_PIN){
        DATE*value=(DATE*)pData;
        *value=other.ToDouble();
        return *this;
    }
    Clear();
    vt=VRT_DATETIME;
    date=other.ToDouble();
    return *this;
}
Variant& Variant::operator =(Date other){
    if(vt==VRT_PIN){
        DATE*value=(DATE*)pData;
        *value=other.ToDouble();
        return *this;
    }
    Clear();
    vt=VRT_DATE;
    date=other.ToDouble();
    return *this;
}
Variant& Variant::operator =(void*other){
    if(vt==VRT_PIN){
        pData=other;
        return *this;
    }
    Clear();
    vt=VRT_PIN;
    pData=other;
    return *this;
}

Variant& Variant::operator =(const Variant& other){
    Clear();
    switch(other.vt){
    case VRT_BOOL:
        this->boolVal=other.boolVal;
        break;
    case VRT_I1:
        this->cVal=other.cVal;
        break;
    case VRT_UI1:
        this->bVal=other.bVal;
        break;
    case VRT_UI2:
        this->uiVal=other.uiVal;
        break;
    case VRT_I2:
        this->iVal=other.iVal;
        break;
    case VRT_UI4:
        this->ulVal=other.ulVal;
        break;
    case VRT_I4:
        this->lVal=other.lVal;
        break;
    case VRT_UI8:
        this->ullVal=other.ullVal;
        break;
    case VRT_I8:
        this->llVal=other.llVal;
        break;
    case VRT_R4:
        this->fltVal=other.fltVal;
        break;
    case VRT_R8:
        this->dblVal=other.dblVal;
        break;
    case VRT_DATETIME:
    case VRT_DATE:
        this->date=other.date;
        break;
    case VRT_BSTR:{
        this->bstrVal=new string(*other.bstrVal);
        break;
    }
    case VRT_PIN:{
        this->pData=other.pData;
        break;
    }
    }
    this->vt=other.vt;
    return *this;
}

bool Variant::ChangeType(VarType vt){
    if(this->vt==vt) return true;
    if(vt==VRT_EMPTY) return false;
    if(this->vt==VRT_EMPTY){
        switch(vt){
        case VRT_BOOL:{
            this->boolVal=false;
            break;
        }
        case VRT_I1:{
          this->cVal=0;
          break;
        }
        case VRT_UI1:{
          this->bVal=0;
          break;
        }
        case VRT_UI2:{
          this->uiVal=0;
          break;
        }
        case VRT_I2:{
          this->iVal=0;
          break;
        }
        case VRT_UI4:{
          this->ulVal=0;
          break;
        }
        case VRT_I4:{
          this->lVal=0;
          break;
        }
        case VRT_UI8:{
          this->ullVal=0;
          break;
        }
        case VRT_I8:{
          this->llVal=0;
          break;
        }
        case VRT_R4:{
          this->fltVal=0;
          break;
        }
        case VRT_R8:{
          this->dblVal=0;
          break;
        }
        case VRT_DATE:
        case VRT_DATETIME:
        {
          this->date=0;
          break;
        }
        case VRT_BSTR:{
          this->bstrVal=new string("");
          break;
        }
        case VRT_PIN:{
          return false;
        }
        }
    }
    else if(this->vt==VRT_BOOL){
        switch(vt){
        case VRT_I1:{
          this->cVal=this->boolVal;
          break;
        }
        case VRT_UI1:{
          this->bVal=this->boolVal;
          break;
        }
        case VRT_UI2:{
          this->uiVal=this->boolVal;
          break;
        }
        case VRT_I2:{
          this->iVal=this->boolVal;
          break;
        }
        case VRT_UI4:{
          this->ulVal=this->boolVal;
          break;
        }
        case VRT_I4:{
          this->lVal=this->boolVal;
          break;
        }
        case VRT_UI8:{
          this->ullVal=this->boolVal;
          break;
        }
        case VRT_I8:{
          this->llVal=this->boolVal;
          break;
        }
        case VRT_R4:{
          this->fltVal=this->boolVal;
          break;
        }
        case VRT_R8:{
          this->dblVal=this->boolVal;
          break;
        }
        case VRT_DATETIME:
        case VRT_DATE:{
          this->date=this->boolVal;
          break;
        }
        case VRT_BSTR:{
          this->bstrVal=new string(to_string((int)this->boolVal));
          break;
        }
        case VRT_PIN:{
          return false;
        }
        }
    }
    else if(this->vt==VRT_I1){
        switch(vt){
        case VRT_BOOL:{
          this->boolVal=(this->cVal!=0);
            break;
        }
        case VRT_UI1:{
          this->bVal=this->cVal;
          break;
        }
        case VRT_UI2:{
          this->uiVal=this->cVal;
          break;
        }
        case VRT_I2:{
          this->iVal=this->cVal;
          break;
        }
        case VRT_UI4:{
          this->ulVal=this->cVal;
          break;
        }
        case VRT_I4:{
          this->lVal=this->cVal;
          break;
        }
        case VRT_UI8:{
          this->ullVal=this->cVal;
          break;
        }
        case VRT_I8:{
          this->llVal=this->cVal;
          break;
        }
        case VRT_R4:{
          this->fltVal=this->cVal;
          break;
        }
        case VRT_R8:{
          this->dblVal=this->cVal;
          break;
        }
        case VRT_DATE:
        case VRT_DATETIME:{
          this->date=this->cVal;
          break;
        }
        case VRT_BSTR:{
          this->bstrVal=new string(to_string(this->cVal));
          break;
        }
        case VRT_PIN:{
          return false;
        }
        }
    }
    else if(this->vt==VRT_UI1){
        switch(vt){
        case VRT_BOOL:{
          this->boolVal=(this->bVal!=0);
            break;
        }
        case VRT_I1:{
          this->cVal=this->bVal;
          break;
        }
        case VRT_UI2:{
          this->uiVal=this->bVal;
          break;
        }
        case VRT_I2:{
          this->iVal=this->bVal;
          break;
        }
        case VRT_UI4:{
          this->ulVal=this->bVal;
          break;
        }
        case VRT_I4:{
          this->lVal=this->bVal;
          break;
        }
        case VRT_UI8:{
          this->ullVal=this->bVal;
          break;
        }
        case VRT_I8:{
          this->llVal=this->bVal;
          break;
        }
        case VRT_R4:{
          this->fltVal=this->bVal;
          break;
        }
        case VRT_R8:{
          this->dblVal=this->bVal;
          break;
        }
        case VRT_DATE:
        case VRT_DATETIME:{
          this->date=this->bVal;
          break;
        }
        case VRT_BSTR:{
          this->bstrVal=new string(to_string(this->bVal));
          break;
        }
        case VRT_PIN:{
          return false;
        }
        }
    }
    else if(this->vt==VRT_I2){
        switch(vt){
        case VRT_BOOL:{
          this->boolVal=(this->iVal!=0);
            break;
        }
        case VRT_I1:{
          this->cVal=this->iVal;
          break;
        }
        case VRT_UI1:{
          this->bVal=this->iVal;
          break;
        }
        case VRT_UI2:{
          this->uiVal=this->iVal;
          break;
        }
        case VRT_UI4:{
          this->ulVal=this->iVal;
          break;
        }
        case VRT_I4:{
          this->lVal=this->iVal;
          break;
        }
        case VRT_UI8:{
          this->ullVal=this->iVal;
          break;
        }
        case VRT_I8:{
          this->llVal=this->iVal;
          break;
        }
        case VRT_R4:{
          this->fltVal=this->iVal;
          break;
        }
        case VRT_R8:{
          this->dblVal=this->iVal;
          break;
        }
        case VRT_DATE:
        case VRT_DATETIME:{
          this->date=this->iVal;
          break;
        }
        case VRT_BSTR:{
          this->bstrVal=new string(to_string(this->iVal));
          break;
        }
        case VRT_PIN:{
          return false;
        }
        }
    }
    else if(this->vt==VRT_UI2){
        switch(vt){
        case VRT_BOOL:{
          this->boolVal=(this->uiVal!=0);
            break;
        }
        case VRT_UI1:{
          this->bVal=this->uiVal;
          break;
        }
        case VRT_I1:{
          this->cVal=this->uiVal;
          break;
        }
        case VRT_I2:{
          this->iVal=this->uiVal;
          break;
        }
        case VRT_UI4:{
          this->ulVal=this->uiVal;
          break;
        }
        case VRT_I4:{
          this->lVal=this->uiVal;
          break;
        }
        case VRT_UI8:{
          this->ullVal=this->uiVal;
          break;
        }
        case VRT_I8:{
          this->llVal=this->uiVal;
          break;
        }
        case VRT_R4:{
          this->fltVal=this->uiVal;
          break;
        }
        case VRT_R8:{
          this->dblVal=this->uiVal;
          break;
        }
        case VRT_DATE:
        case VRT_DATETIME:{
          this->date=this->uiVal;
          break;
        }
        case VRT_BSTR:{
          this->bstrVal=new string(to_string(this->uiVal));
          break;
        }
        case VRT_PIN:{
          return false;
        }
        }
    }
    else if(this->vt==VRT_I4){
        switch(vt){
        case VRT_BOOL:{
          this->boolVal=(this->lVal!=0);
            break;
        }
        case VRT_UI1:{
          this->bVal=this->lVal;
          break;
        }
        case VRT_I1:{
          this->cVal=this->lVal;
          break;
        }
        case VRT_UI2:{
          this->uiVal=this->lVal;
          break;
        }
        case VRT_I2:{
          this->iVal=this->lVal;
          break;
        }
        case VRT_UI4:{
          this->ulVal=this->lVal;
          break;
        }
        case VRT_UI8:{
          this->ullVal=this->lVal;
          break;
        }
        case VRT_I8:{
          this->llVal=this->lVal;
          break;
        }
        case VRT_R4:{
          this->fltVal=this->lVal;
          break;
        }
        case VRT_R8:{
          this->dblVal=this->lVal;
          break;
        }
        case VRT_DATE:
        case VRT_DATETIME:{
          this->date=this->lVal;
          break;
        }
        case VRT_BSTR:{
          this->bstrVal=new string(to_string(this->lVal));
          break;
        }
        case VRT_PIN:{
          return false;
        }
        }
    }
    else if(this->vt==VRT_UI4){
        switch(vt){
        case VRT_BOOL:{
          this->boolVal=(this->ulVal!=0);
            break;
        }
        case VRT_UI1:{
          this->bVal=this->ulVal;
          break;
        }
        case VRT_I1:{
          this->cVal=this->ulVal;
          break;
        }
        case VRT_UI2:{
          this->uiVal=this->ulVal;
          break;
        }
        case VRT_I2:{
          this->iVal=this->ulVal;
          break;
        }
        case VRT_I4:{
          this->lVal=this->ulVal;
          break;
        }
        case VRT_UI8:{
          this->ullVal=this->ulVal;
          break;
        }
        case VRT_I8:{
          this->llVal=this->ulVal;
          break;
        }
        case VRT_R4:{
          this->fltVal=this->ulVal;
          break;
        }
        case VRT_R8:{
          this->dblVal=this->ulVal;
          break;
        }
        case VRT_DATE:
        case VRT_DATETIME:{
          this->date=this->ulVal;
          break;
        }
        case VRT_BSTR:{
          this->bstrVal=new string(to_string(this->ulVal));
          break;
        }
        case VRT_PIN:{
          return false;
        }
        }
    }
    else if(this->vt==VRT_I8){
        switch(vt){
        case VRT_BOOL:{
          this->boolVal=(this->llVal!=0);
            break;
        }
        case VRT_UI1:{
          this->bVal=this->llVal;
          break;
        }
        case VRT_I1:{
          this->cVal=this->llVal;
          break;
        }
        case VRT_UI2:{
          this->uiVal=this->llVal;
          break;
        }
        case VRT_I2:{
          this->iVal=this->llVal;
          break;
        }
        case VRT_UI4:{
          this->ulVal=this->llVal;
          break;
        }
        case VRT_I4:{
          this->lVal=this->llVal;
          break;
        }
        case VRT_UI8:{
          this->ullVal=this->llVal;
          break;
        }
        case VRT_R4:{
          this->fltVal=this->llVal;
          break;
        }
        case VRT_R8:{
          this->dblVal=this->llVal;
          break;
        }
        case VRT_DATE:
        case VRT_DATETIME:{
          this->date=this->llVal;
          break;
        }
        case VRT_BSTR:{
          this->bstrVal=new string(to_string(this->llVal));
          break;
        }
        case VRT_PIN:{
          return false;
        }
        }
    }
    else if(this->vt==VRT_UI8){
        switch(vt){
        case VRT_BOOL:{
          this->boolVal=(this->ullVal!=0);
            break;
        }
        case VRT_UI1:{
          this->bVal=this->ullVal;
          break;
        }
        case VRT_I1:{
          this->cVal=this->ullVal;
          break;
        }
        case VRT_UI2:{
          this->uiVal=this->ullVal;
          break;
        }
        case VRT_I2:{
          this->iVal=this->ullVal;
          break;
        }
        case VRT_UI4:{
          this->ulVal=this->ullVal;
          break;
        }
        case VRT_I4:{
          this->lVal=this->ullVal;
          break;
        }
        case VRT_I8:{
          this->llVal=this->ullVal;
          break;
        }
        case VRT_R4:{
          this->fltVal=this->ullVal;
          break;
        }
        case VRT_R8:{
          this->dblVal=this->ullVal;
          break;
        }
        case VRT_DATE:
        case VRT_DATETIME:{
          this->date=this->ullVal;
          break;
        }
        case VRT_BSTR:{
          this->bstrVal=new string(to_string(this->ullVal));
          break;
        }
        case VRT_PIN:{
          return false;
        }
        }
    }
    else if(this->vt==VRT_R4){
        switch(vt){
        case VRT_BOOL:{
          this->boolVal=(this->fltVal!=0);
            break;
        }
        case VRT_UI1:{
          this->bVal=this->fltVal;
          break;
        }
        case VRT_I1:{
          this->cVal=this->fltVal;
          break;
        }
        case VRT_UI2:{
          this->uiVal=this->fltVal;
          break;
        }
        case VRT_I2:{
          this->iVal=this->fltVal;
          break;
        }
        case VRT_UI4:{
          this->ulVal=this->fltVal;
          break;
        }
        case VRT_I4:{
          this->lVal=this->fltVal;
          break;
        }
        case VRT_UI8:{
          this->ullVal=this->fltVal;
          break;
        }
        case VRT_I8:{
          this->llVal=this->fltVal;
          break;
        }
        case VRT_R8:{
          this->dblVal=this->fltVal;
          break;
        }
        case VRT_DATE:{
            this->date=(LONGLONG)this->fltVal;
            break;
        }
        case VRT_DATETIME:{
          this->date=this->fltVal;
          break;
        }
        case VRT_BSTR:{
          this->bstrVal=new string(to_string(this->fltVal));
          break;
        }
        case VRT_PIN:{
          return false;
        }
        }
    }
    else if(this->vt==VRT_R8){
        switch(vt){
        case VRT_BOOL:{
          this->boolVal=(this->dblVal!=0);
            break;
        }
        case VRT_UI1:{
          this->bVal=this->dblVal;
          break;
        }
        case VRT_I1:{
          this->cVal=this->dblVal;
          break;
        }
        case VRT_UI2:{
          this->uiVal=this->dblVal;
          break;
        }
        case VRT_I2:{
          this->iVal=this->dblVal;
          break;
        }
        case VRT_UI4:{
          this->ulVal=this->dblVal;
          break;
        }
        case VRT_I4:{
          this->lVal=this->dblVal;
          break;
        }
        case VRT_UI8:{
          this->ullVal=this->dblVal;
          break;
        }
        case VRT_I8:{
          this->llVal=this->dblVal;
          break;
        }
        case VRT_R4:{
          this->fltVal=this->dblVal;
          break;
        }
        case VRT_DATE:{
            this->date=(LONGLONG)this->dblVal;
            break;
        }
        case VRT_DATETIME:{
          this->date=this->dblVal;
          break;
        }
        case VRT_BSTR:{
          this->bstrVal=new string(to_string(this->dblVal));
          break;
        }
        case VRT_PIN:{
          return false;
        }
        }
    }
    else if(this->vt==VRT_DATE||this->vt==VRT_DATETIME){
        switch(vt){
        case VRT_BOOL:{
          this->boolVal=(this->date!=0);
            break;
        }
        case VRT_UI1:{
          this->bVal=this->date;
          break;
        }
        case VRT_I1:{
          this->cVal=this->date;
          break;
        }
        case VRT_UI2:{
          this->uiVal=this->date;
          break;
        }
        case VRT_I2:{
          this->iVal=this->date;
          break;
        }
        case VRT_UI4:{
          this->ulVal=this->date;
          break;
        }
        case VRT_I4:{
          this->lVal=this->date;
          break;
        }
        case VRT_UI8:{
          this->ullVal=this->date;
          break;
        }
        case VRT_I8:{
          this->llVal=this->date;
          break;
        }
        case VRT_R4:{
          this->fltVal=this->date;
          break;
        }
        case VRT_R8:{
          this->dblVal=this->date;
          break;
        }
        case VRT_DATE:
            this->date=(LONGLONG)this->date;
            break;
        case VRT_DATETIME:{
            break;
        }
        case VRT_BSTR:{
           if(this->vt==VRT_DATE){
               Date dt(this->date);
               this->bstrVal=new string(dt.ToString());
           }
           else{
               DateTime dt(this->date);
               this->bstrVal=new string(dt.ToString(tstFull));
           }
           break;
        }
        case VRT_PIN:{
          return false;
        }
        }
    }
    else if(this->vt==VRT_BSTR){
        switch(vt){
        case VRT_BOOL:{
          bool bV=atoi((*this->bstrVal).c_str());
          delete this->bstrVal;
          this->boolVal=bV;
          break;
        }
        case VRT_UI1:{
          BYTE bV=atoi((*this->bstrVal).c_str());
          delete this->bstrVal;
          this->bVal=bV;
          break;
        }
        case VRT_I1:{
            CHAR bV=atoi((*this->bstrVal).c_str());
            delete this->bstrVal;
            this->cVal=bV;
            break;
        }
        case VRT_UI2:{
            USHORT bV=atoi((*this->bstrVal).c_str());
            delete this->bstrVal;
            this->uiVal=bV;
            break;
        }
        case VRT_I2:{
            SHORT bV=atoi((*this->bstrVal).c_str());
            delete this->bstrVal;
            this->iVal=bV;
            break;
        }
        case VRT_UI4:{
            ULONG bV=atoi((*this->bstrVal).c_str());
            delete this->bstrVal;
            this->ulVal=bV;
            break;
        }
        case VRT_I4:{
            LONG bV=atoi((*this->bstrVal).c_str());
            delete this->bstrVal;
            this->lVal=bV;
            break;
        }
        case VRT_UI8:{
            ULONGLONG bV=atoi((*this->bstrVal).c_str());
            delete this->bstrVal;
            this->ullVal=bV;
            break;
        }
        case VRT_I8:{
            LONGLONG bV=atoi((*this->bstrVal).c_str());
            delete this->bstrVal;
            this->llVal=bV;
            break;
        }
        case VRT_R4:{
            FLOAT bV=atof((*this->bstrVal).c_str());
            delete this->bstrVal;
            this->fltVal=bV;
            break;
        }
        case VRT_R8:{
            DOUBLE bV=atof((*this->bstrVal).c_str());
            delete this->bstrVal;
            this->dblVal=bV;
            break;
        }
        case VRT_DATE:{
          Date dt;
          dt.FromString(*this->bstrVal);
          delete this->bstrVal;
          this->date=(LONGLONG)dt.ToDouble();
          break;
        }
        case VRT_DATETIME:{
          DateTime dt;
          dt.FromString(*this->bstrVal);
          delete this->bstrVal;
          this->date=dt.ToDouble();
          break;
        }
        case VRT_PIN:{
          return false;
        }
        }
    }
    else if(this->vt==VRT_PIN){
        return false;
    }
    this->vt=vt;
    return true;
}

string Variant::ToSqlString(){
    switch(vt){
    case VRT_EMPTY:
        return "null";
    case VRT_I4:
        return to_string(lVal);
    case VRT_R4:
        return to_string(fltVal);
    case VRT_R8:
        return to_string(dblVal);
    case VRT_I8:
        return to_string(llVal);
    case VRT_I2:
        return to_string(iVal);
    case VRT_UI4:
        return to_string(ulVal);
    case VRT_UI2:
        return to_string(uiVal);
    case VRT_UI1:
        return to_string(bVal);
    case VRT_UI8:
        return to_string(ullVal);
    case VRT_I1:
        return to_string(cVal);
    case VRT_BSTR:
        return "'"+(*bstrVal)+"'";
    case VRT_DATE:{
        Date dt(date);
        return "'"+dt.ToString()+"'";
    }
    case VRT_DATETIME:{
        DateTime dt(date);
        return "'"+dt.ToString(tstFull)+"'";
    }
    }
    return "null";
}

bool Variant::asBool(){
    if(vt!=VRT_PIN){
        ChangeType(VRT_BOOL);
        return boolVal;
    }
    bool*value=(bool*)pData;
    return *value;
}
char Variant::asChar(){
    if(vt!=VRT_PIN){
        ChangeType(VRT_I1);
        return cVal;
    }
    char*value=(char*)pData;
    return *value;
}
BYTE Variant::asByte(){
    if(vt!=VRT_PIN){
        ChangeType(VRT_UI1);
        return bVal;
    }
    BYTE*value=(BYTE*)pData;
    return *value;
}
SHORT Variant::asShort(){
    if(vt!=VRT_PIN){
        ChangeType(VRT_I2);
        return iVal;
    }
    SHORT*value=(SHORT*)pData;
    return *value;
}
SHORT Variant::asUShort(){
    if(vt!=VRT_PIN){
        ChangeType(VRT_UI2);
        return uiVal;
    }
    USHORT*value=(USHORT*)pData;
    return *value;
}
int Variant::asInt(){
    if(vt!=VRT_PIN){
        ChangeType(VRT_I4);
        return lVal;
    }
    LONG*value=(LONG*)pData;
    return *value;
}
UINT Variant::asUInt(){
    if(vt!=VRT_PIN){
        ChangeType(VRT_UI4);
        return ulVal;
    }
    UINT*value=(UINT*)pData;
    return *value;
}

ULONGLONG Variant::asULong(){
    if(vt!=VRT_PIN){
        ChangeType(VRT_UI8);
        return lVal;
    }
    ULONGLONG*value=(ULONGLONG*)pData;
    return *value;
}
LONGLONG Variant::asLong(){
    if(vt!=VRT_PIN){
        ChangeType(VRT_I8);
        return ulVal;
    }
    LONG*value=(LONG*)pData;
    return *value;
}
float Variant::asSingle(){
    if(vt!=VRT_PIN){
        ChangeType(VRT_R4);
        return fltVal;
    }
    FLOAT*value=(FLOAT*)pData;
    return *value;
}
double Variant::asDouble(){
    if(vt!=VRT_PIN){
        ChangeType(VRT_R8);
        return dblVal;
    }
    DOUBLE*value=(DOUBLE*)pData;
    return *value;
}
Date Variant::asDate(){
    if(vt!=VRT_PIN){
        ChangeType(VRT_DATE);
        Date dt(date);
        return dt;
    }
    DATE*value=(DATE*)pData;
    Date dt(*value);
    return dt;
}
DateTime Variant::asDateTime(){
    if(vt!=VRT_PIN){
        ChangeType(VRT_DATETIME);
        DateTime dt(date);
        return dt;
    }
    DATE*value=(DATE*)pData;
    DateTime dt(*value);
    return dt;
}
string Variant::asString(){
    if(vt!=VRT_PIN){
        ChangeType(VRT_BSTR);
        return *bstrVal;
    }
    if(pData==nullptr) return "";
    string*value=(string*)pData;
    return *value;
}

}
