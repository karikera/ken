#ifndef _K2_3DDRAW
#define _K2_3DDRAW

#include "Main.h"
#include "3DMath.h"

#if (DD_DISPLAYCOLOR==8)
#define DrawTriangle DrawTriangle8
#define DrawTriangle_Z DrawTriangle8_Z
#define DrawTriangle_T DrawTriangle8_T
#define DrawTriangle_TC DrawTriangle8_TC
#endif
#if (DD_DISPLAYCOLOR==16)
#define DrawTriangle DrawTriangle16
#define DrawTriangle_Z DrawTriangle16_Z
#endif
#if (DD_DISPLAYCOLOR==32)
#define DrawTriangle DrawTriangle32
#define DrawTriangle_Z DrawTriangle32_Z
#endif
#define MAKEWORDPOINT(x,y) (DWORD)(((WORD)(y)<<16)|(WORD)(x))

typedef struct MODEL3D *LPMODEL3D;
typedef const MODEL3D CMODEL3D,*LPCMODEL3D;
typedef struct INTVECTOR3D *LPINTVECTOR3D;
typedef const INTVECTOR3D CINTVECTOR3D,LPCINTVECTOR3D;

extern MATRIX3DPOS DrawMatrix;
extern LPVOID Texture;
extern DWORD DrawVector[];

void DrawTriangle8(DWORD pt1,DWORD pt2,DWORD pt3);
void DrawTriangle8_Z(DWORD pt1,DWORD pt2,DWORD pt3,int Z1,int Z2,int Z3);
void DrawTriangle8_T(DWORD pt1,DWORD pt2,DWORD pt3,DWORD Tex1,DWORD Tex2,DWORD Tex3);
void DrawTriangle8_TC(DWORD pt1,DWORD pt2,DWORD pt3,DWORD Tex1,DWORD Tex2,DWORD Tex3);
LPMODEL3D new_Model3DBall(int w,int h);

struct MODEL3D{
	LPVECTOR3D VectorList;
	DWORD VectorCount;
	LPDWORD* Connect;
	LPDWORD Texture;
	DWORD DataFlags;
	MODEL3D(LPVECTOR3D Vector,DWORD Count,DWORD* Conn,DWORD ConnCount,LPDWORD texture,DWORD dataflags=0);
	~MODEL3D();
	void DrawCW();
	void DrawCW_Z();
	void DrawCW_ZT();
	void DrawCCW();
	void DrawCCW_Z();
	void DrawCCW_ZT();
};

#endif