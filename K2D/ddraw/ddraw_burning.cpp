#include "stdafx.h"

#include "HMDSelf.h"
#include "HMDDDraw.h"

Variable(DRAWTABLE,DrawTable);
Variable(KDRAWTOOL,DrawTool);
VariableEnum(DRAWTABLE,DTStack,DTSTACKSIZE);
Variable(DWORD,DTStackPos);
Variable(DWORD,WinPixelSize);
Variable(LPIMAGE,img_WinBack);
Variable(LPDIRECTDRAWSURFACE,off_WinBack);
Variable(DCTARGET,DCTarget);
Variable(HDC,hDC);

Variable(LPDIRECTDRAW,lpDD);
Variable(LPDIRECTDRAWSURFACE,lpSF);
Variable(LPDIRECTDRAWSURFACE,lpSF2);
Variable(LPDIRECTDRAWCLIPPER,lpClip);
Variable(DDSURFACEDESC,ddsd);
Variable(DDSURFACEDESC,locksd);
Variable(DWORD,LockCount);


DLL void HMDDT_SetTri(int ToX,int ToY,DWORD W){
	_asm{
		//시작 길이
		mov eax,W;
		shl eax,HALFDOT;
		or eax,((1<<HALFDOT)-1);
		mov DrawTool.Len,eax;

		//추가 길이
		mov eax,W;
		shl eax,HALFDOT;
		neg eax;
		cdq;
		idiv ToY;
		mov DrawTool.Tan,eax;

		//점프 길이
		mov eax,ToX;
		shl eax,HALFDOT;
		cdq;
		idiv ToY;
		mov ebx,DrawTable.Pitch;
		shl ebx,HALFDOT;
		add eax,ebx;
		mov DrawTool.Skip,eax;

		//끝 길이
		imul eax,ToY;
		mov DrawTool.End,eax;
	};}

DLL void HMDDT_SetTriR(int ToX,int ToY,DWORD W){
	_asm{
		//시작 길이
		mov DrawTool.Len,((1<<HALFDOT)-1);

		//추가 길이
		mov eax,W;
		shl eax,HALFDOT;
		cdq;
		idiv ToY;
		mov DrawTool.Tan,eax;
		
		//점프 길이
		mov eax,ToX;
		shl eax,HALFDOT;
		cdq;
		idiv ToY;
		mov ebx,DrawTable.Pitch;
		shl ebx,HALFDOT;
		add eax,ebx;
		mov DrawTool.Skip,eax;

		//끝 길이
		imul eax,ToY;
		mov DrawTool.End,eax;
	};}

DLL void HMDDT_SetPeral(int ToX,int ToY,DWORD W){
	_asm{
		//시작 길이
		mov eax,W;
		shl eax,HALFDOT;
		mov DrawTool.Len,eax;

		//점프 길이
		mov eax,ToX;
		shl eax,HALFDOT;
		mov DrawTool.Tan,0;
		cdq;
		idiv ToY;
		mov ebx,DrawTable.Pitch;
		shl ebx,HALFDOT;
		add eax,ebx;
		mov DrawTool.Skip,eax;

		//끝 길이
		imul eax,ToY;
		mov DrawTool.End,eax;
	};}

DLL void HMDDT_SetTrape(int ToX,int ToY,DWORD W,DWORD W2){
	_asm{
		//시작 길이
		mov eax,W;
		shl eax,HALFDOT;
		mov DrawTool.Len,eax;

		//추가 길이
		mov eax,W2;
		sub eax,W;
		shl eax,HALFDOT;
		cdq;
		idiv ToY;
		mov DrawTool.Tan,eax;

		//점프 길이
		mov eax,ToX;
		shl eax,HALFDOT;
		idiv ToY;
		mov ebx,DrawTable.Pitch;
		shl ebx,HALFDOT;
		add eax,ebx;
		mov DrawTool.Skip,eax;

		//끝 길이
		imul eax,ToY;
		mov DrawTool.End,eax;
	};}

