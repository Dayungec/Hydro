#include "imagelump.h"
namespace SGIS{

Erode::Erode(BYTE*img, LONG imgWidth, LONG imgHeight, LONG imgCode, bool excludeBorder)
{
    this->img = img;
    this->imgWidth = imgWidth;
    this->imgHeight = imgHeight;
    this->imgCode = imgCode;
    this->excludeBorder = excludeBorder;
}

Erode::~Erode()
{

}

void Erode::Compute(BYTE*output, LONG times) {
    for (int k = imgHeight * imgWidth - 1; k >= 0; k--) {
        output[k] = ((img[k] == imgCode) ? 1 : 0);
    }
    LONG Rows = this->imgHeight;
    LONG Cols = this->imgWidth;
    bool IsBorder;
    LONG Pos;
    int oi, oj;
    for (int i = 0; i < Rows; i++)
    {
        Pos = i * Cols;
        for (int j = 0; j < Cols; j++)
        {
            if (0 == output[Pos])
            {
                Pos++;
                continue;
            }
            IsBorder = false;
            for (int p = 0; p < 4; p++)
            {
                oi = i + aroundy4[p];
                oj = j + aroundx4[p];
                if ((oi < 0) || (oi >= Rows) || (oj < 0) || (oj >= Cols))
                {
                    if(excludeBorder) continue;
                    IsBorder = true;
                    break;
                }
                LONG ppPos = oi * Cols + oj;
                if (0 == output[ppPos])
                {
                    IsBorder = true;
                    break;
                }
            }
            if (IsBorder)
            {
                output[Pos]=2;
            }
            Pos++;
        }
    }
    LONG OtherPos;
    int FromState = 2;
    int ToState = 3;
    for (int k = 0; k < times; k++)
    {
        for (int i = 0; i < Rows; i++)
        {
            for (int j = 0; j < Cols; j++)
            {
                Pos = i * Cols + j;
                if (FromState != output[Pos]) continue;
                if (k < times - 1)
                {
                    for (int p = 0; p < 4; p++)
                    {
                        oi = i + aroundy4[p];
                        oj = j + aroundx4[p];
                        if ((oi < 0) || (oi >= Rows) || (oj < 0) || (oj >= Cols)) continue;
                        OtherPos = oi * Cols + oj;
                        if (1 == output[OtherPos])
                        {
                            output[OtherPos]=ToState;
                        }
                    }
                }
                output[Pos]=0;
            }
        }
        if (ToState == 3)
        {
            FromState = 3;
            ToState = 2;
        }
        else
        {
            FromState = 2;
            ToState = 3;
        }
    }
}
void Erode::Compute(LONG times) {
    Compute(img,times);
}
bool Erode::ComputeOneTime() {
    BYTE*output = img;
    if (imgCode != 1) {
        for (int k = imgHeight * imgWidth - 1; k >= 0; k--) {
            output[k] = ((img[k] == imgCode) ? 1 : 0);
        }
    }
    LONG Rows = this->imgHeight;
    LONG Cols = this->imgWidth;
    bool IsBorder;
    LONG Pos;
    int oi, oj;
    bool hasBorder = false;
    for (int i = 0; i < Rows; i++)
    {
        Pos = i * Cols;
        for (int j = 0; j < Cols; j++)
        {
            if (0 == output[Pos])
            {
                Pos++;
                continue;
            }
            IsBorder = false;
            for (int p = 0; p < 4; p++)
            {
                oi = i + aroundy4[p];
                oj = j + aroundx4[p];
                if ((oi < 0) || (oi >= Rows) || (oj < 0) || (oj >= Cols))
                {
                    if (excludeBorder) continue;
                    IsBorder = true;
                    break;
                }
                LONG ppPos = oi * Cols + oj;
                if (0 == output[ppPos])
                {
                    IsBorder = true;
                    break;
                }
            }
            if (IsBorder)
            {
                hasBorder = true;
                output[Pos] = 2;
            }
            Pos++;
        }
    }
    if (!hasBorder) return false;
    for (int k = Cols * Rows - 1; k >= 0; k--) {
        if (output[k] == 2) output[k] = 0;
    }
    return true;
}
Expode::Expode(BYTE*img, LONG imgWidth, LONG imgHeight, LONG imgCode,bool excludeBorder)
{
    this->img = img;
    this->imgWidth = imgWidth;
    this->imgHeight = imgHeight;
    this->imgCode = imgCode;
    this->excludeBorder = excludeBorder;
}

Expode::~Expode()
{

}

void Expode::Compute(BYTE*output, LONG times,BYTE outerCode) {
    for (int k = imgHeight * imgWidth - 1; k >= 0; k--) {
        output[k] = ((img[k] == imgCode) ? 1 : 0);
    }
    LONG Rows = this->imgHeight;
    LONG Cols = this->imgWidth;
    bool IsBorder;
    int oi, oj;
    LONG Pos;
    for (int i = 0; i < Rows; i++)
    {
        Pos = i * Cols;
        for (int j = 0; j < Cols; j++)
        {
            if (0 == output[Pos])
            {
                Pos++;
                continue;
            }
            IsBorder = false;
            for (int p = 0; p < 4; p++)
            {
                oi = i + aroundy4[p];
                oj = j + aroundx4[p];
                if ((oi < 0) || (oi >= Rows) || (oj < 0) || (oj >= Cols))
                {
                    if (excludeBorder) continue;
                    IsBorder = true;
                    break;
                }
                LONG ppPos = oi * Cols + oj;
                if (0 == output[ppPos])
                {
                    IsBorder = true;
                    break;
                }
            }
            if (IsBorder)
            {
                output[Pos]=3;
            }
            Pos++;
        }
    }
    LONG OtherPos;
    int FromState = 3;
    int ToState = 4;
    for (int k = 0; k < times; k++)
    {
        for (int i = 0; i < Rows; i++)
        {
            for (int j = 0; j < Cols; j++)
            {
                Pos = i * Cols + j;
                if (FromState != output[Pos]) continue;
                for (int p = 0; p < 4; p++)
                {
                    oi = i + aroundy4[p];
                    oj = j + aroundx4[p];
                    if ((oi < 0) || (oi >= Rows) || (oj < 0) || (oj >= Cols)) continue;
                    OtherPos = oi * Cols + oj;
                    if (0 == output[OtherPos])
                    {
                        output[OtherPos]=ToState;
                    }
                }
                output[Pos]=2;
            }
        }
        if (ToState == 4)
        {
            FromState = 4;
            ToState = 3;
        }
        else
        {
            FromState = 3;
            ToState = 4;
        }
    }
    for (int k = imgHeight * imgWidth - 1; k >= 0; k--) {
        if (output[k] >1) output[k] = outerCode;
    }
}

void Expode::Compute(LONG times,BYTE outerCode) {
    Compute(img,times,outerCode);
}

InnerExpode::InnerExpode(BYTE*img, LONG imgWidth, LONG imgHeight, LONG imgCode)
{
    this->img = img;
    this->imgWidth = imgWidth;
    this->imgHeight = imgHeight;
    this->imgCode = imgCode;
}

InnerExpode::~InnerExpode()
{

}

void InnerExpode::Compute(BYTE*output) {
    for (int k = imgHeight * imgWidth - 1; k >= 0; k--) {
        output[k] = ((img[k] == imgCode) ? 1 : 0);
    }
    LONG Rows = this->imgHeight;
    LONG Cols = this->imgWidth;
    int counter;
    int oi, oj;
    LONG Pos;
    for (int i = 0; i < Rows; i++)
    {
        Pos = i * Cols;
        for (int j = 0; j < Cols; j++)
        {
            if (1 == output[Pos])
            {
                Pos++;
                continue;
            }
            counter = 0;
            for (int p = 0; p < 4; p++)
            {
                oi = i + aroundy4[p];
                oj = j + aroundx4[p];
                if ((oi < 0) || (oi >= Rows) || (oj < 0) || (oj >= Cols)) continue;
                LONG ppPos = oi * Cols + oj;
                if (1 == output[ppPos])
                {
                    counter++;
                    if(counter>=5) break;
                }
            }
            if (counter>=5)
            {
                output[Pos]=2;
            }
            Pos++;
        }
    }
    Pos=0;
    for (int i = 0; i < Rows; i++)
    {
        for (int j = 0; j < Cols; j++)
        {
            Pos = i * Cols + j;
            if(output[Pos]==2) output[Pos]=1;
        }
    }
}

void InnerExpode::Compute(){
    Compute(img);
}

ImageBorder::ImageBorder(BYTE*img, LONG imgWidth, LONG imgHeight, LONG imgCode)
{
    this->img = img;
    this->imgWidth = imgWidth;
    this->imgHeight = imgHeight;
    this->imgCode = imgCode;
}

ImageBorder::~ImageBorder()
{

}

void ImageBorder::Compute(BYTE*output) {
    for (int k = imgHeight * imgWidth - 1; k >= 0; k--) {
        output[k] = ((img[k] == imgCode) ? 255 : 0);
    }
    LONG Rows = this->imgHeight;
    LONG Cols = this->imgWidth;
    bool IsBorder;
    LONG Pos;
    int oi, oj;
    for (int i = 0; i < Rows; i++)
    {
        Pos = i * Cols;
        for (int j = 0; j < Cols; j++)
        {
            if (0 == output[Pos])
            {
                Pos++;
                continue;
            }
            IsBorder = false;
            for (int p = 0; p < 4; p++)
            {
                oi = i + aroundy4[p];
                oj = j + aroundx4[p];
                if ((oi < 0) || (oi >= Rows) || (oj < 0) || (oj >= Cols))
                {
                    IsBorder = true;
                    break;
                }
                LONG ppPos = oi * Cols + oj;
                if (0 == output[ppPos])
                {
                    IsBorder = true;
                    break;
                }
            }
            if (IsBorder)
            {
                output[Pos]=1;
            }
            Pos++;
        }
    }
    LONG OtherPos;
    int FromState = 1;
    int ToState = 2;
    bool bContinue;
    for (int k = 0; k < 253; k++)
    {
        bContinue=false;
        for (int i = 0; i < Rows; i++)
        {
            for (int j = 0; j < Cols; j++)
            {
                Pos = i * Cols + j;
                if (FromState != output[Pos]) continue;
                for (int p = 0; p < 4; p++)
                {
                    oi = i + aroundy4[p];
                    oj = j + aroundx4[p];
                    if ((oi < 0) || (oi >= Rows) || (oj < 0) || (oj >= Cols)) continue;
                    OtherPos = oi * Cols + oj;
                    if (255 == output[OtherPos])
                    {
                        output[OtherPos]=ToState;
                        bContinue=true;
                    }
                }
            }
        }
        if(!bContinue) break;
        FromState++;
        ToState++;
    }

}

void ImageBorder::Compute(){
    Compute(img);
}

Thin::Thin(BYTE*img, LONG imgWidth, LONG imgHeight, LONG imgCode,bool excludeBorder)
{
    this->img = img;
    this->imgWidth = imgWidth;
    this->imgHeight = imgHeight;
    this->imgCode = imgCode;
    this->excludeBorder = excludeBorder;
}

Thin::~Thin()
{

}

bool Thin::IsSpecialOfThin(BYTE*output, int x, int y)
{
    bool iscontinue;
    int temp0, temp1, temp2;
    for (int k = 1; k <= 2; k++)
    {
        iscontinue = false;
        LONG Pos = x + 1 + (y + 1)*imgWidth;
        int iV = output[Pos];
        if (iV != 0){
            temp0 = output[Pos - 1];
            temp1 = output[Pos + 1];
            if ((temp0 == 0) && (temp1 != 0))
            {
                temp2 = output[Pos + 2];
                if (temp2 == 0)
                {
                    iscontinue = true;
                    x = x + 1;
                    y = y + 1;
                }
            }
            else if ((temp0 != 0) && (temp1 == 0))
            {
                temp2 = output[Pos - 2];
                if (temp2 == 0)
                {
                    iscontinue = true;
                    y = y + 1;
                }
            }
        }
        if (iscontinue == false) return false;
    }
    return iscontinue;
}

bool Thin::CanThined(BYTE*output, int x, int y)
{
    int num = 0, oddnum = 0;
    for (int i = 0; i < 4; i++) bi[i] = 0;
    for (int i = 0; i < 8; i++)
    {
        if ((x + aroundx8[i] < 0) || (x + aroundx8[i] >= imgWidth) || (y + aroundy8[i] < 0) || (y + aroundy8[i] >= imgHeight))
            around[i] = 0;
        else
            around[i] = output[x + aroundx8[i] + (y + aroundy8[i])*imgWidth];
        if (around[i] != 0)
        {
            around[i] = 1;
            num++;
            if (i % 2 == 1)
            {
                oddnum = oddnum + 1;
                bi[(i - 1) / 2] = 1;
                bi[(i + 1) / 2 % 4] = 1;
            }
        }
    }
    switch (oddnum)
    {
    case 0:
    {
        if (num > 1) return false;
        if ((num == 1) && (around[0] == 0)) return false;
        return true;
    }
    case 1:
    {
        if ((bi[0] == 0) && (around[0] == 1))
            return false;
        else if ((bi[1] == 0) && (around[2] == 1))
            return false;
        else if ((bi[2] == 0) && (around[4] == 1))
            return false;
        else if ((bi[3] == 0) && (around[6] == 1))
            return false;
        break;
    }
    case 2:
    {
        if (bi[0] + bi[1] + bi[2] + bi[3] == 4)
            return false;
        else
        {
            if ((bi[0] == 0) && (around[0] == 1))
                return false;
            else if ((bi[1] == 0) && (around[2] == 1))
                return false;
            else if ((bi[2] == 0) && (around[4] == 1))
                return false;
            else if ((bi[3] == 0) && (around[6] == 1))
                return false;
            break;
        }
    }
    default:
    {
        if (num > 6) return false;
        return true;
    }
    }
    if (num == 2)
    {
        if ((around[5] == 1) && (y < imgHeight - 3) && (x > 2) && (x < imgWidth - 4))
        {
            if (around[4] == 1)
            {
                if (0 == output[x + 2 + (y + 1)*imgWidth]) return !(IsSpecialOfThin(output, x, y + 1));
            }
            else if (around[6] == 1)
            {
                if (0 == output[x - 2 + (y + 1)*imgWidth]) return !(IsSpecialOfThin(output, x - 1, y + 1));
            }
        }
    }
    else if (num == 1)
    {
        if ((x - 1 >= 0) && (y - 2 >= 0))
        {
            if (0 == output[x - 1 + (y - 2)*imgWidth]) return false;
        }
        if ((x - 2 >= 0) && (y - 1 >= 0))
        {
            if (0 == output[x - 2 + (y - 1)*imgWidth]) return false;
        }
    }
    return true;
}

void Thin::ThinArray(BYTE*output)
{
    long Cols = this->imgWidth;
    long Rows = this->imgHeight;
    long regionxnum, regionynum;
    regionxnum = Cols / 25;
    regionynum = Rows / 25;
    if (regionxnum * 25 < Cols) regionxnum++;
    if (regionynum * 25 < Rows) regionynum++;
    if (regionxnum < 1) regionxnum = 1;
    if (regionynum < 1) regionynum = 1;
    int difx, dify;
    difx = 25;
    dify = 25;
    long allnum = regionxnum * regionynum;
    long*ptnum = new long[allnum];
    LONG i, j;
    int m;
    long tempi, tempj;
    long k;
    for (k = allnum - 1; k >= 0; k--)
        ptnum[k] = 0;
    long currentx, currenty;
    LONG Pos, oPos;
    for (i = 0; i < Rows; i++)
    {
        currenty = i / dify;
        if (currenty >= regionynum) currenty = regionynum - 1;
        for (j = 0; j < Cols; j++)
        {
            Pos = j + i * Cols;
            if (1 != output[Pos]) continue;
            bool isborder = false;
            for (m = 0; m <= 3; m++)
            {
                tempj = j + aroundx4[m];
                tempi = i + aroundy4[m];
                if ((tempj < 0) || (tempj >= Cols) || (tempi < 0) || (tempi >= Rows))
                {
                    if(excludeBorder) continue;
                    isborder = true;
                    break;
                }
                oPos = tempj + tempi * Cols;
                if (0 == output[oPos])
                {
                    isborder = true;
                    break;
                }
            }
            if (isborder == true)
            {
                output[Pos] = 2;
                currentx = j / difx;
                if (currentx >= regionxnum) currentx = regionxnum - 1;
                ptnum[currenty*regionxnum + currentx]++;
            }
        }
    }
    int times = 0;
    long currentindex;
    long fromi, fromj, toj, toi;
    long tempcurrentx, tempcurrenty;
    bool iscontinue;
    int fromborder = 2, toborder = 3;
    do
    {
        iscontinue = false;
        for (currenty = 0; currenty < regionynum; currenty++)
        {
            fromi = currenty * dify;
            if (currenty < regionynum - 1)
                toi = fromi + dify;
            else
                toi = Rows;
            currentindex = currenty * regionxnum;
            for (currentx = 0; currentx < regionxnum; currentx++)
            {
                if (ptnum[currentindex] == 0)
                {
                    currentindex++;
                    continue;
                }
                fromj = currentx * difx;
                if (currentx < regionxnum - 1)
                    toj = fromj + difx;
                else
                    toj = Cols;
                for (i = fromi; i < toi; i++)
                {
                    for (j = fromj; j < toj; j++)
                    {
                        Pos = j + i * Cols;
                        if (fromborder == output[Pos])
                        {
                            ptnum[currentindex]--;
                            if (CanThined(output, j, i))
                            {
                                output[Pos]=0;
                                for (m = 0; m <= 3; m++)
                                {
                                    tempj = j + aroundx4[m];
                                    tempi = i + aroundy4[m];
                                    if ((tempi < 0) || (tempi >= Rows) || (tempj < 0) || (tempj >= Cols)) continue;
                                    oPos = tempj + tempi * Cols;
                                    if (1 == output[oPos])
                                    {
                                        output[oPos]=toborder;
                                        tempcurrenty = tempi / dify;
                                        if (tempcurrenty >= regionynum) tempcurrenty = regionynum - 1;
                                        tempcurrentx = tempj / difx;
                                        if (tempcurrentx >= regionxnum) tempcurrentx = regionxnum - 1;
                                        ptnum[regionxnum*tempcurrenty + tempcurrentx]++;
                                        iscontinue = true;
                                    }
                                }
                            }
                            else
                                output[Pos]=1;
                        }
                    }
                }
                currentindex++;
            }
        }
        if (toborder == 2)
        {
            fromborder = 2;
            toborder = 3;
        }
        else
        {
            fromborder = 3;
            toborder = 2;
        }
        times++;
    } while (iscontinue);
    delete[]ptnum;
    for (int k = imgHeight * imgWidth - 1; k >= 0; k--) {
        if (output[k] == fromborder) output[k] = 1;
    }
}

void Thin::ThinArray(BYTE*output,BYTE*mask){
    long Cols = this->imgWidth;
    long Rows = this->imgHeight;
    long regionxnum, regionynum;
    regionxnum = Cols / 25;
    regionynum = Rows / 25;
    if (regionxnum * 25 < Cols) regionxnum++;
    if (regionynum * 25 < Rows) regionynum++;
    if (regionxnum < 1) regionxnum = 1;
    if (regionynum < 1) regionynum = 1;
    int difx, dify;
    difx = 25;
    dify = 25;
    long allnum = regionxnum * regionynum;
    long*ptnum = new long[allnum];
    LONG i, j;
    int m;
    long tempi, tempj;
    long k;
    for (k = allnum - 1; k >= 0; k--)
        ptnum[k] = 0;
    long currentx, currenty;
    LONG Pos, oPos;
    for (i = 0; i < Rows; i++)
    {
        currenty = i / dify;
        if (currenty >= regionynum) currenty = regionynum - 1;
        for (j = 0; j < Cols; j++)
        {
            Pos = j + i * Cols;
            if (1 != output[Pos]) continue;
            bool isborder = false;
            for (m = 0; m <= 3; m++)
            {
                tempj = j + aroundx4[m];
                tempi = i + aroundy4[m];
                if ((tempj < 0) || (tempj >= Cols) || (tempi < 0) || (tempi >= Rows))
                {
                    if(excludeBorder) continue;
                    isborder = true;
                    break;
                }
                oPos = tempj + tempi * Cols;
                if (0 == output[oPos])
                {
                    isborder = true;
                    break;
                }
            }
            if (isborder == true)
            {
                output[Pos] = 2;
                currentx = j / difx;
                if (currentx >= regionxnum) currentx = regionxnum - 1;
                ptnum[currenty*regionxnum + currentx]++;
            }
        }
    }
    int times = 0;
    long currentindex;
    long fromi, fromj, toj, toi;
    long tempcurrentx, tempcurrenty;
    bool iscontinue;
    int fromborder = 2, toborder = 3;
    do
    {
        iscontinue = false;
        for (currenty = 0; currenty < regionynum; currenty++)
        {
            fromi = currenty * dify;
            if (currenty < regionynum - 1)
                toi = fromi + dify;
            else
                toi = Rows;
            currentindex = currenty * regionxnum;
            for (currentx = 0; currentx < regionxnum; currentx++)
            {
                if (ptnum[currentindex] == 0)
                {
                    currentindex++;
                    continue;
                }
                fromj = currentx * difx;
                if (currentx < regionxnum - 1)
                    toj = fromj + difx;
                else
                    toj = Cols;
                for (i = fromi; i < toi; i++)
                {
                    for (j = fromj; j < toj; j++)
                    {
                        Pos = j + i * Cols;
                        if (fromborder == output[Pos])
                        {
                            ptnum[currentindex]--;
                            if(mask[Pos]!=0){
                                output[Pos]=1;
                            }
                            else if (CanThined(output, j, i))
                            {
                                output[Pos]=0;
                                for (m = 0; m <= 3; m++)
                                {
                                    tempj = j + aroundx4[m];
                                    tempi = i + aroundy4[m];
                                    if ((tempi < 0) || (tempi >= Rows) || (tempj < 0) || (tempj >= Cols)) continue;
                                    oPos = tempj + tempi * Cols;
                                    if (1 == output[oPos])
                                    {
                                        output[oPos]=toborder;
                                        tempcurrenty = tempi / dify;
                                        if (tempcurrenty >= regionynum) tempcurrenty = regionynum - 1;
                                        tempcurrentx = tempj / difx;
                                        if (tempcurrentx >= regionxnum) tempcurrentx = regionxnum - 1;
                                        ptnum[regionxnum*tempcurrenty + tempcurrentx]++;
                                        iscontinue = true;
                                    }
                                }
                            }
                            else
                                output[Pos]=1;
                        }
                    }
                }
                currentindex++;
            }
        }
        if (toborder == 2)
        {
            fromborder = 2;
            toborder = 3;
        }
        else
        {
            fromborder = 3;
            toborder = 2;
        }
        times++;
    } while (iscontinue);
    delete[]ptnum;
    for (int k = imgHeight * imgWidth - 1; k >= 0; k--) {
        if (output[k] == fromborder) output[k] = 1;
    }
}

void Thin::ThinArray(BYTE*output, vector<Point2D<int>>&reservedPoints){
    long Cols = this->imgWidth;
    long Rows = this->imgHeight;
    long regionxnum, regionynum;
    regionxnum = Cols / 25;
    regionynum = Rows / 25;
    if (regionxnum * 25 < Cols) regionxnum++;
    if (regionynum * 25 < Rows) regionynum++;
    if (regionxnum < 1) regionxnum = 1;
    if (regionynum < 1) regionynum = 1;
    int difx, dify;
    difx = 25;
    dify = 25;
    long allnum = regionxnum * regionynum;
    long*ptnum = new long[allnum];
    LONG i, j;
    int m;
    long tempi, tempj;
    long k;
    for (k = allnum - 1; k >= 0; k--)
        ptnum[k] = 0;
    long currentx, currenty;
    LONG Pos, oPos;
    for (i = 0; i < Rows; i++)
    {
        currenty = i / dify;
        if (currenty >= regionynum) currenty = regionynum - 1;
        for (j = 0; j < Cols; j++)
        {
            Pos = j + i * Cols;
            if (1 != output[Pos]) continue;
            bool isborder = false;
            for (m = 0; m <= 3; m++)
            {
                tempj = j + aroundx4[m];
                tempi = i + aroundy4[m];
                if ((tempj < 0) || (tempj >= Cols) || (tempi < 0) || (tempi >= Rows))
                {
                    if(excludeBorder) continue;
                    isborder = true;
                    break;
                }
                oPos = tempj + tempi * Cols;
                if (0 == output[oPos])
                {
                    isborder = true;
                    break;
                }
            }
            if (isborder == true)
            {
                output[Pos] = 2;
                currentx = j / difx;
                if (currentx >= regionxnum) currentx = regionxnum - 1;
                ptnum[currenty*regionxnum + currentx]++;
            }
        }
    }
    int times = 0;
    long currentindex;
    long fromi, fromj, toj, toi;
    long tempcurrentx, tempcurrenty;
    bool iscontinue;
    int fromborder = 2, toborder = 3;
    bool bExists;
    do
    {
        iscontinue = false;
        for (currenty = 0; currenty < regionynum; currenty++)
        {
            fromi = currenty * dify;
            if (currenty < regionynum - 1)
                toi = fromi + dify;
            else
                toi = Rows;
            currentindex = currenty * regionxnum;
            for (currentx = 0; currentx < regionxnum; currentx++)
            {
                if (ptnum[currentindex] == 0)
                {
                    currentindex++;
                    continue;
                }
                fromj = currentx * difx;
                if (currentx < regionxnum - 1)
                    toj = fromj + difx;
                else
                    toj = Cols;
                for (i = fromi; i < toi; i++)
                {
                    for (j = fromj; j < toj; j++)
                    {
                        Pos = j + i * Cols;
                        if (fromborder == output[Pos])
                        {
                            ptnum[currentindex]--;
                            bExists=false;
                            for(int l=0;l<reservedPoints.size();l++){
                                if((reservedPoints[l].X==j)&&(reservedPoints[l].Y==i)){
                                    bExists=true;
                                    break;
                                }
                            }
                            if(bExists){
                                output[Pos]=1;
                            }
                            else if (CanThined(output, j, i))
                            {
                                output[Pos]=0;
                                for (m = 0; m <= 3; m++)
                                {
                                    tempj = j + aroundx4[m];
                                    tempi = i + aroundy4[m];
                                    if ((tempi < 0) || (tempi >= Rows) || (tempj < 0) || (tempj >= Cols)) continue;
                                    oPos = tempj + tempi * Cols;
                                    if (1 == output[oPos])
                                    {
                                        output[oPos]=toborder;
                                        tempcurrenty = tempi / dify;
                                        if (tempcurrenty >= regionynum) tempcurrenty = regionynum - 1;
                                        tempcurrentx = tempj / difx;
                                        if (tempcurrentx >= regionxnum) tempcurrentx = regionxnum - 1;
                                        ptnum[regionxnum*tempcurrenty + tempcurrentx]++;
                                        iscontinue = true;
                                    }
                                }
                            }
                            else
                                output[Pos]=1;
                        }
                    }
                }
                currentindex++;
            }
        }
        if (toborder == 2)
        {
            fromborder = 2;
            toborder = 3;
        }
        else
        {
            fromborder = 3;
            toborder = 2;
        }
        times++;
    } while (iscontinue);
    delete[]ptnum;
    for (int k = imgHeight * imgWidth - 1; k >= 0; k--) {
        if (output[k] == fromborder) output[k] = 1;
    }
}

void Thin::Compute(BYTE*output) {
    for (int k = imgHeight * imgWidth - 1; k >= 0; k--) {
        output[k] = ((img[k] == imgCode)?1:0);
    }
    ThinArray(output);
}

void Thin::Compute(BYTE*output,BYTE*mask) {
    for (int k = imgHeight * imgWidth - 1; k >= 0; k--) {
        output[k] = ((img[k] == imgCode)?1:0);
    }
    ThinArray(output,mask);
}

void Thin::ComputeWithReservedPoints(BYTE*output,vector<Point2D<int>>&reservedPoints){
    for (int k = imgHeight * imgWidth - 1; k >= 0; k--) {
        output[k] = ((img[k] == imgCode)?1:0);
    }
    ThinArray(output,reservedPoints);
}

void Thin::Compute() {
    Compute(img);
}

void Thin::ComputeWithMask(BYTE*mask){
    Compute(img,mask);
}

void Thin::ComputeWithReservedPoints(vector<Point2D<int>>&reservedPoints){
    ComputeWithReservedPoints(img,reservedPoints);
}

ImageLump::ImageLump() {
    stack = nullptr;
    stackCount = 0;
    minx = miny = maxx = maxy = 0;
    pin = 0;
}
ImageLump::~ImageLump() {
    if (stack != nullptr) delete[]stack;
}

void ImageLump::AllocStack(LONG cols, LONG rows) {
    if (this->cols*this->rows != cols * rows) {
        if (stack != NULL) delete[]stack;
        this->cols = cols;
        this->rows = rows;
        stack = new LONG[cols*rows];
    }
    else {
        this->cols = cols;
        this->rows = rows;
    }
    stackCount = 0;
    minx = miny = maxx = maxy = 0;
    pin = 0;
}
void ImageLump::EmptyStack() {
    stackCount = 0;
    minx = miny = maxx = maxy = 0;
    pin = 0;
}
LONG*ImageLump::GetStack() {
    return stack;
}
bool ImageLump::PushBack(int x, int y) {
    if (stackCount >= cols * rows) return false;
    stack[stackCount] = x + y * cols;
    if (stackCount == 0) {
        minx = maxx = x;
        miny = maxy = y;
    }
    else {
        if (x < minx) minx = x;
        if (x > maxx) maxx = x;
        if (y < miny) miny = y;
        if (y > maxy) maxy = y;
    }
    stackCount++;
    return true;
}
bool ImageLump::PushBack(LONG pos){
    if (stackCount >= cols * rows) return false;
    int x=pos%cols;
    int y=pos/cols;
    stack[stackCount] = x + y * cols;
    if (stackCount == 0) {
        minx = maxx = x;
        miny = maxy = y;
    }
    else {
        if (x < minx) minx = x;
        if (x > maxx) maxx = x;
        if (y < miny) miny = y;
        if (y > maxy) maxy = y;
    }
    stackCount++;
    return true;
}
LONG ImageLump::PopUp() {
    if (pin < stackCount) return stack[pin++];
    return -1;
}

void ImageLump::Reset(){
    pin=0;
}

bool ImageLump::IntersectBorder() {
    if(stackCount==0) return false;
    return ((minx <= 0) || (maxx >= cols - 1) || (miny <= 0) || (maxy >= rows - 1));
}
LONG ImageLump::GetCount() {
    return stackCount;
}
LONG ImageLump::GetLeft() {
    return minx;
}
LONG ImageLump::GetTop() {
    return miny;
}
LONG ImageLump::GetLumpWidth() {
    return (maxx - minx + 1);
}
LONG ImageLump::GetLumpHeight() {
    return (maxy - miny + 1);
}
LONG ImageLump::GetCols() {
    return cols;
}
LONG ImageLump::GetRows() {
    return rows;
}
void ImageLump::Expand(int w) {
    minx -= w;
    if (minx < 0) minx = 0;
    maxx += w;
    if (maxx >= cols) maxx = cols - 1;
    miny -= w;
    if (miny < 0) miny = 0;
    maxy += w;
    if (maxy >= rows) maxy = rows - 1;
}
double ImageLump::GetDensity() {
    return ((double)stackCount) / (maxx - minx + 1) / (maxy - miny + 1);
}

LONG ImageLump::GetPixel(int nIndex) {
    return stack[nIndex];
}

AutoPtr<ImageLump>ImageLump::CreateCompactLump() {
    ImageLump*clmp = new ImageLump();
    clmp->stack = new LONG[stackCount];
    memcpy(clmp->stack, stack, stackCount * sizeof(LONG));
    clmp->minx = minx;
    clmp->miny = miny;
    clmp->maxx = maxx;
    clmp->maxy = maxy;
    clmp->cols = cols;
    clmp->rows = rows;
    clmp->stackCount = stackCount;
    return clmp;
 }
ImageLumpBlock::ImageLumpBlock() {
    this->buffer = nullptr;
    cols=0;
    rows=0;
    lumpWidth=0;
    lumpHeight=0;
    left=0;
    top=0;
}

ImageLumpBlock::~ImageLumpBlock() {
    if (buffer != nullptr) delete[]buffer;
}

bool ImageLumpBlock::CreateImageBlock(int cols,int rows){
    if((cols==0)||(rows==0)) return false;
    this->lumpWidth =cols;
    this->lumpHeight=rows;
    this->left = 0;
    this->top =  0;
    int formerSize=this->cols*this->rows;
    if(formerSize!=cols*rows){
        if (buffer != nullptr) delete []buffer;
        buffer = new BYTE[cols*rows];
    }
    this->cols=cols;
    this->rows=rows;
    return true;
}

bool ImageLumpBlock::CreateImageBlock(ImageLump*lump) {
    int stackCount = lump->GetCount();
    this->lumpWidth = lump->GetLumpWidth();
    this->lumpHeight=lump->GetLumpHeight();
    if((lumpWidth==0)||(lumpHeight==0)) return false;
    this->left = lump->GetLeft();
    this->top = lump->GetTop();
    int formerSize=cols*rows;
    cols = lump->GetCols();
    rows = lump->GetRows();
    if(formerSize!=cols*rows){
        if (buffer != nullptr) delete []buffer;
        buffer = new BYTE[cols*rows];
    }
    this->cols=lump->GetCols();
    this->rows=lump->GetRows();
    memset(buffer, 0, lumpWidth*lumpHeight);
    LONG*stack = lump->GetStack();
    for (int k = 0; k < stackCount; k++) {
        LONG pos = stack[k];
        int ix = pos % cols;
        int iy = pos / cols;
        buffer[(ix - left) + (iy - top)*lumpWidth] = 1;
    }
    return true;
}

bool ImageLumpBlock::UpdateImageBlock(AutoArray<Point2D<int>>&pointArray){
    if (buffer == nullptr) return false;
    int ptSize=pointArray.GetSize();
    if(ptSize==0) return false;
    int minx,miny,maxx,maxy;
    minx=maxx=pointArray[0].X;
    miny=maxy=pointArray[0].Y;
    for(int k=1;k<ptSize;k++){
        Point2D<int>&pt=pointArray[k];
        if((pt.X<0)||(pt.X>=cols)||(pt.Y<0)||(pt.Y>=rows)) return false;
        if(pt.X<minx) minx=pt.X;
        if(pt.Y<miny) miny=pt.Y;
        if(pt.X>maxx) maxx=pt.X;
        if(pt.Y>maxy) maxy=pt.Y;
    }
    this->left=minx;
    this->top=miny;
    this->lumpWidth=maxx-minx+1;
    this->lumpHeight=maxy-miny+1;
    memset(buffer, 0, lumpWidth*lumpHeight);
    for(int k=0;k<ptSize;k++){
        Point2D<int>&pt=pointArray[k];
        int nPos=pt.X-left+(pt.Y-top)*lumpWidth;
        buffer[nPos]=1;
    }
    return true;
}

bool ImageLumpBlock::UpdateImageBlock(vector<Point2D<int>>&pointArray){
    if (buffer == nullptr) return false;
    int ptSize=pointArray.size();
    if(ptSize==0) return false;
    int minx,miny,maxx,maxy;
    minx=maxx=pointArray[0].X;
    miny=maxy=pointArray[0].Y;
    for(int k=1;k<ptSize;k++){
        Point2D<int>&pt=pointArray[k];
        if((pt.X<0)||(pt.X>=cols)||(pt.Y<0)||(pt.Y>=rows)) return false;
        if(pt.X<minx) minx=pt.X;
        if(pt.Y<miny) miny=pt.Y;
        if(pt.X>maxx) maxx=pt.X;
        if(pt.Y>maxy) maxy=pt.Y;
    }
    this->left=minx;
    this->top=miny;
    this->lumpWidth=maxx-minx+1;
    this->lumpHeight=maxy-miny+1;
    memset(buffer, 0, lumpWidth*lumpHeight);
    for(int k=0;k<ptSize;k++){
        Point2D<int>&pt=pointArray[k];
        int nPos=pt.X-left+(pt.Y-top)*lumpWidth;
        buffer[nPos]=1;
    }
    return true;
}

bool ImageLumpBlock::CopyFrom(ImageLumpBlock*other){
    if((other->GetLumpWidth()!=lumpWidth)||(other->GetLumpHeight()!=lumpHeight)) return false;
    memcpy(buffer,other->buffer,lumpWidth*lumpHeight);
    return true;
}

int ImageLumpBlock::GetMaxValue(){
    if (buffer == nullptr) return 0;
    int maxv=0;
    for(int k=lumpWidth*lumpHeight-1;k>=0;k--){
        if(buffer[k]>maxv) maxv=buffer[k];
    }
    return maxv;
}

bool ImageLumpBlock::MakeForeGroundOne(){
    if (buffer == nullptr) return false;
    for(int k=lumpWidth*lumpHeight-1;k>=0;k--){
        if(buffer[k]>0) buffer[k]=1;
    }
    return true;
}

void ImageLumpBlock::ExpandAndErode(int times) {
    Expode expd(buffer, lumpWidth, lumpHeight, 1);
    expd.Compute(times);
    Erode erd(buffer, lumpWidth, lumpHeight, 1);
    erd.Compute(times);
}

void ImageLumpBlock::ErodeAndExpand(int times) {
    Erode erd(buffer, lumpWidth, lumpHeight, 1);
    erd.Compute(times);
    Expode expd(buffer, lumpWidth, lumpHeight, 1);
    expd.Compute(times);
}

void ImageLumpBlock::ExpodeOnly(int times,bool labelDiffer){
    Expode expd(buffer, lumpWidth, lumpHeight, 1);
    if(labelDiffer)
        expd.Compute(times,2);
    else
        expd.Compute(times);
}

void ImageLumpBlock::ThinImage(){
    Thin thin(buffer, lumpWidth, lumpHeight, 1);
    thin.Compute();
}

void ImageLumpBlock::ThinImage(BYTE*mask){
    Thin thin(buffer, lumpWidth, lumpHeight, 1);
    thin.ComputeWithMask(mask);
}

void ImageLumpBlock::ThinImage(vector<Point2D<int>>&reservedPoints){
    Thin thin(buffer, lumpWidth, lumpHeight, 1);
    for(int k=0;k<reservedPoints.size();k++){
        Point2D<int>&dpt=reservedPoints[k];
        dpt.X-=this->left;
        dpt.Y-=this->top;
    }
    thin.ComputeWithReservedPoints(reservedPoints);
}

void ImageLumpBlock::ExtractBorder(){
    ImageBorder ib(buffer, lumpWidth, lumpHeight, 1);
    ib.Compute();
}

void ImageLumpBlock::UpdateImageLump(ImageLump*lump) {
    lump->EmptyStack();
    LONG pos = 0;
    for (int i = 0; i < lumpHeight; i++) {
        for (int j = 0; j < lumpWidth; j++) {
            if (buffer[pos] > 0) lump->PushBack(j + left, i + top);
            pos++;
        }
    }
}

BYTE*ImageLumpBlock::GetBuffer() {
    return buffer;
}
LONG ImageLumpBlock::GetLeft(){
    return left;
}
LONG ImageLumpBlock::GetTop(){
    return top;
}
LONG ImageLumpBlock::GetLumpWidth(){
    return lumpWidth;
}
LONG ImageLumpBlock::GetLumpHeight(){
    return lumpHeight;
}
LONG ImageLumpBlock::GetCols() {
    return cols;
}
LONG ImageLumpBlock::GetRows() {
    return rows;
}

AutoPtr<RasterDataset>ImageLumpBlock::CreateRasterDataset(RasterTarget*target){
    if((lumpWidth==0)||(lumpHeight==0)) return nullptr;
    AutoPtr<SAEnvironment>pEnvi(new SAEnvironment());
    pEnvi->Left()=0;
    pEnvi->Top()=0;
    pEnvi->XCellSize()=1;
    pEnvi->YCellSize()=1;
    pEnvi->Cols()=lumpWidth;
    pEnvi->Rows()=lumpHeight;
    pEnvi->SetSpatialReference(AutoPtr<SpatialReference>(new SpatialReference()));
    AutoPtr<RasterDataset>rd=target->CreateRasterDataset(pEnvi.get(),rdtByte,1,0);
    if(rd==nullptr) return nullptr;
    AutoPtr<RasterBand>band=rd->GetRasterBand(0);
    FLOAT*fVs=new FLOAT[lumpWidth*lumpHeight];
    for(int k=lumpWidth*lumpHeight-1;k>=0;k--){
        fVs[k]=buffer[k];
    }
    band->SaveBlockData(0,0,lumpWidth,lumpHeight,fVs);
    delete []fVs;
    return rd;
}


ImageLumpPolygon::ImageLumpPolygon() {
    this->lump = nullptr;
    Cols = 0;
    Rows = 0;
    imageCols = 0;
    imageRows = 0;
    b0 = b1 = nullptr;
}
ImageLumpPolygon::~ImageLumpPolygon() {
    if (b0 != nullptr) delete[]b0;
    if (b1 != nullptr) delete[]b1;
    for (int k = rings.size() - 1; k >= 0; k--)
    {
        if (rings[k]->GetPolyIndex() == 0) delete rings[k];
    }
    rings.clear();
    for (int k = polys.size() - 1; k >= 0; k--) delete polys[k];
    polys.clear();
}

void ImageLumpPolygon::InitialBorder() {
    long AllNum = 0;
    int nx, ny;
    int V1, V2;
    float fValue;
    BYTE*data = imageLumpBlock.GetBuffer();
    for (int i = 0; i <= Rows; i++)
    {
        for (int j = 0; j <= Cols; j++)
        {
            nx = j; ny = i - 1;
            if ((nx >= Cols) || (ny < 0))
                V1 = INT_NO_DATA;
            else
            {
                fValue = data[ny*Cols + nx];
                V1 = fValue;
            }
            nx = j; ny = i;
            if ((nx >= Cols) || (ny >= Rows))
                V2 = INT_NO_DATA;
            else
            {
                fValue = data[ny*Cols + nx];
                V2 = fValue;
            }
            if (V1 == V2)
                b0[AllNum] = 0;
            else
                b0[AllNum] = 1;
            nx = j - 1; ny = i;
            if ((nx < 0) || (ny >= Rows))
                V1 = INT_NO_DATA;
            else
            {
                fValue = data[ny*Cols + nx];
                V1 = fValue;
            }
            if (V1 == V2)
                b1[AllNum] = 0;
            else
                b1[AllNum] = 1;
            AllNum++;
        }
    }

}

int ImageLumpPolygon::GetStateOfBorderPixel(int x, int y)
{
    OrderArray<int>pArray;
    int nx, ny;
    int FVs[4];
    nx = x - 1; ny = y - 1;
    float fV;
    if ((nx < 0) || (ny < 0))
        FVs[0] = INT_NO_DATA;
    else
    {
        fV = data[ny*Cols + nx];
        FVs[0] = fV;
    }
    nx = x; ny = y - 1;
    if ((nx >= Cols) || (ny < 0))
        FVs[1] = INT_NO_DATA;
    else
    {
        fV = data[ny*Cols + nx];
        FVs[1] = fV;
    }
    nx = x; ny = y;
    if ((nx >= Cols) || (ny >= Rows))
        FVs[2] = INT_NO_DATA;
    else
    {
        fV = data[ny*Cols + nx];
        FVs[2] = fV;
    }
    nx = x - 1; ny = y;
    if ((nx < 0) || (ny >= Rows))
        FVs[3] = INT_NO_DATA;
    else
    {
        fV = data[ny*Cols + nx];
        FVs[3] = fV;
    }
    if (FVs[0] == FVs[2])
    {
        if ((FVs[1] != FVs[0]) && (FVs[3] != FVs[0])) return 2;
    }
    if (FVs[1] == FVs[3])
    {
        if ((FVs[0] != FVs[1]) && (FVs[2] != FVs[3])) return 2;
    }
    for (int k = 0; k < 4; k++) pArray.Add(FVs[k]);
    if (pArray.GetSize() > 2) return 2;
    return 0;
}

void ImageLumpPolygon::GetNextPoint(int Deri, int&Posi, int&x, int&y, int ri)
{
    switch (Posi)
    {
    case 0:
        x++;
        break;
    case 1:
        y++;
        break;
    case 2:
        x--;
        break;
    case 3:
        y--;
        break;
    }
    Posi = Posi + 2;
    if (Posi >= 4) Posi -= 4;
    int NextP;
    int State;
    long Pos;
    if (Deri == 0)
    {
        for (int p = Posi - 1; p > Posi - 4; p--)
        {
            NextP = p;
            if (NextP < 0) NextP += 4;
            if (NextP == 0)
            {
                if ((x > Cols) || (y > Rows)) continue;
                Pos = y * (Cols + 1) + x;
                State = b0[Pos];
                if (State == 1)
                {
                    b0[Pos] = 2;
                    break;
                }
                else if (State == 3)
                {
                    b0[Pos] = 0;
                    break;
                }
            }
            else if (NextP == 1)
            {
                if ((y > Rows) || (x > Cols)) continue;
                Pos = y * (Cols + 1) + x;
                State = b1[Pos];
                if (State == 1)
                {
                    b1[Pos] = 2;
                    break;
                }
                else if (State == 3)
                {
                    b1[Pos] = 0;
                    break;
                }
            }
            else if (NextP == 2)
            {
                if ((x <= 0) || (y > Rows)) continue;
                Pos = y * (Cols + 1) + x - 1;
                State = b0[Pos];
                if (State == 1)
                {
                    b0[Pos] = 3;
                    break;
                }
                else if (State == 2)
                {
                    b0[Pos] = 0;
                    break;
                }
            }
            else if (NextP == 3)
            {
                if ((x > Cols) || (y <= 0)) continue;
                Pos = (y - 1)*(Cols + 1) + x;
                State = b1[Pos];
                if (State == 1)
                {
                    b1[Pos] = 3;
                    break;
                }
                else if (State == 2)
                {
                    b1[Pos] = 0;
                    break;
                }
            }
        }
        Posi = NextP;
    }
    else
    {
        for (int p = Posi - 1; p > Posi - 4; p--)
        {
            NextP = p;
            if (NextP < 0) NextP += 4;
            if (NextP == 0)
            {
                if ((x > Cols) || (y > Rows)) continue;
                Pos = y * (Cols + 1) + x;
                State = b0[Pos];
                if (State == 1)
                {
                    b0[Pos] = 2;
                    break;
                }
                else if (State == 3)
                {
                    b0[Pos] = 0;
                    break;
                }
            }
            else if (NextP == 1)
            {
                if ((x > Cols) || (y > Rows)) continue;
                Pos = y * (Cols + 1) + x;
                State = b1[Pos];
                if (State == 1)
                {
                    b1[Pos] = 2;
                    break;
                }
                else if (State == 3)
                {
                    b1[Pos] = 0;
                    break;
                }
            }
            else if (NextP == 2)
            {
                if ((x <= 0) || (y > Rows)) continue;
                Pos = y * (Cols + 1) + x - 1;
                State = b0[Pos];
                if (State == 1)
                {
                    b0[Pos] = 3;
                    break;
                }
                else if (State == 2)
                {
                    b0[Pos] = 0;
                    break;
                }
            }
            else if (NextP == 3)
            {
                if ((x > Cols) || (y <= 0)) continue;
                Pos = (y - 1)*(Cols + 1) + x;
                State = b1[Pos];
                if (State == 1)
                {
                    b1[Pos] = 3;
                    break;
                }
                else if (State == 2)
                {
                    b1[Pos] = 0;
                    break;
                }
            }
        }
        Posi = NextP;
    }
}

bool ImageLumpPolygon::PursueRing(int x, int y) {
    long Posi = y * (Cols + 1) + x;
    int State;
    State = b0[Posi];
    Point2D<double> pt;
    int nx, ny;
    int posi;
    int ri = -1;
    LONG V;
    LONG lV;
    bool HasPursued = false;
    DOUBLE left = lump->GetLeft();
    DOUBLE top = lump->GetTop();
    if ((State == 1) || (State == 3))
    {
        if ((x < Cols) && (y < Rows))
        {
            lV = data[y*Cols + x];
            if (lV == 1)
            {
                RRing*ring = new RRing(0, x, y);
                rings.push_back(ring);
                pt.X = Left + (x + left) * xCellSize;
                pt.Y = Top - (y + top) * yCellSize;
                V = data[y*Cols + x];
                ring->SetRingValue(lV);
                ring->AddPoint(pt, GetStateOfBorderPixel(x, y));
                if (State == 1)
                    b0[Posi] = 2;
                else if (State == 3)
                    b0[Posi] = 0;
                ri = rings.size();
                nx = x; ny = y;
                posi = 0;
                while (true)
                {
                    GetNextPoint(0, posi, nx, ny, ri);
                    pt.X = Left + (nx + left) * xCellSize;
                    pt.Y = Top - (ny + top) * yCellSize;
                    ring->AddPoint(pt, GetStateOfBorderPixel(nx, ny));
                    if ((nx == x) && (ny == y))
                    {
                        break;
                    }
                }
                HasPursued = true;
            }
        }
    }
    State = b1[Posi];
    if ((State == 1) || (State == 3))
    {
        if ((x > 0) && (y < Rows))
        {
            lV = data[y*Cols + x - 1];
            if (lV == 1)
            {
                RRing*ring = new RRing(1, x, y);
                V = data[y*Cols + x - 1];
                ring->SetRingValue(V);
                rings.push_back(ring);
                pt.X = Left + (x + left) * xCellSize;
                pt.Y = Top - (y + top) * yCellSize;
                ring->AddPoint(pt, GetStateOfBorderPixel(x, y));
                if (State == 1)
                    b1[Posi] = 2;
                else if (State == 3)
                    b1[Posi] = 0;
                nx = x; ny = y;
                posi = 1;
                while (true)
                {
                    GetNextPoint(1, posi, nx, ny, ri);
                    pt.X = Left + (nx + left) * xCellSize;
                    pt.Y = Top - (ny + top) * yCellSize;
                    ring->AddPoint(pt, GetStateOfBorderPixel(nx, ny));
                    if ((nx == x) && (ny == y)) break;
                }
                HasPursued = true;
            }
        }
    }
    return HasPursued;
}

void ImageLumpPolygon::RegisterRings() {
    int w = lump->GetLumpWidth();
    int h = lump->GetLumpHeight();
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            if (PursueRing(j, i))
            {
                if (rings.size() >= 65535) break;
            }
        }
    }
}

