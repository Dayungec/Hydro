#include "geometryoperator.h"
#include "GDAL/geometryfactory.h"
namespace SGIS{

EnvelopeCorners::EnvelopeCorners(double xmin, double ymin, double xmax, double ymax)
    :pArray(false)
{
    formperPos = -1;
    firstPos = -1;
    this->xmin = xmin;
    this->ymin = ymin;
    this->xmax = xmax;
    this->ymax = ymax;
    firstEnter = false;
}
EnvelopeCorners::~EnvelopeCorners() {

}
void EnvelopeCorners::AddCorner(double fromv, double tov) {
    if (fromv == tov) return;
    corners.push_back(EnvelopeCorner(fromv, tov));
}
void EnvelopeCorners::BeginCorners() {
    pArray.Clear();
    topopts.clear();
    corners.clear();
    states.clear();
    formperPos = -1;
    firstPos = -1;
    firstEnter = false;
}
void EnvelopeCorners::EndCorners() {
    if (firstPos >= 0) {
        if(firstEnter)
           AddCorner(firstPos, formperPos);
        else
            AddCorner(formperPos,firstPos);
    }
    states.resize(topopts.size());
    int cornerCount = corners.size();
    for (int k = 0; k < cornerCount; k++) {
        EnvelopeCorner corner = corners[k];
        LONG fp, tp;
        fp=pArray.Find(corner.frompos);
        tp=pArray.Find(corner.topos);
        int curIndex = fp;
        while (true) {
            states[curIndex]++;
            curIndex += 1;
            if (curIndex >= cornerCount) curIndex -= cornerCount;
            if (curIndex == tp) {
                break;
            }
        }
        curIndex = tp;
        while (true) {
            states[curIndex]--;
            curIndex ++;
            if (curIndex >= cornerCount) curIndex -= cornerCount;
            if (curIndex == fp) {
                break;
            }
        }
    }
}
int EnvelopeCorners::LeaveEnvelope(double bv) {
    LONG nPos=pArray.Find(bv);
    if (nPos < 0) {
        nPos=pArray.Add(bv);
        topopts.insert(begin(topopts)+nPos, nullptr);
    }
    if (formperPos >= 0) AddCorner(formperPos, bv);
    if (firstPos < 0) {
        firstPos = bv;
        firstEnter = false;
    }
    formperPos = bv;
    return nPos;
}
int EnvelopeCorners::EnterEnvelope(double bv) {
    LONG nPos=pArray.Find(bv);
    if (nPos < 0) {
        nPos=pArray.Add(bv);
        topopts.insert(begin(topopts)+nPos,nullptr);
    }
    if (formperPos>=0) AddCorner(bv, formperPos);
    if (firstPos < 0) {
        firstPos = bv;
        firstEnter = true;
    }
    formperPos = bv;
    return nPos;
}
TopoPoint*EnvelopeCorners::GetPoint(int nIndex) {
    return topopts[nIndex];
}
void EnvelopeCorners::SetPoint(int nIndex,TopoPoint*pt) {
    topopts[nIndex] = pt;
}
void EnvelopeCorners::AddCorners(TopoLines*tLns) {
    double fv, tv;
    int count = states.size();
    for (int k = 0; k < count; k++) {
        if (states[k] < 0) {
            fv=pArray[k];
            TopoPoint*ftp = topopts[k];
            TopoPoint*ttp;
            if (k < count - 1) {
                ttp = topopts[k + 1];
                tv=pArray[k+1];
            }
            else {
                ttp = topopts[0];
                tv=pArray[0];
            }
            TopoLine*tline = new TopoLine(ftp,ttp);
            if (tv < fv) tv += 4;
            int ij = 0;
            for (int j = fv; j < (int)tv; j++) {
                ij = j;
                if (ij >= 4) ij-=4;
                if (ij == 0)
                    tline->vertices.push_back(Point2D<double>(xmin, ymax));
                else if (ij == 1)
                    tline->vertices.push_back(Point2D<double>(xmax, ymax));
                else if (ij == 2)
                    tline->vertices.push_back(Point2D<double>(xmax, ymin));
                else
                    tline->vertices.push_back(Point2D<double>(xmin, ymin));
            }
            tline->AddAttributes(1);
            tLns->AddLine(tline);
        }
    }
}
int EnvelopeCorners::GetPointCount() {
    return topopts.size();
}

EnvelopeClipper::EnvelopeClipper(double xmin,double ymin,double xmax,double ymax) {
    this->xmin = xmin;
    this->ymin = ymin;
    this->xmax = xmax;
    this->ymax = ymax;
    neighborCodes[0] = 1;
    neighborCodes[1] = 2;
    neighborCodes[2] = 3;
    neighborCodes[3] = 8;
    neighborCodes[4] = 0;
    neighborCodes[5] = 4;
    neighborCodes[6] = 7;
    neighborCodes[7] = 6;
    neighborCodes[8] = 5;
};
EnvelopeClipper::~EnvelopeClipper() {

}

AutoPtr<Point> EnvelopeClipper::ClipPoint(Point*ppt) {
    DOUBLE x, y;
    ppt->GetCoord(&x,&y);
    if ((x >= xmin) && (x <= xmax) && (y >= ymin) && (y <= ymax)) {
        Point*newpt=new Point();
        newpt->PutCoord(x, y);
        return newpt;
    }
    return nullptr;
}

AutoPtr<Points> EnvelopeClipper::ClipPoints(Points*ppts) {
    LONG ptCount=ppts->GetSize();
    DOUBLE x, y;
    Points*newpts=new Points();
    LONG validCount = 0;
    for (int k = 0; k < ptCount; k++) {
        ppts->GetItem(k, x, y);
        if ((x >= xmin) && (x <= xmax) && (y >= ymin) && (y <= ymax)) {
            newpts->Add(x, y);
            validCount++;
        }
    }
    if (validCount == 0){
        delete newpts;
        return nullptr;
    }
    else
        return newpts;
}

AutoPtr<Envelope> EnvelopeClipper::ClipEnvelope(Envelope*env) {
    double xMin, yMin, xMax, yMax;
    env->GetCoord(&xMin, &yMax, &xMax, &yMin);
    if ((xmin > xMax ) || (xmax < xMin ) || (ymin > yMax ) || (ymax < yMin ))
    {
        return nullptr;
    }
    Envelope*pNew=new Envelope();
    pNew->Left()=max(xMin, xmin);
    pNew->Right()=min(xMax, xmax);
    pNew->Bottom()=max(yMin, ymin);
    pNew->Top()=min(yMax, ymax);
    return pNew;
}

int EnvelopeClipper::IntersectBorder(double x1, double y1, double x2, double y2, int borderCode, double&ix1, double&iy1, double&ix2, double&iy2) {
    if (borderCode == 0) {
        if ((x1 == xmin) && (x2 == xmin)) {
            if ((y1 >= ymin) && (y1 <= ymax)&&(y2>=ymin)&&(y2<=ymax)) {
                ix1 = x1;
                iy1 = y1;
                ix2 = x2;
                iy2 = y2;
                return 2;
            }
            else if ((y1 >= ymin) && (y1 <= ymax)) {
                if (y2 > ymax) {
                    ix1 = x1;
                    iy1 = y1;
                    ix2 = x2;
                    iy2 = ymax;
                    return 2;
                }
                else {
                    ix1 = x1;
                    iy1 = y1;
                    ix2 = x2;
                    iy2 = ymin;
                    return 2;
                }
            }
            else if ((y2 >= ymin) && (y2 <= ymax)) {
                if (y1 > ymax) {
                    ix1 = x1;
                    iy1 = ymax;
                    ix2 = x2;
                    iy2 = y2;
                    return 2;
                }
                else {
                    ix1 = x1;
                    iy1 = ymin;
                    ix2 = x2;
                    iy2 = y2;
                    return 2;
                }
            }
        }
        if ((x1 < xmin) && (x2 < xmin)) return 0;
        if ((x1 > xmin) && (x2 > xmin)) return 0;
        double y=(y2 - y1) / (x2 - x1)*(xmin - x1) + y1;
        if ((y >= ymin) && (y <= ymax)) {
            ix1 = xmin;
            iy1 = y;
            return 1;
        }
    }
    else if (borderCode == 1) {
        if ((y1 == ymax) && (y2 == ymax)) {
            if ((x1 >= xmin) && (x1 <= xmax) && (x2 >= xmin) && (x2 <= xmax)) {
                ix1 = x1;
                iy1 = y1;
                ix2 = x2;
                iy2 = y2;
                return 2;
            }
            else if ((x1 >= xmin) && (x1 <= xmax)) {
                if (x2 > xmax) {
                    ix1 = x1;
                    iy1 = y1;
                    ix2 = xmax;
                    iy2 = y2;
                    return 2;
                }
                else {
                    ix1 = x1;
                    iy1 = y1;
                    ix2 = xmin;
                    iy2 = y2;
                    return 2;
                }
            }
            else if ((x2 >= xmin) && (x2 <= xmax)) {
                if (x1 > xmax) {
                    ix1 = xmax;
                    iy1 = y1;
                    ix2 = x2;
                    iy2 = y2;
                    return 2;
                }
                else {
                    ix1 = xmin;
                    iy1 = y1;
                    ix2 = x2;
                    iy2 = y2;
                    return 2;
                }
            }
        }
        if ((y1 < ymax) && (y2 < ymax)) return 0;
        if ((y1 > ymax) && (y2 > ymax)) return 0;
        double x = (x2 - x1) / (y2 - y1)*(ymax - y1) + x1;
        if ((x >= xmin) && (x <= xmax)) {
            ix1 = x;
            iy1 = ymax;
            return 1;
        }
    }
    else if (borderCode == 2) {
        if ((x1 == xmax) && (x2 == xmax)) {
            if ((y1 >= ymin) && (y1 <= ymax) && (y2 >= ymin) && (y2 <= ymax)) {
                ix1 = x1;
                iy1 = y1;
                ix2 = x2;
                iy2 = y2;
                return 2;
            }
            else if ((y1 >= ymin) && (y1 <= ymax)) {
                if (y2 > ymax) {
                    ix1 = x1;
                    iy1 = y1;
                    ix2 = x2;
                    iy2 = ymax;
                    return 2;
                }
                else {
                    ix1 = x1;
                    iy1 = y1;
                    ix2 = x2;
                    iy2 = ymin;
                    return 2;
                }
            }
            else if ((y2 >= ymin) && (y2 <= ymax)) {
                if (y1 > ymax) {
                    ix1 = x1;
                    iy1 = ymax;
                    ix2 = x2;
                    iy2 = y2;
                    return 2;
                }
                else {
                    ix1 = x1;
                    iy1 = ymin;
                    ix2 = x2;
                    iy2 = y2;
                    return 2;
                }
            }
        }
        if ((x1 < xmax) && (x2 < xmax)) return 0;
        if ((x1 > xmax) && (x2 > xmax)) return 0;
        double y = (y2 - y1) / (x2 - x1)*(xmax - x1) + y1;
        if ((y >= ymin) && (y <= ymax)) {
            ix1 = xmax;
            iy1 = y;
            return 1;
        }
    }
    else if (borderCode == 3) {
        if ((y1 == ymin) && (y2 == ymin)) {
            if ((x1 >= xmin) && (x1 <= xmax) && (x2 >= xmin) && (x2 <= xmax)) {
                ix1 = x1;
                iy1 = y1;
                ix2 = x2;
                iy2 = y2;
                return 2;
            }
            else if ((x1 >= xmin) && (x1 <= xmax)) {
                if (x2 > xmax) {
                    ix1 = x1;
                    iy1 = y1;
                    ix2 = xmax;
                    iy2 = y2;
                    return 2;
                }
                else {
                    ix1 = x1;
                    iy1 = y1;
                    ix2 = xmin;
                    iy2 = y2;
                    return 2;
                }
            }
            else if ((x2 >= xmin) && (x2 <= xmax)) {
                if (x1 > xmax) {
                    ix1 = xmax;
                    iy1 = y1;
                    ix2 = x2;
                    iy2 = y2;
                    return 2;
                }
                else {
                    ix1 = xmin;
                    iy1 = y1;
                    ix2 = x2;
                    iy2 = y2;
                    return 2;
                }
            }
        }
        if ((y1 < ymin) && (y2 < ymin)) return 0;
        if ((y1 > ymin) && (y2 > ymin)) return 0;
        double x = (x2 - x1) / (y2 - y1)*(ymin - y1) + x1;
        if ((x >= xmin) && (x <= xmax)) {
            ix1 = x;
            iy1 = ymin;
            return 1;
        }
    }
    return 0;
}

int EnvelopeClipper::IntersectBorder(double x1, double y1, double x2, double y2, double&ix1, double&iy1, double&ix2, double&iy2) {
    double rx1, ry1;
    int rightNum = 0;
    for (int k = 0; k < 4; k++) {
        int num = IntersectBorder(x1, y1, x2, y2, k, ix1, iy1, ix2, iy2);
        if (num == 2)
            return 2;
        else if (num == 1) {
            if (rightNum == 0) {
                rx1 = ix1;
                ry1 = iy1;
            }
            else if (rightNum == 1) {
                ix2 = ix1;
                iy2 = iy1;
                ix1 = rx1;
                iy1 = ry1;
                double dist1 = sqrt((ix1 - x1)*(ix1 - x1) + (iy1 - y1)*(iy1 - y1));
                double dist2 = sqrt((ix2 - x1)*(ix2 - x1) + (iy2 - y1)*(iy2 - y1));
                if (dist1 > dist2) {
                    rx1 = ix2;
                    ry1 = iy2;
                    ix2 = ix1;
                    iy2 = iy1;
                    ix1 = rx1;
                    iy1 = ry1;
                }
                return 2;
            }
            rightNum++;
        }
    }
    if (rightNum > 0) {
        ix1 = rx1;
        iy1 = ry1;
    }
    return rightNum;
}

AutoPtr<Segment> EnvelopeClipper::ClipSegment(Segment*seg) {
    DOUBLE x1, y1, x2, y2;
    seg->GetCoord(&x1, &y1, &x2, &y2);
    int pos1=getNbPos(x1, y1);
    int pos2= getNbPos(x2, y2);
    if ((pos1 == 0) && (pos2 == 0)) {
        Segment*newSeg=new Segment();
        newSeg->PutCoord(x1, y1, x2, y2);
        return newSeg;
    }
    if ((pos1 != 0) && (pos2 != 0)) {
        DOUBLE xMin, yMin, xMax, yMax;
        xMin = min(x1, y1);
        yMin = min(y1, y2);
        xMax = max(x1, y1);
        yMax = max(y1, y2);
        if ((xMin > xmax) || (xMax < xmin) || (yMin > ymax) || (yMax < ymin)) {
            return nullptr;
        }
        int dif = ((pos1 % 2 == 1) ? 2 : 1);
        int difPos = pos2 - pos1;
        if (difPos < 0) difPos += 8;
        if (difPos <= dif) {
            return nullptr;
        }
        double rx1, ry1;
        double ix1, iy1, ix2, iy2;
        int rightNum = 0;
        for (int k = 0; k < 4; k++) {
            int num = IntersectBorder(x1, y1, x2, y2, k, ix1, iy1, ix2, iy2);
            if (num == 2) {
                Segment*newSeg=new Segment();
                newSeg->PutCoord(ix1, iy1, ix2, iy2);
                return newSeg;
            }
            else if (num == 1) {
                if (rightNum == 0) {
                    rx1 = ix1;
                    ry1 = iy1;
                }
                else if (rightNum == 1) {
                    Segment*newSeg=new Segment();
                    newSeg->PutCoord(rx1, ry1, ix1, iy1);
                    return newSeg;
                }
                rightNum++;
            }
        }

    }
    else if (pos1 == 0) {
        double ix1, iy1, ix2, iy2;
        for (int k = 0; k < 4; k++) {
            int num = IntersectBorder(x1, y1, x2, y2, k, ix1, iy1, ix2, iy2);
            if (num >0) {
                Segment*newSeg=new Segment();
                newSeg->PutCoord(x1, y1, ix1,iy1);
                return newSeg;
            }
        }
    }
    else if (pos2 == 0) {
        double ix1, iy1, ix2, iy2;
        for (int k = 0; k < 4; k++) {
            int num = IntersectBorder(x1, y1, x2, y2, k, ix1, iy1, ix2, iy2);
            if (num > 0) {
                Segment*newSeg=new Segment();
                newSeg->PutCoord(ix1,iy1,x2,y2);
                return newSeg;
            }
        }
    }
    return nullptr;
}

AutoPtr<Geometry> EnvelopeClipper::ClipEllipse(Ellipse*elli) {
    AutoPtr<Polygon>poly=elli->ConvertToPolygon(true);
    return ClipPolygon(poly.get());
}

vector<AutoPtr<Path>>EnvelopeClipper::ClipPath(Path*pPath) {
    AutoPtr<Points>pts=AutoPtr<Path>(pPath,true);
    LONG ptNum=pts->GetSize();
    if (ptNum < 2) return vector<AutoPtr<Path>>();
    DOUBLE*data=pts->GetXYs();
    DOUBLE x1, y1,x2,y2;
    x1 = data[0];
    y1 = data[1];
    LONG nIndex = 2;
    LONG bc1 = getNbPos(x1, y1);
    LONG bc2;
    Points*subpts=new Points();
    if (bc1 == 0) subpts->Add(x1, y1);
    double ix1, iy1, ix2,iy2;
    int interseNum;
    vector<AutoPtr<Path>>subpathes;
    for (int k = 1; k < ptNum; k++) {
        x2 = data[nIndex++];
        y2 = data[nIndex++];
        if ((x1 == x2) && (y1 == y2)) continue;
        bc2 = getNbPos(x2, y2);
        if ((bc1 == 0) && (bc2 == 0)) {
            subpts->Add(x2, y2);
        }
        else if ((bc1 == 0) && (bc2 != 0)) {
            interseNum=IntersectBorder(x1, y1, x2, y2, ix1, iy1, ix2, iy2);
            subpts->Add(ix1, iy1);
            subpathes.push_back(subpts);
            subpts=new Points();
        }
        else if ((bc1 != 0) && (bc2 == 0)) {
            interseNum = IntersectBorder(x1, y1, x2, y2, ix1, iy1, ix2, iy2);
            subpts->Add(ix1, iy1);
        }
        else if ((bc1 != 0) && (bc2 != 0)) {
            if(bc1==bc2)
                interseNum = 0;
            else {
                int dif = ((bc1 % 2 == 1) ? 2 : 1);
                int difPos = bc2 - bc1;
                if (difPos < 0) difPos += 8;
                if (difPos <= dif)
                    interseNum = 0;
                else
                    interseNum = IntersectBorder(x1, y1, x2, y2, ix1, iy1, ix2, iy2);
            }
            if (interseNum == 2) {
                subpts->Add(ix1, iy1);
                subpts->Add(ix2, iy2);
                subpathes.push_back(subpts);
                subpts=new Points();
            }
        }
        x1 = x2;
        y1 = y2;
        bc1 = bc2;
    }
    LONG ptsCount=subpts->GetSize();
    if (ptsCount >= 2) {
        Path*pPath=(Path*)subpts;
        subpathes.push_back(pPath);
        return subpathes;
    }
    delete subpts;
    return subpathes;
}

vector<AutoPtr<Ring>> EnvelopeClipper::ClipRing(Ring*pPath,bool clockwise) {
    /*基本思路：
    对于Ring的边界，记录Enter至Leave之间的线段，在TopoBuilder中以一条TopoLine表示
    每一条TopoLine起始节点，记录为TopoPoint

    对于矩形边界，由TopoPoint划分为很多片段，依据Enter和Leave的顺序，对之间的片段按以下思路处理
    封闭Enter至Leave对。具体来讲，最后一次的Enter或者Leave，需要与最开始的Leave或者Enter，组成一个Enter至Leave对
    从Enter至Leave，按顺时针顺序，之间的片段加1，之外的片段减去1
    所有Enter和Leave对，对片段的赋值完成后，负数的片段为剪裁后保留的边界

    将保留的边界，按顺时针顺序，加入为TopoBuilder的TopoLine
    利用TopoBuilder构建面
    */
    AutoPtr<Points>pts=AutoPtr<Ring>(pPath,true);
    vector<AutoPtr<Ring>> subpathes;
    LONG ptNum=pts->GetSize();
    if (ptNum < 2) return subpathes ;
    DOUBLE*data=pts->GetXYs();
    DOUBLE x1, y1, x2, y2;
    x1 = data[0];
    y1 = data[1];
    LONG nIndex = 2;
    LONG bc1 = getNbPos(x1, y1);
    LONG bc2;
    TopoBuilder pBuilder;
    AutoPtr<Envelope>env(new Envelope(this->xmin,this->ymax,this->xmax,this->ymin));
    pBuilder.SetFullExtent(env.get());
    TopoPoints*tPts = pBuilder.GetPoints();
    TopoLines*tLns = pBuilder.GetLines();
    TopoPoint*fpt, *tpt,*firstPt;
    TopoLine*tline;
    firstPt = NULL;
    if (bc1 == 0) {
        fpt = tPts->AddPoint(x1, y1);
        firstPt = fpt;
        tline = new TopoLine();
        tline->fromNode = fpt;
        tline->AddAttributes(1);
    }
    double ix1, iy1, ix2, iy2;
    int interseNum;
    OrderArray<double>pArray(false);
    LONG nPos;
    double bv;
    bool hasIn = (bc1==0);
    //CEnvelopeCorners  Enter和Leave对的管理
    EnvelopeCorners corners(xmin,ymin,xmax,ymax);
    if (clockwise) {
        for (int k = 1; k < ptNum; k++) {
            x2 = data[nIndex++];
            y2 = data[nIndex++];
            if ((x1 == x2) && (y1 == y2)) continue;
            bc2 = getNbPos(x2, y2);
            if (bc2 == 0) hasIn = true;
            if ((bc1 == 0) && (bc2 == 0))
                tline->vertices.push_back(Point2D<double>(x2, y2));
            else if ((bc1 == 0) && (bc2 != 0)) {
                interseNum = IntersectBorder(x1, y1, x2, y2, ix1, iy1, ix2, iy2);
                if (interseNum == 1) {
                    bv = GetBorderValue(ix1, iy1);
                    nPos = corners.LeaveEnvelope(bv);
                    if (corners.GetPoint(nPos) == NULL) {
                        tpt = tPts->AddPoint(ix1, iy1);
                        corners.SetPoint(nPos, tpt);
                    }
                    else
                        tpt = corners.GetPoint(nPos);
                    tline->toNode = tpt;
                    tpt->AddLine(tline);
                    tLns->AddLine(tline);
                    fpt = NULL;
                }
                else if (interseNum == 2) {
                    tline->vertices.push_back(Point2D<double>(ix1, iy1));
                    bv = GetBorderValue(ix2, iy2);
                    nPos = corners.LeaveEnvelope(bv);
                    if (corners.GetPoint(nPos) == NULL) {
                        tpt = tPts->AddPoint(ix2, iy2);
                        corners.SetPoint(nPos, tpt);
                    }
                    else
                        tpt = corners.GetPoint(nPos);
                    tline->toNode = tpt;
                    tpt->AddLine(tline);
                    tLns->AddLine(tline);
                    fpt = NULL;
                }
            }
            else if ((bc1 != 0) && (bc2 == 0)) {
                interseNum = IntersectBorder(x1, y1, x2, y2, ix1, iy1, ix2, iy2);
                if (interseNum == 1) {
                    bv = GetBorderValue(ix1, iy1);
                    nPos = corners.EnterEnvelope(bv);
                    if (corners.GetPoint(nPos) == NULL) {
                        fpt = tPts->AddPoint(ix1, iy1);
                        corners.SetPoint(nPos, fpt);
                    }
                    else
                        fpt = corners.GetPoint(nPos);
                    tline = new TopoLine();
                    tline->fromNode = fpt;
                    tline->vertices.push_back(Point2D<double>(x2, y2));
                    tline->AddAttributes(1);
                }
                else if (interseNum == 2) {
                    bv = GetBorderValue(ix1, iy1);
                    nPos = corners.EnterEnvelope(bv);
                    if (corners.GetPoint(nPos) == NULL) {
                        fpt = tPts->AddPoint(ix1, iy1);
                        corners.SetPoint(nPos, fpt);
                    }
                    else
                        fpt = corners.GetPoint(nPos);
                    tline = new TopoLine();
                    tline->fromNode = fpt;
                    if((ix2!=x2)||(iy2!=y2)) tline->vertices.push_back(Point2D<double>(ix2, iy2));
                    tline->vertices.push_back(Point2D<double>(x2, y2));
                    tline->AddAttributes(1);
                }
            }
            else if ((bc1 != 0) && (bc2 != 0)) {
                int dif = ((bc1 % 2 == 1) ? 2 : 1);
                int difPos = bc2 - bc1;
                if (difPos < 0) difPos += 8;
                if (difPos <= dif)
                    interseNum = 0;
                else
                    interseNum = IntersectBorder(x1, y1, x2, y2, ix1, iy1, ix2, iy2);
                if (interseNum == 2) {
                    bv = GetBorderValue(ix1, iy1);
                    nPos = corners.EnterEnvelope(bv);
                    if (corners.GetPoint(nPos) == NULL) {
                        fpt = tPts->AddPoint(ix1, iy1);
                        corners.SetPoint(nPos, fpt);
                    }
                    else
                        fpt = corners.GetPoint(nPos);
                    tline = new TopoLine();
                    tline->fromNode = fpt;
                    tline->AddAttributes(1);
                    bv = GetBorderValue(ix2, iy2);
                    nPos = corners.LeaveEnvelope(bv);
                    if (corners.GetPoint(nPos) == NULL) {
                        tpt = tPts->AddPoint(ix2, iy2);
                        corners.SetPoint(nPos, tpt);
                    }
                    else
                        tpt = corners.GetPoint(nPos);
                    tline->toNode = tpt;
                    fpt->AddLine(tline);
                    tpt->AddLine(tline);
                    tLns->AddLine(tline);
                    fpt = NULL;
                }
            }
            x1 = x2;
            y1 = y2;
            bc1 = bc2;
        }
    }
    else {
        for (int k = ptNum-1; k >=1; k--) {
            x2 = data[k*2];
            y2 = data[k * 2+1];
            if ((x1 == x2) && (y1 == y2)) continue;
            bc2 = getNbPos(x2, y2);
            if (bc2 == 0) hasIn = true;
            if ((bc1 == 0) && (bc2 == 0))
                tline->vertices.push_back(Point2D<double>(x2, y2));
            else if ((bc1 == 0) && (bc2 != 0)) {
                interseNum = IntersectBorder(x1, y1, x2, y2, ix1, iy1, ix2, iy2);
                bv = GetBorderValue(ix1, iy1);
                nPos = corners.LeaveEnvelope(bv);
                if (corners.GetPoint(nPos) == NULL) {
                    tpt = tPts->AddPoint(ix1, iy1);
                    corners.SetPoint(nPos, tpt);
                }
                else
                    tpt = corners.GetPoint(nPos);
                tline->toNode = tpt;
                tpt->AddLine(tline);
                tLns->AddLine(tline);
                fpt = NULL;
            }
            else if ((bc1 != 0) && (bc2 == 0)) {
                interseNum = IntersectBorder(x1, y1, x2, y2, ix1, iy1, ix2, iy2);
                bv = GetBorderValue(ix1, iy1);
                nPos = corners.EnterEnvelope(bv);
                if (corners.GetPoint(nPos) == NULL) {
                    fpt = tPts->AddPoint(ix1, iy1);
                    corners.SetPoint(nPos, fpt);
                }
                else
                    fpt = corners.GetPoint(nPos);
                tline = new TopoLine();
                tline->fromNode = fpt;
                tline->vertices.push_back(Point2D<double>(x2, y2));
                tline->AddAttributes(1);
            }
            else if ((bc1 != 0) && (bc2 != 0)) {
                int dif = ((bc1 % 2 == 1) ? 2 : 1);
                int difPos = bc2 - bc1;
                if (difPos < 0) difPos += 8;
                if (difPos <= dif)
                    interseNum = 0;
                else
                    interseNum = IntersectBorder(x1, y1, x2, y2, ix1, iy1, ix2, iy2);
                if (interseNum == 2) {
                    bv = GetBorderValue(ix1, iy1);
                    nPos = corners.EnterEnvelope(bv);
                    if (corners.GetPoint(nPos) == NULL) {
                        fpt = tPts->AddPoint(ix1, iy1);
                        corners.SetPoint(nPos, fpt);
                    }
                    else
                        fpt = corners.GetPoint(nPos);
                    tline = new TopoLine();
                    tline->fromNode = fpt;
                    tline->AddAttributes(1);
                    bv = GetBorderValue(ix2, iy2);
                    nPos = corners.LeaveEnvelope(bv);
                    if (corners.GetPoint(nPos) == NULL) {
                        tpt = tPts->AddPoint(ix2, iy2);
                        corners.SetPoint(nPos, tpt);
                    }
                    else
                        tpt = corners.GetPoint(nPos);
                    tline->toNode = tpt;
                    fpt->AddLine(tline);
                    tpt->AddLine(tline);
                    tLns->AddLine(tline);
                    fpt = NULL;
                }
            }
            x1 = x2;
            y1 = y2;
            bc1 = bc2;
        }
    }
    if (firstPt != NULL) {
        tline->toNode = firstPt;
        fpt->AddLine(tline);
        firstPt->AddLine(tline);
        tLns->AddLine(tline);
    }
    int ptCount = corners.GetPointCount();
    if (ptCount == 0) {
        if (hasIn) {
            Points*pppts=(Points*)pPath;
            subpathes.push_back(pppts->Clone());
        }
        else {
            AutoPtr<Point>ppt(new Point());
            ppt->PutCoord(xmin,ymax);
            bool IsOk=pPath->IsPointIn(ppt.get());
            if (IsOk) {
                AutoPtr<Envelope>pEnv(new Envelope);
                pEnv->PutCoord(xmin, ymax, xmax, ymin);
                AutoPtr<Polygon>poly=pEnv->ConvertToPolygon();
                AutoPtr<Ring>rng=poly->GetItem(0);
                subpathes.push_back(rng);
            }
        }
        return subpathes;
    }
    //封闭Enter至Leave对，给片段赋值
    corners.EndCorners();
    //将保留的边界，按顺时针顺序，加入为TopoBuilder的TopoLine
    corners.AddCorners(tLns);
    pBuilder.BuidLineTopologyNoIntersect();
    pBuilder.BuildPolygonTopology();

    vector<AutoPtr<Polygon>>polys;
    TopoRings*topoRings = pBuilder.GetRings();
    TopoRing*tRing = topoRings->rootRing;
    while (tRing != NULL)
    {
        AutoPtr<Polygon>poly=tRing->CreatePolygon(false);
        polys.push_back(poly);
        tRing = tRing->child;
    }
    for (int p = 0; p < polys.size(); p++) {
        AutoPtr<Polygon>poly = polys[p];
        LONG rc=poly->GetSize();
        for (int k = 0; k < rc; k++) {
            AutoPtr<Ring>rin=poly->GetItem(k);
            if (clockwise) {
                if (k > 0) {
                    AutoPtr<Points>ppts(rin);
                    ppts->Reverse();
                }
            }
            else {
                if (k == 0) {
                    AutoPtr<Points>ppts(rin);
                    ppts->Reverse();
                }
            }
            subpathes.push_back(rin);
        }
    }
    return subpathes;
}

AutoPtr<Polyline>EnvelopeClipper::ClipPolyline(Polyline*pln) {
    AutoPtr<Polyline>newln=new Polyline();
    LONG counts=pln->GetSize();
    for (int k = 0; k < counts; k++) {
        AutoPtr<Path>pPath=pln->GetItem(k);
        vector<AutoPtr<Path>>pathes=ClipPath(pPath.get());
        if (pathes.size() == 0) continue;
        for (int j = 0; j < pathes.size();j++) {
            newln->Add(pathes[j]);
        }
    }
    counts=newln->GetSize();
    if (counts == 0) {
        return nullptr;
    }
    return newln;
}
AutoPtr<Geometry>EnvelopeClipper::ClipPolygon(Polygon*poly) {
    LONG counts=poly->GetSize();
    vector<AutoPtr<Ring>>outerRings;
    vector<AutoPtr<Ring>>innerRings;
    int pathCount;
    for (int k = 0; k < counts; k++) {
        AutoPtr<Ring>pRing=poly->GetItem(k);
        vector<AutoPtr<Ring>>pathes=ClipRing(pRing.get(),(k==0));
        pathCount = pathes.size();
        if (pathCount == 0) {
            if (k == 0) {
                return nullptr;
            }
        }
        if (k == 0) {
            for (int j = 0; j < pathCount; j++) outerRings.push_back(pathes[j]);
        }
        else {
            for (int j = 0; j < pathCount; j++) innerRings.push_back(pathes[j]);
        }
    }
    if (outerRings.size() == 1) {
        Polygon*newpoly=new Polygon();
        newpoly->Add(outerRings[0]);
        for (int k = 0; k < innerRings.size(); k++) {
            newpoly->Add(innerRings[k]);
        }
        return newpoly;
    }
    else {
        MultiPolygon*newpoly=new MultiPolygon();
        bool IsOk;
        for (int p = 0; p < outerRings.size(); p++) {
            AutoPtr<Polygon>newp(new Polygon);
            newp->Add(outerRings[p]);
            for (int k = innerRings.size()-1; k >=0 ; k--) {
                IsOk=outerRings[p]->IsRingIn(innerRings[k].get());
                if (IsOk) {
                    newp->Add(innerRings[k]);
                    innerRings.erase(begin(innerRings)+k);
                }
            }
            newpoly->Add(newp);
        }
        return newpoly;
    }
    return nullptr;
}

AutoPtr<Geometry>EnvelopeClipper::ClipMultiPolygon(MultiPolygon*poly) {
    AutoPtr<MultiPolygon>newpoly(new MultiPolygon());
    LONG subCount=poly->GetSize();
    for (int k = 0; k < subCount; k++) {
        AutoPtr<Polygon>ply=poly->GetItem(k);
        AutoPtr<Geometry>geo=ClipPolygon(ply.get());
        if (geo == nullptr) continue;
        AutoPtr<Polygon>subpoly(geo);
        if (subpoly != NULL) {
            newpoly->Add(subpoly);
        }
        AutoPtr<MultiPolygon>submpoly(geo);
        if (submpoly != NULL) {
            LONG subC=submpoly->GetSize();
            for (int p = 0; p < subC; p++) {
                AutoPtr<Polygon>subp=submpoly->GetItem(p);
                newpoly->Add(subp);
            }
        }
    }
    LONG counts=newpoly->GetSize();
    if (counts == 0) {
        return nullptr;
    }
    else if (counts == 1) {
        AutoPtr<Polygon>newp=newpoly->GetItem(0);
        return newp;
    }
    return newpoly;
}
AutoPtr<Geometry>EnvelopeClipper::ClipGeometryCollection(GeometryCollection*geoCol) {
    AutoPtr<GeometryCollection>newpoly(new GeometryCollection());
    LONG subCount=geoCol->GetSize();
    for (int k = 0; k < subCount; k++) {
        AutoPtr<Geometry>ply=geoCol->GetItem(k);
        AutoPtr<Geometry>geo=ClipGeometry(ply.get());
        if (geo == nullptr) continue;
        newpoly->Add(geo);
    }
    LONG counts=newpoly->GetSize();
    if (counts == 0) {
        return nullptr;
    }
    return newpoly;
}

AutoPtr<Geometry>EnvelopeClipper::ClipGeometry(Geometry*geo) {
    GeometryType geoType=geo->GetType();
    switch (geoType) {
    case gtPoint: {
        return ClipPoint((Point*)geo);
    }
    case gtPoints: {
        return ClipPoints((Points*)geo);
    }
    case gtSegment: {
        return ClipSegment((Segment*)geo);
    }
    case gtEllipse: {
        return ClipEllipse((Ellipse*)geo);
    }
    case gtEnvelope: {
        return ClipEnvelope((Envelope*)geo);
    }
    case gtPolyline: {
        return ClipPolyline((Polyline*)geo);
    }
    case gtPolygon: {
        return ClipPolygon((Polygon*)geo);
    }
    case gtMultiPolygon: {
        return ClipMultiPolygon((MultiPolygon*)geo);
    }
    case gtCollection: {
        return ClipGeometryCollection((GeometryCollection*)geo);
    }
    }
}

int EnvelopeClipper::getNbPos(double x, double y) {
    int xPos = ((x < xmin) ? -1 : ((x > xmax) ? 1 : 0));
    int yPos = ((y > ymax) ? -1 : ((y < ymin) ? 1 : 0));
    return neighborCodes[yPos * 3 + xPos + 4];
}
double EnvelopeClipper::GetBorderValue(double x, double y) {
    if (x == xmin)
        return (y - ymin) / (ymax - ymin);
    else if(y==ymax)
        return 1+(x - xmin) / (xmax - xmin);
    else if(x==xmax)
        return 2+(ymax - y) / (ymax - ymin);
    else if (y == ymin) {
        double v= 3 + (xmax - x) / (xmax - xmin);
        if (v >= 4) return 0;
        return v;
    }
    return -1;
}

GeometryOperator::GeometryOperator(AutoPtr<Geometry>geo){
    this->pGeo=geo;
}
GeometryOperator::~GeometryOperator(){

}
AutoPtr<Geometry>GeometryOperator::GetGeometry(){
    return pGeo;
}
void GeometryOperator::SetGeometry(AutoPtr<Geometry>geo){
    this->pGeo=geo;
}

double GeometryOperator::Distance(Geometry*other){
    if((pGeo==nullptr)||(other==nullptr)) return 0;
    OGRGeometry*ogrgeo=GDALGeometryFactory::ConvertGeometry(pGeo.get());
    OGRGeometry*othergeo=GDALGeometryFactory::ConvertGeometry(other);
    double dist=ogrgeo->Distance(othergeo);
    OGRGeometryFactory::destroyGeometry(ogrgeo);
    OGRGeometryFactory::destroyGeometry(othergeo);
    return dist;
}
AutoPtr<Geometry>GeometryOperator::Buffer(DOUBLE dist,LONG nQuadSegs){
    if(pGeo==nullptr) return nullptr;
    OGRGeometry*ogrgeo=GDALGeometryFactory::ConvertGeometry(pGeo.get());
    OGRGeometry*othergeo=ogrgeo->Buffer(dist,nQuadSegs);
    if(othergeo==nullptr)
    {
        OGRGeometryFactory::destroyGeometry(ogrgeo);
        return nullptr;
    }
    AutoPtr<Geometry>newgeo=GeometryFactory::CreateGeometry(othergeo);
    OGRGeometryFactory::destroyGeometry(ogrgeo);
    OGRGeometryFactory::destroyGeometry(othergeo);
    return newgeo;
}
AutoPtr<Geometry>GeometryOperator::Intersection(Geometry*other){
    if((pGeo==nullptr)||(other==nullptr)) return nullptr;
    AutoPtr<Envelope>env1(pGeo);
    if (env1 != nullptr) {
        return env1->ClipGeometry(other);
    }
    else {
        AutoPtr<Envelope>env2=AutoPtr<Geometry>(other,true);
        if (env2 != nullptr) {
            return env2->ClipGeometry(pGeo.get());
        }
    }
    OGRGeometry*ogrgeo=GDALGeometryFactory::ConvertGeometry(pGeo.get());
    OGRGeometry*othergeo=GDALGeometryFactory::ConvertGeometry(other);
    OGRGeometry*resultgeo=ogrgeo->Intersection(othergeo);
    if(resultgeo==nullptr)
    {
        OGRGeometryFactory::destroyGeometry(ogrgeo);
        OGRGeometryFactory::destroyGeometry(othergeo);
        return nullptr;
    }
    AutoPtr<Geometry>newgeo=GeometryFactory::CreateGeometry(resultgeo);
    OGRGeometryFactory::destroyGeometry(ogrgeo);
    OGRGeometryFactory::destroyGeometry(othergeo);
    OGRGeometryFactory::destroyGeometry(resultgeo);
    return newgeo;
}
AutoPtr<Geometry>GeometryOperator::Union(Geometry*other){
    if((pGeo==nullptr)||(other==nullptr)) return nullptr;
    OGRGeometry*ogrgeo=GDALGeometryFactory::ConvertGeometry(pGeo.get());
    OGRGeometry*othergeo=GDALGeometryFactory::ConvertGeometry(other);
    OGRGeometry*resultgeo=ogrgeo->Union(othergeo);
    if(resultgeo==nullptr)
    {
        OGRGeometryFactory::destroyGeometry(ogrgeo);
        OGRGeometryFactory::destroyGeometry(othergeo);
        return nullptr;
    }
    AutoPtr<Geometry>newgeo=GeometryFactory::CreateGeometry(resultgeo);
    OGRGeometryFactory::destroyGeometry(ogrgeo);
    OGRGeometryFactory::destroyGeometry(othergeo);
    OGRGeometryFactory::destroyGeometry(resultgeo);
    return newgeo;
}
AutoPtr<Geometry>GeometryOperator::Difference(Geometry*other){
    if((pGeo==nullptr)||(other==nullptr)) return nullptr;
    OGRGeometry*ogrgeo=GDALGeometryFactory::ConvertGeometry(pGeo.get());
    OGRGeometry*othergeo=GDALGeometryFactory::ConvertGeometry(other);
    OGRGeometry*resultgeo=ogrgeo->Difference(othergeo);
    if(resultgeo==nullptr)
    {
        OGRGeometryFactory::destroyGeometry(ogrgeo);
        OGRGeometryFactory::destroyGeometry(othergeo);
        return nullptr;
    }
    AutoPtr<Geometry>newgeo=GeometryFactory::CreateGeometry(resultgeo);
    OGRGeometryFactory::destroyGeometry(ogrgeo);
    OGRGeometryFactory::destroyGeometry(othergeo);
    OGRGeometryFactory::destroyGeometry(resultgeo);
    return newgeo;
}
AutoPtr<Geometry>GeometryOperator::SymmetricDifference(Geometry*other){
    if((pGeo==nullptr)||(other==nullptr)) return nullptr;
    OGRGeometry*ogrgeo=GDALGeometryFactory::ConvertGeometry(pGeo.get());
    OGRGeometry*othergeo=GDALGeometryFactory::ConvertGeometry(other);
    OGRGeometry*resultgeo=ogrgeo->SymmetricDifference(othergeo);
    if(resultgeo==nullptr)
    {
        OGRGeometryFactory::destroyGeometry(ogrgeo);
        OGRGeometryFactory::destroyGeometry(othergeo);
        return nullptr;
    }
    AutoPtr<Geometry>newgeo=GeometryFactory::CreateGeometry(resultgeo);
    OGRGeometryFactory::destroyGeometry(ogrgeo);
    OGRGeometryFactory::destroyGeometry(othergeo);
    OGRGeometryFactory::destroyGeometry(resultgeo);
    return newgeo;
}

}