DLL void HMDDT_Draw(){
	_asm{
		push esi;
		push edi;
		xor edx,edx;
		mov ebx,DrawTool.Len;
		mov eax,DrawTool.Color;
DrawLoop:
		mov ecx,ebx;
		sar ecx,HALFDOT;
		mov edi,edx;
		sar edi,HALFDOT;
		shl edi,2;
		add edi,DrawTool.Dest;
		rep stosd;
		add edx,DrawTool.Skip;
		add ebx,DrawTool.Tan;
		cmp edx,DrawTool.End;
		jne DrawLoop;
		pop edi;
		pop esi;
	};}

DLL KRESULT HMDDD_ResizeBS(LPCRECT rect){
	SDEL(img_WinBack);
	SREL(off_WinBack);
	img_WinBack=new IMAGE(rect->right,rect->bottom,1,0);
	if(ERR(img_WinBack->kr)){
		_asm sub esp,4*4;
		HMDER_AddA(("IMAGE::IMAGE\n\
					width:%d\n\
					height:%d\n\
					page:%d\n\
					useTransparent:%d"));
		_asm add esp,4*4;
		return KR_ERR;}
	if(ERR(HMDDD_CreateOffScreen(rect->right,rect->bottom,&off_WinBack))){
		_asm sub esp,3*4;
		HMDER_AddA(("HMDDD_CreateOffScreen\n\
					Width:%d\n\
					Height:%d\n\
					Surface:%08X"));
		_asm add esp,3*4;
		return KR_ERR;}
	return KR_OK;}

