#ifndef BASE_GLOBLE_H
#define BASE_GLOBLE_H


#include <vector>
#include <string>
#include <string.h>
#include <memory>
#include <map>
#include "math.h"
using namespace std;

//#define Q_OS_LINUX
#define Q_OS_WIN
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#endif
#endif

namespace SGIS{

//typedef bool BOOL;
typedef char CHAR;
typedef unsigned char BYTE;
typedef unsigned short USHORT;
typedef short SHORT;
typedef unsigned int UINT;
typedef int INT;
typedef float FLOAT;
typedef double DOUBLE;
typedef long LONG;
typedef unsigned long ULONG;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef double DATE;


#define PI 3.1415926535898
#define DPI PI*2
#define E_V 2.71828182845904523536
#define _MAX_PATH 260

#ifdef Q_OS_LINUX
#define SGIS_EXPORT __attribute ((visibility("default")))
#else
#define SGIS_EXPORT __declspec(dllexport)
#endif

const int aroundx4[4]={-1,0,1,0};
const int aroundy4[4]={0,-1,0,1};
const int aroundx8[8]={-1,0,1,1,1,0,-1,-1};
const int aroundy8[8]={-1,-1,-1,0,1,1,1,0};

}

#endif // BASE_GLOBLE_H