void ImageLumpPolygon::RegisterPolygons()
{
    int Size = rings.size();
    for (int k = 0; k < Size; k++)
    {
        RRing*ring = rings[k];
        if (ring->GetDeri() == 0)
        {
            RPolygon*poly = new RPolygon();
            poly->AddRing(ring);
            polys.push_back(poly);
            ring->SetPolyIndex(polys.size());
        }
    }
    for (int k = 0; k < Size; k++)
    {
        RRing*ring = rings[k];
        int rV = ring->GetRingValue();
        if (ring->GetDeri() == 1)
        {
            double minarea = 0;
            int minindex = -1;
            for (int j = 0; j < Size; j++)
            {
                RRing*oring = rings[j];
                if (oring->GetDeri() != 0) continue;
                if (oring->GetRingValue() != rV) continue;
                if (oring->IsRingIn(ring))
                {
                    double a = oring->GetArea();
                    if (minindex == -1)
                    {
                        minarea = a;
                        minindex = j;
                    }
                    else if (a < minarea)
                    {
                        minarea = a;
                        minindex = j;
                    }
                }
            }
            if (minindex >= 0)
            {
                RRing*oring = rings[minindex];
                int rIndex = oring->GetPolyIndex() - 1;
                RPolygon*poly = polys[rIndex];
                poly->AddRing(ring);
                ring->SetPolyIndex(rIndex + 1);
            }
        }
    }
}