KRESULT HMDDD_StartupNC(BOOL Full,DWORD Width,DWORD Height){
	HRESULT hr;
	DWORD Flag;
	hr=DirectDrawCreate(0,&lpDD,0);
	if(FAILED(hr)){
		_asm sub esp,3*4;
		HMDER_AddA(("DirectDrawCreate\n\
					lpGuid:%08X\n\
					lplpDD:%08X\n\
					pUnkOuter:%08X"));
		_asm add esp,3*4;
		return KR_ERR;}
	if(Full) Flag=DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN;
	else Flag=DDSCL_NORMAL;
	hr=lpDD->SetCooperativeLevel(Full?hWnd:0,Flag);
	if(FAILED(hr)){
		_asm sub esp,2*4;
		HMDER_AddA(("LPDIRECTDRAW::SetCooperativeLevel\n\
					HWND:%08X\n\
					DWORD:%08X"));
		_asm add esp,2*4;
		return KR_ERR;}
	if(Full){
		hr=lpDD->SetDisplayMode(Width,Height,32);
		if(FAILED(hr)){
			_asm sub esp,3*4;
			HMDER_AddA(("LPDIRECTDRAW::SetDisplayMode\n\
						DWORD:%d\n\
						DWORD:%d\n\
						DWORD:%d"));
			_asm add esp,3*4;
			return KR_ERR;}}
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize=sizeof(ddsd);
	ddsd.dwFlags=DDSD_CAPS;
	ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;
	if(Full){
		ddsd.dwFlags|=DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps|=DDSCAPS_FLIP|DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount=1;}
	hr=lpDD->CreateSurface(&ddsd,&lpSF,0);
	if(FAILED(hr)){
		_asm sub esp,2*4;
		HMDER_AddA(("LPDIRECTDRAW::CreateSurface\n\
					dwSize:%d\n\
					dwFlags:%08X\n\
					dwHeight:%d\n\
					dwWidth:%d\n\
					lPitch/dwLinearSize:%d\n\
					dwBackBufferCount:%d\n\
					dwMipMapCount/dwZBufferBitDepth/dwRefreshRate:%d\n\
					dwAlphaBitDepth:%d\n\
					dwReserved:%d\n\
					lpSurface:%08X\n\
					ddckCKDestOverlay:(%08X,%08X)\n\
					ddckCKDestBlt:(%08X,%08X)\n\
					ddckCKSrcOverlay:(%08X,%08X)\n\
					ddckCKSrcBlt:(%08X,%08X)\n\
					ddpfPixelFormat:{\n\
					dwSize:%d\n\
					dwFlags:%08X\n\
					dwFourCC:%08X\n\
					BitCount:%d\n\
					dwRBitMask:%08X\n\
					dwGBitMask:%08X\n\
					dwBBitMask:%08X\n\
					AlphaBitMask:%08X\n\
					}\n\
					ddsCaps:%08X\n\
					LPDIRECTDRAWSURFACE:%08X\n\
					IUnknown:%08X"),ddsd);
		_asm add esp,2*4;
		return KR_ERR;}
	if(Full){
		DDSCAPS ddscaps;
		ddscaps.dwCaps=DDSCAPS_BACKBUFFER;
		hr=lpSF2->GetAttachedSurface(&ddscaps,&lpSF);
		if(FAILED(hr)){
			_asm sub esp,2*4;
			HMDER_AddA(("LPDIRECTDRAWSURFACE::GetAttachedSurface\n\
						LPDDSCAPS:\n\
						LPDIRECTDRAWSURFACE:%08X"));
			_asm add esp,2*4;
			return KR_ERR;}}
	else{
		hr=lpDD->CreateClipper(0,&lpClip,0); 
		if(FAILED(hr)){
			_asm sub esp,3*4;
			HMDER_AddA(("LPDIRECTDRAW::CreateClipper\n\
						DWORD:%08X\n\
						LPDIRECTDRAWCLIPPER:%08X\n\
						IUnknown:%08X"));
			_asm add esp,3*4;
			return KR_ERR;}
		hr=lpClip->SetHWnd(0,hWnd);
		if(FAILED(hr)){
			_asm sub esp,2*4;
			HMDER_AddA(("LPDIRECTDRAWCLIPPER::SetHWnd\n\
						DWORD:%08X\n\
						HWND:%08X"));
			_asm add esp,2*4;
			return KR_ERR;}
        hr=lpSF->SetClipper(lpClip); 
		if(FAILED(hr)){
			_asm sub esp,4;
			HMDER_AddA(("LPDIRECTDRAWSURFACE::SetCliper\n\
						LPDIRECTDRAWCLIPPER:%08X"));
			_asm add esp,4;
			return KR_ERR;}}
	DDSURFACEDESC ddsd2;
	if(ERR(HMDDD_Lock(&ddsd2,1))){
		_asm sub esp,2*4;
		HMDER_AddA(("HMDDD_Lock\n\
					ddsd:%08X\n\
					Wait:True(%d)"));
		_asm add esp,2*4;
		return KR_ERR;}
	WinPixelSize=ddsd2.ddpfPixelFormat.dwRGBBitCount;
	HMDDD_Unlock();
	return KR_OK;}

