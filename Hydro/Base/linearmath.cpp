#include "linearmath.h"

namespace SGIS{

REGISTER(Matrix4x4)

Matrix4x4::Matrix4x4()
{
    rawData=new double[16];
    memset(rawData,0,16*sizeof(double));
}

Matrix4x4::Matrix4x4(double*rawData){
    this->rawData=new double[16];
    memcpy(this->rawData,rawData,16*sizeof(double));
}

Matrix4x4::Matrix4x4(const vector<double>&data){
    rawData=new double[16];
    int size=data.size();
    if(size>16) size=16;
    memcpy(this->rawData,data.data(),size*sizeof(double));
}

Matrix4x4::~Matrix4x4()
{
    delete []rawData;
}

double*Matrix4x4::GetRawData(){
    return rawData;
}
double&Matrix4x4::operator [](int nIndex){
    return rawData[nIndex];
}

double&Matrix4x4::Value(int row,int col){
     return rawData[col*4+row];
}

void Matrix4x4::SetRow(int rowIndex,const vector<double>&values){
    int size=values.size();
    if(size>4) size=4;
    for(int k=0;k<size;k++){
        rawData[rowIndex+k*4]=values[k];
    }
}

void Matrix4x4::GetRow(int rowIndex,vector<double>&values){
    int size=values.size();
    if(size>4) size=4;
    for(int k=0;k<size;k++){
        values[k]=rawData[rowIndex+k*4];
    }
}

void Matrix4x4::SetCol(int colIndex,const vector<double>&values){
    int size=values.size();
    if(size>4) size=4;
    for(int k=0;k<size;k++){
        rawData[k+colIndex*4]=values[k];
    }
}

void Matrix4x4::GetCol(int colIndex,vector<double>&values){
    int size=values.size();
    if(size>4) size=4;
    for(int k=0;k<size;k++){
        values[k]=rawData[k+colIndex*4];
    }
}

void Matrix4x4::Identity(){
    for(int k=0;k<16;k++)
    {
        rawData[k]=((k%5==0)?1:0);
    }
}

void Matrix4x4::CopyFromOther(Matrix4x4*other){
    memcpy(rawData,other->rawData,sizeof(double)*16);
}

AutoPtr<Matrix4x4>Matrix4x4::Inverse(){
    vector<DOUBLE*>A;
    vector<DOUBLE*>ret;
    A.resize(4);
    ret.resize(4);
    for(int i=0;i<4;i++)
    {
        DOUBLE*as=new DOUBLE[4];
        as[0]=rawData[i];
        as[1]=rawData[i+4];
        as[2]=rawData[i+8];
        as[3]=rawData[i+12];
        A[i]=as;
        as=new DOUBLE[4];
        as[0]=0;as[1]=0;as[2]=0;as[3]=0;
        as[i]=1;
        ret[i]=as;
    }
    double maxV,V;
    for ( int j = 0; j < 4; ++j )  //每一列
    {
        int p = j;
        maxV = ((A[j][j]>=0)?(A[j][j]):(-A[j][j]));
        for ( int i = j+1; i < 4; ++i )  // 找到第j列中元素绝对值最大行
        {
            V=((A[i][j]>=0)?(A[i][j]):(-A[i][j]));
            if( maxV < V)
            {
                p = i;
                maxV = V;
            }
        }
        if ( maxV < 1e-20 )
        {
            for(int p=0;p<4;p++)
            {
                delete []A[p];
                delete []ret[p];
            }
            return nullptr;
        }
        if ( j!= p )
        {
            DOUBLE*tempV=A[j];
            A[j]=A[p];
            A[p]=tempV;
            tempV=ret[j];
            ret[j]=ret[p];
            ret[p]=tempV;
        }
        double d = A[j][j];
        for ( int i = j; i < 4; ++i ) A[j][i] /= d;
        for ( int i = 0; i < 4; ++i ) ret[j][i] /= d;
        for ( int i = 0; i < 4; ++i )
        {
            if ( i != j )
            {
                double q = A[i][j];
                for ( int k = j; k < 4; ++k )
                {
                    A [i][k] -= q * A[j][k];
                }
                for ( int k = 0; k < 4; ++k )
                {
                    ret[i][k] -= q * ret[j][k];
                }
            }
        }
    }
    AutoPtr<Matrix4x4>pNew(new Matrix4x4());
    DOUBLE*dpin=pNew->rawData;
    for(int p=0;p<4;p++)
    {
        dpin[p]=ret[p][0];
        dpin[p+4]=ret[p][1];
        dpin[p+8]=ret[p][2];
        dpin[p+12]=ret[p][3];
        delete []A[p];
        delete []ret[p];
    }
    return pNew;
}

void Matrix4x4::Transpose(){
    DOUBLE temp;
    for(int i=0;i<4;i++)
    {
        for(int j=i+1;j<4;j++)
        {
            temp=rawData[i*4+j];
            rawData[i*4+j]=rawData[j*4+i];
            rawData[j*4+i]=temp;
        }
    }
}

AutoPtr<Matrix4x4>Matrix4x4::Multiply(Matrix4x4*other){
    AutoPtr<Matrix4x4>pNew(new Matrix4x4());
    double*matrixdata=other->rawData;
    double*newdata=pNew->rawData;
    for(int i=0;i<4;i++)
    {
        for(int j=0;j<4;j++)
        {
            newdata[i+j*4]=rawData[i]*matrixdata[j*4]+rawData[i+4]*matrixdata[1+j*4]+rawData[i+8]*matrixdata[2+j*4]+rawData[i+12]*matrixdata[3+j*4];
        }
    }
    return pNew;
}

AutoPtr<Matrix4x4>Matrix4x4::Clone(){
    AutoPtr<Matrix4x4>pNew(new Matrix4x4());
    memcpy(pNew->rawData,rawData,16*sizeof(double));
    return pNew;
}

bool Matrix4x4::Read(BufferReader*br){
    for(int k=0;k<16;k++){
        rawData[k]=br->Read<double>();
    }
    return true;
}
void Matrix4x4::Write(BufferWriter*bw){
    for(int k=0;k<16;k++){
        bw->Write<double>(rawData[k]);
    }
}

string Matrix4x4::GetTypeName(){
    return "Matrix4x4";
}

bool Matrix4x4::ReadFromJson(JsonReader*jr,JsonNode node){
   return false;
}
string Matrix4x4::ToJson(){
    int nPos=0;
    string sJson="[";
    for(int k=0;k<4;k++){
        string cJson="[";
        for(int j=0;j<4;j++){
            if(j==0)
                cJson=to_string(rawData[nPos]);
            else
                cJson+=","+to_string(rawData[nPos]);
            nPos++;
        }
        cJson+="]";
        if(k==0)
             sJson=cJson;
        else
            cJson+=","+cJson;
    }
    sJson+="]";
    return sJson;
}

}