AutoPtr<Polygon> ImageLumpPolygon::get_Polygon(LONG index) {
    RPolygon*poly = polys[index];
    AutoPtr<Polygon>pPoly(new Polygon());
    int Count = poly->GetCount();
    for (int k = 0; k < Count; k++)
    {
        RRing*ring = poly->GetRing(k);
        AutoPtr<Points>pts(new Points());
        int rCount = ring->GetCount();
        DOUBLE*rawData = ring->GetRawData();
        pts->PutCoordByPtr(rCount, rawData);
        AutoPtr<Ring>pRing(pts);
        pPoly->Add(pRing);
    }
    return pPoly;
}

AutoPtr<FeatureClass> ImageLumpPolygon::CreatePolygonFeatureClass(FeatureClassTarget*target) {
    AutoPtr<TableDescCreator>pCreator(new TableDescCreator());
    pCreator->AddField({"ID", vftInteger, 6, 0});
    pCreator->AddField({"GRIDCODE", vftInteger, 10, 0});
    AutoPtr<TableDesc>pDesc=pCreator->CreateTableDesc();
    AutoPtr<SpatialReference>psp(new SpatialReference());
    AutoPtr<FeatureClass>pfc=target->CreateFeatureClass(psp.get(), vstPolygon, pDesc.get());
    if (pfc == nullptr) return nullptr;
    long m;
    RPolygon*poly;
    long PolyCount = polys.size();
    Point2D<double> pt;
    RRing*ring;
    vector<Variant>fields;
    fields.resize(2);
    string sV;
    AutoPtr<FeatureClassEdit>pEdit(pfc);
    float fV;
    for (m = 0; m < PolyCount; m++)
    {
        poly = polys[m];
        AutoPtr<Polygon>pPoly=get_Polygon(m);
        ring = poly->GetRing(0);
        fV = data[Cols*ring->GetFirstY() + ring->GetFirstX()];
        fields[0]=Variant(m);
        fields[1]=Variant(fV);
        pEdit->AddFeatureEx(pPoly, fields);
    }
    return pfc;
}