DLL KRESULT HMDDD_Lock(LPDDSURFACEDESC ddsd,BOOL Wait){
	HRESULT hr;
	DDSURFACEDESC tempsd;
	if(!ddsd) ddsd=&tempsd;
	if(LockCount==0){
		ddsd->dwSize=sizeof(DDSURFACEDESC);
		if(img_WinBack){
			ddsd->lPitch=(ddsd->dwWidth=img_WinBack->Width)<<2;
			ddsd->dwHeight=img_WinBack->Height;
			ddsd->lpSurface=img_WinBack->Get(0,0);
			DCTarget=DCT_OFFSCREEN;}
		else if(lpSF2){
			hr=lpSF2->Lock(0,ddsd,Wait?DDLOCK_WAIT:DDLOCK_DONOTWAIT,0);
			DCTarget=DCT_SURFACE2;}
		else{
			hr=lpSF->Lock(0,ddsd,Wait?DDLOCK_WAIT:DDLOCK_DONOTWAIT,0);
			DCTarget=DCT_SURFACE;}
		if(FAILED(hr)){
			_asm sub esp,4*4;
			HMDER_AddA(("LPDIRECTSURFACE::Lock\n\
						LPRECT:%08X\n\
						LPDDSURFACEDESC:%08X\n\
						DWORD:%08X\n\
						HANDLE:%08X"));
			_asm add esp,4*4;
			return KR_ERR;}
		locksd=*ddsd;
		DTStack[DTStackPos]=DrawTable;
		DrawTable.Width=ddsd->dwWidth;
		DrawTable.Height=ddsd->dwHeight;
		DrawTable.Pitch=ddsd->lPitch>>2;
		DrawTable.Page=1;
		DrawTable.Surface=(LPDWORD)ddsd->lpSurface;
		DTStackPos++;}
	else if(LockCount==0xffffffff){
		HMDER_AddA(("LockCount==0xffffffff"));
		return KR_ERR;}
	else *ddsd=locksd;
	LockCount++;
	return KR_OK;}

DLL KRESULT HMDDD_Unlock(){
	HRESULT hr;
	LockCount--;
	if(LockCount==0){
		switch(DCTarget){
			case DCT_OFFSCREEN:{
				if(!off_WinBack) break;
				DDSURFACEDESC ddsd;
				ddsd.dwSize=sizeof(ddsd);
				if(FAILED(off_WinBack->Lock(0,&ddsd,DDLOCK_WAIT,0))){
					_asm sub esp,4*4;
					HMDER_AddA("LPDIRECTDRAWSURFACE::Lock\n\
							   LPRECT:%08X\n\
							   LPDDSURFACEDESC:%08X\n\
							   DWORD:%08X\n\
							   HANDLE:%08X");
					_asm add esp,4*4;
					return KR_ERR;}
				_asm push esi;
				_asm push edi;
				int WLoop,Skip;
				WORD Temp;
				switch(WinPixelSize){
					case 16:
						WLoop=ddsd.dwWidth;
						Skip=ddsd.lPitch-ddsd.dwWidth*2;
						_asm{
							mov edi,ddsd.lpSurface;
							mov esi,img_WinBack;
							mov esi,dword ptr[esi+IMAGE::Image];
							mov ecx,ddsd.dwHeight;
Loop16H:
							push ecx;
							mov ecx,WLoop;
Loop16W:
							lodsd;
							mov ebx,eax;
							mov edx,eax;
							and ebx,0xF8;
							sar ebx,3;
							and edx,0xFC00;
							sar edx,5;
							and eax,0xF80000;
							sar eax,8;
							or eax,edx;
							or eax,ebx;
							mov ebx,ecx;
							and ebx,1;
							cmp ebx,0;
							jne ifzero;
							mov Temp,ax;
							jmp ifnzero;
ifzero:
							shl eax,16;
							mov ax,Temp;
							stosd;
ifnzero:
							loop Loop16W;
							add edi,Skip;
							pop ecx;
							loop Loop16H;
						}
						break;
					case 24:
						WLoop=ddsd.dwWidth;
						Skip=ddsd.lPitch-(ddsd.dwWidth*4+2)/3;
						_asm{
							mov edi,ddsd.lpSurface;
							mov esi,img_WinBack;
							mov esi,dword ptr[esi+IMAGE::Image];
							mov ecx,ddsd.dwHeight;
Loop24H:
							push ecx;
							mov ecx,WLoop;
Loop24W:
							movsd;
							sub edi,1;
							loop Loop24W;
							add edi,Skip;
							pop ecx;
							loop Loop24H;
						}
						break;
					case 32:
						WLoop=ddsd.dwWidth;
						Skip=ddsd.lPitch-ddsd.dwWidth*4;
						_asm{
							mov edi,ddsd.lpSurface;
							mov esi,img_WinBack;
							mov esi,dword ptr[esi+IMAGE::Image];
							mov ecx,ddsd.dwHeight;
Loop32H:
							push ecx;
							mov ecx,WLoop;
							rep movsd;
							add edi,Skip;
							pop ecx;
							loop Loop32H;
						}
						break;}
				_asm pop edi;
				_asm pop esi;
				off_WinBack->Unlock(0);
				hr=DD_OK;
				break;}
			case DCT_SURFACE: 
				hr=lpSF->Unlock(0);
				break;
			case DCT_SURFACE2:
				hr=lpSF2->Unlock(0);
				break;}
		if(FAILED(hr)){
			_asm sub esp,4;
			HMDER_AddA(("LPDIRECTSURFACE::Unlock\n\
						LPVOID:%08X"));
			_asm add esp,4;
			return KR_ERR;}}
	else if(LockCount==-1){
		LockCount=0;
		HMDER_AddA(("LockCount==-1"));}
	HMDDD_TargetRelease();
	return KR_OK;}

