#ifndef GEOMETRY3D_H
#define GEOMETRY3D_H
#include "Base/autoptr.h"
#include "Point.h"

namespace SGIS{

template<typename T>
struct Point3D{
    T X;
    T Y;
    T Z;
    Point3D(){
      X=Y=Z=0;
    };
    Point3D(T X,T Y,T Z){
        this->X=X;
        this->Y=Y;
        this->Z=Z;
    };
    Point3D(const Point3D<T>&other){
        this->X=other.X;
        this->Y=other.Y;
        this->Z=other.Z;
    };
    Point3D(Point&ppt){
        double x,y,z;
        ppt.GetCoord(&x,&y);
        X=x;
        Y=y;
        Z=ppt.Z();
    };
    void PutCoord(T x,T y,T z){
        X=x;
        Y=y;
        Z=z;
    };
    void ToPoint(Point&pt){
        pt.PutCoord(X,Y);
        pt.Z()=Z;
    };
    Point3D<T>&operator =(const Point3D<T>&other){
        this->X=other.X;
        this->Y=other.Y;
        this->Z=other.Z;
        return *this;
    };
    Point3D<T>&operator =(Point*other){
        this->X=other->X();
        this->Y=other->Y();
        this->Z=other->Z();
        return *this;
    };
    Point3D operator + (const Point3D&other){
       return Point3D(X+other.X,Y+other.Y,Z+other.Z);
    };
    Point3D operator + (T v){
       return Point3D(X+v,Y+v,Z+v);
    };
    Point3D operator - (const Point3D&other){
       return Point3D(X-other.X,Y-other.Y,Z-other.Z);
    };
    Point3D operator * (const Point3D&other){
       return Point3D(X*other.X,Y*other.Y,Z*other.Z);
    };
    Point3D operator * (T v){
       return Point3D(X*v,Y*v,Z*v);
    };
    Point3D operator - (T v){
       return Point3D(X-v,Y-v,Z-v);
    };
    Point3D operator / (const Point3D&other){
       return Point3D(X/other.X,Y/other.Y,Z/other.Z);
    };
    Point3D operator / (T v){
       return Point3D(X/v,Y/v,Z/v);
    };
    Point3D&operator += (const Point3D&other){
       X+=other.X;
       Y+=other.Y;
       Z+=other.Z;
       return *this;
    };
    Point3D&operator -= (const Point3D&other){
       X-=other.X;
       Y-=other.Y;
       Z-=other.Z;
       return *this;
    };
    Point3D&operator *= (const Point3D&other){
       X*=other.X;
       Y*=other.Y;
       Z*=other.Z;
       return *this;
    };
    Point3D&operator /= (const Point3D&other){
       X/=other.X;
       Y/=other.Y;
       Z/=other.Z;
       return *this;
    };
    Point3D Abs(){
       return Point3D(fabs(X),fabs(Y),fabs(Z));
    };

    double DistanceTo(Point3D<T>other){
        return sqrt((X-other.X)*(X-other.X)+(Y-other.Y)*(Y-other.Y)+(Z-other.Z)*(Z-other.Z));
    };
    double DistanceTo(T x,T y,T z){
        return sqrt((X-x)*(X-x)+(Y-y)*(Y-y)+(Z-z)*(Z-z));
    };
};

};

#endif // GEOMETRY3D_H