vector<AutoPtr<Geometry>> ImageLumpPolygon::ExtractPolygons(ImageLump*lump, DOUBLE Left, DOUBLE Top, DOUBLE xCellSize ,DOUBLE yCellSize, LONG maxIslands) {
    this->lump = lump;
    if (this->imageCols*this->imageRows != lump->GetCols()*lump->GetRows()) {
        if (b0 != nullptr) delete[]b0;
        if (b1 != nullptr) delete[]b1;
        b0 = new BYTE[(lump->GetCols() + 1)*(lump->GetRows() + 1)];
        b1 = new BYTE[(lump->GetCols() + 1)*(lump->GetRows() + 1)];
    }
    for (int k = rings.size() - 1; k >= 0; k--)
    {
        if (rings[k]->GetPolyIndex() == 0) delete rings[k];
    }
    rings.clear();
    for (int k = polys.size() - 1; k >= 0; k--) delete polys[k];
    polys.clear();
    lump->Expand(4);
    this->Left = Left;
    this->Top = Top;
    this->xCellSize = xCellSize;
    this->yCellSize = yCellSize;
    imageCols = lump->GetCols();
    imageRows = lump->GetRows();
    Cols = lump->GetLumpWidth();
    Rows = lump->GetLumpHeight();
    imageLumpBlock.CreateImageBlock(lump);
    imageLumpBlock.ExpandAndErode(2);
    data = imageLumpBlock.GetBuffer();
    InitialBorder();
    RegisterRings();
    RegisterPolygons();
    long PolyCount = polys.size();
    Point2D<double> pt;
    vector<AutoPtr<Geometry>> geos;
    for (int m = 0; m < PolyCount; m++)
    {
        AutoPtr<Polygon>geo=get_Polygon(m);
        LONG ringCount=geo->GetSize();
        if (maxIslands > 0) {
            if (ringCount >= maxIslands + 1) {
                continue;
            }
        }
        geos.push_back(geo);
    }
    return geos;
}