DLL void HMDDD_Cleanup(){
	SDEL(img_WinBack);
	SREL(off_WinBack);
	SREL(lpClip);
	SREL(lpSF2);
	SREL(lpSF);
	SREL(lpDD);}

DLL KRESULT HMDDD_CreateOffScreen(DWORD Width,DWORD Height,LPDIRECTDRAWSURFACE* Surface){
	HRESULT hr;
	DDSURFACEDESC ddsd;
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize=sizeof(ddsd);
	ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY;
	ddsd.dwWidth=Width;
	ddsd.dwHeight=Height;
	hr=lpDD->CreateSurface(&ddsd,Surface,0);
	if(FAILED(hr)){
		_asm sub esp,3*4;
		HMDER_AddA(("LPDIRECTDRAW::CreateSurface\n\
					LPDDSURFACEDESC:%08X\n\
					LPDIRECTDRAWSURFACE:%08X\n\
					IUnknown:%08X"));
		_asm add esp,3*4;
		return KR_ERR;}
	return KR_OK;}

DLL void HMDDD_Triangle(LPPOINT lpcpt){
	if((DWORD)lpcpt[0].x>=DrawTable.Width
		|| (DWORD)lpcpt[1].x>=DrawTable.Width
		|| (DWORD)lpcpt[2].x>=DrawTable.Width) return;
	if((DWORD)lpcpt[0].y>=DrawTable.Height
		|| (DWORD)lpcpt[1].y>=DrawTable.Height
		|| (DWORD)lpcpt[2].y>=DrawTable.Height) return;
	POINT pt[3];
	int X,W,ToY;
	POINT npt;
	SoltPointY(pt,lpcpt,3);
	if(pt[2].y==pt[0].y) return;
	npt.x=CutOnY(pt[0].x,pt[0].y,pt[2].x,pt[2].y,pt[1].y);
	npt.y=pt[1].y;
	if(npt.x>pt[1].x){
		X=pt[1].x;
		W=(npt.x-pt[1].x+1);}
	else{
		X=npt.x;
		W=(pt[1].x-npt.x+1);}
	if(!W) return;
	if(ToY=npt.y-pt[0].y){
		HMDDT_SetDest(pt[0].x,pt[0].y);
		HMDDT_SetTriR(X-pt[0].x,ToY,W);
		HMDDT_Draw();}
	if(ToY=pt[2].y-npt.y){
		HMDDT_SetDest(X,npt.y);
		HMDDT_SetTri(pt[2].x-X,ToY,W);
		HMDDT_Draw();}}

DLL DWORD HMDDD_GDIColor(DWORD Color){
	_asm{
		mov eax,Color;
		mov ebx,eax;
		ror eax,16;
		mov al,bl;
		ror eax,16;
		ror ebx,16;
		mov al,bl;
	};}

