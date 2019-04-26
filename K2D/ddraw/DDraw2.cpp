#include"ddraw.h"

#define WIN32_LEAN_AND_MEAN
#include<ddraw.h>
#include<math.h>
#pragma comment(lib,"ddraw.lib")
using namespace K2;

#define SFLOCK(Surf,ddsd) (LPDIRECTDRAWSURFACE(Surf)->Lock(0,&(ddsd),DDLOCK_WAIT,0))

LPVOID Surface,ZBuffer;
DWORD Pitch;
int SFWidth,SFHeight,SFColorBits;
int ZBufferOffset;

LPVOID C_Surface,C_ZBuffer;
DWORD C_Pitch;
int C_SFWidth,C_SFHeight,C_SFColorBits;

DWORD DrawColor;
int DISPLAYWIDTHVAR,DISPLAYHEIGHTVAR,DISPLAYCOLORVAR;

LPVOID DDZBuffer;
LPDIRECTDRAW lpDD;
LPDIRECTDRAWSURFACE lpSF,lpSF2;
LPDIRECTDRAWPALETTE lpPal;
LPDIRECTDRAWCLIPPER lpClip;

MODEL2D::MODEL2D(LPVOID bits,DWORD width,DWORD height,LPVOID zbuff,DWORD dataflags){
	DataFlags=dataflags;
	Width=width;
	Height=height;
	if(DataFlags&DF_DONOTNEW){
		Bits=bits;
		ZBits=zbuff;}
	else{
		Bits=new BYTE[Width*Height];
		if(bits) memcpy(Bits,bits,Width*Height);
		ZBits=new BYTE[Width*Height];
		if(zbuff) memcpy(ZBits,zbuff,Width*Height);
	}}

MODEL2D::~MODEL2D(){
	if(!(DataFlags&DF_DONOTDELETE)){
		DEL(Bits);
		DEL(ZBits);}}

void MODEL2D::Draw(int x,int y){
	int DrawWidth,DrawHeight;
	int DrawX,DrawY;
	int OffsetX,OffsetY;
	_asm{
		mov ecx,this;

		//오른쪽 클립
		mov eax,SFWidth;
		sub eax,x;

		cmp eax,[ecx+Width];
		jl DidCutW;
		mov eax,[ecx+Width];
DidCutW:
		mov DrawWidth,eax;

		//아래 클립
		mov ebx,SFHeight;
		sub ebx,y;
		cmp ebx,[ecx+Height];
		jl DidCutH;
		mov ebx,[ecx+Height];
DidCutH:
		mov DrawHeight,ebx;

		//왼쪽 클립
		xor edi,edi;
		mov edx,x;
		or edx,edx;
		jge SkipCutX;
		add DrawWidth,edx;
		neg edx;
		mov edi,edx;
		xor edx,edx;
SkipCutX:
		mov DrawX,edx;
		mov OffsetX,edi;

		//위쪽 클립		
		xor ebx,ebx;
		mov eax,y;
		or eax,eax;
		jge SkipCutY;
		add DrawHeight,eax;
		neg eax;
		mov ebx,eax;
		xor eax,eax;
SkipCutY:
		mov DrawY,eax;
		mov OffsetY,ebx;
	}
	if(DrawWidth<=0 || DrawHeight<=0) goto FunctionOut;
	int LeftBytes=DrawWidth&0x3;
	_asm{
		mov ecx,this;

		mov edi,DrawY; //DEST
		imul edi,Pitch;
		add edi,DrawX;
		add edi,Surface;

		mov esi,OffsetY; //SRC
		imul esi,[ecx+Width];
		add esi,OffsetX;
		add esi,[ecx+Bits];

		mov edx,Pitch; //DEST 다음줄
		sub edx,DrawWidth;

		mov ebx,[ecx+Width]; //SRC 다음줄
		Sub ebx,DrawWidth;

		shr DrawWidth,2;
		mov ecx,DrawHeight;
HLoop:
		push ecx;
		mov ecx,DrawWidth;
		rep movsd;
		mov ecx,LeftBytes;
		rep movsb;
		pop ecx;
		add edi,edx;
		add esi,ebx;
		loop HLoop;
FunctionOut:
	}}

void MODEL2D::Draw_T(int x,int y){
	int DrawWidth,DrawHeight;
	int DrawX,DrawY;
	int OffsetX,OffsetY;
	_asm{
		mov ecx,this;

		//오른쪽 클립
		mov eax,SFWidth;
		sub eax,x;
		cmp eax,[ecx+Width];
		jl DidCutW;
		mov eax,[ecx+Width];
DidCutW:
		mov DrawWidth,eax;

		//아래 클립
		mov ebx,SFHeight;
		sub ebx,y;
		cmp ebx,[ecx+Height];
		jl DidCutH;
		mov ebx,[ecx+Height];
DidCutH:
		mov DrawHeight,ebx;

		//왼쪽 클립
		xor edi,edi;
		mov edx,x;
		or edx,edx;
		jge SkipCutX;
		add DrawWidth,edx;
		neg edx;
		mov edi,edx;
		xor edx,edx;
SkipCutX:
		mov DrawX,edx;
		mov OffsetX,edi;

		//위쪽 클립		
		xor ebx,ebx;
		mov eax,y;
		or eax,eax;
		jge SkipCutY;
		add DrawHeight,eax;
		neg eax;
		mov ebx,eax;
		xor eax,eax;
SkipCutY:
		mov DrawY,eax;
		mov OffsetY,ebx;
	}
	if(DrawWidth<=0 || DrawHeight<=0) goto FunctionOut;
	_asm{
		mov ecx,this;

		mov edx,DrawY; //DEST
		imul edx,Pitch;
		add edx,DrawX;
		add edx,Surface;
		add edx,DrawWidth;

		mov edi,OffsetY; //SRC
		imul edi,[ecx+Width];
		add edi,OffsetX;
		add edi,[ecx+Bits];

		mov ebx,[ecx+Width]; //SRC 다음줄
		sub ebx,DrawWidth;

		mov eax,DrawColor;
		mov ecx,DrawHeight;
HLoop:
		push ecx;
		mov ecx,DrawWidth;
DrawContinue:
		repe scasb;
		je EndContinue;
		
		neg ecx;
		mov ah,[edi-1];
		mov [edx+ecx-1],ah;
		neg ecx;
		jne DrawContinue;
EndContinue:
		pop ecx;
		add edx,Pitch;
		add edi,ebx;
		loop HLoop;
FunctionOut:
	}}

void MODEL2D::Draw_M(int x,int y){
	int DrawWidth,DrawHeight;
	int DrawX,DrawY;
	int OffsetX,OffsetY;
	_asm{
		mov ecx,this;

		//오른쪽 클립
		mov eax,SFWidth;
		sub eax,x;
		cmp eax,[ecx+Width];
		jl DidCutW;
		mov eax,[ecx+Width];
DidCutW:
		mov DrawWidth,eax;

		//아래 클립
		mov ebx,SFHeight;
		sub ebx,y;
		cmp ebx,[ecx+Height];
		jl DidCutH;
		mov ebx,[ecx+Height];
DidCutH:
		mov DrawHeight,ebx;

		//왼쪽 클립
		xor edi,edi;
		mov edx,x;
		or edx,edx;
		jge SkipCutX;
		add DrawWidth,edx;
		neg edx;
		mov edi,edx;
		xor edx,edx;
SkipCutX:
		mov DrawX,edx;
		mov OffsetX,edi;

		//위쪽 클립		
		xor ebx,ebx;
		mov eax,y;
		or eax,eax;
		jge SkipCutY;
		add DrawHeight,eax;
		neg eax;
		mov ebx,eax;
		xor eax,eax;
SkipCutY:
		mov DrawY,eax;
		mov OffsetY,ebx;
	}
	if(DrawWidth<=0 || DrawHeight<=0) goto FunctionOut;
	DWORD LeftBytes=DrawWidth&0x3;
	_asm{
		mov ecx,this;

		mov edi,DrawY; //DEST
		imul edi,Pitch;
		add edi,DrawX;
		add edi,Surface;

		mov esi,[ecx+Height]; //SRC
		sub esi,OffsetY;
		dec esi;
		imul esi,[ecx+Width];
		add esi,OffsetX;
		add esi,[ecx+Bits];

		mov edx,Pitch; //DEST 다음줄
		sub edx,DrawWidth;

		mov ebx,[ecx+Width]; //SRC 다음줄
		add ebx,DrawWidth;

		shr DrawWidth,2;
		mov ecx,DrawHeight;
HLoop:
		push ecx;
		mov ecx,DrawWidth;
		rep movsd;
		mov ecx,DrawWidth;
		rep movsb;
		pop ecx;
		add edi,edx;
		sub esi,ebx;
		loop HLoop;
FunctionOut:
	}}