ImageLumpCircle::ImageLumpCircle(int r)
{
    radius = r;
    state = false;
    pixelIndex = 0;
    allIn=false;
}

ImageLumpCircle::~ImageLumpCircle()
{
}

void ImageLumpCircle::CreateCircle()
{
    double difa = atan(1.0 / radius);
    int fx = -1;
    int fy = -1;
    for (double a = 0; a < 6.2831852; a += difa)
    {
        int ix = round(cos(a)*radius);
        int iy = round(sin(a)*radius);
        if ((fx == ix) && (fy == iy)) continue;
        poses.push_back(Point2D<int>(ix, iy));
        angs.push_back(a);
        fx = ix;
        fy = iy;
    }
}

void ImageLumpCircle::BuildCircle(ImageLumpBlock*lumpBlock, int x, int y) {
    allIn=false;
    int Left = lumpBlock->GetLeft();
    int Top = lumpBlock->GetTop();
    int lumpw= lumpBlock->GetLumpWidth();
    int lumph = lumpBlock->GetLumpHeight();
    int cpCount = GetPixelCount();
    LONG pos;
    int px, py;
    bool state = false;
    BeginCircle();
    BYTE*buffer = lumpBlock->GetBuffer();
    for (int k = 0; k < cpCount; k++)
    {
        px = poses[k].X + x- Left;
        py = poses[k].Y + y - Top;
        if ((px < 0) || (px >= lumpw) || (py < 0) || (py >= lumph))
            state = false;
        else
        {
            pos = px + py * lumpw;
            state = (buffer[pos]==1);
        }
        SetNextCirclePixel(state);
    }
    EndCircle();
}

