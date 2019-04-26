#ifndef _K2_DDRAW
#define _K2_DDRAW

#include"Main.h"

#if defined(DD_DISPLAYCOLOR) && (DD_DISPLAYCOLOR!=8) && (DD_DISPLAYCOLOR!=16) && (DD_DISPLAYCOLOR!=32)
#error 256 색, 16비트 32비트 만 지원합니다.
#endif

#ifdef DD_USEPALETTE
#if !defined(DD_DISPLAYCOLOR) || (DD_DISPLAYCOLOR!=8)
#error 256 색이 아니면 파레트를 사용하지 못합니다.
#endif
#endif

#ifdef DD_DISPLAYCOLOR
#define DD_COLORCOUNT (1 << DD_DISPLAYCOLOR)
#endif
#if (DD_DISPLAYCOLOR==8)
#define DD_PALETTEFLAGS DDPCAPS_8BIT
#define SetColor SetColor8
#define DrawEllipse DrawEllipse8
#define DrawFill DrawFill8
#define DrawFill_And DrawFill8_And
#define DrawLine DrawLine8
#define DrawLine_Or DrawLine8_Or
#define DrawHLine DrawHLine8
#define DrawVLine DrawVLine8
#define DrawHLine_Or DrawHLine8_Or
#define DrawVLine_Or DrawVLine8_Or
#define DrawRect DrawRect8
#endif
#if (DD_DISPLAYCOLOR==16)
#define SetColor SetColor16
#define DrawEllipse DrawEllipse16
#define DrawFill DrawFill16
#define DrawFill_And DrawFill16_And
#define DrawLine DrawLine16
#define DrawHLine DrawHLine16
#define DrawVLine DrawVLine16
#define DrawRect DrawRect16
#endif
#if (DD_DISPLAYCOLOR==32)
#define SetColor SetColor32
#define DrawEllipse DrawEllipse32
#define DrawFill DrawFill32
#define DrawFill_And DrawFill32_And
#define DrawLine DrawLine32
#define DrawHLine DrawHLine32
#define DrawVLine DrawVLine32
#define DrawRect DrawRect32
#endif

#if (defined(DD_DISPLAYWIDTH) || defined(DD_DISPLAYHEIGHT) || defined(DD_DISPLAYCOLOR))
#define DD_DISPLAYCHANGE
#endif

#ifndef DD_DISPLAYWIDTH
#define DD_DISPLAYWIDTH DISPLAYWIDTHVAR
extern int DISPLAYWIDTHVAR;
#endif
#ifndef DD_DISPLAYHEIGHT
#define DD_DISPLAYHEIGHT DISPLAYHEIGHTVAR
extern int DISPLAYHEIGHTVAR;
#endif
#ifndef DD_DISPLAYCOLOR
#define DD_DISPLAYCOLOR DISPLAYCOLORVAR
extern int DISPLAYCOLORVAR;
#endif

#ifdef DD_ZBUFFER
extern LPVOID ZBuffer;
extern int ZBufferOffset;
#endif

#define RGBSET(r,g,b) RGB(max(min(r,255),0),max(min(g,255),0),max(min(b,255),0))

extern LPVOID Surface;
extern DWORD Pitch,DrawColor;
extern int SFWidth,SFHeight,SFColorBits;

#define ClearZBuffer() memset(ZBuffer,0xff,Pitch*SFHeight*DD_DISPLAYCOLOR)

typedef struct MODEL2D *LPMODEL2D;
typedef const MODEL2D CMODEL2D,*LPCMODEL2D;

struct MODEL2D{
	LPVOID Bits,ZBits;
	DWORD DataFlags;
	DWORD Width,Height;
	MODEL2D(LPVOID bits,DWORD width,DWORD height,LPVOID zbuff,DWORD dataflags);
	LPMODEL2D new_FromBitmap(LPDWORD Palette,LPCTSTR strFileName);
	~MODEL2D();
	void Draw(int x,int y);
	void Draw_T(int x,int y);
	void Draw_M(int x,int y);
	void Draw_TM(int x,int y);
	void Draw_And(int x,int y);
	void Draw_ROr(int x,int y,int w,int h,int OffsetX,int OffsetY);
	void Draw_R(int x,int y,int w,int h,int OffsetX,int OffsetY);
	void Draw_RT(int x,int y,int w,int h,int OffsetX,int OffsetY);
	void Lock();
};

LPMODEL2D new_Model2DFromSEB(LPCTSTR str);
LPMODEL2D new_Model2DFromBMP(LPDWORD Palette,LPCTSTR strFileName);

BOOL StartDDraw();
void ClearDDraw();
BOOL LockDDraw();
void UnlockDDraw();
void CatchLockData();
HDC DDrawGetDC();
LPVOID CreateOffscreen(int Width,int Height);
void DDrawReleaseDC(HDC hDC);
void SetColor32(DWORD Color);
void SetColor16(WORD Color);
void SetColor8(BYTE Color);
void DrawEllipse8(int Left,int Top,int Right,int Bottom);
void DrawFill32(int Left,int Top,int Right,int Bottom);
void DrawFill16(int Left,int Top,int Right,int Bottom);
void DrawFill8(int Left,int Top,int Right,int Bottom);
void DrawFill8_And(int Left,int Top,int Right,int Bottom);
void DrawLine32(int x,int y,int tx,int ty);
void DrawLine16(int x,int y,int tx,int ty);
void DrawLine8(int x,int y,int tx,int ty);
void DrawLine8_Or(int x,int y,int tx,int ty);
void DrawHLine8(int x,int y,int Len);
void DrawVLine8(int x,int y,int Len);
void DrawHLine16(int x,int y,int Len);
void DrawVLine16(int x,int y,int Len);
void DrawHLine32(int x,int y,int Len);
void DrawVLine32(int x,int y,int Len);
void DrawHLine8_Or(int x,int y,int Len);
void DrawVLine8_Or(int x,int y,int Len);
void DrawRect8(int Left,int Top,int Right,int Bottom);
void DrawRect16(int Left,int Top,int Right,int Bottom);
void DrawRect32(int Left,int Top,int Right,int Bottom);
void SetDIBMP(LPBITMAP Bitmap,LPCVOID B8From,LPCVOID Palette);
int SetFromDIBMP(LPVOID B8To,LPBITMAP Bitmap,LPVOID Palette);
void VerticalMirrorBits(LPVOID To,LPCVOID From,DWORD Width,DWORD Height);
DWORD ChgRGB(DWORD Color);
HBITMAP CreateDIBS(int Width,int Height,WORD BitsCount,const DWORD *Palette,LPVOID Bitmap);
void SetToBMPBits(LPVOID Bits,LPCVOID From,DWORD Height,DWORD WidthBytes);
LPVOID new_BitsFromSEB(LPCTSTR FileName,LPDWORD Width,LPDWORD Height);
void SetSurface(LPVOID surface,DWORD Width,DWORD Height,LPVOID zbuffer);
void ClearLockData();

#if (DD_DISPLAYCOLOR==8)
BOOL SetPaletteDDraw(LPDWORD Palette);
#endif

#ifdef DD_DOUBLEBUFFER 
void UpdateDDraw();
BOOL CreateBackBuffer();
void ReleaseBackBuffer();
#endif

#endif