void MODEL2D::Draw_TM(int x,int y){
	int DrawWidth,DrawHeight;
	int DrawX,DrawY;
	int OffsetX,OffsetY;
	_asm{
		mov ecx,this;

		//오른쪽 클립
		mov eax,SFWidth;
		sub eax,x;
		cmp eax,[ecx+Width];
		jl DidCutW;
		mov eax,[ecx+Width];
DidCutW:
		mov DrawWidth,eax;

		//아래 클립
		mov ebx,SFHeight;
		sub ebx,y;
		cmp ebx,[ecx+Height];
		jl DidCutH;
		mov ebx,[ecx+Height];
DidCutH:
		mov DrawHeight,ebx;

		//왼쪽 클립
		xor edi,edi;
		mov edx,x;
		or edx,edx;
		jge SkipCutX;
		add DrawWidth,edx;
		neg edx;
		mov edi,edx;
		xor edx,edx;
SkipCutX:
		mov DrawX,edx;
		mov OffsetX,edi;

		//위쪽 클립		
		xor ebx,ebx;
		mov eax,y;
		or eax,eax;
		jge SkipCutY;
		add DrawHeight,eax;
		neg eax;
		mov ebx,eax;
		xor eax,eax;
SkipCutY:
		mov DrawY,eax;
		mov OffsetY,ebx;
	}
	if(DrawWidth<=0 || DrawHeight<=0) goto FunctionOut;
	_asm{
		mov ecx,this;

		mov edx,DrawY; //DEST
		imul edx,Pitch;
		add edx,DrawX;
		add edx,Surface;
		add edx,DrawWidth;

		mov edi,[ecx+Height]; //SRC
		sub edi,OffsetY;
		dec edi;
		imul edi,[ecx+Width];
		add edi,OffsetX;
		add edi,[ecx+Bits];

		mov ebx,[ecx+Width]; //SRC 다음줄
		add ebx,DrawWidth;

		mov eax,DrawColor;
		mov ecx,DrawHeight;
HLoop:
		push ecx;
		mov ecx,DrawWidth;
DrawContinue:
		repe scasb;
		je EndContinue;
		
		neg ecx;
		mov ah,[edi-1];
		mov [edx+ecx-1],ah;
		neg ecx;
		
		jne DrawContinue;
EndContinue:
		pop ecx;
		add edx,Pitch;
		sub edi,ebx;
		loop HLoop;
FunctionOut:
	}}

void MODEL2D::Draw_And(int x,int y){
	int DrawWidth,DrawHeight;
	int DrawX,DrawY;
	int OffsetX,OffsetY;
	_asm{
		mov ecx,this;

		//오른쪽 클립
		mov eax,SFWidth;
		sub eax,x;

		cmp eax,[ecx+Width];
		jl DidCutW;
		mov eax,[ecx+Width];
DidCutW:
		mov DrawWidth,eax;

		//아래 클립
		mov ebx,SFHeight;
		sub ebx,y;
		cmp ebx,[ecx+Height];
		jl DidCutH;
		mov ebx,[ecx+Height];
DidCutH:
		mov DrawHeight,ebx;

		//왼쪽 클립
		xor edi,edi;
		mov edx,x;
		or edx,edx;
		jge SkipCutX;
		add DrawWidth,edx;
		neg edx;
		mov edi,edx;
		xor edx,edx;
SkipCutX:
		mov DrawX,edx;
		mov OffsetX,edi;

		//위쪽 클립		
		xor ebx,ebx;
		mov eax,y;
		or eax,eax;
		jge SkipCutY;
		add DrawHeight,eax;
		neg eax;
		mov ebx,eax;
		xor eax,eax;
SkipCutY:
		mov DrawY,eax;
		mov OffsetY,ebx;
	}
	if(DrawWidth<=0 || DrawHeight<=0) goto FunctionOut;
	int LeftBytes=DrawWidth&0x3;
	_asm{
		mov ecx,this;

		mov edi,DrawY; //DEST
		imul edi,Pitch;
		add edi,DrawX;
		add edi,Surface;

		mov esi,OffsetY; //SRC
		imul esi,[ecx+Width];
		add esi,OffsetX;
		add esi,[ecx+Bits];

		mov edx,Pitch; //DEST 다음줄
		sub edx,DrawWidth;

		mov ebx,[ecx+Width]; //SRC 다음줄
		Sub ebx,DrawWidth;

		shr DrawWidth,2;
		mov ecx,DrawHeight;
HLoop:
		push ecx;
		mov ecx,DrawWidth;
		jcxz Skip1;
WLoop1:
		lodsd;
		and eax,[edi];
		stosd;
		loop WLoop1;
Skip1:
		mov ecx,LeftBytes;
		jcxz Skip2;
WLoop2:
		lodsb;
		and al,[edi];
		stosb;
		loop WLoop2;
Skip2:
		pop ecx;
		add edi,edx;
		add esi,ebx;
		loop HLoop;
FunctionOut:
	}}

void MODEL2D::Draw_ROr(int x,int y,int w,int h,int OffsetX,int OffsetY){
	int DrawWidth,DrawHeight;
	int DrawX,DrawY;
	_asm{
		mov ecx,this;

		//오른쪽 클립
		mov eax,SFWidth;
		sub eax,x;

		cmp eax,w;
		jl DidCutW;
		mov eax,w;
DidCutW:
		mov DrawWidth,eax;

		//아래 클립
		mov ebx,SFHeight;
		sub ebx,y;
		cmp ebx,h;
		jl DidCutH;
		mov ebx,h;
DidCutH:
		mov DrawHeight,ebx;

		//왼쪽 클립
		xor edi,edi;
		mov edx,x;
		or edx,edx;
		jge SkipCutX;
		add DrawWidth,edx;
		neg edx;
		mov edi,edx;
		xor edx,edx;
SkipCutX:
		mov DrawX,edx;
		add OffsetX,edi;

		//위쪽 클립		
		xor ebx,ebx;
		mov eax,y;
		or eax,eax;
		jge SkipCutY;
		add DrawHeight,eax;
		neg eax;
		mov ebx,eax;
		xor eax,eax;
SkipCutY:
		mov DrawY,eax;
		add OffsetY,ebx;
	}
	if(DrawWidth<=0 || DrawHeight<=0) goto FunctionOut;
	int LeftBytes=DrawWidth&0x3;
	_asm{
		mov ecx,this;

		mov edi,DrawY; //DEST
		imul edi,Pitch;
		add edi,DrawX;
		add edi,Surface;

		mov esi,OffsetY; //SRC
		imul esi,[ecx+Width];
		add esi,OffsetX;
		add esi,[ecx+Bits];

		mov edx,Pitch; //DEST 다음줄
		sub edx,DrawWidth;

		mov ebx,[ecx+Width]; //SRC 다음줄
		Sub ebx,DrawWidth;

		shr DrawWidth,2;
		mov ecx,DrawHeight;
HLoop:
		push ecx;
		mov ecx,DrawWidth;
		jcxz Skip1;
WLoop1:
		lodsd;
		or eax,[edi];
		stosd;
		loop WLoop1;
Skip1:
		mov ecx,LeftBytes;
		jcxz Skip2;
WLoop2:
		lodsb;
		or al,[edi];
		stosb;
		loop WLoop2;
Skip2:
		pop ecx;
		add edi,edx;
		add esi,ebx;
		loop HLoop;
FunctionOut:
	}}