int ImageLumpCircle::GetRadius(){
    return radius;
}
void ImageLumpCircle::RemoveIntersects(double maxanglerange) {
    int interNum = this->GetIntersectNum();
    for (int k = interNum - 1; k >= 0; k--) {
        double ang=GetIntersectAngleRange(k);
        if (ang > maxanglerange) {
            breakPos.erase(std::begin(breakPos) + k*2 , std::begin(breakPos) + k*2+2);
        }
    }
}

void ImageLumpCircle::BeginCircle()
{
    pixelIndex = 0;
    state = false;
    breakPos.clear();
    allIn=false;
}
void ImageLumpCircle::SetNextCirclePixel(bool s)
{
    if ((!state) && (s))
    {
        breakPos.push_back(pixelIndex);
        state = s;
    }
    else if ((state) && (!s))
    {
        breakPos.push_back(pixelIndex - 1);
        state = s;
    }
    pixelIndex++;
}

void ImageLumpCircle::EndCircle()
{
    if (state)
    {
        if (breakPos.size() >= 1)
        {
            if (breakPos[0] == 0)
            {
                breakPos[0] = breakPos[breakPos.size() - 1];
                breakPos.erase(std::begin(breakPos) + breakPos.size()-1);
                if(breakPos.size()==0) allIn=true;
            }
            else
            {
                breakPos.push_back(poses.size() - 1);
            }
        }
    }
}
int ImageLumpCircle::GetPixelCount()
{
    return poses.size();
}
Point2D<int> ImageLumpCircle::GetPixelPoint(int nIndex)
{
    return poses[nIndex];
}
int ImageLumpCircle::GetIntersectNum()
{
    return breakPos.size() / 2;
}
int ImageLumpCircle::round(double x)
{
    int ix = x;
    if (x >= 0)
        return ((x - ix < 0.5) ? ix : ix + 1);
    else
        return ((ix - x < 0.5) ? ix : ix - 1);
}