DLL void HMDDD_UpdateBS(){
	if(!lpSF) return;
	POINT pt={0,0};
	RECT crect;
	ClientToScreen(hWnd,&pt);
	GetClientRect(hWnd,&crect);
	RECT rect={pt.x,pt.y,pt.x+crect.right,pt.y+crect.bottom};
	lpSF->Blt(&rect,off_WinBack,&crect,DDBLT_WAIT,0);}

DLL IMAGE::IMAGE(DWORD width,DWORD height,DWORD page,DWORD useTransparent){
	memset(this,0,sizeof(IMAGE));
	Image=new DWORD[Size=(width*height*page)];
	if(!Image){
		_asm sub esp,4;
		HMDER_AddA(("new BYTE\n\
					Size: %d"));
		_asm add esp,4;
		kr=KR_ERR;
		return;}
	Width=width;
	Height=height;
	Page=page;
	UseTransparent=useTransparent;}

DLL IMAGE::~IMAGE(){
	SDEL(Image);}

DLL IMAGE::IMAGE(LPCSTR str,DWORD TransparentColor){
	memset(this,0,sizeof(IMAGE));
	HANDLE Bitmap=LoadImageA(0,str,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_DEFAULTSIZE|LR_CREATEDIBSECTION);
	SetHandle((HBITMAP)Bitmap,TransparentColor);
	DeleteObject(Bitmap);}

DLL IMAGE::IMAGE(LPCWSTR str,DWORD TransparentColor){
	memset(this,0,sizeof(IMAGE));
	HANDLE Bitmap=LoadImageW(0,str,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_DEFAULTSIZE|LR_CREATEDIBSECTION);
	SetHandle((HBITMAP)Bitmap,TransparentColor);
	DeleteObject(Bitmap);}

DLL IMAGE::IMAGE(UINT id,DWORD TransparentColor){
	memset(this,0,sizeof(IMAGE));
	HANDLE Bitmap=LoadImage(hInstance,MAKEINTRESOURCE(id),IMAGE_BITMAP,0,0,LR_DEFAULTSIZE|LR_CREATEDIBSECTION);
	SetHandle((HBITMAP)Bitmap,TransparentColor);
	DeleteObject(Bitmap);}

DLL KRESULT IMAGE::SetHandle(HBITMAP Bitmap,DWORD TransparentColor){
	if(!Bitmap || Bitmap==INVALID_HANDLE_VALUE){
		Bitmap=0;
		_asm sub esp,(6*4);
		HMDER_AddA("LoadImage\n\
				 hInst:%08X\n\
				 name:%08X\n\
				 type:%08X\n\
				 cx:%d\n\
				 cy:%d\n\
				 fuLoad:%08X");
		_asm add esp,(6*4);
		return KR_ERR;}
	BITMAP BitmapObj;
	GetObject(Bitmap,sizeof(BITMAP),&BitmapObj);
		memset(this,0,sizeof(IMAGE));
	Width=BitmapObj.bmWidth;
	Height=BitmapObj.bmHeight;
	Page=1;
	Image=new DWORD[Size=(Width*Height)];
	if(!Image){
		_asm sub esp,4;
		HMDER_AddA(("new BYTE\n\
					size:%d"));
		_asm add esp,4;
		return KR_ERR;}
	UseTransparent=(TransparentColor!=TCOLOR);
	DWORD HLoop=Height;
	DWORD WLoop=Width;
	_asm{
		push edi;
		push esi;
		mov ebx,this;
		mov edi,[ebx+Image];

		mov esi,BitmapObj.bmHeight;
		dec esi;
		imul esi,BitmapObj.bmWidthBytes;
		add esi,BitmapObj.bmBits;

		mov edx,BitmapObj.bmWidth;
		imul edx,3;
		add edx,BitmapObj.bmWidthBytes;

		mov ecx,BitmapObj.bmHeight;
LoopH:
		push ecx;
		mov ecx,BitmapObj.bmWidth;
LoopW:
		lodsd;
		and eax,0xffffff;
		xchg ah,al;
		ror eax,8;
		xchg ah,al;
		rol eax,8;
		xchg ah,al;
		cmp eax,TransparentColor;
		jne NoTColor;
		mov eax,TCOLOR;
NoTColor:
		stosd;
		dec esi;
		loop LoopW;
		pop ecx;
		sub esi,edx;
		loop LoopH;
		pop esi;
		pop edi;
	}
	return KR_OK;}

