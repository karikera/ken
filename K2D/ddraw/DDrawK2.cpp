
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