double ImageLumpCircle::GetIntersectAngle(int nIndex)
{
    int pos1 = breakPos[nIndex << 1];
    int pos2 = breakPos[(nIndex << 1) + 1];
    double angle;
    if (angs[pos2] < angs[pos1])
        angle = (angs[pos2] + angs[pos1] + 6.2831852) / 2.0;
    else
        angle = (angs[pos2] + angs[pos1]) / 2.0;
    if (angle >= 6.2831852) angle -= 6.2831852;
    return angle;
}
double ImageLumpCircle::GetIntersectFromAngle(int nIndex)
{
    int pos1 = breakPos[nIndex << 1];
    int pos2 = breakPos[(nIndex << 1) + 1];
    if (pos1 <= pos2)
    {
        if (pos1 > 0) return (angs[pos1 - 1] + angs[pos1]) / 2;
        return 0;
    }
    else
    {
        if (pos1 < poses.size() - 1) return (angs[pos1 + 1] + angs[pos1]) / 2;
        return 6.2831852;
    }
}
double ImageLumpCircle::GetIntersectToAngle(int nIndex)
{
    int pos1 = breakPos[nIndex << 1];
    int pos2 = breakPos[(nIndex << 1) + 1];
    if (pos1 <= pos2)
    {
        if (pos2 < poses.size() - 1) return (angs[pos2 + 1] + angs[pos2]) / 2;
        return 6.2831852;
    }
    else
    {
        if (pos2 > 0) return (angs[pos2 - 1] + angs[pos2]) / 2;
        return 0;
    }
}
double ImageLumpCircle::GetIntersectAngleRange(int nIndex)
{
    double ang1 = GetIntersectFromAngle(nIndex);
    double ang2 = GetIntersectToAngle(nIndex);
    if (ang1 <= ang2) return ang2 - ang1;
    return ang2 + 6.2831852 - ang1;
}
bool ImageLumpCircle::IsIntersectIn(int nIndex, double ang)
{
    int pos1 = breakPos[nIndex << 1];
    int pos2 = breakPos[(nIndex << 1) + 1];
    double ang1 = GetIntersectFromAngle(nIndex);
    double ang2 = GetIntersectToAngle(nIndex);
    if (pos1 <= pos2) return ((ang >= ang1) && (ang <= ang2));
    return ((ang >= ang1) && (ang <= ang2 + 6.2831852));
}
Point2D<int> ImageLumpCircle::GetCenterPoint(int nIndex)
{
    int pos1 = breakPos[nIndex << 1];
    int pos2 = breakPos[(nIndex << 1) + 1];
    int pos;
    if (pos2 < pos1)
    {
        pos = (pos1 + pos2 + poses.size()) / 2;
        if (pos >= poses.size()) pos -= poses.size();
    }
    else
        pos = (pos1 + pos2) / 2;
    return poses[pos];
}
Point2D<int> ImageLumpCircle::GetLeftPoint(int nIndex)
{
    int pos1 = breakPos[(nIndex << 1)];
    return poses[pos1];
}
Point2D<int> ImageLumpCircle::GetRightPoint(int nIndex)
{
    int pos2 = breakPos[(nIndex << 1) + 1];
    return poses[pos2];
}
double ImageLumpCircle::GetTotalInnerAngleRange(){
    if(allIn) return DPI;
    int interNum=this->GetIntersectNum();
    if(interNum==0) return 0;
    double tr=0;
    for(int k=0;k<interNum;k++){
        tr+=this->GetIntersectAngleRange(k);
    }
    return tr;
}

double ImageLumpCircle::GetMaxInnerAngleRange(){
    if(allIn) return DPI;
    int interNum=this->GetIntersectNum();
    if(interNum==0) return 0;
    double tr=0;
    for(int k=0;k<interNum;k++){
        tr=max(tr,this->GetIntersectAngleRange(k));
    }
    return tr;
}
bool ImageLumpCircle::IsCircleInner()
{
    return allIn;
}