DLL void IMAGE::DrawAll(int x,int y){
	RECT rect={0,0,Width,Height*Page};
	Draw(x,y,&rect);}

DLL void IMAGE::DrawS(int x,int y){
	RECT rect={0,0,Width,Height};
	if(UseTransparent) DrawT(x,y,&rect);
	else Draw(x,y,&rect);}

DLL void IMAGE::DrawSC(int x,int y){
	RECT rect={0,0,Width,Height};
	if(UseTransparent) DrawCT(x,y,&rect);
	else{
		OffsetRect(&rect,x,y);
		HMDDD_Rectangle(&rect);}}

DLL void IMAGE::Draw(int x,int y,LPCRECT rect){
	int l,t,w,h;
	if(x<0){
		l=rect->left-x;
		if(l>rect->right) return;
		x=0;}
	else{
		if((UINT)x>=DrawTable.Width) return;
		l=rect->left;}
	if(y<0){
		t=rect->top-y;
		if(t>rect->bottom) return;
		y=0;}
	else{
		if((UINT)y>=DrawTable.Height) return;
		t=rect->top;}
	if((UINT)rect->right+x>DrawTable.Width) w=DrawTable.Width-x;
	else w=rect->right-l;
	if((UINT)rect->bottom+y>DrawTable.Height) h=DrawTable.Height-y;
	else h=rect->bottom-t;
	if(w<=0 || h<=0) return;
	HMDDT_SetDest(x,y);
	_asm{
		push esi;
		push edi;
		mov ebx,this;
		mov edi,DrawTool.Dest;
		mov esi,t;
		imul esi,dword ptr[ebx+Width];
		add esi,l;
		shl esi,2;
		add esi,dword ptr[ebx+Image];

		mov edx,dword ptr[ebx+Width];
		sub edx,w;
		shl edx,2;
		
		mov eax,DrawTable.Pitch;
		sub eax,w;
		shl eax,2;
		mov ecx,h;
HLoop:
		mov ebx,ecx;
		mov ecx,w;
		rep movsd;
		mov ecx,ebx;
		add edi,eax;
		add esi,edx;
		loop HLoop;
		pop edi;
		pop esi;
	};}

DLL void IMAGE::DrawC(int x,int y,LPCRECT rect){
	int l,t,w,h;
	if(x<0){
		l=rect->left-x;
		if(l>rect->right) return;
		x=0;}
	else{
		if((UINT)x>=DrawTable.Width) return;
		l=rect->left;}
	if(y<0){
		t=rect->top-y;
		if(t>rect->bottom) return;
		y=0;}
	else{
		if((UINT)y>=DrawTable.Height) return;
		t=rect->top;}
	if((UINT)rect->right+x>DrawTable.Width) w=DrawTable.Width-x;
	else w=rect->right-l;
	if((UINT)rect->bottom+y>DrawTable.Height) h=DrawTable.Height-y;
	else h=rect->bottom-t;
	if(w<=0 || h<=0) return;
	HMDDT_SetDest(x,y);
	int AddSI=(Width-w)*4;
	int AddDI=(DrawTable.Pitch-w)*4;
	_asm{
		push esi;
		push edi;
		mov ebx,this;
		mov edi,DrawTool.Dest;
		mov esi,t;
		imul esi,dword ptr[ebx+Width];
		add esi,l;
		shl esi,2;
		add esi,dword ptr[ebx+Image];

		mov ecx,h;
HLoop:
		mov ebx,ecx;
		mov ecx,w;
WLoop:
		lodsd;
		and eax,DrawTool.Color;
		stosd;
		loop WLoop;
		mov ecx,ebx;
		add edi,AddDI;
		add esi,AddSI;
		loop HLoop;
		pop edi;
		pop esi;
	};}

