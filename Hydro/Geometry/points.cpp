#include "points.h"
#include "geometrybase.h"
#include "Point.h"
#include "envelope.h"
#include "polygon.h"
#include "math.h"
#include "Base/classfactory.h"
#include "spatialreference.h"
#include "segmentindexes.h"

namespace SGIS{

REGISTER(Points)

Points::Points()
{
    XMin=YMin=XMax=YMax=ZMin=ZMax=0;
    IsExtentValid=false;
}
Points::~Points(){

}
//IGeometry:
void Points::ComputeEnvelope(){
    IsExtentValid=true;
    int Size=Zs.size();
    if(Size==0)
    {
        XMin=YMin=XMax=YMax=ZMin=ZMax=0;
        return;
    }
    XMin=XMax=XYs[0];
    YMin=YMax=XYs[1];
    ZMin=ZMax=Zs[0];
    LONG pos=2;
    for(int k=1;k<Size;k++)
    {
        DOUBLE X,Y,Z;
        X=XYs[pos++];
        Y=XYs[pos++];
        Z=Zs[k];
        if(X<XMin) XMin=X;
        if(X>XMax) XMax=X;
        if(Y<YMin) YMin=Y;
        if(Y>YMax) YMax=Y;
        if(Z<ZMin) ZMin=Z;
        if(Z>ZMax) ZMax=Z;
    }
}

AutoPtr<Envelope>Points::GetExtent(){
    if(!IsExtentValid) ComputeEnvelope();
    return AutoPtr<Envelope>(new Envelope(XMin,YMax,XMax,YMin));
}
GeometryType Points::GetType(){
    return gtPoints;
}
bool Points::Equals(Geometry*other){
    if(GetType()!=other->GetType()) return false;
    Points*points=(Points*)other;
    LONG count1,count2;
    count1=Zs.size();
    count2=points->Zs.size();
    if(count1!=count2) return false;
    LONG pos=0;
    double x1,y1,x2,y2;
    for(int k=0;k<count1;k++)
    {
        x1=XYs[pos];
        y1=XYs[pos+1];
        x2=points->XYs[pos];
        y2=points->XYs[pos+1];
        pos+=2;
        if((x1==x2)||(y1==y2)) return false;
    }
    return true;
}

bool Points::Intersects(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return ppt->Intersects(this);
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->Intersects(this);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->Intersects(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Intersects(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=ppts->GetExtent();
        AutoPtr<Envelope>tEnv;
        tEnv=GetExtent();
        bool IsIn;
        IsIn=tEnv->Intersects(pEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG ptCount;
        ptCount=ppts->GetSize();
        LONG pCount=Zs.size();
        DOUBLE ox,oy;
        for(int k=0;k<ptCount;k++)
        {
            ppts->GetItem(k,ox,oy);
            LONG pos=0;
            DOUBLE x,y;
            for(int p=0;p<pCount;p++)
            {
                x=XYs[pos++];
                y=XYs[pos++];
                if((fabs(x-ox)<=VertexTolerance)&&(fabs(y-oy)<=VertexTolerance))
                {
                    return true;
                }
            }
        }
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polyline->GetExtent();
        AutoPtr<Envelope>tEnv;
        tEnv=GetExtent();
        bool IsIn;
        IsIn=tEnv->Intersects(pEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG pCount=Zs.size();
        DOUBLE x,y;
        AutoPtr<Point>ppt(new Point());
        LONG pos=0;
        for(int p=0;p<pCount;p++)
        {
            x=XYs[pos++];
            y=XYs[pos++];
            ppt->PutCoord(x,y);
            IsIn=pEnv->IsPointIn(ppt.get());
            if(!IsIn)
            {
                return false;
            }
            bool IsOn;
            IsOn=polyline->CanCaptureBorder(ppt.get(),VertexTolerance);
            if(IsOn)
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polygon->GetExtent();
        AutoPtr<Envelope>tEnv;
        tEnv=GetExtent();
        bool IsIn;
        IsIn=tEnv->Intersects(pEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG pCount=Zs.size();
        DOUBLE x,y;
        AutoPtr<Point>ppt(new Point());
        LONG pos=0;
        for(int p=0;p<pCount;p++)
        {
            x=XYs[pos++];
            y=XYs[pos++];
            ppt->PutCoord(x,y);
            IsIn=pEnv->IsPointIn(ppt.get());
            if(!IsIn)
            {
                return false;
            }
            IsIn=polygon->IsPointIn(ppt.get());
            if(IsIn)
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=mpolygon->GetExtent();
        AutoPtr<Envelope>tEnv;
        tEnv=GetExtent();
        bool IsIn;
        IsIn=tEnv->Intersects(pEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG partCount;
        partCount=mpolygon->GetSize();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>subpoly;
            subpoly=mpolygon->GetItem(k);
            IsIn=Intersects(subpoly.get());
            if(IsIn)
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=nullptr)
    {
        LONG subCount;
        subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo;
            pGeo=pCol->GetItem(k);
            IsInter=Intersects(pGeo.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    return false;
}
bool Points::Disjoint(Geometry*other){
    return (!Intersects(other));
}
bool Points::Touches(Geometry*other){
    AutoPtr<Envelope>pEnv;
    pEnv=other->GetExtent();
    AutoPtr<Envelope>tEnv;
    tEnv=GetExtent();
    bool IsIn;
    IsIn=tEnv->Intersects(pEnv.get());
    if(!IsIn)
    {
        return false;
    }
    LONG count=Zs.size();
    DOUBLE x,y;
    AutoPtr<Point>ppt(new Point());
    LONG pos=0;
    for(int p=0;p<count;p++)
    {
        x=XYs[pos++];
        y=XYs[pos++];
        ppt->PutCoord(x,y);
        IsIn=ppt->Touches(other);
        if(IsIn)
        {
            return true;
        }
    }
    return false;
}
bool Points::Crosses(Geometry*other){
    return false;
}
bool Points::Within(Geometry*other){
    AutoPtr<Envelope>tEnvi;
    tEnvi=GetExtent();
    AutoPtr<Envelope>pEnv;
    pEnv=other->GetExtent();
    bool IsIn;
    IsIn=pEnv->Contains(tEnvi.get());
    if(!IsIn)
    {
        return false;
    }
    LONG count=Zs.size();
    DOUBLE x,y;
    AutoPtr<Point>ppt(new Point());
    LONG pos=0;
    for(int p=0;p<count;p++)
    {
        x=XYs[pos++];
        y=XYs[pos++];
        ppt->PutCoord(x,y);
        IsIn=ppt->Within(other);
        if(!IsIn)
        {
            return false;
        }
    }
    return true;
}
bool Points::Contains(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return ppt->Within(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return ppts->Within(this);
    }
    return false;
}
bool Points::Overlaps(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return ppt->Overlaps(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return ppt->Intersects(this);
    }
    return false;
}

AutoPtr<Geometry>Points::Clone(){
    Points*other=new Points();
    other->XYs.resize(XYs.size());
    other->Zs.resize(Zs.size());
    memcpy(other->XYs.data(),XYs.data(),XYs.size()*sizeof(double));
    memcpy(other->Zs.data(),Zs.data(),Zs.size()*sizeof(double));
    other->IsExtentValid=IsExtentValid;
    other->XMin=XMin;
    other->YMin=YMin;
    other->XMax=XMax;
    other->YMax=YMax;
    return other;
}
void Points::AffineTrans(Point*AnchorPoint,DOUBLE RotateAngle,DOUBLE ZoomXRatio,DOUBLE ZoomYRatio,DOUBLE DiffX,DOUBLE DiffY){
    DOUBLE Ax,Ay;
    DOUBLE X,Y;
    AnchorPoint->GetCoord(&Ax,&Ay);
    double difx,dify;
    difx=DiffX+Ax;
    dify=DiffY+Ay;
    if(RotateAngle!=0)
    {
        double px,py;
        double cosv=cos(RotateAngle);
        double sinv=sin(RotateAngle);
        int pos=0;
        for(int k=Zs.size()-1;k>=0;k--)
        {
            X=XYs[pos];
            Y=XYs[pos+1];
            px=X-Ax;
            py=Y-Ay;
            X=(px*cosv-py*sinv)*ZoomXRatio+difx;
            Y=(px*sinv+py*cosv)*ZoomYRatio+dify;
            XYs[pos]=X;
            XYs[pos+1]=Y;
            pos+=2;
        }
    }
    else
    {
        int pos=0;
        for(int k=Zs.size()-1;k>=0;k--)
        {
            X=XYs[pos];
            Y=XYs[pos+1];
            X=(X-Ax)*ZoomXRatio+difx;
            Y=(Y-Ay)*ZoomYRatio+dify;
            XYs[pos]=X;
            XYs[pos+1]=Y;
            pos+=2;
        }
    }
    IsExtentValid=false;
}
bool Points::Project(CoordinateTransformation*pTrans){
    bool CanTransform=pTrans->CanTransform();
    if(!CanTransform) return false;
    LONG Count=Zs.size();
    DOUBLE*xys=XYs.data();
    DOUBLE*txs=new DOUBLE[Zs.size()];
    DOUBLE*tys=new DOUBLE[Zs.size()];
    LONG pos=0;
    for(int k=0;k<Count;k++)
    {
        txs[k]=xys[pos++];
        tys[k]=xys[pos++];
    }
    bool IsOk=pTrans->TransformArray(txs,tys,Zs.size());
    if(IsOk)
    {
        pos=0;
        for(int k=0;k<Count;k++)
        {
            xys[pos]=txs[k];
            xys[pos+1]=tys[k];
            pos+=2;
        }
        ComputeEnvelope();
        delete[]txs;
        delete[]tys;
        return true;
    }
    delete[]txs;
    delete[]tys;
    return false;
}
void Points::Scale(DOUBLE xr,DOUBLE yr,DOUBLE zr){
    int Size=Zs.size();
    int pos=0;
    for(int k=0;k<Size;k++)
    {
        DOUBLE X,Y;
        X=XYs[pos];Y=XYs[pos+1];
        X=X*xr;
        Y=Y*yr;
        XYs[pos++]=X;
        XYs[pos++]=Y;
        Zs[k]*=zr;
    }
    if(!IsExtentValid)
        ComputeEnvelope();
    else
    {
        XMin=XMin*xr;
        XMax=XMax*xr;
        YMin=YMin*yr;
        YMax=YMax*yr;
    }
}
void Points::Move(DOUBLE difx,DOUBLE dify,DOUBLE difz){
    int Size=Zs.size();
    int pos=0;
    for(int k=0;k<Size;k++)
    {
        DOUBLE X,Y;
        X=XYs[pos];Y=XYs[pos+1];
        X+=difx;
        Y+=dify;
        XYs[pos++]=X;
        XYs[pos++]=Y;
        Zs[k]+=difz;
    }
    if(!IsExtentValid)
        ComputeEnvelope();
    else
    {
        XMin=XMin+difx;
        XMax=XMax+difx;
        YMin=YMin+dify;
        YMax=YMax+dify;
    }
}
//BufferWritable:
bool Points::Read(BufferReader*br){
    int size=br->Read<int>();
    XYs.resize(size<<1);
    Zs.resize(size);
    int nIndex=0;
    for(int k=0;k<size;k++){
        XYs[nIndex++]=br->Read<double>();
        XYs[nIndex++]=br->Read<double>();
    }
    IsExtentValid=false;
    return true;
}

void Points::Write(BufferWriter*bw){
    int size=Zs.size();
    bw->Write<int>(size);
    int nIndex=0;
    for(int k=0;k<size;k++){
        bw->Write<double>(XYs[nIndex++]);
        bw->Write<double>(XYs[nIndex++]);
    }
}
string Points::GetTypeName(){
    return "Points";
}
bool Points::ReadFromJson(JsonReader*jr,JsonNode node){
   int size=node.size();
   XYs.resize(size<<1);
   Zs.resize(size);
   for(int k=0;k<size;k++){
       JsonNode child=node[k];
       if(child.size()<2){
           continue;
       }
       double x=child[0].asDouble();
       double y=child[1].asDouble();
       XYs[(k<<1)]=x;
       XYs[(k<<1)+1]=y;
   }
   return true;
}

string Points::ToJson(){
    int pointsCount=Zs.size();
    string resultString="[";
    int index=0;
    for(int k=0;k<pointsCount;k++){
        double x=XYs[index++];
        double y=XYs[index++];
        if(k>0)
            resultString+=",["+to_string(x)+","+to_string(y)+"]";
        else
            resultString+="["+to_string(x)+","+to_string(y)+"]";
    }
    resultString+="]";
    return resultString;
}
//Geometry3D:
AutoPtr<Envelope>Points::GetExtent3D(){
    if(!IsExtentValid) ComputeEnvelope();
    double minz=0,maxz=0;
    int size=Zs.size();
    if(size>0){
        minz=maxz=Zs[0];
        for(int k=1;k<size;k++){
            if(Zs[k]<minz) minz=Zs[k];
            if(Zs[k]>maxz) maxz=Zs[k];
        }
    }
     return AutoPtr<Envelope>(new Envelope(XMin,YMax,XMax,YMin,minz,maxz));
}
void Points::Rotate(DOUBLE RotateAngle,Vector*axis){
    double rx,ry,rz;
    axis->GetCoord(&rx,&ry,&rz);
    double L=sqrt(rx*rx+ry*ry+rz*rz);
    RotateAngle=RotateAngle/180*PI;
    FLOAT cosa=cos(RotateAngle);
    FLOAT sina=sin(RotateAngle);
    if(L!=0)
    {
        rx=rx/L;
        ry=ry/L;
        rz=rz/L;
    }
    int Size=Zs.size();
    int pos=0;
    for(int k=0;k<Size;k++)
    {
        DOUBLE X,Y,Z;
        X=XYs[pos];Y=XYs[pos+1];Z=Zs[k];
        innerRotate(sina,cosa,rx,ry,rx,X,Y,Z);
        XYs[pos++]=X;
        XYs[pos++]=Y;
        Zs[k]=Z;
    }
    ComputeEnvelope();
}

bool Points::Read3D(BufferReader*br){
    int size=br->Read<int>();
    XYs.resize(size<<1);
    Zs.resize(size);
    int nIndex=0;
    for(int k=0;k<size;k++){
        XYs[nIndex++]=br->Read<double>();
        XYs[nIndex++]=br->Read<double>();
        Zs[k]=br->Read<double>();
    }
    IsExtentValid=false;
    return true;
}

void Points::Write3D(BufferWriter*bw){
    int size=Zs.size();
    bw->Write<int>(size);
    int nIndex=0;
    for(int k=0;k<size;k++){
        bw->Write<double>(XYs[nIndex++]);
        bw->Write<double>(XYs[nIndex++]);
         bw->Write<double>(Zs[k]);
    }
}
bool Points::ReadFromJson3D(JsonReader*jr,JsonNode node){
    int size=node.size();
    XYs.resize(size<<1);
    Zs.resize(size);
    for(int k=0;k<size;k++){
        JsonNode child=node[k];
        if(child.size()<2){
            continue;
        }
        double x=child[0].asDouble();
        double y=child[1].asDouble();
        double z=0;
        if(child.size()>2) z=child[2].asDouble();
        XYs[(k<<1)]=x;
        XYs[(k<<1)+1]=y;
        Zs[k]=z;
    }
    return true;
}
string Points::ToJson3D(){
    int pointsCount=Zs.size();
    string resultString="[";
    int index=0;
    for(int k=0;k<pointsCount;k++){
        double x=XYs[index++];
        double y=XYs[index++];
        double z=Zs[index++];
        if(k>0)
            resultString+=",["+to_string(x)+","+to_string(y)+","+to_string(z)+"]";
        else
            resultString+=",["+to_string(x)+","+to_string(y)+","+to_string(z)+"]";
    }
    resultString+="]";
    return resultString;
}

void Points::Add(double x,double y,double z){
    int size=XYs.size();
    XYs.resize(size+2);
    XYs[size]=x;
    XYs[size+1]=y;
    Zs.push_back(z);
    if(IsExtentValid)
    {
        if(x<XMin) XMin=x;
        if(x>XMax) XMax=x;
        if(y<YMin) YMin=y;
        if(y>YMax) YMax=y;
        if(z<ZMin) ZMin=z;
        if(z>ZMax) ZMax=z;
    }
}

int Points::GetSize(){
    return Zs.size();
}
void Points::SetSize(int size){
    XYs.resize(size<<1);
    Zs.resize(size);
    IsExtentValid=false;
}

void Points::Insert(int nIndex,double x,double y,double z){
    XYs.insert(begin(XYs)+(nIndex<<1),{x,y});
    Zs.insert(begin(Zs)+nIndex,z);
    if(IsExtentValid)
    {
        if(x<XMin) XMin=x;
        if(x>XMax) XMax=x;
        if(y<YMin) YMin=y;
        if(y>YMax) YMax=y;
        if(z<ZMin) ZMin=z;
        if(z>ZMax) ZMax=z;
    }
}
void Points::Remove(int nIndex){
    XYs.erase(begin(XYs)+(nIndex<<1),begin(XYs)+(nIndex<<1)+2);
    Zs.erase(begin(Zs)+nIndex);
    IsExtentValid=false;
}

void Points::Clear(){
    XYs.clear();
    Zs.clear();
    IsExtentValid=false;
}

void Points::GetItem(int nIndex,double&x,double&y){
    x=XYs[nIndex<<1];
    y=XYs[(nIndex<<1)+1];
}

void Points::GetItem(int nIndex,double&x,double&y,double&z){
    x=XYs[nIndex<<1];
    y=XYs[(nIndex<<1)+1];
    z=Zs[nIndex];
}

void Points::SetItem(int nIndex,double x,double y){
    XYs[nIndex<<1]=x;
    XYs[(nIndex<<1)+1]=y;
    IsExtentValid=false;
}

void Points::SetItem(int nIndex,double x,double y,double z){
    XYs[nIndex<<1]=x;
    XYs[(nIndex<<1)+1]=y;
    Zs[nIndex]=z;
     IsExtentValid=false;
}

void Points::PutCoordByPtr(int ptCount,DOUBLE*data){
    XYs.resize(ptCount*2);
    Zs.resize(ptCount);
    memcpy(XYs.data(),data,sizeof(DOUBLE)*ptCount*2);
    memset(Zs.data(),0,sizeof(DOUBLE)*ptCount);
    IsExtentValid=false;
}

AutoPtr<Point>Points::operator [](int nIndex){
    AutoPtr<Point>ppt(new Point());
    ppt->PutCoord(XYs[nIndex<<1],XYs[(nIndex<<1)+1],Zs[nIndex]);
    return ppt;
}

double*Points::GetXYs(){
    return XYs.data();
}
double*Points::GetZs(){
    return Zs.data();
}

void Points::Reverse(){
    int Size=Zs.size();
    if(Size<2)
    {
        return;
    }
    int SemiSize=Size/2;
    DOUBLE temp;
    int pos;
    int pos1;
    for(int k=0;k<SemiSize;k++)
    {
        pos=(k<<1);
        pos1=((Size-k-1)<<1);
        temp=XYs[pos];
        XYs[pos]=XYs[pos1];
        XYs[pos1]=temp;
        temp=XYs[pos+1];
        XYs[pos+1]=XYs[pos1+1];
        XYs[pos1+1]=temp;
        temp=Zs[k];
        Zs[k]=Zs[Size-k-1];
        Zs[Size-k-1]=temp;
    }
}

double Points::GetLength(){
    int Size=Zs.size();
    if(Size==0) return 0;
    DOUBLE len=0;
    DOUBLE X1,Y1,X2,Y2;
    X1=XYs[0];
    Y1=XYs[1];
    LONG pos=2;
    for(int k=1;k<Size;k++)
    {
        X2=XYs[pos++];
        Y2=XYs[pos++];
        len+=sqrt(pow(X2-X1,2.0)+pow(Y2-Y1,2.0));
        X1=X2;Y1=Y2;
    }
    return len;
}

bool Points::Smooth(double tolerance){
    int pointSize=Zs.size();
    if(pointSize<3) return false;
    DOUBLE*xys=XYs.data();
    Spline sp(xys,pointSize);
    sp.Generate();
    int toSize=sp.GetCurveCount(tolerance)+1;
    Point2D<double>*dpt=new Point2D<double>[toSize];
    int Size=1;
    dpt[0].X=xys[0];
    dpt[0].Y=xys[1];
    sp.GetCurve(dpt,tolerance,Size);
    XYs.resize(toSize*2);
    memcpy(XYs.data(),dpt,toSize*16);
    delete []dpt;
    Zs.resize(toSize);
    IsExtentValid=false;
    return true;
}
bool Points::SmoothByPtNum(int insertPointsNum){
    int pointSize=Zs.size();
    if(pointSize<3) return false;
    DOUBLE*xys=XYs.data();
    DOUBLE lastX,lastY;
    lastX=xys[(pointSize-1)*2];
    lastY=xys[(pointSize-1)*2+1];
    Spline sp(xys,pointSize);
    sp.Generate();
    int toSize=(pointSize-1)*insertPointsNum+pointSize;
    Point2D<double>*dpt=new Point2D<double>[toSize];
    dpt[0].X=XYs[0];
    dpt[0].Y=XYs[1];
    int Size=1;
    sp.GetCurveByPointNum(dpt,insertPointsNum,Size);
    XYs.resize(toSize*2);
    memcpy(XYs.data(),dpt,toSize*16);
    XYs[(toSize-1)*2]=lastX;
    XYs[(toSize-1)*2+1]=lastY;
    delete []dpt;
    Zs.resize(toSize);
    IsExtentValid=false;
    return true;
}
void Points::Simplify(double toler){
    LONG ptsCount=Zs.size();
    if(ptsCount<=2) return;
    int pin = 0;
    double maxtoler = 0;
    int node1,node2;
    double dist;
    double x = 0, y = 0;
    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    int*stackfromNodes = new int[ptsCount];
    int*stacktoNodes = new int[ptsCount];
    double*tolerances = new double[ptsCount];
    for (int k = 0; k < ptsCount; k++) {
        tolerances[k] = 0;
        stackfromNodes[k] = 0;
        stacktoNodes[k] = 0;
    }
    stackfromNodes[pin] = 0;
    stacktoNodes[pin] = ptsCount-1;
    pin++;
    AutoPtr<Segment>seg(new Segment());
    AutoPtr<Point>ppt(new Point());
    for (int k = 0; k < pin; k++)
    {
        if (k >= pin) break;
        node1 = stackfromNodes[k];
        node2 = stacktoNodes[k];
        if ((node1 - node2 == 1) || (node2 - node1 == 1)) continue;
        x1=XYs[node1<<1];
        y1=XYs[(node1<<1)+1];
        x2=XYs[node1<<1];
        y2=XYs[(node1<<1)+1];
        seg->PutCoord(x1, y1, x2, y2);
        int pos = -1;
        double maxdist = 0;
        for (int j = node1 + 1; j < node2; j++)
        {
           x=XYs[(j<<1)];
           y=XYs[(j<<1)+1];
           ppt->PutCoord(x, y);
           dist=ppt->DistanceToSegment(seg.get());
           if (dist > maxdist)
           {
               maxdist = dist;
               pos = j;
           }
        }
       if (pos == -1) continue;
       if(maxdist<toler) continue;
       tolerances[pos] = maxdist;
       if (pos > node1 + 1)
       {
           stackfromNodes[pin] = node1;
           stacktoNodes[pin] = pos;
           pin++;
       }
       if (node2 > pos + 1)
       {
           stackfromNodes[pin] = pos;
           stacktoNodes[pin] = node2;
           pin++;
       }
       if(maxdist>maxtoler) maxtoler=maxdist;
    }
    if(maxtoler<toler) maxtoler=toler;
    tolerances[0]=maxtoler;
    tolerances[ptsCount-1]=maxtoler;
    int curPin=0;
    for (int k = 0; k < ptsCount; k++)
    {
         if (tolerances[k] >= toler)
         {
             XYs[(curPin<<1)]=XYs[(k<<1)];
             XYs[(curPin<<1)+1]=XYs[(k<<1)+1];
             Zs[curPin]=Zs[k];
             curPin++;
         }
    }
    delete []stackfromNodes;
    delete []stacktoNodes;
    delete []tolerances;
    XYs.resize(curPin<<1);
    Zs.resize(curPin);
}

void Points::SimplifyByPtNum(int ptNum){
    if(ptNum<2) ptNum=2;
    LONG ptsCount=Zs.size();
    if(ptNum>=ptsCount) return;
    int pin = 0;
    double maxtoler = 0;
    int node1,node2;
    double dist;
    double x = 0, y = 0;
    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    int*stackfromNodes = new int[ptsCount];
    int*stacktoNodes = new int[ptsCount];
    double*tolerances = new double[ptsCount];
    for (int k = 0; k < ptsCount; k++) {
        tolerances[k] = 0;
        stackfromNodes[k] = 0;
        stacktoNodes[k] = 0;
    }
    stackfromNodes[pin] = 0;
    stacktoNodes[pin] = ptsCount-1;
    pin++;
    AutoPtr<Segment>seg(new Segment());
    AutoPtr<Point>ppt(new Point());
    OrderArray<double>pArray(true,ptNum);
    for (int k = 0; k < pin; k++)
    {
        if (k >= pin) break;
        node1 = stackfromNodes[k];
        node2 = stacktoNodes[k];
        if ((node1 - node2 == 1) || (node2 - node1 == 1)) continue;
        x1=XYs[node1<<1];
        y1=XYs[(node1<<1)+1];
        x2=XYs[node1<<1];
        y2=XYs[(node1<<1)+1];
        seg->PutCoord(x1, y1, x2, y2);
        int pos = -1;
        double maxdist = 0;
        for (int j = node1 + 1; j < node2; j++)
        {
           x=XYs[(j<<1)];
           y=XYs[(j<<1)+1];
           ppt->PutCoord(x, y);
           dist=ppt->DistanceToSegment(seg.get());
           if (dist > maxdist)
           {
               maxdist = dist;
               pos = j;
           }
        }
       if (pos == -1) continue;
       if(pArray.GetSize()>=ptNum){
          if(maxdist<-pArray[pArray.GetSize()-1]) continue;
       }
       tolerances[pos] = maxdist;
       pArray.Add(-maxdist);
       if (pos > node1 + 1)
       {
           stackfromNodes[pin] = node1;
           stacktoNodes[pin] = pos;
           pin++;
       }
       if (node2 > pos + 1)
       {
           stackfromNodes[pin] = pos;
           stacktoNodes[pin] = node2;
           pin++;
       }
       if(maxdist>maxtoler) maxtoler=maxdist;
    }
    tolerances[0]=maxtoler;
    tolerances[ptsCount-1]=maxtoler;
    pArray.Add(-maxtoler);
    pArray.Add(-maxtoler);
    double mintoler=-pArray[pArray.GetSize()-1];
    int hasNum=1;
    int curPin=pArray.GetSize()-2;
    if(curPin>=0){
        while(pArray[curPin]==-mintoler){
            hasNum++;
            curPin--;
            if(curPin<0) break;
        }
    }
    curPin=0;
    int addedNum=0;
    for (int k = 0; k < ptsCount; k++)
    {
         if (tolerances[k]>mintoler)
         {
             XYs[(curPin<<1)]=XYs[(k<<1)];
             XYs[(curPin<<1)+1]=XYs[(k<<1)+1];
             Zs[curPin]=Zs[k];
             curPin++;
         }
         else if(tolerances[k]==mintoler){
             if(addedNum<hasNum){
                 XYs[(curPin<<1)]=XYs[(k<<1)];
                 XYs[(curPin<<1)+1]=XYs[(k<<1)+1];
                 Zs[curPin]=Zs[k];
                 curPin++;
             }
             addedNum++;
         }
    }
    delete []stackfromNodes;
    delete []stacktoNodes;
    delete []tolerances;
    XYs.resize(curPin<<1);
    Zs.resize(curPin);
}

bool Points::CanCaptureBorder(Point*ppt,double tolerance){
    int ptCount=Zs.size();
    if(ptCount<2) return false;
    AutoPtr<Envelope>pEnv=GetExtent();
    DOUBLE xmin,ymin,xmax,ymax;
    pEnv->GetCoord(&xmin,&ymax,&xmax,&ymin);
    DOUBLE X,Y;
    ppt->GetCoord(&X,&Y);
    if((X<xmin-tolerance)||(X>xmax+tolerance)||(Y<ymin-tolerance)||(Y>ymax+tolerance)) return false;
    AutoPtr<Segment>seg(new Segment());
    seg->Lx1()=XYs[0];
    seg->Ly1()=XYs[1];
    int pos=2;
    for(int k=1;k<ptCount;k++)
    {
        seg->Lx2()=XYs[pos++];
        seg->Ly2()=XYs[pos++];
        if(seg->IsPointOnSegment(X,Y,tolerance)) return true;
        seg->Lx1()=seg->Lx2();
        seg->Ly1()=seg->Ly2();
    }
    return false;
}

AutoPtr<Point>Points::CaptureBorder(Point*ppt,double tolerance){
    int ptCount=Zs.size();
    if(ptCount<2) return nullptr;
    AutoPtr<Envelope>pEnv=GetExtent();
    DOUBLE xmin,ymin,xmax,ymax;
    pEnv->GetCoord(&xmin,&ymax,&xmax,&ymin);
    DOUBLE X,Y;
    ppt->GetCoord(&X,&Y);
    if((X<xmin-tolerance)||(X>xmax+tolerance)||(Y<ymin-tolerance)||(Y>ymax+tolerance)) return nullptr;
    Segment2D<double>seg;
    seg.Fuzzy=tolerance;
    seg.lnx1=XYs[0];
    seg.lny1=XYs[1];
    int pos=2;
    DOUBLE nearX,nearY;
    DOUBLE capX,capY;
    double dist;
    double minDist=-1;
    for(int k=1;k<ptCount;k++)
    {
        seg.lnx2=XYs[pos++];
        seg.lny2=XYs[pos++];
        if((fabs(seg.lnx1-seg.lnx2)<=VertexTolerance)&&(fabs(seg.lny1-seg.lny2)<=VertexTolerance)) continue;
        dist=seg.PtMinDistToSegment(X,Y,nearX,nearY);
        if(dist>tolerance)
        {
            seg.lnx1=seg.lnx2;
            seg.lny1=seg.lny2;
            continue;
        }
        if(minDist==-1)
        {
            capX=nearX;
            capY=nearY;
            minDist=dist;
        }
        else if(dist<minDist)
        {
            capX=nearX;
            capY=nearY;
            minDist=dist;
        }
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lny2;
    }
    if(minDist==-1) return nullptr;
    AutoPtr<Point>tpt(new Point());
    tpt->PutCoord(capX,capY);
    return tpt;
}

AutoPtr<Point>Points::ReturnPoint(double length){
    int Size=Zs.size();
    if(Size<1) return nullptr;
    if(Size<2)
    {
        if(length==0) return new Point(XYs[0],XYs[1]);
        return nullptr;
    }
    double len=0;
    double curlen;
    DOUBLE X1,Y1,X2,Y2;
    X1=XYs[0];
    Y1=XYs[1];
    long pos=2;
    for(int k=1;k<Size;k++)
    {
        X2=XYs[pos++];
        Y2=XYs[pos++];
        curlen=sqrt(pow(X2-X1,2.0)+pow(Y2-Y1,2.0));
        len+=curlen;
        if((len>=length)||(k==Size-1))
        {
            float ratio=(curlen-len+length)/curlen;
            return new Point(X1+(X2-X1)*ratio,Y1+(Y2-Y1)*ratio);
        }
        X1=X2;Y1=Y2;
    }
    return nullptr;
}
AutoPtr<Points>Points::ReturnPoints(const vector<double>&lengths){
    int Size=Zs.size();
    if(Size==0) return nullptr;
    int Num=lengths.size();
    if(Num==0) return nullptr;
    //double*dlens=(double*)lengths->pvData;
    Points*newPts=new Points();
    DOUBLE*xs=new DOUBLE[Num];
    DOUBLE*ys=new DOUBLE[Num];
    if(Size==1)
    {
        for(int k=0;k<Num;k++)
        {
            xs[k]=XYs[0];
            ys[k]=XYs[1];
        }
    }
    else
    {
        float len=0;
        float curlen;
        float CurIndexLen=lengths[0];
        int CurIndex=0;
        bool NeedContinue=true;
        DOUBLE X1,Y1,X2,Y2;
        X1=XYs[0];
        Y1=XYs[1];
        long pos=2;
        for(int k=1;k<Size;k++)
        {
            X2=XYs[pos++];
            Y2=XYs[pos++];
            curlen=sqrt(pow(X1-X2,2.0)+pow(Y1-Y2,2.0));
            if(curlen==0) continue;
            len+=curlen;
            while(len>=CurIndexLen)
            {
                float ratio=(curlen-len+CurIndexLen)/curlen;
                if(ratio<0) ratio=0;
                xs[CurIndex]=X1+(X2-X1)*ratio;
                ys[CurIndex]=Y1+(Y2-Y1)*ratio;
                if(CurIndex>=Num-1)
                {
                    CurIndex++;
                    NeedContinue=false;
                    break;
                }
                CurIndex++;
                CurIndexLen=lengths[CurIndex];
            }
            if(!NeedContinue) break;
            X1=X2;Y1=Y2;
        }
        if(CurIndex<=Num-1)
        {
            for(int k=CurIndex;k<Num;k++)
            {
                xs[k]=X2;
                ys[k]=Y2;
            }
        }
    }
    for(long k=0;k<Num;k++)
    {
        newPts->Add(xs[k],ys[k]);
    }
    delete []xs;
    delete []ys;
    return newPts;
}

AutoPtr<Points>Points::ReturnPoints(double fromlen,double interval){
    int Size=Zs.size();
    if(Size==0) return nullptr;
    double totallen=GetLength();
    if(totallen<fromlen) return nullptr;
    int Num=(totallen-fromlen)/interval;
    if(Num*interval<totallen-fromlen) Num++;
    if(Num==0) return nullptr;
    //double*dlens=(double*)lengths->pvData;
    Points*newPts=new Points();
    DOUBLE*xs=new DOUBLE[Num];
    DOUBLE*ys=new DOUBLE[Num];
    if(Size==1)
    {
        for(int k=0;k<Num;k++)
        {
            xs[k]=XYs[0];
            ys[k]=XYs[1];
        }
    }
    else
    {
        float len=0;
        float curlen;
        float CurIndexLen=fromlen;
        int CurIndex=0;
        bool NeedContinue=true;
        DOUBLE X1,Y1,X2,Y2;
        X1=XYs[0];
        Y1=XYs[1];
        long pos=2;
        for(int k=1;k<Size;k++)
        {
             if(CurIndexLen>totallen) break;
            X2=XYs[pos++];
            Y2=XYs[pos++];
            curlen=sqrt(pow(X1-X2,2.0)+pow(Y1-Y2,2.0));
            if(curlen==0) continue;
            len+=curlen;
            while(len>=CurIndexLen)
            {
                float ratio=(curlen-len+CurIndexLen)/curlen;
                if(ratio<0) ratio=0;
                xs[CurIndex]=X1+(X2-X1)*ratio;
                ys[CurIndex]=Y1+(Y2-Y1)*ratio;
                if(CurIndex>=Num-1)
                {
                    CurIndex++;
                    NeedContinue=false;
                    break;
                }
                CurIndex++;
                CurIndexLen=fromlen+interval*CurIndex;
                if(CurIndexLen>len) break;
            }
            if(!NeedContinue) break;
            X1=X2;Y1=Y2;
        }
        if(CurIndex<=Num-1)
        {
            for(int k=CurIndex;k<Num;k++)
            {
                xs[k]=X2;
                ys[k]=Y2;
            }
        }
    }
    for(long k=0;k<Num;k++)
    {
        newPts->Add(xs[k],ys[k]);
    }
    delete []xs;
    delete []ys;
    return newPts;
}

double Points::GetLength3D(){
    int Size=Zs.size();
    if(Size==0) return 0;
    DOUBLE len=0;
    DOUBLE X1,Y1,X2,Y2,Z1,Z2;
    X1=XYs[0];
    Y1=XYs[1];
    Z1=Zs[0];
    LONG pos=2;
    for(int k=1;k<Size;k++)
    {
        X2=XYs[pos++];
        Y2=XYs[pos++];
        Z2=Zs[k];
        len+=sqrt(pow(X2-X1,2.0)+pow(Y2-Y1,2.0)+pow(Z2-Z1,2.0));
        X1=X2;Y1=Y2;Z1=Z2;
    }
    return len;
}

double Points::GetPerimeter(){
    int Size=Zs.size();
    if(Size==0) return 0;
    DOUBLE len=0;
    DOUBLE X1,Y1,X2,Y2;
    X1=XYs[0];
    Y1=XYs[1];
    LONG pos=2;
    for(int k=1;k<=Size;k++)
    {
        if(k<Size){
            X2=XYs[pos++];
            Y2=XYs[pos++];
        }else{
            X2=XYs[0];
            Y2=XYs[1];
        }
        len+=sqrt(pow(X2-X1,2.0)+pow(Y2-Y1,2.0));
        X1=X2;Y1=Y2;
    }
    return len;
}

double Points::GetArea(){
    int Size=Zs.size();
    if(Size==0) return 0;
    DOUBLE a=0;
    DOUBLE X1,Y1,X2,Y2;
    X1=XYs[0];
    Y1=XYs[1];
    LONG pos=2;
    for(int k=1;k<=Size;k++)
    {
        if(k<Size){
            X2=XYs[pos++];
            Y2=XYs[pos++];
        }else{
            X2=XYs[0];
            Y2=XYs[1];
        }
        a+=(X2-X1)*(Y2+Y1);
        X1=X2;Y1=Y2;
    }
    return a*0.5;
}

double Points::GetDiameter(int&maxIndex1, int&maxIndex2){
    maxIndex1 = -1;
    maxIndex2 = -1;
    LONG ptCount=Zs.size();
    if(ptCount==0) return 0;
    double x1,y1,x2,y2;
    double diam=0;
    bool findFirst=false;
    x1=x2=XYs[0];
    y1=y2=XYs[1];
    maxIndex1=maxIndex2=0;
    double tx=x2+1,ty=y2+1;
    while(true){
        int nIndex=0;
        for(int j=0;j<ptCount;j++){
            double x=XYs[nIndex];
            double y=XYs[nIndex+1];
            double dist=sqrt((x1-x)*(x1-x)+(y1-y)*(y1-y));
            if(dist>diam){
                tx=x;
                ty=y;
                maxIndex2=j;
                diam=dist;
            }
            nIndex++;
        }
        if((tx==x2)&&(ty==y2)){
            break;
        }
        x2=x1;
        y2=y1;
        x1=tx;
        y1=ty;
        maxIndex1=maxIndex2;
    }
    return diam;
    /*
    LONG ptCount=Zs.size();
    LONG nIndex = 0;
    double maxDist = 0;
    maxIndex1 = -1;
    maxIndex2 = -1;
    for (int k = 0; k < ptCount; k++) {
        double maxp = 0;
        int maxIndex = -1;
        double x1 = XYs[nIndex];
        double y1 = XYs[nIndex + 1];
        for (int j = k+1; j < ptCount; j++) {
            double x2 = XYs[(j << 1)];
            double y2 = XYs[(j << 1) + 1];
            double dist = sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
            if (dist > maxp) {
                maxp = dist;
                maxIndex = j;
            }
        }
        if (maxp > maxDist) {
            maxDist = maxp;
            maxIndex1 = k;
            maxIndex2 = maxIndex;
        }
        nIndex += 2;
    }
    return maxDist;
    */
}

double Points::GetDiameter(){
    LONG ptCount=Zs.size();
    if(ptCount==0) return 0;
    double x1,y1,x2,y2;
    double diam=0;
    bool findFirst=false;
    x1=x2=XYs[0];
    y1=y2=XYs[1];
    double tx=x2+1,ty=y2+1;
    while(true){
        int nIndex=0;
        for(int j=0;j<ptCount;j++){
            double x=XYs[nIndex];
            double y=XYs[nIndex+1];
            double dist=sqrt((x1-x)*(x1-x)+(y1-y)*(y1-y));
            if(dist>diam){
                tx=x;
                ty=y;
                diam=dist;
            }
            nIndex+=2;
        }
        if((tx==x2)&&(ty==y2)){
            break;
        }
        x2=x1;
        y2=y1;
        x1=tx;
        y1=ty;
    }
    return diam;
    /*
    LONG ptCount=Zs.size();
    LONG nIndex = 0;
    double maxDist = 0;
    maxIndex1 = -1;
    maxIndex2 = -1;
    for (int k = 0; k < ptCount; k++) {
        double maxp = 0;
        int maxIndex = -1;
        double x1 = XYs[nIndex];
        double y1 = XYs[nIndex + 1];
        for (int j = k+1; j < ptCount; j++) {
            double x2 = XYs[(j << 1)];
            double y2 = XYs[(j << 1) + 1];
            double dist = sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
            if (dist > maxp) {
                maxp = dist;
                maxIndex = j;
            }
        }
        if (maxp > maxDist) {
            maxDist = maxp;
            maxIndex1 = k;
            maxIndex2 = maxIndex;
        }
        nIndex += 2;
    }
    return maxDist;
    */
}

bool Points::SmoothRing(double tolerance){
    int pointSize=Zs.size();
    if(pointSize<3) return false;
    DOUBLE*xys=XYs.data();
    if((fabs(xys[2*pointSize-1]-xys[1])<=0.000001)&&(fabs(xys[2*pointSize-2]-xys[0])<=0.000001)) pointSize--;
    if(pointSize<3) return false;
    Spline sp(xys,pointSize);
    sp.GenerateClosed();
    int toSize=sp.GetClosedCurveCount(tolerance);
    Point2D<double>*dpt=new Point2D<double>[toSize];
    int Size=0;
    sp.GetClosedCurve(dpt,tolerance,Size);
    XYs.resize(toSize*2);
    memcpy(XYs.data(),dpt,toSize*16);
    delete []dpt;
    Zs.resize(toSize);
    IsExtentValid=false;
    return true;
}

bool Points::SmoothRingByPtNum(int insertPointsNum){
    int pointSize=Zs.size();
    if(pointSize<3) return false;
    DOUBLE*xys=XYs.data();
    if((fabs(xys[2*pointSize-1]-xys[1])<=0.000001)&&(fabs(xys[2*pointSize-2]-xys[0])<=0.000001)) pointSize--;
    if(pointSize<3) return false;
    DOUBLE lastX,lastY;
    lastX=xys[(pointSize-1)*2];
    lastY=xys[(pointSize-1)*2+1];
    Spline sp(xys,pointSize);
    sp.GenerateClosed();

    int toSize=pointSize*insertPointsNum+pointSize;
    Point2D<double>*dpt=new Point2D<double>[toSize];
    int Size=0;
    sp.GetClosedCurveByPointNum(dpt,insertPointsNum,Size);
    XYs.resize(toSize*2);
    memcpy(XYs.data(),dpt,toSize*16);
    delete []dpt;
    Zs.resize(toSize);
    IsExtentValid=false;
    return true;
}


void Points::SimplifyRing(double toler){
    LONG ptsCount=Zs.size();
    if(ptsCount<=3) return;
    int pin = 0;
    double maxtoler = 0;
    int node1, node2;
    double tolerd;
    double dist;
    maxtoler = tolerd = GetDiameter(node1, node2);
    int*stackfromNodes = new int[ptsCount];
    int*stacktoNodes = new int[ptsCount];
    double*tolerances = new double[ptsCount];
    for (int k = 0; k < ptsCount; k++) {
        tolerances[k] = 0;
        stackfromNodes[k] = 0;
        stacktoNodes[k] = 0;
    }
    if (node1 >= 0)
    {
        stackfromNodes[pin] = node1;
        stacktoNodes[pin] = node2;
        pin++;
        stackfromNodes[pin] = node2;
        stacktoNodes[pin] = node1;
        pin++;
        tolerances[node1] = tolerd;
        tolerances[node2] = tolerd;
    }
    double x = 0, y = 0;
    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    AutoPtr<Segment>seg(new Segment());
    AutoPtr<Point>ppt(new Point());
    int validCount=2;
    double mintoler=tolerd;
    for (int k = 0; k < pin; k++)
    {
        if (k >= pin) break;
        node1 = stackfromNodes[k];
        node2 = stacktoNodes[k];
        if ((node1 - node2 == 1) || (node2 - node1 == 1)) continue;
        x1=XYs[node1<<1];
        y1=XYs[(node1<<1)+1];
        x2=XYs[node2<<1];
        y2=XYs[(node2<<1)+1];
        seg->PutCoord(x1, y1, x2, y2);
        int pos = -1;
        double maxdist = 0;
        if (node1 < node2)
        {
            for (int j = node1 + 1; j < node2; j++)
            {
                x=XYs[j<<1];
                y=XYs[(j<<1)+1];
                ppt->PutCoord(x, y);
                dist=ppt->DistanceToSegment(seg.get());
                if (dist > maxdist)
                {
                    maxdist = dist;
                    pos = j;
                }
            }
            if (pos == -1) continue;
            if((maxdist>toler)&&(validCount>=3)) continue;
            tolerances[pos] = maxdist;
            validCount++;
            if (pos > node1 + 1)
            {
                stackfromNodes[pin] = node1;
                stacktoNodes[pin] = pos;
                pin++;
            }
            if (node2 > pos + 1)
            {
                stackfromNodes[pin] = pos;
                stacktoNodes[pin] = node2;
                pin++;
            }
        }
        else
        {
            for (int j = node1 + 1; j < ptsCount; j++)
            {
                x=XYs[j<<1];
                y=XYs[(j<<1)+1];
                ppt->PutCoord(x, y);
                dist=ppt->DistanceToSegment(seg.get());
                if (dist > maxdist)
                {
                    maxdist = dist;
                    pos = j;
                }
            }
            for (int j = 0; j < node2; j++)
            {
                x=XYs[j<<1];
                y=XYs[(j<<1)+1];
                ppt->PutCoord(x, y);
                dist=ppt->DistanceToSegment(seg.get());
                if (dist > maxdist)
                {
                    maxdist = dist;
                    pos = j;
                }
            }
            if (pos == -1) continue;
            if((maxdist>toler)&&(validCount>=3)) continue;
            tolerances[pos] = maxdist;
            validCount++;
            int lpos = node1 + 1;
            if (lpos == ptsCount) lpos = 0;
            if (lpos != pos)
            {
                stackfromNodes[pin] = node1;
                stacktoNodes[pin] = pos;
                pin++;
            }
            lpos = node2 - 1;
            if (lpos < 0) lpos += ptsCount;
            if (lpos != pos)
            {
                stackfromNodes[pin] = pos;
                stacktoNodes[pin] = node2;
                pin++;
            }
        }
        if(maxdist<mintoler) mintoler=maxdist;
    }
    int curPin=0;
    if(toler<mintoler) toler=mintoler;
    for (int k = 0; k < ptsCount; k++)
    {
         if (tolerances[k] >= toler)
         {
             XYs[(curPin<<1)]=XYs[(k<<1)];
             XYs[(curPin<<1)+1]=XYs[(k<<1)+1];
             Zs[curPin]=Zs[k];
             curPin++;
         }
    }
    delete []stackfromNodes;
    delete []stacktoNodes;
    delete []tolerances;
    XYs.resize(curPin<<1);
    Zs.resize(curPin);
}
void Points::SimplifyRingByPtNum(int ptNum){
    LONG ptsCount=Zs.size();
    if(ptsCount<=3) return;
    int pin = 0;
    double maxtoler = 0;
    int node1, node2;
    double tolerd;
    double dist;
    maxtoler = tolerd = GetDiameter(node1, node2);
    int*stackfromNodes = new int[ptsCount];
    int*stacktoNodes = new int[ptsCount];
    double*tolerances = new double[ptsCount];
    for (int k = 0; k < ptsCount; k++) {
        tolerances[k] = 0;
        stackfromNodes[k] = 0;
        stacktoNodes[k] = 0;
    }
    if (node1 >= 0)
    {
        stackfromNodes[pin] = node1;
        stacktoNodes[pin] = node2;
        pin++;
        stackfromNodes[pin] = node2;
        stacktoNodes[pin] = node1;
        pin++;
        tolerances[node1] = tolerd;
        tolerances[node2] = tolerd;
    }
    OrderArray<double>pArray(true,ptNum);
    pArray.Add(tolerd);
    pArray.Add(tolerd);
    double x = 0, y = 0;
    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    AutoPtr<Segment>seg(new Segment());
    AutoPtr<Point>ppt(new Point());
    for (int k = 0; k < pin; k++)
    {
        if (k >= pin) break;
        node1 = stackfromNodes[k];
        node2 = stacktoNodes[k];
        if ((node1 - node2 == 1) || (node2 - node1 == 1)) continue;
        x1=XYs[node1<<1];
        y1=XYs[(node1<<1)+1];
        x2=XYs[node2<<1];
        y2=XYs[(node2<<1)+1];
        seg->PutCoord(x1, y1, x2, y2);
        int pos = -1;
        double maxdist = 0;
        if (node1 < node2)
        {
            for (int j = node1 + 1; j < node2; j++)
            {
                x=XYs[j<<1];
                y=XYs[(j<<1)+1];
                ppt->PutCoord(x, y);
                dist=ppt->DistanceToSegment(seg.get());
                if (dist > maxdist)
                {
                    maxdist = dist;
                    pos = j;
                }
            }
            if (pos == -1) continue;
            if(pArray.GetSize()>=ptNum){
               if(maxdist<-pArray[pArray.GetSize()-1]) continue;
            }
            tolerances[pos] = maxdist;
            if (pos > node1 + 1)
            {
                stackfromNodes[pin] = node1;
                stacktoNodes[pin] = pos;
                pin++;
            }
            if (node2 > pos + 1)
            {
                stackfromNodes[pin] = pos;
                stacktoNodes[pin] = node2;
                pin++;
            }
        }
        else
        {
            for (int j = node1 + 1; j < ptsCount; j++)
            {
                x=XYs[j<<1];
                y=XYs[(j<<1)+1];
                ppt->PutCoord(x, y);
                dist=ppt->DistanceToSegment(seg.get());
                if (dist > maxdist)
                {
                    maxdist = dist;
                    pos = j;
                }
            }
            for (int j = 0; j < node2; j++)
            {
                x=XYs[j<<1];
                y=XYs[(j<<1)+1];
                ppt->PutCoord(x, y);
                dist=ppt->DistanceToSegment(seg.get());
                if (dist > maxdist)
                {
                    maxdist = dist;
                    pos = j;
                }
            }
            if (pos == -1) continue;
            if(pArray.GetSize()>=ptNum){
               if(maxdist<-pArray[pArray.GetSize()-1]) continue;
            }
            tolerances[pos] = maxdist;
            int lpos = node1 + 1;
            if (lpos == ptsCount) lpos = 0;
            if (lpos != pos)
            {
                stackfromNodes[pin] = node1;
                stacktoNodes[pin] = pos;
                pin++;
            }
            lpos = node2 - 1;
            if (lpos < 0) lpos += ptsCount;
            if (lpos != pos)
            {
                stackfromNodes[pin] = pos;
                stacktoNodes[pin] = node2;
                pin++;
            }
        }
    }
    double mintoler=-pArray[pArray.GetSize()-1];
    int hasNum=1;
    int curPin=pArray.GetSize()-2;
    if(curPin>=0){
        while(pArray[curPin]==-mintoler){
            hasNum++;
            curPin--;
            if(curPin<0) break;
        }
    }
    curPin=0;
    int addedNum=0;
    for (int k = 0; k < ptsCount; k++)
    {
         if (tolerances[k]>mintoler)
         {
             XYs[(curPin<<1)]=XYs[(k<<1)];
             XYs[(curPin<<1)+1]=XYs[(k<<1)+1];
             Zs[curPin]=Zs[k];
             curPin++;
         }
         else if(tolerances[k]==mintoler){
             if(addedNum<hasNum){
                 XYs[(curPin<<1)]=XYs[(k<<1)];
                 XYs[(curPin<<1)+1]=XYs[(k<<1)+1];
                 Zs[curPin]=Zs[k];
                 curPin++;
             }
             addedNum++;
         }
    }
    delete []stackfromNodes;
    delete []stacktoNodes;
    delete []tolerances;
    XYs.resize(curPin<<1);
    Zs.resize(curPin);
}

bool Points::CanCaptureRingBorder(Point*ppt,double tolerance){
    int ptCount=Zs.size();
    if(ptCount<2) return false;
    AutoPtr<Envelope>pEnv=GetExtent();
    DOUBLE xmin,ymin,xmax,ymax;
    pEnv->GetCoord(&xmin,&ymax,&xmax,&ymin);
    DOUBLE X,Y;
    ppt->GetCoord(&X,&Y);
    if((X<xmin-tolerance)||(X>xmax+tolerance)||(Y<ymin-tolerance)||(Y>ymax+tolerance)) return false;
    AutoPtr<Segment>seg(new Segment());
    seg->Lx1()=XYs[0];
    seg->Ly1()=XYs[1];
    int pos=2;
    for(int k=1;k<=ptCount;k++)
    {
        if(k<ptCount){
           seg->Lx2()=XYs[pos++];
           seg->Ly2()=XYs[pos++];
        }
        else{
            seg->Lx2()=XYs[0];
            seg->Ly2()=XYs[1];
        }
        if(seg->IsPointOnSegment(X,Y,tolerance)) return true;
        seg->Lx1()=seg->Lx2();
        seg->Ly1()=seg->Ly2();
    }
    return false;
}

AutoPtr<Point>Points::CaptureRingBorder(Point*ppt,double tolerance){
    int ptCount=Zs.size();
    if(ptCount<2) return nullptr;
    AutoPtr<Envelope>pEnv=GetExtent();
    DOUBLE xmin,ymin,xmax,ymax;
    pEnv->GetCoord(&xmin,&ymax,&xmax,&ymin);
    DOUBLE X,Y;
    ppt->GetCoord(&X,&Y);
    if((X<xmin-tolerance)||(X>xmax+tolerance)||(Y<ymin-tolerance)||(Y>ymax+tolerance)) return nullptr;
    Segment2D<double>seg;
    seg.Fuzzy=tolerance;
    seg.lnx1=XYs[0];
    seg.lny1=XYs[1];
    int pos=2;
    DOUBLE nearX,nearY;
    DOUBLE capX,capY;
    double dist;
    double minDist=-1;
    for(int k=1;k<=ptCount;k++)
    {
        if(k<ptCount)
        {
            seg.lnx2=XYs[pos++];
            seg.lny2=XYs[pos++];
        }
        else
        {
            seg.lnx2=XYs[0];
            seg.lny2=XYs[1];
        }
        if((fabs(seg.lnx1-seg.lnx2)<=VertexTolerance)&&(fabs(seg.lny1-seg.lny2)<=VertexTolerance)) continue;
        dist=seg.PtMinDistToSegment(X,Y,nearX,nearY);
        if(dist>tolerance)
        {
            seg.lnx1=seg.lnx2;
            seg.lny1=seg.lny2;
            continue;
        }
        if(minDist==-1)
        {
            capX=nearX;
            capY=nearY;
            minDist=dist;
        }
        else if(dist<minDist)
        {
            capX=nearX;
            capY=nearY;
            minDist=dist;
        }
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lny2;
    }
    if(minDist==-1) return nullptr;
    AutoPtr<Point>tpt(new Point());
    tpt->PutCoord(capX,capY);
    return tpt;
}
bool Points::IsPointIn(Point*pt){
    DOUBLE x,y;
    pt->GetCoord(&x,&y);
    int Size=Zs.size();
    if(Size<3) return false;
    double af,at,a=0;
    DOUBLE X1,Y1,X2,Y2;
    X1=XYs[0];
    Y1=XYs[1];
    if(x==X1)
    {
        if(Y1>y)
            af=PI/2;
        else if(y==Y1)
            af=0;
        else
            af=PI*3/2;
    }
    else
    {
        af=atan((Y1-y)/(X1-x));
        if(X1<x) af=af+PI;
    }
    double dt;
    long pos=2;
    for(int k=1;k<=Size;k++)
    {
        if(k<Size)
        {
            X2=XYs[pos++];
            Y2=XYs[pos++];
        }
        else
        {
            X2=XYs[0];
            Y2=XYs[1];
        }
        if(x==X2)
        {
            if(Y2>y)
                at=PI/2;
            else if(y==Y2)
                at=0;
            else
                at=PI*3/2;
        }
        else
        {
            at=atan((Y2-y)/(X2-x));
            if(X2<x) at=at+PI;
        }
        dt=at-af;
        if(dt<0) dt=DPI+dt;
        if(dt>PI) dt=-(DPI-dt);
        a=a+dt;
        af=at;
        X1=X2;
        Y1=Y2;
    }
    double fa=fabs(a);
    if(fa>=VertexTolerance)
    {
        double de=fmod(fa,DPI);
        if(de>DPI-de) de=DPI-de;
        return (de<VertexTolerance);
    }
    return CanCaptureRingBorder(pt,VertexTolerance);
}

bool Points::IsRingIn(Ring*other){
    Points*innerPoints=(Points*)other;
    LONG innerCount=innerPoints->GetSize();
    if(innerCount<3) return false;
    AutoPtr<Envelope>innerEnv=innerPoints->GetExtent();
    AutoPtr<Envelope>outerEnv=GetExtent();
    DOUBLE xmin,ymin,xmax,ymax;
    innerEnv->GetCoord(&xmin,&ymax,&xmax,&ymin);
    DOUBLE xmin2,ymin2,xmax2,ymax2;
    outerEnv->GetCoord(&xmin2,&ymax2,&xmax2,&ymin2);
    if((xmin<xmin2)||(xmax>xmax2)||(ymin<ymin2)||(ymax>ymax2)) return false;
    AutoPtr<Point>ppt(new Point());
    Segment2D<double>seg;
    innerPoints->GetItem(0,seg.lnx1,seg.lnx2);
    ppt->PutCoord(seg.lnx1,seg.lnx2);
    bool IsIn=IsPointIn(ppt.get());
    if(!IsIn) return false;

    AutoPtr<Polygon>poly(new Polygon());
    AutoPtr<Ring>ring(this,true);
    poly->Add(ring);
    SegmentIndexes segIndexes(poly,8);
    segIndexes.BuildSpatialIndex();
    int state=0;
    DOUBLE area=GetArea();
    int key=4;
    if(area<0) key=1;
    for(int k=1;k<=innerCount;k++)
    {
        if(k<innerCount)
            innerPoints->GetItem(k,seg.lnx2,seg.lny2);
        else
            innerPoints->GetItem(0,seg.lnx2,seg.lny2);
        int s;
        if(area>=0)
            s=segIndexes.GetRingSegmentIntersectOuterState(seg,k);
        else
            s=segIndexes.GetRingSegmentIntersectInnerState(seg,k);
        state=(s|state);
        if(state&key) return false;
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lny2;
    }
    return true;
}
AutoPtr<Point>Points::GetGeometryCenter(){
    double cx=0,cy=0;
    int size=Zs.size();
    if(size==0) return nullptr;
    int nIndex=0;
    for(int k=0;k<size;k++){
        cx+=XYs[nIndex++];
        cy+=XYs[nIndex++];
    }
    cx/=size;
    cy/=size;
    return AutoPtr<Point>(new Point(cx,cy));
}
AutoPtr<Point>Points::GetGavityCenter(){
    int ptsCount=Zs.size();
    DOUBLE X1, Y1, X2, Y2;
    X1 = XYs[0];
    Y1 = XYs[1];
    if(ptsCount==2){
        X2 = XYs[2];
        Y2 = XYs[3];
        return new Point((X1+X2)/2,(Y1+Y2)/2);
    }
    else if(ptsCount==3){
        X2 = XYs[4];
        Y2 = XYs[5];
        if((X1==X2)&&(Y1==Y2)){
            X2 = XYs[2];
            Y2 = XYs[3];
            return new Point((X1+X2)/2,(Y1+Y2)/2);
        }
    }
    int nIndex = 2;
    double sumxAi = 0;
    double sumAi = 0;
    double sumyAi = 0;
    for (int k = 1; k <= ptsCount; k++) {
        if (k == ptsCount) {
            X2 = XYs[0];
            Y2 = XYs[1];
        }
        else {
            X2 = XYs[nIndex++];
            Y2 = XYs[nIndex++];
        }
        double Ai = (Y2 + Y1)*(X1 - X2) ;
        double xAi = (X2*X2 + X2 * X1 + X1 * X1)*(Y2 - Y1);
        double yAi = (Y2*Y2 + Y2 * Y1 + Y1 * Y1)*(X1 - X2);
        sumxAi += xAi;
        sumyAi += yAi;
        sumAi += Ai;
        X1 = X2;
        Y1 = Y2;
    }
    double cx = sumxAi / sumAi/3;
    double cy = sumyAi / sumAi/3;
    return AutoPtr<Point>(new Point(cx,cy));
}

AutoPtr<Point>Points::ReturnRingPoint(double length){
    int Size=Zs.size();
    if(Size<1) return nullptr;
    if(Size<2)
    {
        if(length==0) return new Point(XYs[0],XYs[1]);
        return nullptr;
    }
    double len=0;
    double curlen;
    DOUBLE X1,Y1,X2,Y2;
    X1=XYs[0];
    Y1=XYs[1];
    long pos=2;
    for(int k=1;k<=Size;k++)
    {
        if(k<Size){
            X2=XYs[pos++];
            Y2=XYs[pos++];
        }
        else{
            X2=XYs[0];
            Y2=XYs[1];
        }
        curlen=sqrt(pow(X2-X1,2.0)+pow(Y2-Y1,2.0));
        len+=curlen;
        if((len>=length)||(k==Size-1))
        {
            float ratio=(curlen-len+length)/curlen;
            return new Point(X1+(X2-X1)*ratio,Y1+(Y2-Y1)*ratio);
        }
        X1=X2;Y1=Y2;
    }
    return nullptr;
}
AutoPtr<Points>Points::ReturnRingPoints(const vector<double>&lengths){
    int Size=Zs.size();
    if(Size==0) return nullptr;
    int Num=lengths.size();
    if(Num==0) return nullptr;
    //double*dlens=(double*)lengths->pvData;
    Points*newPts=new Points();
    DOUBLE*xs=new DOUBLE[Num];
    DOUBLE*ys=new DOUBLE[Num];
    if(Size==1)
    {
        for(int k=0;k<Num;k++)
        {
            xs[k]=XYs[0];
            ys[k]=XYs[1];
        }
    }
    else
    {
        float len=0;
        float curlen;
        float CurIndexLen=lengths[0];
        int CurIndex=0;
        bool NeedContinue=true;
        DOUBLE X1,Y1,X2,Y2;
        X1=XYs[0];
        Y1=XYs[1];
        long pos=2;
        for(int k=1;k<=Size;k++)
        {
            if(k<Size){
                X2=XYs[pos++];
                Y2=XYs[pos++];
            }
            else{
                X2=XYs[0];
                Y2=XYs[1];
            }
            curlen=sqrt(pow(X1-X2,2.0)+pow(Y1-Y2,2.0));
            if(curlen==0) continue;
            len+=curlen;
            while(len>=CurIndexLen)
            {
                float ratio=(curlen-len+CurIndexLen)/curlen;
                if(ratio<0) ratio=0;
                xs[CurIndex]=X1+(X2-X1)*ratio;
                ys[CurIndex]=Y1+(Y2-Y1)*ratio;
                if(CurIndex>=Num-1)
                {
                    NeedContinue=false;
                    break;
                }
                CurIndex++;
                CurIndexLen=lengths[CurIndex];
            }
            if(!NeedContinue) break;
            X1=X2;Y1=Y2;
        }
        if(CurIndex<=Num-1)
        {
            for(int k=CurIndex;k<Num;k++)
            {
                xs[k]=X2;
                ys[k]=Y2;
            }
        }
    }
    for(long k=0;k<Num;k++)
    {
        newPts->Add(xs[k],ys[k]);
    }
    delete []xs;
    delete []ys;
    return newPts;
}

AutoPtr<Points>Points::ReturnRingPoints(double fromlen,double interval){
    int Size=Zs.size();
    if(Size==0) return nullptr;
    double totallen=GetLength();
    if(totallen<fromlen) return nullptr;
    int Num=(totallen-fromlen)/interval;
    if(Num*interval<totallen-fromlen) Num++;
    if(Num==0) return nullptr;
    //double*dlens=(double*)lengths->pvData;
    Points*newPts=new Points();
    DOUBLE*xs=new DOUBLE[Num];
    DOUBLE*ys=new DOUBLE[Num];
    if(Size==1)
    {
        for(int k=0;k<Num;k++)
        {
            xs[k]=XYs[0];
            ys[k]=XYs[1];
        }
    }
    else
    {
        float len=0;
        float curlen;
        float CurIndexLen=fromlen;
        int CurIndex=0;
        bool NeedContinue=true;
        DOUBLE X1,Y1,X2,Y2;
        X1=XYs[0];
        Y1=XYs[1];
        long pos=2;
        for(int k=1;k<=Size;k++)
        {
             if(CurIndexLen>totallen) break;
            if(k<Size){
                X2=XYs[pos++];
                Y2=XYs[pos++];
            }
            else{
                X2=XYs[0];
                Y2=XYs[1];
            }
            curlen=sqrt(pow(X1-X2,2.0)+pow(Y1-Y2,2.0));
            if(curlen==0) continue;
            len+=curlen;
            while(len>=CurIndexLen)
            {
                float ratio=(curlen-len+CurIndexLen)/curlen;
                if(ratio<0) ratio=0;
                xs[CurIndex]=X1+(X2-X1)*ratio;
                ys[CurIndex]=Y1+(Y2-Y1)*ratio;
                if(CurIndex>=Num-1)
                {
                    NeedContinue=false;
                    break;
                }
                CurIndex++;
                CurIndexLen=fromlen+interval*CurIndex;
                if(CurIndexLen>len) break;
            }
            if(!NeedContinue) break;
            X1=X2;Y1=Y2;
        }
        if(CurIndex<=Num-1)
        {
            for(int k=CurIndex;k<Num;k++)
            {
                xs[k]=X2;
                ys[k]=Y2;
            }
        }
    }
    for(long k=0;k<Num;k++)
    {
        newPts->Add(xs[k],ys[k]);
    }
    delete []xs;
    delete []ys;
    return newPts;
}


double Points::GetPerimeter3D(){
    int Size=Zs.size();
    if(Size==0) return 0;
    DOUBLE len=0;
    DOUBLE X1,Y1,X2,Y2,Z1,Z2;
    X1=XYs[0];
    Y1=XYs[1];
    Z1=Zs[0];
    LONG pos=2;
    for(int k=1;k<Size;k++)
    {
        if(k<Size){
            X2=XYs[pos++];
            Y2=XYs[pos++];
            Z2=Zs[k];
        }else{
            X2=XYs[0];
            Y2=XYs[1];
            Z2=Zs[0];
        }
        len+=sqrt(pow(X2-X1,2.0)+pow(Y2-Y1,2.0)+pow(Z2-Z1,2.0));
        X1=X2;Y1=Y2;Z1=Z2;
    }
    return len;
}

}
