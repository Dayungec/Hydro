#include "rasterbandtool.h"
namespace SGIS{

RasterBandTool::RasterBandTool(RasterBand*pBand)
{
    this->pBand=pBand;
    RasterDesp desp=pBand->GetRasterDesp();
    rows=desp.rows;
    cols=desp.cols;
    XCellSize=desp.xCellSize;
    YCellSize=desp.yCellSize;
    Left=desp.left;
    Top=desp.top;
    NoData=pBand->GetNodata();
    psp=pBand->GetSpatialReference();
    biTrans=new BiCoordinateTransformation();
    biTrans->SetFromSpatialReference(psp);
}

RasterBandTool::~RasterBandTool(){

}

void RasterBandTool::Attach(RasterBand*pBand){
    this->pBand=pBand;
    RasterDesp desp=pBand->GetRasterDesp();
    rows=desp.rows;
    cols=desp.cols;
    XCellSize=desp.xCellSize;
    YCellSize=desp.yCellSize;
    Left=desp.left;
    Top=desp.top;
    NoData=pBand->GetNodata();
    psp=pBand->GetSpatialReference();
    biTrans=new BiCoordinateTransformation();
    biTrans->SetFromSpatialReference(psp);
}


Rect2D<double> RasterBandTool::MapToPixelCoord(Rect2D<double>&MapExtent)
{
    return Rect2D<double>((MapExtent.Left-Left)/XCellSize,(Top-MapExtent.Top)/YCellSize,(MapExtent.Right-Left)/XCellSize,(Top-MapExtent.Bottom)/YCellSize);
}

Rect2D<double> RasterBandTool::PixelToMapCoord(Rect2D<double>&PixelExtent)
{
    if(PixelExtent.Top>PixelExtent.Bottom)
    {
        double temp=PixelExtent.Top;
        PixelExtent.Top=PixelExtent.Bottom;
        PixelExtent.Bottom=temp;
    }
    return Rect2D<double>(PixelExtent.Left*XCellSize+Left,Top-PixelExtent.Top*YCellSize,(PixelExtent.Right+1)*XCellSize+Left,Top-(PixelExtent.Bottom+1)*YCellSize);
}

bool RasterBandTool::innerGetInterpolatedBlock(LONG x1,LONG y1,LONG x2,LONG y2,LONG BuffX,LONG BuffY,float* pData){
    int Width=BuffX;
    int Height=BuffY;
    Rect2D<int> ImageRect;
    float nodata=NoData;
    ImageRect=Rect2D<int>(x1,y1,x2,y2);
    ImageRect.NormalizeClientRect();
    x1=ImageRect.Left;x2=ImageRect.Right;
    y1=ImageRect.Top;y2=ImageRect.Bottom;
    if((ImageRect.Width()+1>=Width)||(ImageRect.Height()+1>=Height))
    {
        pBand->GetBlockData(ImageRect.Left, ImageRect.Top, ImageRect.Width()+1, ImageRect.Height()+1,BuffX, BuffY,pData);
    }
    else
    {
        int Difx=1;
        int Dify=1;
        ImageRect.Left-=1;
        ImageRect.Top-=1;
        ImageRect.Right+=1;
        ImageRect.Bottom+=1;
        if(ImageRect.Left<0)
        {
            Difx=0;
            ImageRect.Left++;
        }
        if(ImageRect.Right>=cols) ImageRect.Right--;
        if(ImageRect.Top<0)
        {
            Dify=0;
            ImageRect.Top++;
        }
        if(ImageRect.Bottom>=rows) ImageRect.Bottom--;
        float*data=new float[(ImageRect.Width()+1)*(ImageRect.Height()+1)];
        pBand->GetBlockData(ImageRect.Left, ImageRect.Top, ImageRect.Width()+1, ImageRect.Height()+1,ImageRect.Width()+1, ImageRect.Height()+1,data);
        long Pos=0;
        double posx,posy;
        int iposx,iposy,iposy1;
        int State;//0--None;1--Left;2--Right;
        long Posi;
        int W=ImageRect.Width();
        int H=ImageRect.Height();
        float ix,ix2;
        double ratiox=((double)(x2-x1+1))/Width;
        double ratioy=((double)(y2-y1+1))/Height;
        for(int i=0;i<Height;i++)
        {
            posy=ratioy*(0.5+i)+Dify;
            iposy=posy;
            if(posy-iposy<0.5)
            {
                iposy1=iposy-1;
                if(iposy1<0) iposy1=iposy;
            }
            else
            {
                iposy1=iposy+1;
                if(iposy1>H) iposy1=iposy;
            }
            for(int j=0;j<Width;j++)
            {
                posx=ratiox*(0.5+j)+Difx;
                iposx=posx;
                Posi=iposy*(W+1)+iposx;
                float v0=data[Posi];
                if(v0==nodata)
                    ix=nodata;
                else
                {
                    if(posx-iposx<0.5)
                    {
                        if(iposx>0)
                        {
                            if(data[Posi-1]!=nodata)
                                State=1;
                            else
                                State=2;
                        }
                        else
                            State=2;
                        if(State==2)
                        {
                            if(iposx+1>W)
                                State=0;
                            else if(data[Posi+1]==nodata)
                                State=0;
                        }
                    }
                    else
                    {
                        if(iposx+1<=W)
                        {
                            float v=data[Posi+1];
                            if(v!=nodata)
                                State=2;
                            else
                                State=1;
                        }
                        else
                            State=1;
                        if(State==1)
                        {
                            if(iposx<1)
                                State=0;
                            else if(data[Posi-1]==nodata)
                                State=0;
                        }
                    }
                    switch(State)
                    {
                    case 0:
                        ix=v0;break;
                    case 1:
                        ix=(posx-iposx+0.5)*(v0-data[Posi-1])+data[Posi-1];break;
                    case 2:
                        ix=(posx-iposx-0.5)*(data[Posi+1]-v0)+v0;break;
                    }
                }
                if(iposy1==iposy)
                {
                    pData[Pos]=ix;
                    Pos++;
                    continue;
                }
                Posi=iposy1*(W+1)+iposx;
                v0=data[Posi];
                if(v0==nodata)
                    ix2=nodata;
                else
                {
                    if(posx-iposx<0.5)
                    {
                        if(iposx>0)
                        {
                            if(data[Posi-1]!=nodata)
                                State=1;
                            else
                                State=2;
                        }
                        else
                            State=2;
                        if(State==2)
                        {
                            if(iposx+1>W)
                                State=0;
                            else if(data[Posi+1]==nodata)
                                State=0;
                        }
                    }
                    else
                    {
                        if(iposx+1<=W)
                        {
                            if(data[Posi+1]!=nodata)
                                State=2;
                            else
                                State=1;
                        }
                        else
                            State=1;
                        if(State==1)
                        {
                            if(iposx<1)
                                State=0;
                            else if(data[Posi-1]==nodata)
                                State=0;
                        }
                    }
                    switch(State)
                    {
                    case 0:
                    {
                        ix2=v0;break;
                    }
                    case 1:
                    {
                        ix2=(posx-iposx+0.5)*(v0-data[Posi-1])+data[Posi-1];break;
                    }
                    case 2:
                    {
                        ix2=(posx-iposx-0.5)*(data[Posi+1]-v0)+v0;break;
                    }
                    }
                }
                State=0;
                if(ix==nodata)
                    State=1;
                if(ix2==nodata)
                    State+=2;
                switch(State)
                {
                case 0:
                    pData[Pos]=(ix2-ix)*(posy-iposy-0.5)/(iposy1-iposy)+ix;break;
                case 1:
                    pData[Pos]=ix2;break;
                case 2:
                    pData[Pos]=ix;break;
                case 3:
                    pData[Pos]=nodata;break;
                }
                Pos++;
            }
        }
        delete []data;
    }
    return true;
}

bool RasterBandTool::innerGetBlockDataByCoord(Point* lt,DOUBLE xCellSize,DOUBLE yCellSize,LONG Width,LONG Height,FLOAT noData,FLOAT borderNodata,SpatialReference*pSpatial,float* pData){
    if(pSpatial!=nullptr)
    {
        biTrans->SetToSpatialReference(AutoPtr<SpatialReference>(pSpatial,true));
        AutoPtr<CoordinateTransformation>pTrans=biTrans->GetCoordinateTransformationRev();
        bool CanTransform=false;
        if(pTrans!=nullptr) CanTransform=pTrans->CanTransform();
        if(CanTransform)
        {
            AutoPtr<CoordinateTransformation>pTransRev=biTrans->GetCoordinateTransformation();
            return innerGetProjBlockDataByCoord(lt,xCellSize,yCellSize,Width,Height,pTrans.get(),pTransRev.get(),noData,borderNodata,pData);
        }
    }
    Point2D<double> LeftTop;
    lt->GetCoord(&LeftTop.X,&LeftTop.Y);
    Rect2D<double> FullExtent=Rect2D<double>(Left,Top,Left+cols*XCellSize,Top-rows*YCellSize);
    Rect2D<double> CurrentExtent(LeftTop.X,LeftTop.Y,LeftTop.X+xCellSize*Width,LeftTop.Y-Height*yCellSize);
    //这是需要获取数据的坐标范围
    if(!FullExtent.IntersectRect(CurrentExtent))
    {
        for(int k=Width*Height-1;k>=0;k--) pData[k]=borderNodata;
        return true;
    }
    //如果CurrentExtent在地图范围外，直接返回
    CurrentExtent=FullExtent.Intersect(CurrentExtent);
    //先获取CurrentExtent和FullExtent的相交区域
    Rect2D<int> ImageRect;
    ImageRect.Left=(CurrentExtent.Left-Left)/XCellSize;
    ImageRect.Top= (Top-CurrentExtent.Top)/YCellSize;
    ImageRect.Right=(CurrentExtent.Right-Left)/XCellSize;
    ImageRect.Bottom= (Top-CurrentExtent.Bottom)/YCellSize;
    ImageRect.InflateRect(1,1,1,1);
    if(ImageRect.Left<0) ImageRect.Left=0;
    if(ImageRect.Top<0) ImageRect.Top=0;
    if(ImageRect.Right>=cols) ImageRect.Right=cols-1;
    if(ImageRect.Bottom>=rows) ImageRect.Bottom=rows-1;
    Rect2D<double> ImageMap;
    ImageMap.Left=Left+XCellSize*ImageRect.Left;
    ImageMap.Top=Top-YCellSize*ImageRect.Top;
    ImageMap.Right=Left+XCellSize*ImageRect.Right+XCellSize;
    ImageMap.Bottom=Top-YCellSize*ImageRect.Bottom-YCellSize;

    float nodata=NoData;
    double semiXCellSize=xCellSize/2;
    double semiYCellSize=yCellSize/2;
    if((xCellSize>=XCellSize)||(yCellSize>=YCellSize)){
        Rect2D<int> pixelRect;
        pixelRect.Left=(ImageMap.Left-Left+xCellSize/2.0)/xCellSize;
        pixelRect.Top= (Top-ImageMap.Top+yCellSize/2.0)/yCellSize;
        pixelRect.Right=(ImageMap.Right-Left-xCellSize/2.0)/xCellSize;
        pixelRect.Bottom= (Top-ImageMap.Bottom-yCellSize/2.0)/yCellSize;
        int rw=pixelRect.Right-pixelRect.Left+1;
        int rh=pixelRect.Bottom-pixelRect.Top+1;
        float*tempdata=new float[rw*rh];
        pBand->GetBlockData(ImageRect.Left, ImageRect.Top, ImageRect.Width()+1, ImageRect.Height()+1,rw,rh,tempdata);
        double Y=LeftTop.Y-semiYCellSize;
        double X;
        int pos=0;
        for(int i=0;i<Height;i++)
        {
            X=LeftTop.X+semiXCellSize;
            int posy=(ImageMap.Top-Y)/yCellSize;
            if((posy<0)||(posy>=rh))
            {
                Y-=yCellSize;
                for(int j=0;j<Width;j++)
                {
                    pData[pos++]=noData;
                }
                continue;
            }
            for(int j=0;j<Width;j++)
            {
                int posx=(X-ImageMap.Left)/xCellSize;
                if((posx<0)||(posx>=rw))
                {
                    pData[pos++]=noData;
                    X+=xCellSize;
                    continue;
                }
                if(tempdata[posx+posy*rw]!=nodata)
                    pData[pos++]=tempdata[posx+posy*rw];
                else
                    pData[pos++]=noData;
                X+=xCellSize;
            }
            Y-=yCellSize;
        }
        delete []tempdata;
        return true;
    }


    int pW,pH;
    pW=ImageRect.Width()+1;
    pH=ImageRect.Height()+1;
    double Y=LeftTop.Y-semiYCellSize;
    long Pos;
    double X;
    int posx,posy;
    LONG formercol=-1;
    float*data=new float[pW];
    for(int i=0;i<Height;i++)
    {
        X=LeftTop.X+semiXCellSize;
        Pos=i*Width;
        posy=(ImageMap.Top-Y)/YCellSize;
        if((posy<0)||(posy>=pH))
        {
            Y-=yCellSize;
            for(int k=0;k<Width;k++) pData[Pos++]=borderNodata;
            continue;
        }
        if(formercol!=posy)
        {
            pBand->GetBlockData(ImageRect.Left, ImageRect.Top+posy, pW, 1,pW,1,data);
            formercol=posy;
        }
        for(int j=0;j<Width;j++)
        {
            posx=(X-ImageMap.Left)/XCellSize;
            if((posx<0)||(posx>=pW))
            {
                X+=xCellSize;
                pData[Pos++]=borderNodata;
                continue;
            }
            if(data[posx]!=nodata)
                pData[Pos]=data[posx];
            else
                pData[Pos]=noData;
            Pos++;
            X+=xCellSize;
        }
        Y-=yCellSize;
    }
    delete []data;
    return true;
}

bool RasterBandTool::innerGetProjBlockDataByCoord(Point* lt,DOUBLE xCellSize,DOUBLE yCellSize,LONG Width,LONG Height,CoordinateTransformation*pTrans,CoordinateTransformation*pTransRev,FLOAT noData,FLOAT borderNoData,float* pData){
    Point2D<double> LeftTop;
    lt->GetCoord(&LeftTop.X,&LeftTop.Y);
    int sW,sH;
    int difx=2;
    int dify=2;
    if(Width<2) difx=1;
    if(Height<2) dify=1;
    if(Width%difx==0)
        sW=Width/difx;
    else
        sW=Width/difx+1;
    if(Height%dify==0)
        sH=Height/dify;
    else
        sH=Height/dify+1;
    Rect2D<double> MapExtent(LeftTop.X,LeftTop.Y,LeftTop.X+xCellSize*Width,LeftTop.Y-Height*yCellSize);
    Rect2D<double> FileExtent=pTrans->TransformEnvelope(MapExtent);//将地图范围转换为图层范围
    Rect2D<double> pixelRect=MapToPixelCoord(FileExtent);//由图层范围得到图层像素范围
    Rect2D<int> PixelRect;
    PixelRect.Left=pixelRect.Left;
    PixelRect.Top=pixelRect.Top;
    PixelRect.Right=pixelRect.Right;
    PixelRect.Bottom=pixelRect.Bottom;
    PixelRect.NormalizeClientRect();
    PixelRect.InflateRect(1,1,1,1);
    int ImageWidth=cols;
    int ImageHeight=rows;
    if(PixelRect.Left<0)
        PixelRect.Left=0;
    else if(PixelRect.Left>=ImageWidth)
        PixelRect.Left=ImageWidth;
    if(PixelRect.Right<0)
        PixelRect.Right=-1;
    else if(PixelRect.Right>=ImageWidth)
        PixelRect.Right=ImageWidth-1;
    if(PixelRect.Top<0)
        PixelRect.Top=0;
    else if(PixelRect.Top>=ImageHeight)
        PixelRect.Top=ImageHeight;
    if(PixelRect.Bottom<0)
        PixelRect.Bottom=-1;
    else if(PixelRect.Bottom>=ImageHeight)
        PixelRect.Bottom=ImageHeight-1;
    long Size=Width*Height;
    if(PixelRect.Left>PixelRect.Right)
    {
        for(long k=0;k<Size;k++) pData[k]=borderNoData;
        return false;
    }
    if(PixelRect.Top>PixelRect.Bottom)
    {
        for(long k=0;k<Size;k++) pData[k]=borderNoData;
        return false;
    }
    //图层像素范围进行纠正
    Rect2D<double> tempRect(PixelRect.Left,PixelRect.Top,PixelRect.Right,PixelRect.Bottom);
    FileExtent=PixelToMapCoord(tempRect);
    //由图层像素范围反算图层范围
    Rect2D<double> PaintExtent=pTransRev->TransformEnvelope(FileExtent);//将图层范围转换为要显示的地图范围
    int pW,pH;//表示文件中与PixelRect成比例的宽和高，依据W和H计算
    double r1=(float)Width/Height;
    double r2=(float)(PixelRect.Width()+1)/(PixelRect.Height()+1);
    if(r1>=r2)
    {
        if(Width<PixelRect.Width()+1)
        {
            pW=Width;
            double fpH=pW/r2;
            pH=fpH;
            if(fpH-pH>=0.5) pH++;
        }
        else
        {
            pW=PixelRect.Width()+1;
            pH=PixelRect.Height()+1;
        }
    }
    else
    {
        if(Height<PixelRect.Height()+1)
        {
            pH=Height;
            double fpW=pH*r2;
            pW=fpW;
            if(fpW-pW>=0.5) pW++;
        }
        else
        {
            pW=PixelRect.Width()+1;
            pH=PixelRect.Height()+1;
        }
    }
    float*data =new float[pW*pH];
    pBand->GetBlockData(PixelRect.Left,PixelRect.Top,PixelRect.Width()+1,PixelRect.Height()+1,pW,pH,data);
    DOUBLE*pX=new DOUBLE[sW*sH];
    DOUBLE*pY=new DOUBLE[sW*sH];
    double lY,lX;
    double dCellXSize=xCellSize*difx;
    double dCellYSize=yCellSize*dify;
    lY=LeftTop.Y-0.5*yCellSize;
    double FromlX=LeftTop.X+0.5*xCellSize;
    int Pos=0;
    for(int i=0;i<Height;i+=dify)
    {
        lX=FromlX;
        for(int j=0;j<Width;j+=difx)
        {
            pX[Pos]=lX;
            pY[Pos]=lY;
            lX+=dCellXSize;
            Pos++;
        }
        lY-=dCellYSize;
    }
    if(pTrans!=nullptr) pTrans->TransformArray(pX,pY,sW*sH);
    float semiXCellSize=xCellSize/2;
    float semiYCellSize=yCellSize/2;
    Pos=0;
    double CellXSize=FileExtent.Width()/pW;
    double CellYSize=FileExtent.Height()/pH;
    lY=LeftTop.Y-semiYCellSize;
    Point2D<int> ppt;
    float nodata=NoData;
    long pos;
    double nX,nY,nX1,nY1,nX2,nY2;
    int di,dj;
    for(int i=0;i<Height;i++)
    {
        di=i/dify;
        int iidif=i%dify;
        lX=LeftTop.X-semiYCellSize;
        for(int j=0;j<Width;j++)
        {
            lX+=xCellSize;
            if((lX<PaintExtent.Left)||(lX>PaintExtent.Right)||(lY<PaintExtent.Bottom)||(lY>PaintExtent.Top))
            {
                pData[Pos++]=borderNoData;
                continue;
            }
            dj=j/difx;
            pos=di*sW+dj;
            int ijdif=j%difx;
            if(iidif==0)
            {
                if(ijdif==0)
                {
                    nX=pX[pos];
                    nY=pY[pos];
                }
                else if(dj<sW-1)
                {
                    nX=pX[pos]+(pX[pos+1]-pX[pos])*ijdif/difx;
                    nY=pY[pos]+(pY[pos+1]-pY[pos])*ijdif/difx;
                }
                else if(dj>1)
                {
                    nX=pX[pos-1]+(pX[pos]-pX[pos-1])*(1+ijdif)/difx;
                    nY=pY[pos-1]+(pY[pos]-pY[pos-1])*(1+ijdif)/difx;
                }
                else
                {
                    nX=pX[pos];
                    nY=pY[pos];
                }
            }
            else if(di<sH-1)
            {
                if(ijdif==0)
                {
                    nX=pX[pos]+(pX[pos+sW]-pX[pos])*iidif/dify;
                    nY=pY[pos]+(pY[pos+sW]-pY[pos])*iidif/dify;
                }
                else if(dj<sW-1)
                {
                    nX1=pX[pos]+(pX[pos+sW]-pX[pos])*iidif/dify;
                    nX2=pX[pos+1]+(pX[pos+sW+1]-pX[pos+1])*iidif/dify;
                    nX=nX1+(nX2-nX1)*ijdif/difx;
                    nY1=pY[pos]+(pY[pos+sW]-pY[pos])*iidif/dify;
                    nY2=pY[pos+1]+(pY[pos+sW+1]-pY[pos+1])*iidif/dify;
                    nY=nY1+(nY2-nY1)*ijdif/difx;
                }
                else if(dj>1)
                {
                    nX1=pX[pos-1]+(pX[pos+sW-1]-pX[pos-1])*iidif/dify;
                    nX2=pX[pos]+(pX[pos+sW]-pX[pos])*iidif/dify;
                    nX=nX1+(nX2-nX1)*(1+ijdif)/difx;
                    nY1=pY[pos-1]+(pY[pos+sW-1]-pY[pos-1])*iidif/dify;
                    nY2=pY[pos]+(pY[pos+sW]-pY[pos])*iidif/dify;
                    nY=nY1+(nY2-nY1)*(1+ijdif)/difx;
                }
                else
                {
                    nX=pX[pos]+(pX[pos+sW]-pX[pos])*iidif/dify;
                    nY=pY[pos]+(pY[pos+sW]-pY[pos])*iidif/dify;
                }
            }
            else if(di>1)
            {
                if(ijdif==0)
                {
                    nX=pX[pos-sW]+(pX[pos]-pX[pos-sW])*(1+iidif)/dify;
                    nY=pY[pos-sW]+(pY[pos]-pY[pos-sW])*(1+iidif)/dify;
                }
                else if(dj<sW-1)
                {
                    nX1=pX[pos-sW]+(pX[pos]-pX[pos-sW])*(1+iidif)/dify;
                    nX2=pX[pos-sW+1]+(pX[pos+1]-pX[pos-sW+1])*(1+iidif)/dify;
                    nX=nX1+(nX2-nX1)*ijdif/difx;
                    nY1=pY[pos-sW]+(pY[pos]-pY[pos-sW])*(1+iidif)/dify;
                    nY2=pY[pos-sW+1]+(pY[pos+1]-pY[pos-sW+1])*(1+iidif)/dify;
                    nY=nY1+(nY2-nY1)*ijdif/difx;
                }
                else if(dj>1)
                {
                    nX1=pX[pos-sW-1]+(pX[pos-1]-pX[pos-sW-1])*(1+iidif)/dify;
                    nX2=pX[pos-sW]+(pX[pos]-pX[pos-sW])*(1+iidif)/dify;
                    nX=nX1+(nX2-nX1)*(1+ijdif)/difx;
                    nY1=pY[pos-sW-1]+(pY[pos-1]-pY[pos-sW-1])*(1+iidif)/dify;
                    nY2=pY[pos-sW]+(pY[pos]-pY[pos-sW])*(1+iidif)/dify;
                    nY=nY1+(nY2-nY1)*(1+ijdif)/difx;
                }
                else
                {
                    nX=pX[pos-sW]+(pX[pos]-pX[pos-sW])*(1+iidif)/dify;
                    nY=pY[pos-sW]+(pY[pos]-pY[pos-sW])*(1+iidif)/dify;
                }
            }
            ppt.X=(nX-FileExtent.Left)/CellXSize;
            ppt.Y=(nY-FileExtent.Top)/CellYSize;
            if((ppt.X<0)||(ppt.X>=pW)||(ppt.Y<0)||(ppt.Y>=pH))
            {
                pData[Pos++]=borderNoData;
                continue;
            }
            else
            {
                pos=ppt.Y*pW+ppt.X;
                if(data[pos]==nodata)
                    pData[Pos]=noData;
                else
                    pData[Pos]=data[pos];
            }
            Pos++;
        }
        lY-=yCellSize;
    }
    delete[]data;
    delete[]pX;
    delete[]pY;
    return true;
}

bool RasterBandTool::innerGetInterpolatedDataBlock(Point* lt,DOUBLE xCellSize,DOUBLE yCellSize,int Width,int Height,float noData,float borderNodata,SpatialReference*pSpatial,float* pData){
    if(pSpatial!=nullptr)
    {
        biTrans->SetToSpatialReference(AutoPtr<SpatialReference>(pSpatial,true));
        AutoPtr<CoordinateTransformation>pTrans=biTrans->GetCoordinateTransformationRev();
        bool CanTransform=false;
        if(pTrans!=nullptr) CanTransform=pTrans->CanTransform();
        if(CanTransform)
        {
            AutoPtr<CoordinateTransformation>pTransRev=biTrans->GetCoordinateTransformation();
            return innerGetInterpolatedDataBlockWithProj(lt,xCellSize,yCellSize,Width,Height,pTrans.get(),pTransRev.get(),noData,borderNodata,pData);
        }
    }
    Point2D<double> LeftTop;
    lt->GetCoord(&LeftTop.X,&LeftTop.Y);
    DOUBLE XMin=Left;
    DOUBLE YMax=Top;
    DOUBLE XMax=Left+cols*XCellSize;
    DOUBLE YMin=Top-rows*YCellSize;
    Rect2D<double> FullExtent=Rect2D<double>(XMin,YMax,XMax,YMin);
    Rect2D<double> CurrentExtent(LeftTop.X,LeftTop.Y,LeftTop.X+xCellSize*Width,LeftTop.Y-Height*yCellSize);
    if(!FullExtent.IntersectRect(CurrentExtent))
    {
        for(int k=Width*Height-1;k>=0;k--) pData[k]=borderNodata;
        return true;
    }
    CurrentExtent=FullExtent.Intersect(CurrentExtent);
    Rect2D<int> ImageRect;
    ImageRect.Left=(CurrentExtent.Left-XMin)/XCellSize;
    ImageRect.Top= (YMax-CurrentExtent.Top)/YCellSize;
    ImageRect.Right=(CurrentExtent.Right-XMin)/XCellSize;
    ImageRect.Bottom= (YMax-CurrentExtent.Bottom)/YCellSize;
    ImageRect.InflateRect(1,1,1,1);
    if(ImageRect.Left<0) ImageRect.Left=0;
    if(ImageRect.Top<0) ImageRect.Top=0;
    if(ImageRect.Right>=cols) ImageRect.Right=cols-1;
    if(ImageRect.Bottom>=rows) ImageRect.Bottom=rows-1;
    //波段对应的有效行列范围：ImageRect
    Rect2D<double> ImageMap;
    ImageMap.Left=XMin+XCellSize*ImageRect.Left;
    ImageMap.Top=YMax-YCellSize*ImageRect.Top;
    ImageMap.Right=XMin+XCellSize*ImageRect.Right+XCellSize;
    ImageMap.Bottom=YMax-YCellSize*ImageRect.Bottom-YCellSize;

    float nodata=NoData;
    double semiXCellSize=xCellSize/2;
    double semiYCellSize=yCellSize/2;


    if((xCellSize>=XCellSize)||(yCellSize>=YCellSize)){
        Rect2D<int> pixelRect;
        pixelRect.Left=(ImageMap.Left-XMin+xCellSize/2.0)/xCellSize;
        pixelRect.Top= (YMax-ImageMap.Top+yCellSize/2.0)/yCellSize;
        pixelRect.Right=(ImageMap.Right-XMin-xCellSize/2.0)/xCellSize;
        pixelRect.Bottom= (YMax-ImageMap.Bottom-yCellSize/2.0)/yCellSize;
        int rw=pixelRect.Right-pixelRect.Left+1;
        int rh=pixelRect.Bottom-pixelRect.Top+1;
        float*tempdata=new float[rw*rh];
        pBand->GetBlockData(ImageRect.Left, ImageRect.Top, ImageRect.Width()+1, ImageRect.Height()+1,rw,rh,tempdata);
        double Y=LeftTop.Y-semiYCellSize;
        double X;
        for(int i=0;i<Height;i++)
        {
            X=LeftTop.X+semiXCellSize;
            int posy=(ImageMap.Top-Y)/yCellSize;
            int pos=i*Width;
            if((posy<0)||(posy>=rh))
            {
                Y-=yCellSize;
                for(int j=0;j<Width;j++)
                {
                    pData[pos++]=noData;
                }
                continue;
            }
            for(int j=0;j<Width;j++)
            {
                int posx=(X-ImageMap.Left)/xCellSize;
                if((posx<0)||(posx>=rw))
                {
                    pData[pos++]=noData;
                    X+=xCellSize;
                    continue;
                }
                if(tempdata[posx+posy*rw]!=nodata)
                    pData[pos++]=tempdata[posx+posy*rw];
                else
                    pData[pos++]=noData;
                X+=xCellSize;
            }
            Y-=yCellSize;
        }
        delete []tempdata;
        return true;
    }


    //波段对应的有效地图范围：ImageMap
    int pW,pH;
    pW=ImageRect.Width()+1;
    pH=ImageRect.Height()+1;

    float*tempdata=new float[pW*3];
    float*tdata=new float[pW];
    float*tdata2=new float[pW*2];
    //SAFEARRAY*tData=SafeArrayCreateVector(VT_R4,0,pW);
    //SAFEARRAY*tData2=SafeArrayCreateVector(VT_R4,0,pW*2);
    double Y=LeftTop.Y-semiYCellSize;
    long Pos;
    float X;
    int iposx,iposy,iposx1,iposy1;
    double posx,posy;
    long Posi;
    int State;
    LONG formercol=-1;
    double ix,ix2;
    //LeftTop:截取的数据左上角
    for(int i=0;i<Height;i++)
    {
        X=LeftTop.X+semiXCellSize;
        Pos=i*Width;
        posy=(ImageMap.Top-Y)/YCellSize;
        iposy=posy;
        if((iposy<0)||(iposy>=pH))
        {
            for(int k=0;k<Width;k++) pData[Pos++]=borderNodata;
            Y-=yCellSize;
            continue;
        }
        if(posy-iposy<0.5)
        {
            iposy1=iposy-1;
            if(iposy1<0) iposy1=iposy;
        }
        else
        {
            iposy1=iposy+1;
            if(iposy1>=pH)
                iposy1=iposy;
        }
        if(formercol!=iposy)
        {
            if(ImageRect.Top+iposy-1<0)
            {
                for(int l=0;l<pW;l++) tempdata[l]=borderNodata;
                if(ImageRect.Top+iposy+1>=rows)
                {
                    pBand->GetBlockData(ImageRect.Left, ImageRect.Top+iposy, ImageRect.Width()+1, 1,pW,1,tdata);
                    float*tlData=(float*)((BYTE*)tempdata+pW*sizeof(float));
                    memcpy(tlData,tdata,pW*sizeof(float));
                    float*mdata=(float*)((BYTE*)tempdata+pW*sizeof(float)*2);
                    for(int l=0;l<pW;l++) mdata[l]=nodata;
                }
                else
                {
                    pBand->GetBlockData(ImageRect.Left, ImageRect.Top+iposy,ImageRect.Width()+1, 2,pW,2,tdata2);
                    float*tlData=(float*)((BYTE*)tempdata+pW*sizeof(float));
                    memcpy(tlData,tdata2,pW*2*sizeof(float));
                }
            }
            else if(ImageRect.Top+iposy+1>=rows)
            {
                pBand->GetBlockData(ImageRect.Left, ImageRect.Top+iposy-1, ImageRect.Width()+1, 2,pW,2,tdata2);
                memcpy(tempdata,tdata2,pW*2*sizeof(float));
                float*mdata=(float*)((BYTE*)tempdata+pW*sizeof(float)*2);
                for(int l=0;l<pW;l++) mdata[l]=nodata;
            }
            else
            {
                pBand->GetBlockData(ImageRect.Left, ImageRect.Top+iposy-1, ImageRect.Width()+1, 3,pW,3,tempdata);
            }
            formercol=posy;
        }
        float*data = tempdata;
        for(int j=0;j<Width;j++)
        {
            posx=(X-ImageMap.Left)/XCellSize;
            iposx=posx;
            if((iposx<0)||(iposx>=pW))
            {
                X+=xCellSize;
                pData[Pos]=borderNodata;
                continue;
            }
            if(data[iposx+pW]!=nodata)
            {
                if(posx-iposx<0.5)
                {
                    if(iposx>0)
                    {
                        if(data[iposx-1+pW]!=nodata)
                            State=1;
                        else
                            State=2;
                    }
                    else
                        State=2;
                    if(State==2)
                    {
                        if(iposx+1>=pW)
                            State=0;
                        else if(data[iposx+pW+1]==nodata)
                            State=0;
                    }
                }
                else
                {
                    if(iposx+1<pW)
                    {
                        if(data[iposx+pW+1]!=nodata)
                            State=2;
                        else
                            State=1;
                    }
                    else
                        State=1;
                    if(State==1)
                    {
                        if(iposx<1)
                            State=0;
                        else if(data[iposx+pW-1]==nodata)
                            State=0;
                    }
                }
                switch(State)
                {
                case 0:
                {
                    ix=data[iposx+pW];break;
                }
                case 1:
                {
                    ix=(posx-iposx+0.5)*(data[iposx+pW]-data[iposx+pW-1])+data[iposx+pW-1];break;
                }
                case 2:
                {
                    ix=(posx-iposx-0.5)*(data[iposx+pW+1]-data[iposx+pW])+data[iposx+pW];break;
                }
                }
                if(iposy1==iposy)
                {
                    pData[Pos]=ix;
                    if(nodata==pData[Pos]) pData[Pos]=noData;
                    Pos++;
                    X+=xCellSize;
                    continue;
                }
                Posi=(iposy1-iposy+1)*pW+iposx;
                if(data[Posi]==nodata)
                    ix2=nodata;
                else
                {
                    if(posx-iposx<0.5)
                    {
                        if(iposx>0)
                        {
                            if(data[Posi-1]!=nodata)
                                State=1;
                            else
                                State=2;
                        }
                        else
                            State=2;
                        if(State==2)
                        {
                            if(iposx+1>=pW)
                                State=0;
                            else if(data[Posi+1]==nodata)
                                State=0;
                        }
                    }
                    else
                    {
                        if(iposx+1<pW)
                        {
                            if(data[Posi+1]!=nodata)
                                State=2;
                            else
                                State=1;
                        }
                        else
                            State=1;
                        if(State==1)
                        {
                            if(iposx<1)
                                State=0;
                            else if(data[Posi-1]==nodata)
                                State=0;
                        }
                    }
                    switch(State)
                    {
                    case 0:
                    {
                        ix2=data[Posi];break;
                    }
                    case 1:
                    {
                        ix2=(posx-iposx+0.5)*(data[Posi]-data[Posi-1])+data[Posi-1];break;
                    }
                    case 2:
                    {
                        ix2=(posx-iposx-0.5)*(data[Posi+1]-data[Posi])+data[Posi];break;
                    }
                    }
                }
                State=0;
                if(ix==nodata)
                    State=1;
                if(ix2==nodata)
                    State+=2;
                switch(State)
                {
                case 0:
                    pData[Pos]=(ix2-ix)*(posy-iposy-0.5)/(iposy1-iposy)+ix;break;
                case 1:
                    pData[Pos]=ix2;break;
                case 2:
                    pData[Pos]=ix;break;
                case 3:
                    pData[Pos]=noData;break;
                }
            }
            else
                pData[Pos]=noData;
            Pos++;
            X+=xCellSize;
        }
        Y-=yCellSize;
    }
    delete []tempdata;
    delete []tdata;
    delete []tdata2;
    return true;
}

bool RasterBandTool::innerGetInterpolatedDataBlockWithProj(Point* lt,DOUBLE xCellSize,DOUBLE yCellSize,int Width,int Height,CoordinateTransformation*pTrans,CoordinateTransformation*pTransRev,float noData,float borderNoData,float* pData){
    Point2D<double> LeftTop;
    lt->GetCoord(&LeftTop.X,&LeftTop.Y);
    Rect2D<double> MapExtent(LeftTop.X,LeftTop.Y,LeftTop.X+xCellSize*Width,LeftTop.Y-Height*yCellSize);
    Rect2D<double> FileExtent=pTrans->TransformEnvelope(MapExtent);//将地图范围转换为图层范围
    Rect2D<double> pixelRect=MapToPixelCoord(FileExtent);
    Rect2D<int> PixelRect;
    PixelRect.Left=pixelRect.Left;
    PixelRect.Top=pixelRect.Top;
    PixelRect.Right=pixelRect.Right;
    PixelRect.Bottom=pixelRect.Bottom;
    PixelRect.NormalizeClientRect();
    PixelRect.InflateRect(1,1,1,1);
    int ImageWidth=cols;
    int ImageHeight=rows;
    if(PixelRect.Left<0)
        PixelRect.Left=0;
    else if(PixelRect.Left>=ImageWidth)
        PixelRect.Left=ImageWidth;
    if(PixelRect.Right<0)
        PixelRect.Right=-1;
    else if(PixelRect.Right>=ImageWidth)
        PixelRect.Right=ImageWidth-1;
    if(PixelRect.Top<0)
        PixelRect.Top=0;
    else if(PixelRect.Top>=ImageHeight)
        PixelRect.Top=ImageHeight;
    if(PixelRect.Bottom<0)
        PixelRect.Bottom=-1;
    else if(PixelRect.Bottom>=ImageHeight)
        PixelRect.Bottom=ImageHeight-1;
    long Size=Width*Height;
    if(PixelRect.Left>PixelRect.Right)
    {
        for(long k=0;k<Size;k++) pData[k]=borderNoData;
        return false;
    }
    if(PixelRect.Top>PixelRect.Bottom)
    {
        for(long k=0;k<Size;k++) pData[k]=borderNoData;
        return false;
    }
    //图层像素范围进行纠正
    Rect2D<double> tempRect(PixelRect.Left,PixelRect.Top,PixelRect.Right,PixelRect.Bottom);
    FileExtent=PixelToMapCoord(tempRect);
    //由图层像素范围反算图层范围
    Rect2D<double> PaintExtent=pTransRev->TransformEnvelope(FileExtent);//将图层范围转换为要显示的地图范围
    int pW,pH;//表示文件中与PixelRect成比例的宽和高，依据W和H计算
    double r1=(float)Width/Height;
    double r2=(float)(PixelRect.Width()+1)/(PixelRect.Height()+1);
    if(r1>=r2)
    {
        if(Width<PixelRect.Width()+1)
        {
            pW=Width;
            double fpH=pW/r2;
            pH=fpH;
            if(fpH-pH>=0.5) pH++;
        }
        else
        {
            pW=PixelRect.Width()+1;
            pH=PixelRect.Height()+1;
        }
    }
    else
    {
        if(Height<PixelRect.Height()+1)
        {
            pH=Height;
            double fpW=pH*r2;
            pW=fpW;
            if(fpW-pW>=0.5) pW++;
        }
        else
        {
            pW=PixelRect.Width()+1;
            pH=PixelRect.Height()+1;
        }
    }
    float*data = new float[pW*pH];
    pBand->GetBlockData(PixelRect.Left,PixelRect.Top,PixelRect.Width()+1,PixelRect.Height()+1,pW,pH,data);
    int sW,sH;
    int difx=2;
    int dify=2;
    if(Width<2) difx=1;
    if(Height<2) dify=1;
    if(Width%difx==0)
        sW=Width/difx;
    else
        sW=Width/difx+1;
    if(Height%dify==0)
        sH=Height/dify;
    else
        sH=Height/dify+1;
    DOUBLE*pX=new DOUBLE[sW*sH];
    DOUBLE*pY=new DOUBLE[sW*sH];
    double lY,lX;
    double dCellXSize=xCellSize*difx;
    double dCellYSize=yCellSize*dify;
    lY=LeftTop.Y-0.5*yCellSize;
    double FromlX=LeftTop.X+0.5*xCellSize;
    int Pos=0;
    for(int i=0;i<Height;i+=dify)
    {
        lX=FromlX;
        for(int j=0;j<Width;j+=difx)
        {
            pX[Pos]=lX;
            pY[Pos]=lY;
            lX+=dCellXSize;
            Pos++;
        }
        lY-=dCellYSize;
    }
    if(pTrans!=nullptr) pTrans->TransformArray(pX,pY,sW*sH);
    double semiXCellSize=xCellSize/2;
    double semiYCellSize=yCellSize/2;
    Pos=0;
    double CellXSize=FileExtent.Width()/pW;
    double CellYSize=FileExtent.Height()/pH;
    //lY=LeftTop.Y-semiYCellSize;
    Point2D<double> dpt;
    float nodata=NoData;
    long pos;
    int iposx,iposy,iposy1;
    int State;
    double ix,ix2;
    double nX,nY,nX1,nY1,nX2,nY2;
    int di,dj;
    lY=LeftTop.Y-semiYCellSize;
    for(int i=0;i<Height;i++)
    {
        di=i/dify;
        int iidif=i%dify;
        lX=LeftTop.X-semiXCellSize;
        for(int j=0;j<Width;j++)
        {
            lX+=xCellSize;
            if((lX<PaintExtent.Left)||(lX>PaintExtent.Right)||(lY<PaintExtent.Bottom)||(lY>PaintExtent.Top))
            {
                pData[Pos++]=borderNoData;
                continue;
            }
            dj=j/difx;
            pos=di*sW+dj;
            int ijdif=j%difx;
            if(iidif==0)
            {
                if(ijdif==0)
                {
                    nX=pX[pos];
                    nY=pY[pos];
                }
                else if(dj<sW-1)
                {
                    nX=pX[pos]+(pX[pos+1]-pX[pos])*ijdif/difx;
                    nY=pY[pos]+(pY[pos+1]-pY[pos])*ijdif/difx;
                }
                else if(dj>1)
                {
                    nX=pX[pos-1]+(pX[pos]-pX[pos-1])*(1+ijdif)/difx;
                    nY=pY[pos-1]+(pY[pos]-pY[pos-1])*(1+ijdif)/difx;
                }
                else
                {
                    nX=pX[pos];
                    nY=pY[pos];
                }
            }
            else if(di<sH-1)
            {
                if(ijdif==0)
                {
                    nX=pX[pos]+(pX[pos+sW]-pX[pos])*iidif/dify;
                    nY=pY[pos]+(pY[pos+sW]-pY[pos])*iidif/dify;
                }
                else if(dj<sW-1)
                {
                    nX1=pX[pos]+(pX[pos+sW]-pX[pos])*iidif/dify;
                    nX2=pX[pos+1]+(pX[pos+sW+1]-pX[pos+1])*iidif/dify;
                    nX=nX1+(nX2-nX1)*ijdif/difx;
                    nY1=pY[pos]+(pY[pos+sW]-pY[pos])*iidif/dify;
                    nY2=pY[pos+1]+(pY[pos+sW+1]-pY[pos+1])*iidif/dify;
                    nY=nY1+(nY2-nY1)*ijdif/difx;
                }
                else if(dj>1)
                {
                    nX1=pX[pos-1]+(pX[pos+sW-1]-pX[pos-1])*iidif/dify;
                    nX2=pX[pos]+(pX[pos+sW]-pX[pos])*iidif/dify;
                    nX=nX1+(nX2-nX1)*(1+ijdif)/difx;
                    nY1=pY[pos-1]+(pY[pos+sW-1]-pY[pos-1])*iidif/dify;
                    nY2=pY[pos]+(pY[pos+sW]-pY[pos])*iidif/dify;
                    nY=nY1+(nY2-nY1)*(1+ijdif)/difx;
                }
                else
                {
                    nX=pX[pos]+(pX[pos+sW]-pX[pos])*iidif/dify;
                    nY=pY[pos]+(pY[pos+sW]-pY[pos])*iidif/dify;
                }
            }
            else if(di>1)
            {
                if(ijdif==0)
                {
                    nX=pX[pos-sW]+(pX[pos]-pX[pos-sW])*(1+iidif)/dify;
                    nY=pY[pos-sW]+(pY[pos]-pY[pos-sW])*(1+iidif)/dify;
                }
                else if(dj<sW-1)
                {
                    nX1=pX[pos-sW]+(pX[pos]-pX[pos-sW])*(1+iidif)/dify;
                    nX2=pX[pos-sW+1]+(pX[pos+1]-pX[pos-sW+1])*(1+iidif)/dify;
                    nX=nX1+(nX2-nX1)*ijdif/difx;
                    nY1=pY[pos-sW]+(pY[pos]-pY[pos-sW])*(1+iidif)/dify;
                    nY2=pY[pos-sW+1]+(pY[pos+1]-pY[pos-sW+1])*(1+iidif)/dify;
                    nY=nY1+(nY2-nY1)*ijdif/difx;
                }
                else if(dj>1)
                {
                    nX1=pX[pos-sW-1]+(pX[pos-1]-pX[pos-sW-1])*(1+iidif)/dify;
                    nX2=pX[pos-sW]+(pX[pos]-pX[pos-sW])*(1+iidif)/dify;
                    nX=nX1+(nX2-nX1)*(1+ijdif)/difx;
                    nY1=pY[pos-sW-1]+(pY[pos-1]-pY[pos-sW-1])*(1+iidif)/dify;
                    nY2=pY[pos-sW]+(pY[pos]-pY[pos-sW])*(1+iidif)/dify;
                    nY=nY1+(nY2-nY1)*(1+ijdif)/difx;
                }
                else
                {
                    nX=pX[pos-sW]+(pX[pos]-pX[pos-sW])*(1+iidif)/dify;
                    nY=pY[pos-sW]+(pY[pos]-pY[pos-sW])*(1+iidif)/dify;
                }
            }
            dpt.X=(nX-FileExtent.Left)/CellXSize;
            dpt.Y=(nY-FileExtent.Top)/CellYSize;
            if((dpt.X<0)||(dpt.X>=pW)||(dpt.Y<0)||(dpt.Y>=pH))
            {
                pData[Pos++]=borderNoData;
                continue;
            }
            iposx=dpt.X;
            iposy=dpt.Y;
            if(dpt.Y-iposy<0.5)
            {
                iposy1=iposy-1;
                if(iposy1<0) iposy1=iposy;
            }
            else
            {
                iposy1=iposy+1;
                if(iposy1>=pH)
                    iposy1=iposy;
            }
            pos=iposy*pW+iposx;
            if(data[pos]==nodata)
            {
                pData[Pos]=noData;
                Pos++;
                continue;
            }
            else
            {
                if(dpt.X-iposx<0.5)
                {
                    if(iposx>0)
                    {
                        if(data[pos-1]!=nodata)
                            State=1;
                        else
                            State=2;
                    }
                    else
                        State=2;
                    if(State==2)
                    {
                        if(iposx+1>=pW)
                            State=0;
                        else if(data[pos+1]==nodata)
                            State=0;
                    }
                }
                else
                {
                    if(iposx+1<pW)
                    {
                        if(data[pos+1]!=nodata)
                            State=2;
                        else
                            State=1;
                    }
                    else
                        State=1;
                    if(State==1)
                    {
                        if(iposx<1)
                            State=0;
                        else if(data[pos-1]==nodata)
                            State=0;
                    }
                }
                switch(State)
                {
                case 0:
                {
                    ix=data[pos];break;
                }
                case 1:
                {
                    ix=(dpt.X-iposx+0.5)*(data[pos]-data[pos-1])+data[pos-1];break;
                }
                case 2:
                {
                    ix=(dpt.X-iposx-0.5)*(data[pos+1]-data[pos])+data[pos];break;
                }
                }
            }
            if(iposy1==iposy)
            {
                pData[Pos]=ix;
                if(nodata==pData[Pos]) pData[Pos]=noData;
                Pos++;
                continue;
            }
            pos=iposy1*pW+iposx;
            if(data[pos]==nodata)
                ix2=nodata;
            else
            {
                if(dpt.X-iposx<0.5)
                {
                    if(iposx>0)
                    {
                        if(data[pos-1]!=nodata)
                            State=1;
                        else
                            State=2;
                    }
                    else
                        State=2;
                    if(State==2)
                    {
                        if(iposx+1>=pW)
                            State=0;
                        else if(data[pos+1]==nodata)
                            State=0;
                    }
                }
                else
                {
                    if(iposx+1<pW)
                    {
                        if(data[pos+1]!=nodata)
                            State=2;
                        else
                            State=1;
                    }
                    else
                        State=1;
                    if(State==1)
                    {
                        if(iposx<1)
                            State=0;
                        else if(data[pos-1]==nodata)
                            State=0;
                    }
                }
                switch(State)
                {
                case 0:
                {
                    ix2=data[pos];break;
                }
                case 1:
                {
                    ix2=(dpt.X-iposx+0.5)*(data[pos]-data[pos-1])+data[pos-1];break;
                }
                case 2:
                {
                    ix2=(dpt.X-iposx-0.5)*(data[pos+1]-data[pos])+data[pos];break;
                }
                }
            }
            State=0;
            if(ix==nodata)
                State=1;
            if(ix2==nodata)
                State+=2;
            switch(State)
            {
            case 0:
                pData[Pos]=(ix2-ix)*(dpt.Y-iposy-0.5)/(iposy1-iposy)+ix;break;
            case 1:
                pData[Pos]=ix2;break;
            case 2:
                pData[Pos]=ix;break;
            case 3:
                pData[Pos]=noData;break;
            }
            Pos++;
        }
        lY-=yCellSize;
    }
    delete []data;
    delete []pX;
    delete []pY;
    return true;
}

bool RasterBandTool::GetBlockData(LONG x,LONG y,LONG Width,LONG Height,LONG buffx,LONG buffy,float*data,bool bInterpolated){
    if((pBand==nullptr)||(Width<=0)||(Height==0)) return false;
    if(bInterpolated)
        innerGetInterpolatedBlock(x, y, x+Width-1,y+Height-1,buffx,buffy,data);
    else
        pBand->GetBlockData(x,y,Width,Height,buffx,buffy,data);
}

bool RasterBandTool::GetBlockDataByCoord(Point*leftTop,DOUBLE xCellSize,DOUBLE yCellSize,LONG Width, LONG Height,float*data,SpatialReference*psp,FLOAT nodata,bool bInterpolated){
    if((pBand==nullptr)||(Width<=0)||(Height==0)) return false;
    if(bInterpolated)
        innerGetInterpolatedDataBlock(leftTop,xCellSize,yCellSize,Width,Height,nodata,nodata,psp,data);
    else
        innerGetBlockDataByCoord(leftTop,xCellSize,yCellSize,Width,Height,nodata,nodata,psp,data);
    return true;
}

bool RasterBandTool::GetBlockDataByCoordEx(Point*leftTop,DOUBLE xCellSize,DOUBLE yCellSize,LONG Width, LONG Height,float*data,SpatialReference*psp,FLOAT nodata,FLOAT borderNodata,bool bInterpolated){
    if((pBand==nullptr)||(Width<=0)||(Height==0)) return false;
    if(bInterpolated)
        innerGetInterpolatedDataBlock(leftTop,xCellSize,yCellSize,Width,Height,nodata,borderNodata,psp,data);
    else
        innerGetBlockDataByCoord(leftTop,xCellSize,yCellSize,Width,Height,nodata,borderNodata,psp,data);
    return true;
}

bool RasterBandTool::GetBlockDataByCoord(const RasterDesp&desp,float*data,SpatialReference*psp,FLOAT nodata,bool bInterpolated){
    if((pBand==nullptr)||(desp.cols<=0)||(desp.rows==0)) return false;
    AutoPtr<Point>leftTop(new Point(desp.left,desp.top));
    if(bInterpolated)
        innerGetInterpolatedDataBlock(leftTop.get(),desp.xCellSize,desp.yCellSize,desp.cols,desp.rows,nodata,nodata,psp,data);
    else
        innerGetBlockDataByCoord(leftTop.get(),desp.xCellSize,desp.yCellSize,desp.cols,desp.rows,nodata,nodata,psp,data);
    return true;
}

AutoPtr<Histogram>RasterBandTool::ComputeHistogram(bool bApproxOK,DOUBLE minValue,DOUBLE maxValue,LONG ColumnsNum){
    if(pBand==nullptr) return nullptr;
    if(ColumnsNum<=0) return nullptr;
    float noData=NoData;
    DOUBLE Dif=(maxValue-minValue)/ColumnsNum;
    int iDif;
    AutoPtr<Histogram>pHist(new Histogram);
    pHist->SetColumns(ColumnsNum);
    pHist->SetMinMaxValue(minValue,maxValue);
    int dof=1;
    if(bApproxOK) dof=2;
    if(bApproxOK)
    {
        int maxs=max(rows,cols);
        if(maxs>1000)
        {
            dof=maxs/1000;
        }
    }
    else
    {
        int maxs=max(rows,cols);
        if(maxs>5000)
        {
            dof=maxs/5000;
        }
    }
    int sCols=cols/dof;
    int sRows=rows/dof;
    if(sCols<1) sCols=1;
    if(sRows<1) sRows=1;
    float*fVs=new float[sCols*sRows];
    pBand->GetBlockData(0,0,cols,rows,sCols,sRows,fVs);
    for(int j=sRows*sCols-1;j>=0;j--)
    {
        if(noData==fVs[j]) continue;
        if(Dif==0)
            iDif=0;
        else
        {
            iDif=(fVs[j]-minValue)/Dif;
            if((iDif<0)||(iDif>=ColumnsNum)) continue;
            pHist->Add(iDif,1);
        }
    }
    delete[]fVs;
    return pHist;
}
AutoPtr<Histogram>RasterBandTool::ComputeHistogramEx(bool bApproxOK,DOUBLE minValue,DOUBLE maxValue,LONG ColumnsNum,IndexedArray<int>&excludeValues){
    if(pBand==nullptr) return nullptr;
    if(ColumnsNum<=0) return nullptr;
    float noData=NoData;
    DOUBLE Dif=(maxValue-minValue)/ColumnsNum;
    int iDif;
    AutoPtr<Histogram>pHist(new Histogram);
    pHist->SetColumns(ColumnsNum);
    pHist->SetMinMaxValue(minValue,maxValue);
    int dof=1;
    if(bApproxOK) dof=2;
    if(bApproxOK)
    {
        int maxs=max(rows,cols);
        if(maxs>1000)
        {
            dof=maxs/1000;
        }
    }
    else
    {
        int maxs=max(rows,cols);
        if(maxs>5000)
        {
            dof=maxs/5000;
        }
    }
    int sCols=cols/dof;
    int sRows=rows/dof;
    if(sCols<1) sCols=1;
    if(sRows<1) sRows=1;
    float*fVs=new float[sCols*sRows];
    pBand->GetBlockData(0,0,cols,rows,sCols,sRows,fVs);
    for(int j=sRows*sCols-1;j>=0;j--)
    {
        if(noData==fVs[j]) continue;
        if(excludeValues.FindValueIndex(fVs[j])>=0) continue;
        if(Dif==0)
            iDif=0;
        else
        {
            iDif=(fVs[j]-minValue)/Dif;
            if((iDif<0)||(iDif>=ColumnsNum)) continue;
            pHist->Add(iDif,1);
        }
    }
    delete[]fVs;
    return pHist;
}
bool RasterBandTool::ComputeStatistics(bool bApproxOK,DOUBLE*minV,DOUBLE*maxV,DOUBLE*meanV,DOUBLE*standV){
    if(pBand==nullptr) return false;
    StatisHistory&pStaHist=pBand->GetStatisHistory();
    if(pStaHist.GetStatis("1",*minV,*maxV,*meanV,*standV))
    {
        return true;
    }
    *meanV=*minV=*maxV=*standV=0;
    double min,max,mean,sum2;
    int Dif=1;
    if(bApproxOK)
    {
        int maxs=rows;
        if(cols>maxs) maxs=cols;
        if(maxs>1000)
        {
            Dif=maxs/1000;
        }
    }
    else
    {
        int maxs=rows;
        if(cols>maxs) maxs=cols;
        if(maxs>5000)
        {
            Dif=maxs/5000;
        }
    }
    LONG sCols=cols/Dif;
    LONG sRows=rows/Dif;
    if(sCols<=0) sCols=1;
    if(sRows<=0) sRows=1;
    float*fVs=new float[sCols*sRows];
    float nodata=NoData;
    min=nodata;
    LONG Count=0;
    sum2=0;
    pBand->GetBlockData(0,0,cols,rows,sCols,sRows,fVs);
    for(int j=sCols*sRows-1;j>=0;j--)
    {
        if(nodata==fVs[j]) continue;
        if(Count==0)
        {
            min=max=fVs[j];
            mean=min;
            Count++;
        }
        else
        {
            if(fVs[j]<min) min=fVs[j];
            if(fVs[j]>max) max=fVs[j];
            mean+=fVs[j];
            sum2+=fVs[j]*fVs[j];
            Count++;
        }
    }
    delete []fVs;
    if(Count==0) return false;
    *standV=sqrt((sum2-Count*pow(mean/Count,2))/Count);
    *minV=min;
    *maxV=max;
    *meanV=(mean/Count);
    pStaHist.AddStatisHist("1",*minV,*maxV,*meanV,*standV);
    return true;
}
bool RasterBandTool::ComputeStatisticsEx(bool bApproxOK,DOUBLE*minV,DOUBLE*maxV,DOUBLE*meanV,DOUBLE*standV,IndexedArray<int>&excludeValues){
    if(pBand==nullptr) return false;
    StatisHistory&pStaHist=pBand->GetStatisHistory();
    if(pStaHist.GetStatis("1",*minV,*maxV,*meanV,*standV))
    {
        return true;
    }
    *meanV=*minV=*maxV=*standV=0;
    double min,max,mean,sum2;
    int Dif=1;
    if(bApproxOK)
    {
        int maxs=rows;
        if(cols>maxs) maxs=cols;
        if(maxs>1000)
        {
            Dif=maxs/1000;
        }
    }
    else
    {
        int maxs=rows;
        if(cols>maxs) maxs=cols;
        if(maxs>5000)
        {
            Dif=maxs/5000;
        }
    }
    LONG sCols=cols/Dif;
    LONG sRows=rows/Dif;
    if(sCols<=0) sCols=1;
    if(sRows<=0) sRows=1;
    float*fVs=new float[sCols*sRows];
    float nodata=NoData;
    min=nodata;
    LONG Count=0;
    sum2=0;
    pBand->GetBlockData(0,0,cols,rows,sCols,sRows,fVs);
    for(int j=sCols*sRows-1;j>=0;j--)
    {
        if(nodata==fVs[j]) continue;
        if(excludeValues.FindValueIndex(fVs[j])>=0) continue;
        if(Count==0)
        {
            min=max=fVs[j];
            mean=min;
            Count++;
        }
        else
        {
            if(fVs[j]<min) min=fVs[j];
            if(fVs[j]>max) max=fVs[j];
            mean+=fVs[j];
            sum2+=fVs[j]*fVs[j];
            Count++;
        }
    }
    delete []fVs;
    if(Count==0) return false;
    *standV=sqrt((sum2-Count*pow(mean/Count,2))/Count);
    *minV=min;
    *maxV=max;
    *meanV=(mean/Count);
    pStaHist.AddStatisHist("1",*minV,*maxV,*meanV,*standV);
    return true;
}
AutoPtr<Histogram>RasterBandTool::ComputeExtentHistogram(bool bApproxOK,Envelope*extent,DOUBLE minValue,DOUBLE maxValue,LONG ColumnsNum){
    if(pBand==nullptr) return nullptr;
    if(ColumnsNum<=0) return nullptr;
    float noData=NoData;
    DOUBLE Dif=(maxValue-minValue)/ColumnsNum;
    int iDif;
    AutoPtr<Histogram>pHist(new Histogram);
    DOUBLE left,top;
    left=pBand->GetRasterDesp().left;
    top=pBand->GetRasterDesp().top;
    DOUBLE cellsizeX,cellsizeY;
    cellsizeX=pBand->GetRasterDesp().xCellSize;
    cellsizeY=pBand->GetRasterDesp().yCellSize;
    DOUBLE el,et,er,eb;
    extent->GetCoord(&el,&et,&er,&eb);
    LONG l,t,r,b;
    l=(el-left)/cellsizeX;
    if(l<0) l=0;
    r=(er-left)/cellsizeX;
    if(r>=cols) r=cols-1;
    t=(top-et)/cellsizeY;
    if(t<0) t=0;
    b=(top-eb)/cellsizeY;
    if(b>=rows) b=rows-1;
    int tcols=(r-l+1);
    int trows=(b-t+1);
    int dof=1;
    if(bApproxOK) dof=2;
    if(bApproxOK)
    {
        int maxs=max(trows,tcols);
        if(maxs>1000)
        {
            dof=maxs/1000;
        }
    }
    else
    {
        int maxs=max(trows,tcols);
        if(maxs>5000)
        {
            dof=maxs/5000;
        }
    }
    int sCols=tcols/dof;
    int sRows=trows/dof;
    if(sCols<1) sCols=1;
    if(sRows<1) sRows=1;
    float*fVs=new float[sCols*sRows];
    pBand->GetBlockData(l,t,tcols,trows,sCols,sRows,fVs);
    for(int j=sRows*sCols-1;j>=0;j--)
    {
        if(noData==fVs[j]) continue;
        if(Dif==0)
            iDif=0;
        else
        {
            iDif=(fVs[j]-minValue)/Dif;
            if((iDif<0)||(iDif>=ColumnsNum)) continue;
            pHist->Add(iDif,1);
        }
    }
    delete []fVs;
    return pHist;
}
AutoPtr<Histogram>RasterBandTool::ComputeExtentHistogramEx(bool bApproxOK,Envelope*extent,DOUBLE minValue,DOUBLE maxValue,LONG ColumnsNum,IndexedArray<int>&excludeValues){
    if(pBand==nullptr) return nullptr;
    if(ColumnsNum<=0) return nullptr;
    float noData=NoData;
    DOUBLE Dif=(maxValue-minValue)/ColumnsNum;
    int iDif;
    AutoPtr<Histogram>pHist(new Histogram);
    DOUBLE left,top;
    left=pBand->GetRasterDesp().left;
    top=pBand->GetRasterDesp().top;
    DOUBLE cellsizeX,cellsizeY;
    cellsizeX=pBand->GetRasterDesp().xCellSize;
    cellsizeY=pBand->GetRasterDesp().yCellSize;
    DOUBLE el,et,er,eb;
    extent->GetCoord(&el,&et,&er,&eb);
    LONG l,t,r,b;
    l=(el-left)/cellsizeX;
    if(l<0) l=0;
    r=(er-left)/cellsizeX;
    if(r>=cols) r=cols-1;
    t=(top-et)/cellsizeY;
    if(t<0) t=0;
    b=(top-eb)/cellsizeY;
    if(b>=rows) b=rows-1;
    int tcols=(r-l+1);
    int trows=(b-t+1);
    int dof=1;
    if(bApproxOK) dof=2;
    if(bApproxOK)
    {
        int maxs=max(trows,tcols);
        if(maxs>1000)
        {
            dof=maxs/1000;
        }
    }
    else
    {
        int maxs=max(trows,tcols);
        if(maxs>5000)
        {
            dof=maxs/5000;
        }
    }
    int sCols=tcols/dof;
    int sRows=trows/dof;
    if(sCols<1) sCols=1;
    if(sRows<1) sRows=1;
    float*fVs=new float[sCols*sRows];
    pBand->GetBlockData(l,t,tcols,trows,sCols,sRows,fVs);
    for(int j=sRows*sCols-1;j>=0;j--)
    {
        if(noData==fVs[j]) continue;
        if(excludeValues.FindValueIndex(fVs[j])>=0) continue;
        if(Dif==0)
            iDif=0;
        else
        {
            iDif=(fVs[j]-minValue)/Dif;
            if((iDif<0)||(iDif>=ColumnsNum)) continue;
            pHist->Add(iDif,1);
        }
    }
    delete []fVs;
    return pHist;
}
bool RasterBandTool::ComputeExtentStatistics(bool bApproxOK,Envelope*extent,DOUBLE*minV,DOUBLE*maxV,DOUBLE*meanV,DOUBLE*standV){
    if(pBand==nullptr) return false;
    StatisHistory&pStaHist=pBand->GetStatisHistory();
    if(pStaHist.GetStatis("1",*minV,*maxV,*meanV,*standV))
    {
        return true;
    }
    *meanV=*minV=*maxV=*standV=0;
    double min,max,mean,sum2;
    int Dif=1;
    DOUBLE left,top;
    left=pBand->GetRasterDesp().left;
    top=pBand->GetRasterDesp().top;
    DOUBLE cellsizeX,cellsizeY;
    cellsizeX=pBand->GetRasterDesp().xCellSize;
    cellsizeY=pBand->GetRasterDesp().yCellSize;
    DOUBLE el,et,er,eb;
    extent->GetCoord(&el,&et,&er,&eb);
    LONG l,t,r,b;
    l=(el-left)/cellsizeX;
    if(l<0) l=0;
    r=(er-left)/cellsizeX;
    if(r>=cols) r=cols-1;
    t=(top-et)/cellsizeY;
    if(t<0) t=0;
    b=(top-eb)/cellsizeY;
    if(b>=rows) b=rows-1;
    int tcols=(r-l+1);
    int trows=(b-t+1);
    if(bApproxOK)
    {
        int maxs=trows;
        if(tcols>maxs) maxs=tcols;
        if(maxs>1000)
        {
            Dif=maxs/1000;
        }
    }
    else
    {
        int maxs=trows;
        if(tcols>maxs) maxs=tcols;
        if(maxs>5000)
        {
            Dif=maxs/5000;
        }
    }
    LONG sCols=tcols/Dif;
    LONG sRows=trows/Dif;
    if(sCols<=0) sCols=1;
    if(sRows<=0) sRows=1;
    float*fVs=new float[sCols*sRows];
    float nodata=NoData;
    min=nodata;
    LONG Count=0;
    sum2=0;
    pBand->GetBlockData(l,t,tcols,trows,sCols,sRows,fVs);
    for(int j=sCols*sRows-1;j>=0;j--)
    {
        if(nodata==fVs[j]) continue;
        if(Count==0)
        {
            min=max=fVs[j];
            mean=min;
            Count++;
        }
        else
        {
            if(fVs[j]<min) min=fVs[j];
            if(fVs[j]>max) max=fVs[j];
            mean+=fVs[j];
            sum2+=fVs[j]*fVs[j];
            Count++;
        }
    }
    delete []fVs;
    if(Count==0) return false;
    *standV=sqrt((sum2-Count*pow(mean/Count,2))/Count);
    *minV=min;
    *maxV=max;
    *meanV=(mean/Count);
    pStaHist.AddStatisHist("1",*minV,*maxV,*meanV,*standV);
    return true;
}
bool RasterBandTool::ComputeExtentStatisticsEx(bool bApproxOK,Envelope*extent,DOUBLE*minV,DOUBLE*maxV,DOUBLE*meanV,DOUBLE*standV,IndexedArray<int>&excludeValues){
    if(pBand==nullptr) return false;
    StatisHistory&pStaHist=pBand->GetStatisHistory();
    if(pStaHist.GetStatis("1",*minV,*maxV,*meanV,*standV))
    {
        return true;
    }
    *meanV=*minV=*maxV=*standV=0;
    double min,max,mean,sum2;
    int Dif=1;
    DOUBLE left,top;
    left=pBand->GetRasterDesp().left;
    top=pBand->GetRasterDesp().top;
    DOUBLE cellsizeX,cellsizeY;
    cellsizeX=pBand->GetRasterDesp().xCellSize;
    cellsizeY=pBand->GetRasterDesp().yCellSize;
    DOUBLE el,et,er,eb;
    extent->GetCoord(&el,&et,&er,&eb);
    LONG l,t,r,b;
    l=(el-left)/cellsizeX;
    if(l<0) l=0;
    r=(er-left)/cellsizeX;
    if(r>=cols) r=cols-1;
    t=(top-et)/cellsizeY;
    if(t<0) t=0;
    b=(top-eb)/cellsizeY;
    if(b>=rows) b=rows-1;
    int tcols=(r-l+1);
    int trows=(b-t+1);
    if(bApproxOK)
    {
        int maxs=trows;
        if(tcols>maxs) maxs=tcols;
        if(maxs>1000)
        {
            Dif=maxs/1000;
        }
    }
    else
    {
        int maxs=trows;
        if(tcols>maxs) maxs=tcols;
        if(maxs>5000)
        {
            Dif=maxs/5000;
        }
    }
    LONG sCols=tcols/Dif;
    LONG sRows=trows/Dif;
    if(sCols<=0) sCols=1;
    if(sRows<=0) sRows=1;
    float*fVs=new float[sCols*sRows];
    float nodata=NoData;
    min=nodata;
    LONG Count=0;
    sum2=0;
    pBand->GetBlockData(l,t,tcols,trows,sCols,sRows,fVs);
    for(int j=sCols*sRows-1;j>=0;j--)
    {
        if(nodata==fVs[j]) continue;
        if(excludeValues.FindValueIndex(fVs[j])>=0) continue;
        if(Count==0)
        {
            min=max=fVs[j];
            mean=min;
            Count++;
        }
        else
        {
            if(fVs[j]<min) min=fVs[j];
            if(fVs[j]>max) max=fVs[j];
            mean+=fVs[j];
            sum2+=fVs[j]*fVs[j];
            Count++;
        }
    }
    delete []fVs;
    if(Count==0) return false;
    *standV=sqrt((sum2-Count*pow(mean/Count,2))/Count);
    *minV=min;
    *maxV=max;
    *meanV=(mean/Count);
    pStaHist.AddStatisHist("1",*minV,*maxV,*meanV,*standV);
    return true;
}

RasterBandCacheBlocks::RasterBandCacheBlocks(AutoPtr<RasterBand>rasterBand,int blockSize){
    this->rasterBand=rasterBand;
    this->desp=rasterBand->GetRasterDesp();
    int cols=desp.cols;
    int rows=desp.rows;
    this->blockSize=blockSize;
    blockXNum=cols/blockSize;
    if(blockXNum*blockSize<cols) blockXNum++;
    blockYNum=rows/blockSize;
    if(blockYNum*blockSize<rows) blockYNum++;
    blocks.resize(blockXNum*blockYNum);
    for(int k=blockXNum*blockYNum-1;k>=0;k--) blocks[k]=nullptr;
}

RasterBandCacheBlocks::~RasterBandCacheBlocks(){

}

AutoPtr<RasterBand>RasterBandCacheBlocks::GetRasterBand(){
    return rasterBand;
}

float RasterBandCacheBlocks::GetPixel(int iX,int iY){
    int bx=iX/blockSize;
    int by=iY/blockSize;
    if((bx<0)||(bx>=blockXNum)||(by<0)||(by>=blockYNum)) return rasterBand->GetNodata();
    int blockId=bx+by*blockXNum;
    int fx=iX-bx*blockSize;
    int fy=iY-by*blockSize;
    int bw=blockSize;
    if(bx*blockSize+bw>desp.cols) bw=desp.cols-bx*blockSize;
    AutoPtr<FixBuffer>fxb=blocks[blockId];
    if(fxb==nullptr){
        int bh=blockSize;
        if(by*blockSize+bh>desp.cols) bh=desp.rows-by*blockSize;
        fxb=new FixBuffer(bw*bh*sizeof(float));
        float*fVs=(float*)fxb->GetBuffer();
        blocks[blockId]=fxb;
        rasterBand->GetBlockData(bx*blockSize,by*blockSize,bw,bh,bw,bh,fVs);
    }
    return ((float*)fxb->GetBuffer())[fx+fy*bw];
}


}