DLL void IMAGE::DrawT(int x,int y,LPCRECT rect){
	int l,t,w,h;
	if(x<0){
		l=rect->left-x;
		if(l>rect->right) return;
		x=0;}
	else{
		if((UINT)x>=DrawTable.Width) return;
		l=rect->left;}
	if(y<0){
		t=rect->top-y;
		if(t>rect->bottom) return;
		y=0;}
	else{
		if((UINT)y>=DrawTable.Height) return;
		t=rect->top;}
	if((UINT)rect->right+x>DrawTable.Width) w=DrawTable.Width-x;
	else w=rect->right-l;
	if((UINT)rect->bottom+y>DrawTable.Height) h=DrawTable.Height-y;
	else h=rect->bottom-t;
	if(w<=0 || h<=0) return;
	HMDDT_SetDest(x,y);
	_asm{
		push esi;
		push edi;
		mov ebx,this;
		mov edi,DrawTool.Dest;
		mov esi,t;
		imul esi,dword ptr[ebx+Width];
		add esi,l;
		shl esi,2;
		add esi,dword ptr[ebx+Image];

		mov edx,dword ptr[ebx+Width];
		sub edx,w;
		shl edx,2;
		
		mov ebx,DrawTable.Pitch;
		sub ebx,w;
		shl ebx,2;
		mov ecx,h;
HLoop:
		push ecx;
		mov ecx,w;
WLoop:
		lodsd;
		cmp eax,TCOLOR;
		je SkipCopy;
		stosd;
		loop WLoop;
		jmp WLoopEnd;
SkipCopy:
		add edi,4;
		loop WLoop;
WLoopEnd:
		pop ecx;
		add edi,ebx;
		add esi,edx;
		loop HLoop;
		pop edi;
		pop esi;
	};}

DLL void IMAGE::DrawCT(int x,int y,LPCRECT rect){
	int l,t,w,h;
	if(x<0){
		l=rect->left-x;
		if(l>rect->right) return;
		x=0;}
	else{
		if((UINT)x>=DrawTable.Width) return;
		l=rect->left;}
	if(y<0){
		t=rect->top-y;
		if(t>rect->bottom) return;
		y=0;}
	else{
		if((UINT)y>=DrawTable.Height) return;
		t=rect->top;}
	if((UINT)rect->right+x>DrawTable.Width) w=DrawTable.Width-x;
	else w=rect->right-l;
	if((UINT)rect->bottom+y>DrawTable.Height) h=DrawTable.Height-y;
	else h=rect->bottom-t;
	if(w<=0 || h<=0) return;
	HMDDT_SetDest(x,y);
	_asm{
		push esi;
		push edi;
		mov ebx,this;
		mov edi,DrawTool.Dest;
		mov esi,t;
		imul esi,dword ptr[ebx+Width];
		add esi,l;
		shl esi,2;
		add esi,dword ptr[ebx+Image];

		mov edx,dword ptr[ebx+Width];
		sub edx,w;
		shl edx,2;
		
		mov ebx,DrawTable.Pitch;
		sub ebx,w;
		shl ebx,2;
		mov ecx,h;
HLoop:
		push ecx;
		mov ecx,w;
WLoop:
		lodsd;
		cmp eax,TCOLOR;
		je SkipCopy;
		and eax,DrawTool.Color;
		stosd;
		loop WLoop;
		jmp WLoopEnd;
SkipCopy:
		add edi,4;
		loop WLoop;
WLoopEnd:
		pop ecx;
		add edi,ebx;
		add esi,edx;
		loop HLoop;
		pop edi;
		pop esi;
	};}