void MODEL2D::Draw_R(int x,int y,int w,int h,int OffsetX,int OffsetY){
	int DrawWidth,DrawHeight;
	int DrawX,DrawY;
	_asm{
		mov ecx,this;

		//오른쪽 클립
		mov eax,SFWidth;
		sub eax,x;

		cmp eax,w;
		jl DidCutW;
		mov eax,w;
DidCutW:
		mov DrawWidth,eax;

		//아래 클립
		mov ebx,SFHeight;
		sub ebx,y;
		cmp ebx,h;
		jl DidCutH;
		mov ebx,h;
DidCutH:
		mov DrawHeight,ebx;

		//왼쪽 클립
		xor edi,edi;
		mov edx,x;
		or edx,edx;
		jge SkipCutX;
		add DrawWidth,edx;
		neg edx;
		mov edi,edx;
		xor edx,edx;
SkipCutX:
		mov DrawX,edx;
		add OffsetX,edi;

		//위쪽 클립		
		xor ebx,ebx;
		mov eax,y;
		or eax,eax;
		jge SkipCutY;
		add DrawHeight,eax;
		neg eax;
		mov ebx,eax;
		xor eax,eax;
SkipCutY:
		mov DrawY,eax;
		add OffsetY,ebx;
	}
	if(DrawWidth<=0 || DrawHeight<=0) goto FunctionOut;
	int LeftBytes=DrawWidth&0x3;
	_asm{
		mov ecx,this;

		mov edi,DrawY; //DEST
		imul edi,Pitch;
		add edi,DrawX;
		add edi,Surface;

		mov esi,OffsetY; //SRC
		imul esi,[ecx+Width];
		add esi,OffsetX;
		add esi,[ecx+Bits];

		mov edx,Pitch; //DEST 다음줄
		sub edx,DrawWidth;

		mov ebx,[ecx+Width]; //SRC 다음줄
		Sub ebx,DrawWidth;

		shr DrawWidth,2;
		mov ecx,DrawHeight;
HLoop:
		push ecx;
		mov ecx,DrawWidth;
		rep movsd;
		mov ecx,LeftBytes;
		rep movsb;
		pop ecx;
		add edi,edx;
		add esi,ebx;
		loop HLoop;
FunctionOut:
	}}

void MODEL2D::Draw_RT(int x,int y,int w,int h,int OffsetX,int OffsetY){
	int DrawWidth,DrawHeight;
	int DrawX,DrawY;
	_asm{
		mov ecx,this;

		//오른쪽 클립
		mov eax,SFWidth;
		sub eax,x;

		cmp eax,w;
		jl DidCutW;
		mov eax,w;
DidCutW:
		mov DrawWidth,eax;

		//아래 클립
		mov ebx,SFHeight;
		sub ebx,y;
		cmp ebx,h;
		jl DidCutH;
		mov ebx,h;
DidCutH:
		mov DrawHeight,ebx;

		//왼쪽 클립
		xor edi,edi;
		mov edx,x;
		or edx,edx;
		jge SkipCutX;
		add DrawWidth,edx;
		neg edx;
		mov edi,edx;
		xor edx,edx;
SkipCutX:
		mov DrawX,edx;
		add OffsetX,edi;

		//위쪽 클립		
		xor ebx,ebx;
		mov eax,y;
		or eax,eax;
		jge SkipCutY;
		add DrawHeight,eax;
		neg eax;
		mov ebx,eax;
		xor eax,eax;
SkipCutY:
		mov DrawY,eax;
		add OffsetY,ebx;
	}
	if(DrawWidth<=0 || DrawHeight<=0) goto FunctionOut;
	_asm{
		mov ecx,this;

		mov edx,DrawY; //DEST
		imul edx,Pitch;
		add edx,DrawX;
		add edx,Surface;
		add edx,DrawWidth;

		mov edi,OffsetY; //SRC
		imul edi,[ecx+Width];
		add edi,OffsetX;
		add edi,[ecx+Bits];

		mov ebx,[ecx+Width]; //SRC 다음줄
		sub ebx,DrawWidth;

		mov eax,DrawColor;
		mov ecx,DrawHeight;
HLoop:
		push ecx;
		mov ecx,DrawWidth;
DrawContinue:
		repe scasb;
		je EndContinue;
		
		neg ecx;
		mov ah,[edi-1];
		mov [edx+ecx-1],ah;
		neg ecx;
		jne DrawContinue;
EndContinue:
		pop ecx;
		add edx,Pitch;
		add edi,ebx;
		loop HLoop;
FunctionOut:
	}}

void MODEL2D::Lock(){
	Surface=Bits;
	Pitch=SFWidth=Width;
	SFHeight=Height;
	SFColorBits=DD_DISPLAYCOLOR;
	ZBuffer=ZBits;
	ZBufferOffset=(int)((LPBYTE)Surface-(LPBYTE)ZBuffer);
}