CircleLineSwitch::CircleLineSwitch(double ang,double angrange,int radius){
    father=nullptr;
    child=nullptr;
    leafNode=this;
    this->ang=ang;
    this->angrange=angrange;
    this->radius=radius;
}
CircleLineSwitch::~CircleLineSwitch(){

}

CircleLineSwitch*CircleLineSwitch::GetLeafNode(){
    return leafNode;
    //CircleLineSwitch*node=this;
    //while(node->child!=nullptr){
        //node=node->child.get();
    //}
    //return node;
}

void CircleLineSwitch::AddChild(AutoPtr<CircleLineSwitch>child){
    CircleLineSwitch*node=this->GetLeafNode();
    node->child=child;
    child->father=node;
    CircleLineSwitch*cnode=child.get();
    while(cnode!=nullptr){
        this->leafNode=cnode;
        cnode=cnode->child.get();
    }
}
double CircleLineSwitch::GetAngDif(double ang,double angrange,double angtoler){
    CircleLineSwitch*leafNode=this->leafNode;
    //while(leafNode->child!=nullptr){
        //leafNode=leafNode->child.get();
    //}
    int fromRadius=leafNode->radius;
    while(leafNode!=nullptr){
        double angdif=ang-leafNode->ang;
        if(angdif<0) angdif+=DPI;
        if(angdif>PI) angdif=DPI-angdif;
        if(angdif>=0.25*(leafNode->angrange+angrange)){
            leafNode=leafNode->father;
            continue;
        }
        if(angdif<angtoler){
            return angdif;
        }
        leafNode=leafNode->father;
        if(leafNode!=nullptr){
            if(leafNode->radius-fromRadius>2) break;
        }
    }
    return -1;
}

int CircleLineSwitch::GetLevels(){
    int level=1;
    CircleLineSwitch*leafNode=this;
    while(leafNode->child!=nullptr){
        leafNode=leafNode->child.get();
        level++;
    }
    return level;
}

double CircleLineSwitch::GetAngle(){
    return ang;
}

double CircleLineSwitch::GetAngleRange(){
    return this->angrange;
}

int CircleLineSwitch::GetRadius(){
    return radius;
}

AutoPtr<CircleLineSwitch>CircleLineSwitch::GetChild(){
    return child;
}
CircleLineSwitch*CircleLineSwitch::GetFather(){
    return father;
}

CircleLineSwitches::CircleLineSwitches(){

}
CircleLineSwitches::~CircleLineSwitches(){

}

int CircleLineSwitches::GetNearest(double ang,double angrange,int radius,double angtoler,double&minangdif){
    double minang=0;
    int angIndex=-1;
    int lsSize=lses.size();
    for(int k=0;k<lsSize;k++){
        CircleLineSwitch*leafNode=lses[k]->GetLeafNode();
        if((leafNode->GetRadius()==radius)) continue;
        if(leafNode->GetRadius()-radius>=3) continue;
        double angdif=lses[k]->GetAngDif(ang,angrange,angtoler);
        if(angdif<0) continue;
        if(angIndex==-1){
            angIndex=k;
            minang=angdif;
        }
        else if(angdif<minang){
            angIndex=k;
            minang=angdif;
        }
    }
    minangdif=minang;
    return angIndex;
}
void CircleLineSwitches::Add(int switchIndex,double ang,double angrange,int radius){
    AutoPtr<CircleLineSwitch>curs=(new CircleLineSwitch(ang,angrange,radius));
    if(switchIndex==-1)
        lses.push_back(curs);
    else
        lses[switchIndex]->AddChild(curs);
}


int CircleLineSwitches::GetSwitchNum(int minlevels){
    int counter=0;
    for(int k=lses.size()-1;k>=0;k--){
        if(lses[k]->GetLevels()>=minlevels) counter++;
    }
    return counter;
}
int CircleLineSwitches::GetSize(){
    return lses.size();
}
void CircleLineSwitches::Remove(int nIndex){
    lses.erase(begin(lses)+nIndex);
}
AutoPtr<CircleLineSwitch>CircleLineSwitches::GetItem(int nIndex){
    return lses[nIndex];
}

ImageLumpCircles::ImageLumpCircles(int fromRadius, int circleCount)
{
    this->fromRadius = fromRadius;
    int toRadius = fromRadius + circleCount;
    for (int k = fromRadius; k < toRadius; k ++)
    {
        ImageLumpCircle*cp = new ImageLumpCircle(k);
        cps.push_back(cp);
    }
}
ImageLumpCircles::~ImageLumpCircles()
{

}
void ImageLumpCircles::CreateCircles()
{
    for (int k = cps.size() - 1; k >= 0; k--)
    {
        cps[k]->CreateCircle();
    }
}

int ImageLumpCircles::GetCircleCount()
{
    return cps.size();
}
AutoPtr<ImageLumpCircle>ImageLumpCircles::GetCircle(int nIndex)
{
    return cps[nIndex];
}
int ImageLumpCircles::GetFromRadius()
{
    return fromRadius;
}

void ImageLumpCircles::BuildCircles(ImageLumpBlock*lumpBlock, int x, int y) {
    for (int k = 0; k < cps.size(); k++) {
        AutoPtr<ImageLumpCircle>cp = cps[k];
        cp->BuildCircle(lumpBlock, x, y);
    }
}

AutoPtr<CircleLineSwitches>ImageLumpCircles::GetLineSwitches(int maxLineWidthInPixels){
    int radiusIndex=cps.size()-1;
    AutoPtr<ImageLumpCircle>cp = cps[radiusIndex];
    int intersectNum = cp->GetIntersectNum();
    AutoPtr<CircleLineSwitches>lses(new CircleLineSwitches());
    double angtoler = maxLineWidthInPixels/(cp->GetRadius()+1.0);
    for (int k = 0; k < intersectNum; k++) {
        double angrange=cp->GetIntersectAngleRange(k);
        if(angrange>angtoler) continue;
        lses->Add(-1,cp->GetIntersectAngle(k),angrange,cp->GetRadius());
    }
    for (int k = radiusIndex-1; k >= 0; k--) {
        AutoPtr<ImageLumpCircle>ocp = cps[k];
        angtoler = maxLineWidthInPixels/(ocp->GetRadius()+1.0);
        int intersectNum2 = ocp->GetIntersectNum();
        vector<int>indices;
        vector<double>angs;
        indices.resize(intersectNum2);
        angs.resize(intersectNum2);
        for (int j = 0; j < intersectNum2; j++) {
            double angrange=ocp->GetIntersectAngleRange(j);
            if(angrange>angtoler) continue;
            double ang2=ocp->GetIntersectAngle(j);
            double minandif;
            int index=lses->GetNearest(ang2,angrange,ocp->GetRadius(),angtoler,minandif);
            indices[j]=index;
            angs[j]=minandif;
        }
        for (int j = 0; j < intersectNum2; j++){
            double angrange=ocp->GetIntersectAngleRange(j);
            if(angrange>angtoler) continue;
            double ang2=ocp->GetIntersectAngle(j);
            if(indices[j]==-1)
                lses->Add(-1,ang2,angrange,ocp->GetRadius());
            else{
                bool valid=true;
                for(int p=j+1;p<intersectNum2;p++){
                    if(indices[p]==indices[j]){
                        if(angs[p]<angs[j]){
                            valid=false;
                            indices[j]=-1;
                            break;
                        }
                        else{
                            indices[p]=-1;
                        }
                    }
                }
                if(valid) lses->Add(indices[j],ang2,angrange,ocp->GetRadius());
            }
        }
    }
    for(int k=lses->GetSize()-1;k>=0;k--){
        AutoPtr<CircleLineSwitch>lw=lses->GetItem(k);
        int firstRadius=lw->GetRadius();
        for(int p=lses->GetSize()-1;p>=0;p--){
            if(k==p) continue;
            AutoPtr<CircleLineSwitch>olw=lses->GetItem(p);
            int lastRadius=olw->GetLeafNode()->GetRadius();
            double toler=3.0/(firstRadius+1);
            if(firstRadius==lastRadius){
                if(fabs(lw->GetAngle()-olw->GetLeafNode()->GetAngle())<=toler){
                    AutoPtr<CircleLineSwitch>lwc=lw->GetChild();
                    if(lwc!=nullptr) olw->AddChild(lwc);
                    lses->Remove(k);
                    break;
                }
            }
            else if(firstRadius==lastRadius+1){
                if(fabs(lw->GetAngle()-olw->GetLeafNode()->GetAngle())<=toler){
                    lw=lw->GetChild();
                    if(lw!=nullptr) lw=lw->GetChild();
                    if(lw!=nullptr) olw->AddChild(lw);
                    lses->Remove(k);
                    break;
                }
            }
            else if(firstRadius==lastRadius-1){
                if(fabs(lw->GetAngle()-olw->GetLeafNode()->GetAngle())<=toler){
                    olw->AddChild(lw);
                    lses->Remove(k);
                    break;
                }
            }
        }

    }
    return lses;
}

}