LPMODEL2D new_Model2DFromSEB(LPCTSTR str){
	DWORD Width,Height;
	DWORD Junk,Size;
	HANDLE hFile;
	LPVOID Data,Bits;
	hFile=CreateFile(str,GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(hFile==INVALID_HANDLE_VALUE) return 0;
	Size=GetFileSize(hFile,&Junk);
	if(Junk) return 0;
	Data=new BYTE[Size-8];
	ReadFile(hFile,&Width,4,&Junk,0);
	ReadFile(hFile,&Height,4,&Junk,0);
	ReadFile(hFile,Data,Size-8,&Junk,0);
	CloseHandle(hFile);
	Bits=new BYTE[Width*Height];
	SEBConvertToBits(Bits,Data,Size-8);
	DEL(Data);
	VerticalMirrorBits(Bits,Bits,Width,Height);
	LPVOID ZBuff=new BYTE[Width*Height];
	memset(ZBuff,0xff,Width*Height);
	return new MODEL2D(Bits,Width,Height,ZBuff,DF_DONOTNEW);}

LPMODEL2D new_Model2DFromBMP(LPDWORD Palette,LPCTSTR strFileName){
	DWORD BitsSize;
	LPBYTE Bits;
	BITMAP Bitmap;
	HBITMAP hBitmap=(HBITMAP)LoadImage(0,strFileName,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_DEFAULTSIZE|LR_CREATEDIBSECTION);
	GetObject(hBitmap,sizeof(BITMAP),&Bitmap);
	Bits=new BYTE[BitsSize=Bitmap.bmWidth*Bitmap.bmHeight];
	DeleteObject(hBitmap);
	switch(Bitmap.bmBitsPixel){
		default: goto Failed;
		case 24:{
			if(!SetFromDIBMP(Bits,&Bitmap,Palette))
				goto Failed;
				}}
	return &MODEL2D(Bits,Bitmap.bmWidth,Bitmap.bmHeight,0,DF_DONOTNEW);
Failed:
	DEL(Bits);
	return 0;}

BOOL StartDDraw(){
	HRESULT hr;
	if(FAILED(hr=DirectDrawCreate(0,&lpDD,0))) return 0;
#ifdef DD_FULLSCREEN
	hr=lpDD->SetCooperativeLevel(hWnd,DDSCL_FULLSCREEN|DDSCL_EXCLUSIVE);
#else
	hr=lpDD->SetCooperativeLevel(0,DDSCL_NORMAL);
#endif
	if(FAILED(hr)) return 0;
#if ((DD_DISPLAYWIDTH==DISPLAYWIDTHVAR) || (DD_DISPLAYHEIGHT==DISPLAYHEIGHTVAR) || (DD_DISPLAYCOLOR==DISPLAYCOLORVAR))
	DDSURFACEDESC display;
	display.dwSize=sizeof(display);
	if(FAILED(hr=lpDD->GetDisplayMode(&display))) return 0;
#if DD_DISPLAYWIDTH==DISPLAYWIDTHVAR
	DISPLAYWIDTHVAR=display.dwWidth;
#endif
#if DD_DISPLAYHEIGHT==DISPLAYHEIGHTVAR
	DISPLAYHEIGHTVAR=display.dwHeight;
#endif
#if DD_DISPLAYCOLOR==DISPLAYCOLORVAR
	DISPLAYCOLORVAR=display.ddpfPixelFormat.dwRGBBitCount;
#endif
#endif

#ifdef DD_DISPLAYCHANGE
	if(FAILED(hr=lpDD->SetDisplayMode(DD_DISPLAYWIDTH,DD_DISPLAYHEIGHT,DD_DISPLAYCOLOR)))
		return 0;
#endif
	DDSURFACEDESC ddsd;
	ddsd.dwSize=sizeof(ddsd);
#if defined(DD_FULLSCREEN) && defined(DD_DOUBLEBUFFER)
	ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE|DDSCAPS_COMPLEX|DDSCAPS_FLIP;
	ddsd.dwFlags=DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
	ddsd.dwBackBufferCount=1;
#else
	ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;
	ddsd.dwFlags=DDSD_CAPS;
#endif
	if(FAILED(hr=lpDD->CreateSurface(&ddsd,&lpSF,0))) return 0;
#ifdef DD_DOUBLEBUFFER
	if(!CreateBackBuffer()) return 0;
#endif
	if(DD_DISPLAYCOLOR!=8 && DD_DISPLAYCOLOR!=16 && DD_DISPLAYCOLOR!=32) return 0;
	return 1;}

void ClearDDraw(){
#ifdef DD_DOUBLEBUFFER
	ReleaseBackBuffer();
#endif
#ifdef DD_ZBUFFER
	DEL(DDZBuffer);
#endif
	REL(lpPal);
	REL(lpSF);
	REL(lpDD);}

BOOL CreateBackBuffer(){
#ifdef DD_DOUBLEBUFFER
	HRESULT hr;
#ifdef DD_FULLSCREEN
	DDSCAPS ddscaps;
	ddscaps.dwCaps=DDSCAPS_BACKBUFFER;
	hr=lpSF->GetAttachedSurface(&ddscaps,&lpSF2);
	if(FAILED(hr)) return 0;
#else
	if(!(lpSF2=LPDIRECTDRAWSURFACE(CreateOffscreen(WIDTH,HEIGHT)))) return 0;

	hr=lpDD->CreateClipper(0,&lpClip,0);
	if(FAILED(hr)) return 0;
	hr=lpClip->SetHWnd(0,hWnd);
	if(FAILED(hr)) return 0;
	hr=lpSF->SetClipper(lpClip);
	if(FAILED(hr)) return 0;
#endif
#endif
	return 1;}

void ReleaseBackBuffer(){
	REL(lpClip);
	REL(lpSF2);}

BOOL LockDDraw(){
	DDSURFACEDESC ddsd;
	ddsd.dwSize=sizeof(ddsd);
	HRESULT hr;
	if(lpSF->IsLost()) lpSF->Restore();
#ifdef DD_DOUBLEBUFFER
	hr=lpSF2->Lock(0,&ddsd,DDLOCK_WAIT,0);
#else
	hr=lpSF->Lock(0,&ddsd,DDLOCK_WAIT,0);
#endif
	if(FAILED(hr)) return 0;
	Surface=(LPBYTE)ddsd.lpSurface;
	SFWidth=ddsd.dwWidth;
	SFHeight=ddsd.dwHeight;
	SFColorBits=ddsd.ddpfPixelFormat.dwRGBBitCount;
	Pitch=ddsd.lPitch;
#ifdef DD_ZBUFFER
	if(!DDZBuffer) DDZBuffer=new BYTE[SFHeight*Pitch*SFColorBits];
	ZBuffer=DDZBuffer;
	ZBufferOffset=(int)((LPBYTE)Surface-(LPBYTE)ZBuffer);
#endif
	return 1;}

void UnlockDDraw(){
#ifdef DD_DOUBLEBUFFER
	lpSF2->Unlock(0);
#else
	lpSF->Unlock(0);
#endif	
}

void UpdateDDraw(){
#ifdef DD_FULLSCREEN
	HRESULT hr=lpSF->Flip(0,DDFLIP_WAIT);
	if(hr==DDERR_SURFACELOST) lpSF->Restore();
#else
	RECT From={0,0,WIDTH,HEIGHT},To;
	POINT pt={0,0};
	ScreenToClient(hWnd,&pt);
	GetClientRect(hWnd,&To);
	OffsetRect(&To,pt.x,pt.y);
	lpSF->Blt(&From,lpSF2,&To,DDBLT_WAIT,0);
#endif
}

void ClearLockData(){
	Pitch=0;
	SFWidth=0;
	SFHeight=0;
	SFColorBits=0;
	ZBuffer=0;
	Surface=0;}

void CatchLockData(){
	_asm{
		mov eax,C_Pitch;
		xchg Pitch,eax;
		mov C_Pitch,eax;
		
		mov eax,C_SFWidth;
		xchg SFWidth,eax;
		mov C_SFWidth,eax;
		
		mov eax,C_SFHeight;
		xchg SFHeight,eax;
		mov C_SFHeight,eax;
	
		mov eax,C_SFColorBits;
		xchg SFColorBits,eax;
		mov C_SFColorBits,eax;

		mov eax,C_ZBuffer;
		xchg ZBuffer,eax;
		mov C_ZBuffer,eax;

		mov eax,C_Surface;
		xchg Surface,eax;
		mov C_Surface,eax;
	}
	ZBufferOffset=(int)((LPBYTE)Surface-(LPBYTE)ZBuffer);}

HDC DDrawGetDC(){
	HDC hDC=0;
#ifdef DD_DOUBLEBUFFER
	lpSF2->GetDC(&hDC);
#else
	lpSF->GetDC(&hDC);
#endif
	return hDC;}

void DDrawReleaseDC(HDC hDC){
#ifdef DD_DOUBLEBUFFER
	lpSF2->ReleaseDC(hDC);
#else
	lpSF->ReleaseDC(hDC);
#endif
}

LPVOID CreateOffscreen(int Width,int Height){
	LPDIRECTDRAWSURFACE Surf;
	DDSURFACEDESC ddsd;
	ddsd.dwSize=sizeof(ddsd);
	ddsd.ddsCaps.dwCaps=DDSCAPS_VIDEOMEMORY;
	ddsd.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_HEIGHT|DDSD_WIDTH;
	ddsd.dwWidth=Width;
	ddsd.dwHeight=Height;
	if(FAILED(lpDD->CreateSurface(&ddsd,&Surf,0))) return 0;
	return Surf;}

void SetColor32(DWORD Color){
	DrawColor=Color;}

void SetColor16(WORD Color){
	_asm{
		mov ax,Color;
		shl eax,16;
		mov ax,Color;
		mov DrawColor,eax;
	}
}

void SetColor8(BYTE Color){
	_asm{
		mov ah,Color;
		mov al,Color;
		shl eax,16;
		mov ah,Color;
		mov al,Color;
		mov DrawColor,eax;
	}
}

void DrawEllipse8(int Left,int Top,int Right,int Bottom){
	static const float Float1=1;
	float Y;
	int Height;
	float W,H;
	float XR,YR;
	if(Left==Right || Top==Bottom) return;
	if(Left>Right){
		_asm mov eax,Left;
		_asm xchg Right,eax;
		_asm mov Left,eax;
	}
	if(Top>Bottom){
		_asm mov ebx,Top;
		_asm xchg Bottom,ebx;
		_asm mov Top,ebx;
	}
	Height=Bottom-Top;
	W=(Right-Left)/2.f;
	H=Height/2.f;
	Y=-H;
	XR=W*W;
	YR=H*H;
	int x=(Right+Left)/2;
	float XRdYR=XR/YR;
	if(Left>=SFWidth || Top>=SFHeight || Right<=0 || Bottom<=0) return;
	//x*x=(xr*xr)-(y*y)*(xr*xr)/(yr*yr);
	int temp;
	_asm{
		mov ebx,Top;
		or ebx,ebx;
		jge ClipTop;
		fild Top;
		fsub Y;
		fchs;
		fstp Y;
		xor ebx,ebx;
ClipTop:
		imul ebx,Pitch;
		add ebx,Surface;
		mov eax,Top;
		add eax,Height;
		sub eax,SFHeight;
		jle ClipBottom;
		sub Height,eax;
ClipBottom:
		
		mov ecx,Height;
EllipseLoop:
		fld Y;
		fmul Y;
		fmul XRdYR;
		fsub XR;
		fchs;
		fstp temp;
		push ecx;
		push temp;
		call sqrtf;
		pop ecx;
		fistp temp;
		cmp temp,0;
		jle SkipDraw;
		mov edi,ebx;
		
		mov eax,temp;
		shl temp,1;
		
		mov edx,x;
		sub edx,eax;
		jge ClipL;
		add temp,edx;
		xor edx,edx;
ClipL:
		add edi,edx;

		mov edx,SFWidth;
		sub edx,x;
		sub edx,eax;
		jge CLipR;
		add temp,edx;
ClipR:

		or ecx,ecx;
		jle SkipDraw;
		mov eax,DrawColor;
		mov ecx,temp;
		and ecx,3;
		rep stosb;
		mov ecx,temp;
		shr ecx,2;
		rep stosd;
SkipDraw:
		pop ecx;

		add ebx,Pitch;
		fld Y;
		fadd Float1;
		fstp Y;
		loop EllipseLoop;
	}}

void DrawFill32(int Left,int Top,int Right,int Bottom){
	if(Left==Right || Top==Bottom) return;
	if(Left>Right){
		_asm mov eax,Left;
		_asm xchg Right,eax;
		_asm mov Left,eax;
	}
	if(Top>Bottom){
		_asm mov ebx,Top;
		_asm xchg Bottom,ebx;
		_asm mov Top,ebx;
	}
	if(Left>=SFWidth || Top>=SFHeight || Right<=0 || Bottom<=0) return;
	if(Left<0) Left=0;
	if(Right>SFWidth) Right=SFWidth;
	if(Top<0) Top=0;
	if(Bottom>SFHeight) Bottom=SFHeight;
	Right-=Left;
	Bottom-=Top;
	Left*=4;
	_asm{
		mov edi,Top;
		imul edi,Pitch;
		mov eax,Left;
		add edi,eax;
		add edi,Surface;
		mov ebx,Right;
		neg ebx;
		shl ebx,2;
		add ebx,Pitch;
		mov eax,DrawColor;
		mov ecx,Bottom;
FillLoop:
		push ecx;
		mov ecx,Right;
		rep stosd;
		pop ecx;
		add edi,ebx;
		loop FillLoop;
	}}

void DrawFill16(int Left,int Top,int Right,int Bottom){
	if(Left==Right || Top==Bottom) return;
	if(Left>Right){
		_asm mov eax,Left;
		_asm xchg Right,eax;
		_asm mov Left,eax;
	}
	if(Top>Bottom){
		_asm mov ebx,Top;
		_asm xchg Bottom,ebx;
		_asm mov Top,ebx;
	}
	if(Left>=SFWidth || Top>=SFHeight || Right<=0 || Bottom<=0) return;
	if(Left<0) Left=0;
	if(Right>SFWidth) Right=SFWidth;
	if(Top<0) Top=0;
	if(Bottom>SFHeight) Bottom=SFHeight;
	Right-=Left;
	Bottom-=Top;
	Left*=2;
	int LeftBytes=Right&0x1;
	_asm{
		mov edi,Top;
		imul edi,Pitch;
		mov eax,Left;
		add edi,eax;
		add edi,Surface;
		mov ebx,Right;
		neg ebx;
		shl ebx,1;
		add ebx,Pitch;
		mov eax,DrawColor;
		shr Right,1;
		mov ecx,Bottom;
FillLoop:
		push ecx;
		mov ecx,Right;
		rep stosd;
		mov ecx,LeftBytes;
		stosw;
		pop ecx;
		add edi,ebx;
		loop FillLoop;
	}}

void DrawFill8(int Left,int Top,int Right,int Bottom){
	if(Left==Right || Top==Bottom) return;
	if(Left>Right){
		_asm mov eax,Left;
		_asm xchg Right,eax;
		_asm mov Left,eax;
	}
	if(Top>Bottom){
		_asm mov ebx,Top;
		_asm xchg Bottom,ebx;
		_asm mov Top,ebx;
	}
	if(Left>=SFWidth || Top>=SFHeight || Right<=0 || Bottom<=0) return;
	if(Left<0) Left=0;
	if(Right>SFWidth) Right=SFWidth;
	if(Top<0) Top=0;
	if(Bottom>SFHeight) Bottom=SFHeight;
	Right-=Left;
	Bottom-=Top;
	int LeftBytes=Right&0x3;
	_asm{
		mov edi,Top;
		imul edi,Pitch;
		mov eax,Left;
		add edi,eax;
		add edi,Surface;
		mov ebx,Right;
		neg ebx;
		add ebx,Pitch;
		mov eax,DrawColor;
		shr Right,2;
		mov ecx,Bottom;
FillLoop:
		push ecx;
		mov ecx,Right;
		rep stosd;
		mov ecx,LeftBytes;
		rep stosb;
		pop ecx;
		add edi,ebx;
		loop FillLoop;
	}}

void DrawFill8_And(int Left,int Top,int Right,int Bottom){
	if(Left==Right || Top==Bottom) return;
	if(Left>Right){
		_asm mov eax,Left;
		_asm xchg Right,eax;
		_asm mov Left,eax;
	}
	if(Top>Bottom){
		_asm mov ebx,Top;
		_asm xchg Bottom,ebx;
		_asm mov Top,ebx;
	}
	if(Left>=SFWidth || Top>=SFHeight || Right<=0 || Bottom<=0) return;
	if(Left<0) Left=0;
	if(Right>SFWidth) Right=SFWidth;
	if(Top<0) Top=0;
	if(Bottom>SFHeight) Bottom=SFHeight;
	Right-=Left;
	Bottom-=Top;
	int LeftBytes=Right&0x3;
	_asm{
		mov edi,Top;
		imul edi,Pitch;
		mov eax,Left;
		add edi,eax;
		add edi,Surface;
		mov ebx,Right;
		neg ebx;
		add ebx,Pitch;
		mov eax,DrawColor;
		shr Right,2;
		mov ecx,Bottom;
FillLoop:
		push ecx;
		mov ecx,Right;
DwordLoop:
		and [edi],eax;
		add edi,4;
		loop DwordLoop;
		mov ecx,LeftBytes;
ByteLoop:
		and [edi],al;
		inc edi;
		loop ByteLoop;
		pop ecx;
		add edi,ebx;
		loop FillLoop;
	}}

void DrawLine32(int x,int y,int tx,int ty){
	if((x<0 && tx<0) || (x>=SFWidth && tx>=SFWidth)) return;
	if((y<0 && ty<0) || (y>=SFHeight && ty>=SFHeight)) return;
	int sx=tx-x;
	int sy=ty-y;
	if(x<0){ y=y-x*sy/sx; x=0;}
	if(y<0){ x=x-y*sx/sy; y=0;}
	if(x>=SFWidth){ y=y+(SFWidth-1-x)*sy/sx; x=SFWidth-1;}
	if(y>=SFHeight){ x=x+(SFHeight-1-y)*sx/sy; y=SFHeight-1;}
	if(tx<0){ ty=ty-tx*sy/sx; tx=0;}
	if(ty<0){ tx=tx-ty*sx/sy; ty=0;}
	if(tx>=SFWidth){ ty=ty+(SFWidth-1-tx)*sy/sx; tx=SFWidth-1;}
	if(ty>=SFHeight){ tx=tx+(SFHeight-1-ty)*sx/sy; ty=SFHeight-1;}

	sx=tx-x;
	sy=ty-y;

	DWORD NPitch=Pitch;
	
	int len=max(abs(sx),abs(sy));
	DWORD XUnder,YUnder;

	_asm{

		mov edi,y;
		imul edi,Pitch;
		add edi,Surface;
		mov esi,x;
		lea edi,[edi+esi*4];
		mov eax,sx;
		
		mov ecx,len;
		jcxz ZeroLen;

		cdq;
		idiv ecx;
		mov esi,eax;
		xor eax,eax;
		or edx,edx;
		jge IsPlus;
		dec esi;
		neg edx;
		div ecx;
		neg eax;
		jmp WasMinus;
IsPlus:
		div ecx;
WasMinus:
		mov XUnder,eax;

		mov eax,sy;
		cdq;
		idiv ecx;
		imul eax,Pitch;
		sar eax,2;
		add esi,eax;
		xor eax,eax;
		or edx,edx;
		jge IsPlus2;
		neg edx;
		div ecx;		
		mov YUnder,eax;

		mov edx,0x80000000;
		mov ebx,edx;
		mov eax,DrawColor;
		dec esi;
		neg NPitch;
		jmp DrawLineLoop;
IsPlus2:
		div ecx;
		mov YUnder,eax;

		mov edx,0x80000000;
		mov ebx,edx;
		mov eax,DrawColor;
		dec esi;

DrawLineLoop:
		stosd;
		ror edi,2;
		add ebx,XUnder;
		adc edi,esi;
		rol edi,2;
		add edx,YUnder;
		jnc NoCarry;
		add edi,NPitch;
NoCarry:
		loop DrawLineLoop;
ZeroLen:
		stosd;
	}}

void DrawLine16(int x,int y,int tx,int ty){
	if((x<0 && tx<0) || (x>=SFWidth && tx>=SFWidth)) return;
	if((y<0 && ty<0) || (y>=SFHeight && ty>=SFHeight)) return;
	int sx=tx-x;
	int sy=ty-y;
	if(x<0){ y=y-x*sy/sx; x=0;}
	if(y<0){ x=x-y*sx/sy; y=0;}
	if(x>=SFWidth){ y=y+(SFWidth-1-x)*sy/sx; x=SFWidth-1;}
	if(y>=SFHeight){ x=x+(SFHeight-1-y)*sx/sy; y=SFHeight-1;}
	if(tx<0){ ty=ty-tx*sy/sx; tx=0;}
	if(ty<0){ tx=tx-ty*sx/sy; ty=0;}
	if(tx>=SFWidth){ ty=ty+(SFWidth-1-tx)*sy/sx; tx=SFWidth-1;}
	if(ty>=SFHeight){ tx=tx+(SFHeight-1-ty)*sx/sy; ty=SFHeight-1;}

	sx=tx-x;
	sy=ty-y;

	DWORD NPitch=Pitch;
	
	int len=max(abs(sx),abs(sy));
	DWORD XUnder,YUnder;

	_asm{
		mov edi,y;
		imul edi,Pitch;
		add edi,Surface;
		mov esi,x;
		lea edi,[edi+esi*2];
		mov eax,sx;
		
		mov ecx,len;
		jcxz ZeroLen;

		cdq;
		idiv ecx;
		mov esi,eax;
		xor eax,eax;
		or edx,edx;
		jge IsPlus;
		dec esi;
		neg edx;
		div ecx;
		neg eax;
		jmp WasMinus;
IsPlus:
		div ecx;
WasMinus:
		mov XUnder,eax;

		mov eax,sy;
		cdq;
		idiv ecx;
		imul eax,Pitch;
		sar eax,1;
		add esi,eax;
		xor eax,eax;
		or edx,edx;
		jge IsPlus2;
		neg edx;
		div ecx;		
		mov YUnder,eax;

		mov edx,0x80000000;
		mov ebx,edx;
		mov eax,DrawColor;
		dec esi;
		neg NPitch;
		jmp DrawLineLoop;
IsPlus2:
		div ecx;
		mov YUnder,eax;

		mov edx,0x80000000;
		mov ebx,edx;
		mov eax,DrawColor;
		dec esi;

DrawLineLoop:
		stosw;
		ror edi,1;
		add ebx,XUnder;
		adc edi,esi;
		rol edi,1;
		add edx,YUnder;
		jnc NoCarry;
		add edi,NPitch;
NoCarry:
		loop DrawLineLoop;
ZeroLen:
		stosw;
	}}

void DrawLine8(int x,int y,int tx,int ty){
	if((x<0 && tx<0) || (x>=SFWidth && tx>=SFWidth)) return;
	if((y<0 && ty<0) || (y>=SFHeight && ty>=SFHeight)) return;
	if(tx==x){
		int len=abs(y-ty)+1;
		y=min(y,ty);
		DrawVLine8(x,y,len);
		return;}
	if(ty==y){
		int len=abs(x-tx)+1;
		x=min(x,tx);
		DrawHLine8(x,y,len);
		return;}
	int sx=tx-x;
	int sy=ty-y;
	if(x<0){ y=y-x*sy/sx; x=0;}
	if(y<0){ x=x-y*sx/sy; y=0;}
	if(x>=SFWidth){ y=y+(SFWidth-1-x)*sy/sx; x=SFWidth-1;}
	if(y>=SFHeight){ x=x+(SFHeight-1-y)*sx/sy; y=SFHeight-1;}
	if(tx<0){ ty=ty-tx*sy/sx; tx=0;}
	if(ty<0){ tx=tx-ty*sx/sy; ty=0;}
	if(tx>=SFWidth){ ty=ty+(SFWidth-1-tx)*sy/sx; tx=SFWidth-1;}
	if(ty>=SFHeight){ tx=tx+(SFHeight-1-ty)*sx/sy; ty=SFHeight-1;}

	sx=tx-x;
	sy=ty-y;

	DWORD NPitch=Pitch;
	
	int len=max(abs(sx),abs(sy));
	DWORD XUnder,YUnder;

	_asm{
		mov edi,y;
		imul edi,Pitch;
		add edi,Surface;
		add edi,x;
		mov eax,sx;
		
		mov ecx,len;
		jcxz ZeroLen;

		cdq;
		idiv ecx;
		mov esi,eax;
		xor eax,eax;
		or edx,edx;
		jge IsPlus;
		dec esi;
		neg edx;
		div ecx;
		neg eax;
		jmp WasMinus;
IsPlus:
		div ecx;
WasMinus:
		mov XUnder,eax;

		mov eax,sy;
		cdq;
		idiv ecx;
		imul eax,Pitch;
		add esi,eax;
		xor eax,eax;
		or edx,edx;
		jge IsPlus2;
		neg edx;
		div ecx;		
		mov YUnder,eax;

		mov edx,0x80000000;
		mov ebx,edx;
		mov eax,DrawColor;
		dec esi;
		neg NPitch;
		jmp DrawLineLoop;
IsPlus2:
		div ecx;
		mov YUnder,eax;

		mov edx,0x80000000;
		mov ebx,edx;
		mov eax,DrawColor;
		dec esi;

DrawLineLoop:
		stosb;
		add ebx,XUnder;
		adc edi,esi;
		add edx,YUnder;
		jnc NoCarry;
		add edi,NPitch;
NoCarry:
		loop DrawLineLoop;
ZeroLen:
		stosb;
	}}

void DrawLine8_Or(int x,int y,int tx,int ty){
	if((x<0 && tx<0) || (x>=SFWidth && tx>=SFWidth)) return;
	if((y<0 && ty<0) || (y>=SFHeight && ty>=SFHeight)) return;
	if(tx==x){
		int len=abs(y-ty)+1;
		y=min(y,ty);
		DrawVLine8_Or(x,y,len);
		return;}
	if(ty==y){
		int len=abs(x-tx)+1;
		x=min(x,tx);
		DrawHLine8_Or(x,y,len);
		return;}
	int sx=tx-x;
	int sy=ty-y;
	if(x<0){ y=y-x*sy/sx; x=0;}
	if(y<0){ x=x-y*sx/sy; y=0;}
	if(x>=SFWidth){ y=y+(SFWidth-1-x)*sy/sx; x=SFWidth-1;}
	if(y>=SFHeight){ x=x+(SFHeight-1-y)*sx/sy; y=SFHeight-1;}
	if(tx<0){ ty=ty-tx*sy/sx; tx=0;}
	if(ty<0){ tx=tx-ty*sx/sy; ty=0;}
	if(tx>=SFWidth){ ty=ty+(SFWidth-1-tx)*sy/sx; tx=SFWidth-1;}
	if(ty>=SFHeight){ tx=tx+(SFHeight-1-ty)*sx/sy; ty=SFHeight-1;}

	sx=tx-x;
	sy=ty-y;

	DWORD NPitch=Pitch;
	
	int len=max(abs(sx),abs(sy));
	DWORD XUnder,YUnder;

	_asm{
		mov edi,y;
		imul edi,Pitch;
		add edi,Surface;
		add edi,x;
		mov eax,sx;
		
		mov ecx,len;
		jcxz ZeroLen;

		cdq;
		idiv ecx;
		mov esi,eax;
		xor eax,eax;
		or edx,edx;
		jge IsPlus;
		dec esi;
		neg edx;
		div ecx;
		neg eax;
		jmp WasMinus;
IsPlus:
		div ecx;
WasMinus:
		mov XUnder,eax;

		mov eax,sy;
		cdq;
		idiv ecx;
		imul eax,Pitch;
		add esi,eax;
		xor eax,eax;
		or edx,edx;
		jge IsPlus2;
		neg edx;
		div ecx;		
		mov YUnder,eax;

		mov edx,0x80000000;
		mov ebx,edx;
		mov eax,DrawColor;
		neg NPitch;
		jmp DrawLineLoop;
IsPlus2:
		div ecx;
		mov YUnder,eax;

		mov edx,0x80000000;
		mov ebx,edx;
		mov eax,DrawColor;

DrawLineLoop:
		or [edi],al;
		add ebx,XUnder;
		adc edi,esi;
		add edx,YUnder;
		jnc NoCarry;
		add edi,NPitch;
NoCarry:
		loop DrawLineLoop;
ZeroLen:
		stosb;
	}}

void DrawHLine8(int x,int y,int Len){
	if(x<0){
		Len+=x;
		x=0;}
	if(x+Len>SFWidth) Len=SFWidth-x;
	if(Len<=0 || y<0 || y>=SFHeight) return;
	_asm{
		mov edi,y;
		imul edi,Pitch;
		add edi,x;
		add edi,Surface;
		mov eax,DrawColor;
		mov ecx,Len;
		shr ecx,2;
		rep stosd;
		mov ecx,Len;
		and ecx,3;
		rep stosb;
	}}

void DrawVLine8(int x,int y,int Len){
	if(y<0){
		Len+=y;
		y=0;}
	if(y+Len>SFWidth) Len=SFWidth-y;
	if(Len<=0 || y<0 || x>=SFWidth) return;
	_asm{
		mov ebx,Pitch;
		dec ebx;
		mov edi,y;
		imul edi,Pitch;
		add edi,x;
		add edi,Surface;
		mov eax,DrawColor;
		mov ecx,Len;
VLineLoop:
		stosb;
		add edi,ebx;
		loop VLineLoop;
	}}

void DrawHLine16(int x,int y,int Len){
	if(x<0){
		Len+=x;
		x=0;}
	if(x+Len>SFWidth) Len=SFWidth-x;
	if(Len<=0 || y<0 || y>=SFHeight) return;
	_asm{
		mov edi,y;
		imul edi,Pitch;
		mov eax,x;
		lea edi,[edi+eax*2];
		add edi,Surface;
		mov eax,DrawColor;
		mov ecx,Len;
		shr ecx,1;
		rep stosd;
		mov ecx,Len;
		and ecx,1;
		rep stosw;
	}}

void DrawVLine16(int x,int y,int Len){
	if(y<0){
		Len+=y;
		y=0;}
	if(y+Len>SFWidth) Len=SFWidth-y;
	if(Len<=0 || y<0 || x>=SFWidth) return;
	_asm{
		mov ebx,Pitch;
		dec ebx;
		mov edi,y;
		imul edi,Pitch;
		mov eax,x;
		lea edi,[edi+eax*2];
		add edi,Surface;
		mov eax,DrawColor;
		mov ecx,Len;
VLineLoop:
		stosw;
		add edi,ebx;
		loop VLineLoop;
	}}

void DrawHLine32(int x,int y,int Len){
	if(x<0){
		Len+=x;
		x=0;}
	if(x+Len>SFWidth) Len=SFWidth-x;
	if(Len<=0 || y<0 || y>=SFHeight) return;
	_asm{
		mov edi,y;
		imul edi,Pitch;
		mov eax,x;
		lea edi,[edi+eax*4];
		add edi,Surface;
		mov eax,DrawColor;
		mov ecx,Len;
		rep stosd;
	}}

void DrawVLine32(int x,int y,int Len){
	if(y<0){
		Len+=y;
		y=0;}
	if(y+Len>SFWidth) Len=SFWidth-y;
	if(Len<=0 || y<0 || x>=SFWidth) return;
	_asm{
		mov ebx,Pitch;
		dec ebx;
		mov edi,y;
		imul edi,Pitch;
		mov eax,x;
		lea edi,[edi+eax*4];
		add edi,Surface;
		mov eax,DrawColor;
		mov ecx,Len;
VLineLoop:
		stosd;
		add edi,ebx;
		loop VLineLoop;
	}}

void DrawHLine8_Or(int x,int y,int Len){
	if(x<0){
		Len+=x;
		x=0;}
	if(x+Len>SFWidth) Len=SFWidth-x;
	if(Len<=0 || y<0 || y>=SFHeight) return;
	_asm{
		mov edi,y;
		imul edi,Pitch;
		add edi,x;
		add edi,Surface;
		mov eax,DrawColor;
		mov ecx,Len;
		shr ecx,2;
		jcxz SkipDword;
DwordLoop:
		and [edi],eax;
		add edi,4;
		loop DwordLoop;
SkipDword:
		mov ecx,Len;
		and ecx,3;
		jcxz SkipByte;
ByteLoop:
		and [edi],al;
		add edi,1;
		loop ByteLoop;
SkipByte:
	}}

void DrawVLine8_Or(int x,int y,int Len){
	if(y<0){
		Len+=y;
		y=0;}
	if(y+Len>SFWidth) Len=SFWidth-y;
	if(Len<=0 || y<0 || x>=SFWidth) return;
	_asm{
		mov ebx,Pitch;
		dec ebx;
		mov edi,y;
		imul edi,Pitch;
		add edi,x;
		add edi,Surface;
		mov eax,DrawColor;
		mov ecx,Len;
VLineLoop:
		and [edi],al;
		add edi,ebx;
		loop VLineLoop;
	}}

void DrawRect8(int Left,int Top,int Right,int Bottom){
	int minx=min(Left,Right);
	int miny=min(Top,Bottom);
	int absx=abs(Right-Left);
	int absy=abs(Bottom-Top);
	DrawHLine8(minx,Top,absx);
	DrawHLine8(minx+1,Bottom,absx);
	DrawVLine8(Left,miny+1,absy);
	DrawVLine8(Right,miny,absy);}

void DrawRect16(int Left,int Top,int Right,int Bottom){
	int minx=min(Left,Right);
	int miny=min(Top,Bottom);
	int absx=abs(Right-Left);
	int absy=abs(Bottom-Top);
	DrawHLine16(minx,Top,absx);
	DrawHLine16(minx+1,Bottom,absx);
	DrawVLine16(Left,miny+1,absy);
	DrawVLine16(Right,miny,absy);}

void DrawRect32(int Left,int Top,int Right,int Bottom){
	int minx=min(Left,Right);
	int miny=min(Top,Bottom);
	int absx=abs(Right-Left);
	int absy=abs(Bottom-Top);
	DrawHLine32(minx,Top,absx);
	DrawHLine32(minx+1,Bottom,absx);
	DrawVLine32(Left,miny+1,absy);
	DrawVLine32(Right,miny,absy);}

void SetDIBMP(LPBITMAP Bitmap,LPCVOID B8From,LPCVOID Palette){
	int Width=Bitmap->bmWidth;
	int SkipLen=(Bitmap->bmWidthBytes-Width*3);
	_asm{
		mov eax,Bitmap;
		mov edi,[eax+BITMAP::bmBits];
		mov esi,B8From;
		mov edx,Palette;
		
		mov ecx,[eax+BITMAP::bmHeight];
		dec ecx;
HLoop:
		push ecx;
		mov ecx,Width;
ChangeLoop:
		xor eax,eax;
		lodsb;
		mov eax,[edx+eax*4];
		stosd;
		dec edi;
		loop ChangeLoop;
		pop ecx;
		add edi,SkipLen;
		loop HLoop;

		mov ecx,Width;
		dec ecx;
ChangeLoop2:
		xor eax,eax;
		lodsb;
		mov eax,[edx+eax*4];
		stosd;
		dec edi;
		loop ChangeLoop2;

		dec edi;
		and dword ptr[edi],0xff;
		xor eax,eax;
		lodsb;
		mov eax,[edx+eax*4];
		shl eax,8;
		or [edi],eax;

	}}

int SetFromDIBMP(LPVOID B8To,LPBITMAP Bitmap,LPVOID Palette){
	int Width=Bitmap->bmWidth;
	int SkipLen=(Bitmap->bmWidthBytes-Bitmap->bmWidth*3);
	int UseColor=0;
	int ESPSave;
	_asm{
		mov ESPSave,esp;
		mov eax,Bitmap;
		mov esi,[eax+BITMAP::bmBits];
		mov edx,B8To;

		lodsd;
		sub esi,2;
		and eax,0xffffff;

		call WriteColor;

		mov ecx,Width;
		dec ecx;
ChangeLoop:
		lodsd;
		dec esi;
		shr eax,8;

		call WriteColor;

		loop ChangeLoop;
		add esi,SkipLen;

		mov ecx,Bitmap;
		mov ecx,[ecx+BITMAP::bmHeight];
		dec ecx;
HLoop:
		push ecx;
		mov ecx,Width;
ChangeLoop2:
		lodsd;
		dec esi;
		shr eax,8;

		call WriteColor;

		loop ChangeLoop2;
		pop ecx;
		add esi,SkipLen;
		loop HLoop;
		pop esi;
	}
	return UseColor;
	_asm{
WriteColor:
		//동일 색 찾기
		push ecx;
		mov ecx,UseColor;
		mov edi,Palette;
		repne scasd;
		je FindSame;
		cmp UseColor,0x100;
		je OutInZero;
		mov ebx,UseColor;
		shl ebx,2;
		add ebx,Palette;
		mov [ebx],eax;
		inc UseColor;
FindSame:
		sub ecx,UseColor;
		inc ecx;
		neg ecx;
		mov [edx],cl;
		inc edx;
		pop ecx;
		ret;
OutInZero:
		mov esp,ESPSave;
	}
	return 0;}

void VerticalMirrorBits(LPVOID To,LPCVOID From,DWORD Width,DWORD Height){
	LPVOID tFrom;
	BOOL IsTempMemory=(abs(int((LPBYTE)From-(LPBYTE)To))<int(Width*Height));
	if(IsTempMemory){
		tFrom=new BYTE[Width*Height];
		memcpy(tFrom,From,Width*Height);}
	else tFrom=(LPVOID)From;
	_asm{
		mov edi,To;
		mov esi,Height;
		dec esi;
		imul esi,Width;
		add esi,tFrom;

		mov edx,Width;
		shl edx,1;
		mov ecx,Height;
VertMLoop:
		push ecx;
		mov ecx,Width;
		shr ecx,2;
		rep movsd;
		mov ecx,Width;
		and ecx,3;
		rep movsb;
		pop ecx;
		sub esi,edx;
		loop VertMLoop;
	}
	if(IsTempMemory) DEL(tFrom);}

void SetToBMPBits(LPVOID Bits,LPCVOID From,DWORD Height,DWORD WidthBytes){
	DWORD BytesStore=WidthBytes%4;
	DWORD OffsetBytes=((4-BytesStore)%4);

	_asm{
		mov esi,From;
		mov edi,Bits;
		shr WidthBytes,2;
		mov ecx,Height;
HLoop:
		push ecx;

		mov ecx,WidthBytes;
		rep movsd;
		mov ecx,BytesStore;
		rep movsb;

		add edi,OffsetBytes;

		pop ecx;
		loop HLoop;
	}}

LPVOID new_BitsFromSEB(LPCTSTR FileName,LPDWORD Width,LPDWORD Height){
	DWORD Junk,Size;
	LPBYTE ReadData;
	HANDLE hFile=CreateFile(FileName,GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(hFile==INVALID_HANDLE_VALUE) return 0;
	ReadData=new BYTE[Size=GetFileSize(hFile,&Junk)];
	ReadFile(hFile,ReadData,Size,&Junk,0);
	*Width=((int*)ReadData)[0];
	*Height=((int*)ReadData)[1];
	LPBYTE SEB=ReadData+8;
	DWORD BitsSize=(*Width)*(*Height);
	LPBYTE Bits=new BYTE[BitsSize];
	SEBConvertToBits(Bits,SEB,Size-8);
	DEL(ReadData);
	CloseHandle(hFile);
	return Bits;}

DWORD ChgRGB(DWORD Color){
	_asm{
		mov eax,Color;
		xchg al,ah;
		ror eax,8;
		xchg al,ah;
		rol eax,8;
		xchg al,ah;
	}
}

HBITMAP CreateDIBS(int Width,int Height,WORD BitsCount,const DWORD *Palette,LPVOID Bitmap){
	HBITMAP hBitmap;
	HDC hDC=GetDC(0);
	LPBITMAPINFO bi=(LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];
	memset(bi,0,sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD));
	bi->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bi->bmiHeader.biBitCount=BitsCount;
	bi->bmiHeader.biCompression=BI_RGB;
	bi->bmiHeader.biPlanes=1;
	bi->bmiHeader.biWidth=Width;
	bi->bmiHeader.biHeight=Height;
	if(Palette) memcpy(bi->bmiColors,Palette,256*sizeof(RGBQUAD));
	hBitmap=CreateDIBSection(hDC,bi,DIB_RGB_COLORS,(Bitmap?(&Bitmap):0),0,0);
	ReleaseDC(0,hDC);
	DEL(bi);
	return hBitmap;}

void SetSurface(LPVOID surface,DWORD Width,DWORD Height,LPVOID zbuffer){
	Surface=surface;
	Pitch=SFWidth=Width;
	SFHeight=Height;
	ZBuffer=zbuffer;
	ZBufferOffset=(int)((LPBYTE)Surface-(LPBYTE)ZBuffer);}

BOOL SetPaletteDDraw(LPDWORD Palette){
	HRESULT hr;
	if(lpPal){
		hr=lpPal->SetEntries(0,0,256,(LPPALETTEENTRY)Palette);
		if(FAILED(hr)) return 0;}
	else{
		hr=lpDD->CreatePalette(DD_PALETTEFLAGS,(LPPALETTEENTRY)Palette,&lpPal,0);
		if(FAILED(hr)) return 0;
		hr=lpSF->SetPalette(lpPal);
		if(FAILED(hr)) return 0;}
	return 1;}
