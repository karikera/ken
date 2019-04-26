#include"3DDraw.h"
#include"DDraw.h"
#include<math.h>

LPVOID Texture;
MATRIX3DPOS DrawMatrix;
DWORD DrawVector[256 + 256];

MODEL3D::MODEL3D(LPVECTOR3D Vector, DWORD Count, DWORD* Conn, DWORD ConnCount, LPDWORD texture, DWORD dataflags) {
	DataFlags = dataflags;
	VectorCount = Count;
	if (DataFlags&DF_DONOTNEW) {
		VectorList = Vector;
		Connect = (LPDWORD*)Conn;
		Texture = texture;
	}
	else {
		VectorList = new VECTOR3D[Count];
		memcpy(VectorList, Vector, sizeof(VECTOR3D)*Count);
		Connect = new LPDWORD[ConnCount + 1];
		Texture = new DWORD[ConnCount + 1];
		if (texture) memcpy(Texture, texture, sizeof(DWORD)*ConnCount);
	}
	for (DWORD i = 0; i < ConnCount; i++) Connect[i] = DrawVector + Conn[i];
	Connect[ConnCount] = NULL;
}

MODEL3D::~MODEL3D() {
	if (!(DataFlags&DF_DONOTDELETE)) {
		DEL(Connect);
		DEL(VectorList);
	}
}

LPMODEL3D new_Model3DBall(int w, int h) {
	h -= 2;
	if (h <= 0) return 0;
	int vcount = w*h + 2;
	int ccount = (w*(h - 1) * 2 + w * 2) * 3;
	LPVECTOR3D VecList = new VECTOR3D[vcount];
	LPDWORD CoList = new DWORD[ccount + 1];
	VecList[0] = VECTOR3D(0, 0, 1);
	VecList[vcount - 1] = VECTOR3D(0, 0, -1);
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			LPVECTOR3D vec = VecList + i + w*j + 1;
			float dir = (i + j*0.5f)*PI * 2 / w;
			float dir2 = (j + 1)*PI / (h + 1);
			vec->x = cosf(dir)*sinf(dir2);
			vec->y = sinf(dir)*sinf(dir2);
			vec->z = cosf(dir2);
		}
	}
	for (int i = 0; i < w; i++) {
		CoList[i * 3] = 0;
		CoList[i * 3 + 1] = 1 + i;
		CoList[i * 3 + 2] = 1 + ((1 + i == w) ? 0 : 1 + i);
	}
	for (int j = 0; j < h - 1; j++) {
		for (int i = 0; i < w; i++) {
			CoList[(w + j*w * 2 + i) * 3] = 1 + (j + 1)*w + i;
			CoList[(w + j*w * 2 + i) * 3 + 1] = 1 + (j)*w + ((1 + i == w) ? 0 : 1 + i);
			CoList[(w + j*w * 2 + i) * 3 + 2] = 1 + (j)*w + i;
			CoList[(w * 2 + j*w * 2 + i) * 3] = 1 + (j + 1)*w + ((1 + i == w) ? 0 : 1 + i);
			CoList[(w * 2 + j*w * 2 + i) * 3 + 1] = 1 + (j)*w + ((1 + i == w) ? 0 : 1 + i);
			CoList[(w * 2 + j*w * 2 + i) * 3 + 2] = 1 + (j + 1)*w + i;
		}
	}
	for (int i = 0; i < w; i++) {
		CoList[ccount - (i + 1) * 3] = (h - 1)*w + 1 + ((1 + i == w) ? 0 : 1 + i);
		CoList[ccount - (i + 1) * 3 + 1] = (h - 1)*w + 1 + i;
		CoList[ccount - (i + 1) * 3 + 2] = vcount - 1;
	}

	return new MODEL3D(VecList, vcount, CoList, ccount, 0, DF_DONOTNEW);
}

void MODEL3D::DrawCCW() {
	_asm {
		mov ecx, this;
		mov ebx, [ecx + VectorList];
		mov esi, offset DrawVector;
		mov edi, offset DrawMatrix;
		mov ecx, [ecx + VectorCount];
	CalculVector:
		fld[ebx + VECTOR3D::x];
		fmul[edi + MATRIX3DPOS::_11];
		fld[ebx + VECTOR3D::y];
		fmul[edi + MATRIX3DPOS::_21];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + MATRIX3DPOS::_31];
		faddp st(1), st;
		fadd[edi + MATRIX3DPOS::x];
		fistp word ptr[esi + 0];

		fld[ebx + VECTOR3D::x];
		fmul[edi + MATRIX3DPOS::_12];
		fld[ebx + VECTOR3D::y];
		fmul[edi + MATRIX3DPOS::_22];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + MATRIX3DPOS::_32];
		faddp st(1), st;
		fadd[edi + MATRIX3DPOS::y];
		fistp word ptr[esi + 2];

		add ebx, (4 * 3);
		add esi, 4;
		loop CalculVector;
	}
	_asm {
		mov eax, this;
		mov esi, [eax + Connect];
		sub esp, 4 * 3;
	DrawLoop:
		//외적 체크
		mov ebx, [esi];
		mov ecx, [esi + 4];
		mov edx, [esi + 8];

		mov ax, [ecx + 2];
		sub ax, [edx + 2];
		imul ax, [ebx + 0];

		mov di, [edx + 2];
		sub di, [ebx + 2];
		imul di, [ecx + 0];
		add ax, di;

		mov di, [ebx + 2];
		sub di, [ecx + 2];
		imul di, [edx + 0];
		add ax, di;

		cmp ax, 0;
		jg SkipDraw;

		mov edi, esp;
		mov eax, [ebx];
		stosd;
		mov eax, [ecx];
		stosd;
		mov eax, [edx];
		stosd;

		call DrawTriangle;
	SkipDraw:
		add esi, 4 * 3;
		cmp dword ptr[esi], 0;
		jne DrawLoop;
		add esp, 4 * 3;
	}
}

void MODEL3D::DrawCCW_Z() {
	_asm {
		mov ecx, this;
		mov ebx, [ecx + VectorList];
		mov esi, offset DrawVector;
		mov edi, offset DrawMatrix;
		mov ecx, [ecx + VectorCount];
	CalculVector:
		fld[ebx + VECTOR3D::x];
		fmul[edi + 8 + 0];
		fld[ebx + VECTOR3D::y];
		fmul[edi + 8 + 12];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + 8 + 24];
		faddp st(1), st;
		fadd[edi + 8 + 36];
		fistp dword ptr[esi + 256 * 4];

		fld[ebx + VECTOR3D::x];
		fmul[edi + 0];
		fld[ebx + VECTOR3D::y];
		fmul[edi + 12];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + 24];
		faddp st(1), st;
		fadd[edi + 36];
		fistp word ptr[esi + 0];

		fld[ebx + VECTOR3D::x];
		fmul[edi + 4 + 0];
		fld[ebx + VECTOR3D::y];
		fmul[edi + 4 + 12];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + 4 + 24];
		faddp st(1), st;
		fadd[edi + 4 + 36];
		fistp word ptr[esi + 2];

		cmp dword ptr[esi + 256 * 4], 0;
		jl NoDraw;
		cmp dword ptr[esi + 256 * 4], 255;
		jg NoDraw;

		add ebx, (4 * 3);
		add esi, 4;
		loop CalculVector;
	}

	int Cross;
	_asm {
		mov eax, this;
		mov esi, [eax + Connect];
		xor edi, edi;
		sub esp, 4 * 6;
	DrawLoop:
		//외적 체크
		mov ebx, [esi];
		mov ecx, [esi + 4];
		mov edx, [esi + 8];

		mov ax, [ecx + 2];
		sub ax, [edx + 2];
		mov di, [ebx + 0];
		cwde;
		xchg edi, eax;
		cwde;
		imul eax, edi;
		mov Cross, eax;

		mov ax, [edx + 2];
		sub ax, [ebx + 2];
		mov di, [ecx + 0];
		cwde;
		xchg edi, eax;
		cwde;
		imul eax, edi;
		add Cross, eax;

		mov ax, [ebx + 2];
		sub ax, [ecx + 2];
		mov di, [edx + 0];
		cwde;
		xchg edi, eax;
		cwde;
		imul eax, edi;
		add eax, Cross;
		jg SkipDraw;

		mov edi, esp;
		mov eax, [ebx];
		stosd;
		mov eax, [ecx];
		stosd;
		mov eax, [edx];
		stosd;
		mov eax, [ebx + 256 * 4];
		stosd;
		mov eax, [ecx + 256 * 4];
		stosd;
		mov eax, [edx + 256 * 4];
		stosd;

		call DrawTriangle_Z;
	SkipDraw:
		add esi, 4 * 3;
		cmp dword ptr[esi], 0;
		jne DrawLoop;
		add esp, 4 * 6;


	NoDraw:
	}
}

void MODEL3D::DrawCCW_ZT() {
	int OffsetTexture = int((LPBYTE)Texture - (LPBYTE)Connect);
	_asm {
		mov ecx, this;
		mov ebx, [ecx + VectorList];
		mov esi, offset DrawVector;
		mov edi, offset DrawMatrix;
		mov ecx, [ecx + VectorCount];
	CalculVector:
		fld[ebx + VECTOR3D::x];
		fmul[edi + 8 + 0];
		fld[ebx + VECTOR3D::y];
		fmul[edi + 8 + 12];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + 8 + 24];
		faddp st(1), st;
		fadd[edi + 8 + 36];
		fistp dword ptr[esi + 256 * 4];

		fld[ebx + VECTOR3D::x];
		fmul[edi + 0];
		fld[ebx + VECTOR3D::y];
		fmul[edi + 12];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + 24];
		faddp st(1), st;
		fadd[edi + 36];
		fistp word ptr[esi + 0];

		fld[ebx + VECTOR3D::x];
		fmul[edi + 4 + 0];
		fld[ebx + VECTOR3D::y];
		fmul[edi + 4 + 12];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + 4 + 24];
		faddp st(1), st;
		fadd[edi + 4 + 36];
		fistp word ptr[esi + 2];

		cmp dword ptr[esi + 256 * 4], 0;
		jl NoDraw;
		cmp dword ptr[esi + 256 * 4], 255;
		jg NoDraw;

		add ebx, (4 * 3);
		add esi, 4;
		loop CalculVector;
	}

	int Cross;
	_asm {
		mov eax, this;
		mov esi, [eax + Connect];
		xor edi, edi;
		sub esp, 4 * 6;
	DrawLoop:
		//외적 체크
		mov ebx, [esi];
		mov ecx, [esi + 4];
		mov edx, [esi + 8];

		mov ax, [ecx + 2];
		sub ax, [edx + 2];
		mov di, [ebx + 0];
		cwde;
		xchg edi, eax;
		cwde;
		imul eax, edi;
		mov Cross, eax;

		mov ax, [edx + 2];
		sub ax, [ebx + 2];
		mov di, [ecx + 0];
		cwde;
		xchg edi, eax;
		cwde;
		imul eax, edi;
		add Cross, eax;

		mov ax, [ebx + 2];
		sub ax, [ecx + 2];
		mov di, [edx + 0];
		cwde;
		xchg edi, eax;
		cwde;
		imul eax, edi;
		add eax, Cross;
		jg SkipDraw;

		mov edi, esp;
		mov eax, [ebx];
		stosd;
		mov eax, [ecx];
		stosd;
		mov eax, [edx];
		stosd;
		mov eax, [ebx + 256 * 4];
		stosd;
		mov eax, [ecx + 256 * 4];
		stosd;
		mov eax, [edx + 256 * 4];
		stosd;
		call DrawTriangle_Z;

		sub edi, 4 * 3;
		add esi, OffsetTexture;
		mov ecx, 3;
		rep movsd;
		sub esi, 4 * 3;
		sub esi, OffsetTexture;

		call DrawTriangle_TC;
	SkipDraw:
		add esi, 4 * 3;
		cmp dword ptr[esi], 0;
		jne DrawLoop;
		add esp, 4 * 6;

	NoDraw:
	}
}

void MODEL3D::DrawCW() {
	_asm {
		mov ecx, this;
		mov ebx, [ecx + VectorList];
		mov esi, offset DrawVector;
		mov edi, offset DrawMatrix;
		mov ecx, [ecx + VectorCount];
	CalculVector:
		fld[ebx + VECTOR3D::x];
		fmul[edi + MATRIX3DPOS::_11];
		fld[ebx + VECTOR3D::y];
		fmul[edi + MATRIX3DPOS::_21];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + MATRIX3DPOS::_31];
		faddp st(1), st;
		fadd[edi + MATRIX3DPOS::x];
		fistp word ptr[esi + 0];

		fld[ebx + VECTOR3D::x];
		fmul[edi + MATRIX3DPOS::_12];
		fld[ebx + VECTOR3D::y];
		fmul[edi + MATRIX3DPOS::_22];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + MATRIX3DPOS::_32];
		faddp st(1), st;
		fadd[edi + MATRIX3DPOS::y];
		fistp word ptr[esi + 2];

		add ebx, (4 * 3);
		add esi, 4;
		loop CalculVector;
	}

	_asm {
		mov eax, this;
		mov esi, [eax + Connect];
		sub esp, 4 * 3;
	DrawLoop:
		//외적 체크
		mov ebx, [esi];
		mov ecx, [esi + 4];
		mov edx, [esi + 8];

		mov ax, [ecx + 2];
		sub ax, [edx + 2];
		imul ax, [ebx + 0];

		mov di, [edx + 2];
		sub di, [ebx + 2];
		imul di, [ecx + 0];
		add ax, di;

		mov di, [ebx + 2];
		sub di, [ecx + 2];
		imul di, [edx + 0];
		add ax, di;

		cmp ax, 0;
		jl SkipDraw;

		mov edi, esp;
		mov eax, [ebx];
		stosd;
		mov eax, [ecx];
		stosd;
		mov eax, [edx];
		stosd;

		call DrawTriangle;
	SkipDraw:
		add esi, 4 * 3;
		cmp dword ptr[esi], 0;
		jne DrawLoop;
		add esp, 4 * 3;
	}
}

void MODEL3D::DrawCW_Z() {
	_asm {
		mov ecx, this;
		mov ebx, [ecx + VectorList];
		mov esi, offset DrawVector;
		mov edi, offset DrawMatrix;
		mov ecx, [ecx + VectorCount];
	CalculVector:
		fld[ebx + VECTOR3D::x];
		fmul[edi + 8 + 0];
		fld[ebx + VECTOR3D::y];
		fmul[edi + 8 + 12];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + 8 + 24];
		faddp st(1), st;
		fadd[edi + 8 + 36];
		fistp dword ptr[esi + 256 * 4];

		fld[ebx + VECTOR3D::x];
		fmul[edi + 0];
		fld[ebx + VECTOR3D::y];
		fmul[edi + 12];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + 24];
		faddp st(1), st;
		fadd[edi + 36];
		fistp word ptr[esi + 0];

		fld[ebx + VECTOR3D::x];
		fmul[edi + 4 + 0];
		fld[ebx + VECTOR3D::y];
		fmul[edi + 4 + 12];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + 4 + 24];
		faddp st(1), st;
		fadd[edi + 4 + 36];
		fistp word ptr[esi + 2];

		cmp dword ptr[esi + 256 * 4], 0;
		jl NoDraw;
		cmp dword ptr[esi + 256 * 4], 255;
		jg NoDraw;

		add ebx, (4 * 3);
		add esi, 4;
		loop CalculVector;
	}

	int Cross;
	_asm {
		mov eax, this;
		mov esi, [eax + Connect];
		xor edi, edi;
		sub esp, 4 * 6;
	DrawLoop:
		//외적 체크
		mov ebx, [esi];
		mov ecx, [esi + 4];
		mov edx, [esi + 8];

		mov ax, [ecx + 2];
		sub ax, [edx + 2];
		mov di, [ebx + 0];
		cwde;
		xchg edi, eax;
		cwde;
		imul eax, edi;
		mov Cross, eax;

		mov ax, [edx + 2];
		sub ax, [ebx + 2];
		mov di, [ecx + 0];
		cwde;
		xchg edi, eax;
		cwde;
		imul eax, edi;
		add Cross, eax;

		mov ax, [ebx + 2];
		sub ax, [ecx + 2];
		mov di, [edx + 0];
		cwde;
		xchg edi, eax;
		cwde;
		imul eax, edi;
		add eax, Cross;
		jl SkipDraw;

		mov edi, esp;
		mov eax, [ebx];
		stosd;
		mov eax, [ecx];
		stosd;
		mov eax, [edx];
		stosd;
		mov eax, [ebx + 256 * 4];
		stosd;
		mov eax, [ecx + 256 * 4];
		stosd;
		mov eax, [edx + 256 * 4];
		stosd;

		call DrawTriangle_Z;
	SkipDraw:
		add esi, 4 * 3;
		cmp dword ptr[esi], 0;
		jne DrawLoop;
		add esp, 4 * 6;

	NoDraw:
	}
}

void MODEL3D::DrawCW_ZT() {
	int OffsetTexture = int((LPBYTE)Texture - (LPBYTE)Connect);
	_asm {
		mov ecx, this;
		mov ebx, [ecx + VectorList];
		mov esi, offset DrawVector;
		mov edi, offset DrawMatrix;
		mov ecx, [ecx + VectorCount];
	CalculVector:
		fld[ebx + VECTOR3D::x];
		fmul[edi + 8 + 0];
		fld[ebx + VECTOR3D::y];
		fmul[edi + 8 + 12];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + 8 + 24];
		faddp st(1), st;
		fadd[edi + 8 + 36];
		fistp dword ptr[esi + 256 * 4];

		fld[ebx + VECTOR3D::x];
		fmul[edi + 0];
		fld[ebx + VECTOR3D::y];
		fmul[edi + 12];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + 24];
		faddp st(1), st;
		fadd[edi + 36];
		fistp word ptr[esi + 0];

		fld[ebx + VECTOR3D::x];
		fmul[edi + 4 + 0];
		fld[ebx + VECTOR3D::y];
		fmul[edi + 4 + 12];
		faddp st(1), st;
		fld[ebx + VECTOR3D::z];
		fmul[edi + 4 + 24];
		faddp st(1), st;
		fadd[edi + 4 + 36];
		fistp word ptr[esi + 2];

		cmp dword ptr[esi + 256 * 4], 0;
		jl NoDraw;
		cmp dword ptr[esi + 256 * 4], 255;
		jg NoDraw;

		add ebx, (4 * 3);
		add esi, 4;
		loop CalculVector;
	}

	int Cross;
	_asm {
		mov eax, this;
		mov esi, [eax + Connect];
		xor edi, edi;
		sub esp, 4 * 6;
	DrawLoop:
		//외적 체크
		mov ebx, [esi];
		mov ecx, [esi + 4];
		mov edx, [esi + 8];

		mov ax, [ecx + 2];
		sub ax, [edx + 2];
		mov di, [ebx + 0];
		cwde;
		xchg edi, eax;
		cwde;
		imul eax, edi;
		mov Cross, eax;

		mov ax, [edx + 2];
		sub ax, [ebx + 2];
		mov di, [ecx + 0];
		cwde;
		xchg edi, eax;
		cwde;
		imul eax, edi;
		add Cross, eax;

		mov ax, [ebx + 2];
		sub ax, [ecx + 2];
		mov di, [edx + 0];
		cwde;
		xchg edi, eax;
		cwde;
		imul eax, edi;
		add eax, Cross;
		jl SkipDraw;

		mov edi, esp;
		mov eax, [ebx];
		stosd;
		mov eax, [ecx];
		stosd;
		mov eax, [edx];
		stosd;
		mov eax, [ebx + 256 * 4];
		stosd;
		mov eax, [ecx + 256 * 4];
		stosd;
		mov eax, [edx + 256 * 4];
		stosd;
		call DrawTriangle_Z;

		sub edi, 4 * 3;
		add esi, OffsetTexture;
		mov ecx, 3;
		rep movsd;
		sub esi, 4 * 3;
		sub esi, OffsetTexture;

		call DrawTriangle_TC;
	SkipDraw:
		add esi, 4 * 3;
		cmp dword ptr[esi], 0;
		jne DrawLoop;
		add esp, 4 * 6;

	NoDraw:
	}
}

void DrawTriangle8_Z(DWORD pt1, DWORD pt2, DWORD pt3, int Z1, int Z2, int Z3) {
	int ZVAdd1, ZHAdd = 0;
	int Width = SFWidth;
	int ZOffset = ZBufferOffset;
	LPVOID ZBufferTarget;
	DWORD Left1, Right1;

	DWORD Left2, Right2;
	int ZVAdd2;
	int Left1Add, Left2Add, Right1Add, Right2Add;
	int x1, y1, x2, y2, x3, y3;
	int MLine, Draw12, Draw23;
	int tx12;
	_asm {
		mov edx, Z1;
		mov esi, Z2;
		mov edi, Z3;

		cmp Z1, 0;
		jl FunctionOut;
		cmp Z2, 0;
		jl FunctionOut;
		cmp Z2, 0;
		jl FunctionOut;

		cmp Z1, 0xff;
		jg FunctionOut;
		cmp Z2, 0xff;
		jg FunctionOut;
		cmp Z2, 0xff;
		jg FunctionOut;

		mov eax, pt1;
		mov ebx, pt2;
		mov ecx, pt3;

		or ax, ax;
		jge PassX_L;
		or bx, bx;
		jge PassX_L;
		or cx, cx;
		jl FunctionOut;
	PassX_L:
		cmp ax, word ptr[SFWidth];
		jl PassX_R;
		cmp bx, word ptr[SFWidth];
		jl PassX_R;
		cmp cx, word ptr[SFWidth];
		jge FunctionOut;
	PassX_R:

		ror eax, 16;
		ror ebx, 16;
		ror ecx, 16;

		or ax, ax;
		jge PassY_T;
		or bx, bx;
		jge PassY_T;
		or cx, cx;
		jl FunctionOut;
	PassY_T:
		cmp ax, word ptr[SFHeight];
		jl PassY_B;
		cmp bx, word ptr[SFHeight];
		jl PassY_B;
		cmp cx, word ptr[SFHeight];
		jge FunctionOut;
	PassY_B:

		cmp cx, bx;
		jnl SkipChange_32;
		xchg ecx, ebx;
		xchg edi, esi;
	SkipChange_32:

		cmp cx, ax;
		jnl SkipChange_31;
		xchg eax, ebx;
		xchg ebx, ecx;
		xchg edx, esi;
		xchg esi, edi;
		jmp SkipChange_21;
	SkipChange_31:

		cmp bx, ax;
		jnl SkipChange_21;
		xchg ebx, eax;
		xchg esi, edx;

	SkipChange_21:

		mov Z1, edx;
		mov Z2, esi;
		mov Z3, edi;

		mov edx, eax;
		cwde;
		mov y1, eax;
		mov ax, bx;
		cwde;
		mov y2, eax;
		mov ax, cx;
		cwde;
		mov y3, eax;

		shr edx, 16;
		shr ebx, 16;
		shr ecx, 16;

		mov ax, dx;
		cwde;
		mov x1, eax;
		mov ax, bx;
		cwde;
		mov x2, eax;
		mov ax, cx;
		cwde;
		mov x3, eax;
	}
	tx12 = x2 - x1;

	if (Draw12 = (y2 - y1)) Right1Add = (tx12 << 16) / Draw12;
	Right1 = (x1 << 16);
	if (Draw23 = (y3 - y2)) Right2Add = ((x3 - x2) << 16) / Draw23;
	Right2 = (x2 << 16);

	_asm {// 1-3 시작점, 증가량 계산

		mov ebx, x1; // x1 HALF 형식으로
		shl ebx, 16;

		mov ecx, y3;
		sub ecx, y1;
		je FunctionOut;
		mov eax, x3;
		sub eax, x1;
		shl eax, 16;
		cdq;
		idiv ecx;
		cmp Draw12, 0;
		jne CheckRightAdd;
		cmp tx12, 0;
		jge SkipChange;
		jmp NoCheckRightAdd;
	CheckRightAdd:
		cmp Right1Add, eax;
		jge SkipChange;
	NoCheckRightAdd:
		mov ecx, eax;
		xchg Right2Add, eax;
		mov Left2Add, eax;
		mov eax, ecx;
		xchg Right1Add, eax;
		mov Left1Add, eax;
		imul ecx, Draw12;
		add ecx, ebx;
		xchg Right2, ecx;
		mov Left2, ecx;
		xchg Right1, ebx;
		mov Left1, ebx;
	}
	if (Draw23) ZVAdd2 = ((((Z3 - Z2) << 16) / (int)Draw23) << 8);
	if (MLine = (int)((Right2 + 0xffff - Left2) >> 16))
		ZHAdd = (((((Z3 - Z1)*(int)Draw12 / (y3 - y1) + Z1 - Z2) << 16) / MLine) << 8);
	if (Draw12) ZVAdd1 = ((((Z2 - Z1) << 16) / (int)Draw12) << 8);
	else Z1 = Z2;
	_asm {
		jmp WasChange;
	SkipChange:
		mov Left1Add, eax;
		mov Left2Add, eax;
		mov Left1, ebx;
		imul eax, Draw12;
		add eax, ebx;
		mov Left2, eax;
	}
	ZVAdd2 = ZVAdd1 = ((((Z3 - Z1) << 16) / (y3 - y1)) << 8);
	if (MLine = (int)((Right2 + 0xffff - Left2) >> 16))
		ZHAdd = (((((int)(Z2 - (Z3 - Z1)*(int)Draw12 / (y3 - y1) - Z1)) << 16) / MLine) << 8);
	_asm {
	WasChange:
		add Right1, 0xffff;
		add Right2, 0xffff;
	}
	if ((x1 >= 0 && y1 >= 0 && x2 >= 0 && y2 >= 0 && x3 >= 0 && y3 >= 0) &&
		(x1 < SFWidth && y1 < SFHeight && x2 < SFWidth && y2 < SFHeight && x3 < SFWidth && y3 < SFHeight)) {
		ZBufferTarget = (LPBYTE)ZBuffer + y1*Pitch;
		Right1 -= Left1;
		_asm {
			//빨리 그리기
			mov eax, Z1;
			or eax, 0x80000000;
			mov ah, byte ptr[DrawColor];

			mov ebx, Right1Add;
			sub ebx, Left1Add;
			mov edx, Left1Add;
			mov ecx, Draw12;
			jcxz SkipDraw1;
			call DrawTri;
		SkipDraw1:

			mov ebx, Right2;
			sub ebx, Left2;
			mov Right1, ebx;
			mov ecx, Left2;
			mov Left1, ecx;
			mov edx, ZVAdd2;
			mov ZVAdd1, edx;

			mov ebx, Right2Add;
			sub ebx, Left2Add;
			mov edx, Left2Add;
			mov ecx, Draw23;
			jcxz SkipDraw2;
			call DrawTri;
		SkipDraw2:

			jmp FunctionOut;
		DrawTri:
		DrawLoop:
			push ecx;

			mov ecx, Right1;
			shr ecx, 16;
			mov edi, Left1;
			shr edi, 16;

			add edi, ZBufferTarget;

			jcxz SkipDraw;
			push eax;
		FillDraw:
			scasb;
			jae SkipFill;
			mov[edi - 1], al;
			add edi, ZBufferOffset;
			mov[edi - 1], ah;
			sub edi, ZBufferOffset;
		SkipFill:
			ror eax, 8;
			add eax, ZHAdd;
			rol eax, 8;
			loop FillDraw;
			pop eax;
		SkipDraw:
			mov ecx, Pitch;
			add ZBufferTarget, ecx;
			add Right1, ebx;
			add Left1, edx;
			ror eax, 8;
			add eax, ZVAdd1;
			rol eax, 8;

			pop ecx;
			loop DrawLoop;
			ret;
		}
	}
	else {
		if (y1 + Draw12 > SFHeight) {
			Draw12 = SFHeight - y1;
			Draw23 = 0;
		}
		else if (y2 + Draw23 > SFHeight - 1) {
			Draw23 = SFHeight - y2;
		}
		if (y2 < 0) {
			Left2 -= Left2Add*y2;
			Right2 -= Right2Add*y2;
			Draw23 += y2;
			_asm {
				mov eax, Z1;
				ror eax, 8;
				mov ebx, Draw12;
				imul ebx, ZVAdd1;
				add eax, ebx;
				mov ebx, y2;
				imul ebx, ZVAdd2;
				sub eax, ebx;
				rol eax, 8;
				mov Z1, eax;
			}
			Draw12 = 0;
			y1 = 0;
		}
		else if (y1 < 0) {
			Left1 -= Left1Add*y1;
			Right1 -= Right1Add*y1;
			Draw12 += y1;
			_asm {
				mov eax, Z1;
				ror eax, 8;
				mov ebx, y1;
				imul ebx, ZVAdd1;
				sub eax, ebx;
				rol eax, 8;
				mov Z1, eax;
			}
			y1 = 0;
		}

		ZBufferTarget = (LPBYTE)ZBuffer + y1*Pitch;
		_asm { //클립해서 그리기
			mov eax, Z1;
			or eax, 0x80000000;
			mov ah, byte ptr[DrawColor];

			mov ebx, Right1Add;
			mov edx, Left1Add;
			mov ecx, Draw12;
			jcxz SkipDraw1_C;
			call DrawTri_C;
		SkipDraw1_C:

			mov ebx, Right2;
			mov Right1, ebx;
			mov ecx, Left2;
			mov Left1, ecx;
			mov edx, ZVAdd2;
			mov ZVAdd1, edx;

			mov ebx, Right2Add;
			mov edx, Left2Add;
			mov ecx, Draw23;
			jcxz SkipDraw2_C;
			call DrawTri_C;
		SkipDraw2_C:

			jmp FunctionOut;
		DrawTri_C:
		DrawLoop_C:
			push ecx;

			push eax;
			mov ecx, Right1;
			shr ecx, 16;
			mov edi, Left1;
			sar edi, 16;
			jge SkipClipping_L;
			imul edi, ZHAdd;
			ror eax, 8;
			sub eax, edi;
			rol eax, 8;
			xor edi, edi;
			or cx, cx;
			jle SkipDraw_C;
		SkipClipping_L:
			cmp ecx, Width;
			jb SkipClipping_R;
			mov ecx, Width;
			cmp ecx, edi;
			jbe SkipDraw_C;
		SkipClipping_R:
			sub ecx, edi;

			or cx, cx;
			jle SkipDraw_C;
			add edi, ZBufferTarget;

		FillDraw_C:
			scasb;

			jae SkipFill_C;
			mov[edi - 1], al;
			add edi, ZOffset;
			mov[edi - 1], ah;
			sub edi, ZOffset;
		SkipFill_C:
			ror eax, 8;
			add eax, ZHAdd;
			rol eax, 8;

			loop FillDraw_C;
		SkipDraw_C:
			pop eax;
			mov ecx, Pitch;
			add ZBufferTarget, ecx;
			add Right1, ebx;
			add Left1, edx;

			ror eax, 8;
			add eax, ZVAdd1;
			rol eax, 8;

			pop ecx;
			loop DrawLoop_C;
			ret;
		}
	}
FunctionOut:;
}

void DrawTriangle8_T(DWORD pt1, DWORD pt2, DWORD pt3, DWORD Tex1, DWORD Tex2, DWORD Tex3) {
	LPVOID TexStart = Texture;
	int EaxSave;
	LPVOID DrawTarget;
	int TVAdd1X, THAdd1X, TVAdd1Y, THAdd1Y;
	int Width = SFWidth;
	DWORD Left1, Right1;

	DWORD Left2, Right2;
	int TVAdd2X, TVAdd2Y, THAdd2X, THAdd2Y;
	int Left1Add, Left2Add, Right1Add, Right2Add;
	int ToZ2;
	int T1X, T2X, T3X;
	int T1Y, T2Y, T3Y;
	int x1, y1, x2, y2, x3, y3;
	int MLine, Draw12, Draw23, Draw13, tx12;
	_asm {
		mov edx, Tex1;
		mov esi, Tex2;
		mov edi, Tex3;

		mov eax, pt1;
		mov ebx, pt2;
		mov ecx, pt3;

		or ax, ax;
		jge PassX_L;
		or bx, bx;
		jge PassX_L;
		or cx, cx;
		jl FunctionOut;
	PassX_L:
		cmp ax, word ptr[SFWidth];
		jl PassX_R;
		cmp bx, word ptr[SFWidth];
		jl PassX_R;
		cmp cx, word ptr[SFWidth];
		jge FunctionOut;
	PassX_R:

		ror eax, 16;
		ror ebx, 16;
		ror ecx, 16;

		or ax, ax;
		jge PassY_T;
		or bx, bx;
		jge PassY_T;
		or cx, cx;
		jl FunctionOut;
	PassY_T:
		cmp ax, word ptr[SFHeight];
		jl PassY_B;
		cmp bx, word ptr[SFHeight];
		jl PassY_B;
		cmp cx, word ptr[SFHeight];
		jge FunctionOut;
	PassY_B:

		cmp cx, bx;
		jnl SkipChange_32;
		xchg ecx, ebx;
		xchg edi, esi;
	SkipChange_32:

		cmp cx, ax;
		jnl SkipChange_31;
		xchg eax, ebx;
		xchg ebx, ecx;
		xchg edx, esi;
		xchg esi, edi;
		jmp SkipChange_21;
	SkipChange_31:

		cmp bx, ax;
		jnl SkipChange_21;
		xchg ebx, eax;
		xchg esi, edx;

	SkipChange_21:

		mov Tex1, edx;
		mov Tex2, esi;
		mov Tex3, edi;

		mov edx, eax;
		cwde;
		mov y1, eax;
		mov ax, bx;
		cwde;
		mov y2, eax;
		mov ax, cx;
		cwde;
		mov y3, eax;

		shr edx, 16;
		shr ebx, 16;
		shr ecx, 16;

		mov ax, dx;
		cwde;
		mov x1, eax;
		mov ax, bx;
		cwde;
		mov x2, eax;
		mov ax, cx;
		cwde;
		mov x3, eax;
	}

	T1X = (((Tex1 & 0xffff) << 8));
	T2X = (((Tex2 & 0xffff) << 8));
	T3X = (((Tex3 & 0xffff) << 8));
	T1Y = (((Tex1 >> 16) << 8));
	T2Y = (((Tex2 >> 16) << 8));
	T3Y = (((Tex3 >> 16) << 8));
	tx12 = x2 - x1;
	if (Draw12 = (y2 - y1)) Right1Add = (tx12 << 16) / Draw12;
	Right1 = (x1 << 16);
	if (Draw23 = (y3 - y2)) Right2Add = ((x3 - x2) << 16) / Draw23;
	Right2 = (x2 << 16);
	Draw13 = y3 - y1;

	_asm {// 1-3 시작점, 증가량 계산

		mov ebx, x1; // x1 HALF 형식으로
		shl ebx, 16;

		mov ecx, y3;
		sub ecx, y1;
		je FunctionOut;
		mov eax, x3;
		sub eax, x1;
		shl eax, 16;
		cdq;
		idiv ecx;
		cmp Draw12, 0;
		jne CheckRightAdd;
		cmp tx12, 0;
		jge SkipChange;
		jmp NoCheckRightAdd;
	CheckRightAdd:
		cmp Right1Add, eax;
		jge SkipChange;
	NoCheckRightAdd:
		mov ecx, eax;
		xchg Right2Add, eax;
		mov Left2Add, eax;
		mov eax, ecx;
		xchg Right1Add, eax;
		mov Left1Add, eax;
		imul ecx, Draw12;
		add ecx, ebx;
		xchg Right2, ecx;
		mov Left2, ecx;
		xchg Right1, ebx;
		mov Left1, ebx;
	}
	if (Draw12) {
		TVAdd1X = (int)(T2X - T1X) / Draw12;
		TVAdd1Y = (int)(T2Y - T1Y) / Draw12;
	}
	if (Draw23) {
		TVAdd2X = (int)(T3X - T2X) / Draw23;
		TVAdd2Y = (int)(T3Y - T2Y) / Draw23;
	}
	if (MLine = ((Right2 + 0xffff - Left2) >> 16)) {
		int TNX;
		if (T3X - T1X < 0) TNX = -int(UINT((T1X - T3X)*Draw12) / UINT(Draw13));
		else TNX = UINT((T3X - T1X)*Draw12) / UINT(Draw13);
		int TNY;
		if (T3Y - T1Y < 0) TNY = -int(UINT((T1Y - T3Y)*Draw12) / UINT(Draw13));
		else TNY = UINT((T3Y - T1Y)*Draw12) / UINT(Draw13);
		THAdd1X = (int)(TNX - TVAdd1X*Draw12) / MLine;
		THAdd1Y = (int)(TNY - TVAdd1Y*Draw12) / MLine;
		THAdd2X = (int)(TNX + T1X - T2X) / MLine;
		THAdd2Y = (int)(TNY + T1Y - T2Y) / MLine;
	}
	ToZ2 = 1;
	_asm {
		jmp WasChange;
	SkipChange:
		mov Left1Add, eax;
		mov Left2Add, eax;
		mov Left1, ebx;
		imul eax, Draw12;
		add eax, ebx;
		mov Left2, eax;
	}
	TVAdd2X = TVAdd1X = (int)(T3X - T1X) / Draw13;
	TVAdd2Y = TVAdd1Y = (int)(T3Y - T1Y) / Draw13;
	if (MLine = ((Right2 + 0xffff - Left2) >> 16)) {
		THAdd2X = THAdd1X = (int)(T2X - T1X - TVAdd1X*Draw12) / MLine;
		THAdd2Y = THAdd1Y = (int)(T2Y - T1Y - TVAdd1Y*Draw12) / MLine;
	}
	ToZ2 = 0;
	_asm {
	WasChange:
		add Right1, 0xffff;
		add Right2, 0xffff;
	}
	if ((x1 >= 0 && y1 >= 0 && x2 >= 0 && y2 >= 0 && x3 >= 0 && y3 >= 0) &&
		(x1 < SFWidth && y1 < SFHeight && x2 < SFWidth && y2 < SFHeight && x3 < SFWidth && y3 < SFHeight)) {
		DrawTarget = (LPBYTE)Surface + y1*Pitch;
		Right1 -= Left1;
		_asm {
			//빨리 그리기
			mov eax, T1Y;
			mov EaxSave, eax;
			mov eax, T1X;

			mov ebx, Right1Add;
			sub ebx, Left1Add;
			mov edx, Left1Add;
			mov ecx, Draw12;
			jcxz SkipDraw1;
			call DrawTri;
		SkipDraw1:
			cmp ToZ2, 0;
			je NoToZ2;
			mov eax, T2Y;
			mov EaxSave, eax;
			mov eax, T2X;
		NoToZ2:
			mov ebx, Right2;
			sub ebx, Left2;
			mov Right1, ebx;
			mov ecx, Left2;
			mov Left1, ecx;

			mov edx, TVAdd2X;
			mov TVAdd1X, edx;
			mov edx, TVAdd2Y;
			mov TVAdd1Y, edx;

			mov edx, THAdd2X;
			mov THAdd1X, edx;
			mov edx, THAdd2Y;
			mov THAdd1Y, edx;

			mov ebx, Right2Add;
			sub ebx, Left2Add;
			mov edx, Left2Add;
			mov ecx, Draw23;
			jcxz SkipDraw2;
			call DrawTri;
		SkipDraw2:

			jmp FunctionOut;
		DrawTri:
		DrawLoop:
			push ecx;

			mov ecx, Right1;
			shr ecx, 16;
			mov edi, Left1;
			shr edi, 16;

			add edi, DrawTarget;

			jcxz SkipDraw;
			push eax;
			push EaxSave;
		FillDraw:
			xor esi, esi;
			mov si, word ptr[EaxSave + 2];
			shl esi, 8;
			add eax, [THAdd1X];
			xchg eax, EaxSave;
			or si, word ptr[EaxSave + 2];
			add eax, [THAdd1Y];
			xchg eax, EaxSave;
			add esi, TexStart;
			movsb;
			loop FillDraw;
			pop EaxSave;
			pop eax;
		SkipDraw:
			mov ecx, Pitch;
			add DrawTarget, ecx;
			add Right1, ebx;
			add Left1, edx;

			add eax, [TVAdd1X];
			xchg eax, EaxSave;
			add eax, [TVAdd1Y];
			xchg eax, EaxSave;

			pop ecx;
			loop DrawLoop;
			ret;
		}
	}
	else {
		if (y1 + Draw12 > SFHeight) {
			Draw12 = SFHeight - y1;
			Draw23 = 0;
		}
		else if (y2 + Draw23 > SFHeight - 1) {
			Draw23 = SFHeight - y2;
		}
		if (y2 < 0) {
			Left2 -= Left2Add*y2;
			Right2 -= Right2Add*y2;
			Draw23 += y2;
			if (ToZ2) {
				T2X -= y2*TVAdd2X;
				T2Y -= y2*TVAdd2Y;
			}
			else {
				T1X += TVAdd1X*Draw12 - y2*TVAdd2X;
				T1Y += TVAdd1Y*Draw12 - y2*TVAdd2Y;
			}
			Draw12 = 0;
			y1 = 0;
		}
		else if (y1 < 0) {
			Left1 -= Left1Add*y1;
			Right1 -= Right1Add*y1;
			Draw12 += y1;
			T1X -= y1*TVAdd1X;
			T1Y -= y1*TVAdd1Y;
			y1 = 0;
		}

		DrawTarget = (LPBYTE)Surface + y1*Pitch;
		_asm { //클립해서 그리기
			mov eax, T1Y;
			mov EaxSave, eax;
			mov eax, T1X;

			mov ebx, Right1Add;
			mov edx, Left1Add;
			mov ecx, Draw12;
			jcxz SkipDraw1_C;
			call DrawTri_C;
		SkipDraw1_C:
			cmp ToZ2, 0;
			je NoToZ2_C;
			mov eax, T2Y;
			mov EaxSave, eax;
			mov eax, T2X;
		NoToZ2_C:

			mov ebx, Right2;
			mov Right1, ebx;
			mov ecx, Left2;
			mov Left1, ecx;

			mov edx, TVAdd2X;
			mov TVAdd1X, edx;
			mov edx, TVAdd2Y;
			mov TVAdd1Y, edx;

			mov edx, THAdd2X;
			mov THAdd1X, edx;
			mov edx, THAdd2Y;
			mov THAdd1Y, edx;

			mov ebx, Right2Add;
			mov edx, Left2Add;
			mov ecx, Draw23;
			jcxz SkipDraw2_C;
			call DrawTri_C;
		SkipDraw2_C:

			jmp FunctionOut;
		DrawTri_C:
		DrawLoop_C:
			push ecx;
			push eax;
			push EaxSave;
			mov ecx, Right1;
			sar ecx, 16;
			jl SkipDraw_C;
			mov edi, Left1;
			sar edi, 16;
			jge SkipClipping_L;
			mov esi, edi;
			imul edi, THAdd1X;
			imul esi, THAdd1Y;
			sub eax, edi;
			sub EaxSave, esi;
			xor edi, edi;
		SkipClipping_L:
			cmp ecx, Width;
			jb SkipClipping_R;
			mov ecx, Width;
			cmp ecx, edi;
			jbe SkipDraw_C;
		SkipClipping_R:
			sub ecx, edi;
			or cx, cx;
			jle SkipDraw_C;

			add edi, DrawTarget;

			jcxz SkipDraw_C;
		FillDraw_C:
			xor esi, esi;
			mov si, word ptr[EaxSave + 2];
			shl esi, 8;
			add eax, [THAdd1X];
			xchg eax, EaxSave;
			or si, word ptr[EaxSave + 2];
			add eax, [THAdd1Y];
			xchg eax, EaxSave;
			add esi, TexStart;
			movsb;
			loop FillDraw_C;
		SkipDraw_C:
			mov ecx, Pitch;
			add DrawTarget, ecx;
			add Right1, ebx;
			add Left1, edx;

			pop EaxSave;
			pop eax;

			add eax, [TVAdd1X];
			xchg eax, EaxSave;
			add eax, [TVAdd1Y];
			xchg eax, EaxSave;

			pop ecx;
			sub ecx, 1;
			jne DrawLoop_C;
			ret;
		}
	}
FunctionOut:;
}

void DrawTriangle8_TC(DWORD pt1, DWORD pt2, DWORD pt3, DWORD Tex1, DWORD Tex2, DWORD Tex3) {
	LPVOID TexStart = Texture;
	int EaxSave;
	LPVOID DrawTarget;
	int TVAdd1X, THAdd1X, TVAdd1Y, THAdd1Y;
	int Width = SFWidth;
	DWORD Left1, Right1;

	DWORD Left2, Right2;
	int TVAdd2X, TVAdd2Y, THAdd2X, THAdd2Y;
	int Left1Add, Left2Add, Right1Add, Right2Add;
	int ToZ2;
	int T1X, T2X, T3X;
	int T1Y, T2Y, T3Y;
	int x1, y1, x2, y2, x3, y3;
	int MLine, Draw12, Draw23, Draw13, tx12;
	_asm {
		mov edx, Tex1;
		mov esi, Tex2;
		mov edi, Tex3;

		mov eax, pt1;
		mov ebx, pt2;
		mov ecx, pt3;

		or ax, ax;
		jge PassX_L;
		or bx, bx;
		jge PassX_L;
		or cx, cx;
		jl FunctionOut;
	PassX_L:
		cmp ax, word ptr[SFWidth];
		jl PassX_R;
		cmp bx, word ptr[SFWidth];
		jl PassX_R;
		cmp cx, word ptr[SFWidth];
		jge FunctionOut;
	PassX_R:

		ror eax, 16;
		ror ebx, 16;
		ror ecx, 16;

		or ax, ax;
		jge PassY_T;
		or bx, bx;
		jge PassY_T;
		or cx, cx;
		jl FunctionOut;
	PassY_T:
		cmp ax, word ptr[SFHeight];
		jl PassY_B;
		cmp bx, word ptr[SFHeight];
		jl PassY_B;
		cmp cx, word ptr[SFHeight];
		jge FunctionOut;
	PassY_B:

		cmp cx, bx;
		jnl SkipChange_32;
		xchg ecx, ebx;
		xchg edi, esi;
	SkipChange_32:

		cmp cx, ax;
		jnl SkipChange_31;
		xchg eax, ebx;
		xchg ebx, ecx;
		xchg edx, esi;
		xchg esi, edi;
		jmp SkipChange_21;
	SkipChange_31:

		cmp bx, ax;
		jnl SkipChange_21;
		xchg ebx, eax;
		xchg esi, edx;

	SkipChange_21:

		mov Tex1, edx;
		mov Tex2, esi;
		mov Tex3, edi;

		mov edx, eax;
		cwde;
		mov y1, eax;
		mov ax, bx;
		cwde;
		mov y2, eax;
		mov ax, cx;
		cwde;
		mov y3, eax;

		shr edx, 16;
		shr ebx, 16;
		shr ecx, 16;

		mov ax, dx;
		cwde;
		mov x1, eax;
		mov ax, bx;
		cwde;
		mov x2, eax;
		mov ax, cx;
		cwde;
		mov x3, eax;
	}

	T1X = (((Tex1 & 0xffff) << 8));
	T2X = (((Tex2 & 0xffff) << 8));
	T3X = (((Tex3 & 0xffff) << 8));
	T1Y = (((Tex1 >> 16) << 8));
	T2Y = (((Tex2 >> 16) << 8));
	T3Y = (((Tex3 >> 16) << 8));
	tx12 = x2 - x1;
	if (Draw12 = (y2 - y1)) Right1Add = (tx12 << 16) / Draw12;
	Right1 = (x1 << 16);
	if (Draw23 = (y3 - y2)) Right2Add = ((x3 - x2) << 16) / Draw23;
	Right2 = (x2 << 16);
	Draw13 = y3 - y1;

	_asm {// 1-3 시작점, 증가량 계산

		mov ebx, x1; // x1 HALF 형식으로
		shl ebx, 16;

		mov ecx, y3;
		sub ecx, y1;
		je FunctionOut;
		mov eax, x3;
		sub eax, x1;
		shl eax, 16;
		cdq;
		idiv ecx;
		cmp Draw12, 0;
		jne CheckRightAdd;
		cmp tx12, 0;
		jge SkipChange;
		jmp NoCheckRightAdd;
	CheckRightAdd:
		cmp Right1Add, eax;
		jge SkipChange;
	NoCheckRightAdd:
		mov ecx, eax;
		xchg Right2Add, eax;
		mov Left2Add, eax;
		mov eax, ecx;
		xchg Right1Add, eax;
		mov Left1Add, eax;
		imul ecx, Draw12;
		add ecx, ebx;
		xchg Right2, ecx;
		mov Left2, ecx;
		xchg Right1, ebx;
		mov Left1, ebx;
	}
	if (Draw12) {
		TVAdd1X = (int)(T2X - T1X) / Draw12;
		TVAdd1Y = (int)(T2Y - T1Y) / Draw12;
	}
	if (Draw23) {
		TVAdd2X = (int)(T3X - T2X) / Draw23;
		TVAdd2Y = (int)(T3Y - T2Y) / Draw23;
	}
	if (MLine = ((Right2 + 0xffff - Left2) >> 16)) {
		int TNX;
		if (T3X - T1X < 0) TNX = -int(UINT((T1X - T3X)*Draw12) / UINT(Draw13));
		else TNX = UINT((T3X - T1X)*Draw12) / UINT(Draw13);
		int TNY;
		if (T3Y - T1Y < 0) TNY = -int(UINT((T1Y - T3Y)*Draw12) / UINT(Draw13));
		else TNY = UINT((T3Y - T1Y)*Draw12) / UINT(Draw13);
		THAdd1X = (int)(TNX - TVAdd1X*Draw12) / MLine;
		THAdd1Y = (int)(TNY - TVAdd1Y*Draw12) / MLine;
		THAdd2X = (int)(TNX + T1X - T2X) / MLine;
		THAdd2Y = (int)(TNY + T1Y - T2Y) / MLine;
	}
	ToZ2 = 1;
	_asm {
		jmp WasChange;
	SkipChange:
		mov Left1Add, eax;
		mov Left2Add, eax;
		mov Left1, ebx;
		imul eax, Draw12;
		add eax, ebx;
		mov Left2, eax;
	}
	TVAdd2X = TVAdd1X = (int)(T3X - T1X) / Draw13;
	TVAdd2Y = TVAdd1Y = (int)(T3Y - T1Y) / Draw13;
	if (MLine = ((Right2 + 0xffff - Left2) >> 16)) {
		THAdd2X = THAdd1X = (int)(T2X - T1X - TVAdd1X*Draw12) / MLine;
		THAdd2Y = THAdd1Y = (int)(T2Y - T1Y - TVAdd1Y*Draw12) / MLine;
	}
	ToZ2 = 0;
	_asm {
	WasChange:
		add Right1, 0xffff;
		add Right2, 0xffff;
	}
	if ((x1 >= 0 && y1 >= 0 && x2 >= 0 && y2 >= 0 && x3 >= 0 && y3 >= 0) &&
		(x1 < SFWidth && y1 < SFHeight && x2 < SFWidth && y2 < SFHeight && x3 < SFWidth && y3 < SFHeight)) {
		DrawTarget = (LPBYTE)Surface + y1*Pitch;
		Right1 -= Left1;
		_asm {
			//빨리 그리기
			mov eax, T1Y;
			mov EaxSave, eax;
			mov eax, T1X;

			mov ebx, Right1Add;
			sub ebx, Left1Add;
			mov edx, Left1Add;
			mov ecx, Draw12;
			jcxz SkipDraw1;
			call DrawTri;
		SkipDraw1:
			cmp ToZ2, 0;
			je NoToZ2;
			mov eax, T2Y;
			mov EaxSave, eax;
			mov eax, T2X;
		NoToZ2:
			mov ebx, Right2;
			sub ebx, Left2;
			mov Right1, ebx;
			mov ecx, Left2;
			mov Left1, ecx;

			mov edx, TVAdd2X;
			mov TVAdd1X, edx;
			mov edx, TVAdd2Y;
			mov TVAdd1Y, edx;

			mov edx, THAdd2X;
			mov THAdd1X, edx;
			mov edx, THAdd2Y;
			mov THAdd1Y, edx;

			mov ebx, Right2Add;
			sub ebx, Left2Add;
			mov edx, Left2Add;
			mov ecx, Draw23;
			jcxz SkipDraw2;
			call DrawTri;
		SkipDraw2:

			jmp FunctionOut;
		DrawTri:
		DrawLoop:
			push ecx;

			mov ecx, Right1;
			shr ecx, 16;
			mov edi, Left1;
			shr edi, 16;

			add edi, DrawTarget;

			jcxz SkipDraw;
			push ebx;
			push eax;
			push EaxSave;
			mov ebx, eax;
			mov al, byte ptr[DrawColor];
		FillDraw:
			xor esi, esi;
			mov si, word ptr[EaxSave + 2];
			shl esi, 8;
			add ebx, [THAdd1X];
			xchg ebx, EaxSave;
			or si, word ptr[EaxSave + 2];
			add ebx, [THAdd1Y];
			xchg ebx, EaxSave;
			add esi, TexStart;
			scasb;
			jne SkipFill;
			dec edi;
			movsb;
		SkipFill:
			loop FillDraw;
			pop EaxSave;
			pop eax;
			pop ebx;
		SkipDraw:
			mov ecx, Pitch;
			add DrawTarget, ecx;
			add Right1, ebx;
			add Left1, edx;

			add eax, [TVAdd1X];
			xchg eax, EaxSave;
			add eax, [TVAdd1Y];
			xchg eax, EaxSave;

			pop ecx;
			loop DrawLoop;
			ret;
		}
	}
	else {
		if (y1 + Draw12 > SFHeight) {
			Draw12 = SFHeight - y1;
			Draw23 = 0;
		}
		else if (y2 + Draw23 > SFHeight - 1) {
			Draw23 = SFHeight - y2;
		}
		if (y2 < 0) {
			Left2 -= Left2Add*y2;
			Right2 -= Right2Add*y2;
			Draw23 += y2;
			if (ToZ2) {
				T2X -= y2*TVAdd2X;
				T2Y -= y2*TVAdd2Y;
			}
			else {
				T1X += TVAdd1X*Draw12 - y2*TVAdd2X;
				T1Y += TVAdd1Y*Draw12 - y2*TVAdd2Y;
			}
			Draw12 = 0;
			y1 = 0;
		}
		else if (y1 < 0) {
			Left1 -= Left1Add*y1;
			Right1 -= Right1Add*y1;
			Draw12 += y1;
			T1X -= y1*TVAdd1X;
			T1Y -= y1*TVAdd1Y;
			y1 = 0;
		}

		DrawTarget = (LPBYTE)Surface + y1*Pitch;
		_asm { //클립해서 그리기
			mov eax, T1Y;
			mov EaxSave, eax;
			mov eax, T1X;

			mov ebx, Right1Add;
			mov edx, Left1Add;
			mov ecx, Draw12;
			jcxz SkipDraw1_C;
			call DrawTri_C;
		SkipDraw1_C:
			cmp ToZ2, 0;
			je NoToZ2_C;
			mov eax, T2Y;
			mov EaxSave, eax;
			mov eax, T2X;
		NoToZ2_C:

			mov ebx, Right2;
			mov Right1, ebx;
			mov ecx, Left2;
			mov Left1, ecx;

			mov edx, TVAdd2X;
			mov TVAdd1X, edx;
			mov edx, TVAdd2Y;
			mov TVAdd1Y, edx;

			mov edx, THAdd2X;
			mov THAdd1X, edx;
			mov edx, THAdd2Y;
			mov THAdd1Y, edx;

			mov ebx, Right2Add;
			mov edx, Left2Add;
			mov ecx, Draw23;
			jcxz SkipDraw2_C;
			call DrawTri_C;
		SkipDraw2_C:

			jmp FunctionOut;
		DrawTri_C:
		DrawLoop_C:
			push ecx;
			push eax;
			push EaxSave;
			mov ecx, Right1;
			sar ecx, 16;
			jl SkipDraw_C;
			mov edi, Left1;
			sar edi, 16;
			jge SkipClipping_L;
			mov esi, edi;
			imul edi, THAdd1X;
			imul esi, THAdd1Y;
			sub eax, edi;
			sub EaxSave, esi;
			xor edi, edi;
		SkipClipping_L:
			cmp ecx, Width;
			jb SkipClipping_R;
			mov ecx, Width;
			cmp ecx, edi;
			jbe SkipDraw_C;
		SkipClipping_R:
			sub ecx, edi;
			or cx, cx;
			jle SkipDraw_C;

			add edi, DrawTarget;

			jcxz SkipDraw_C;
			push ebx;
			mov ebx, eax;
			mov al, byte ptr[DrawColor];
		FillDraw_C:
			xor esi, esi;
			mov si, word ptr[EaxSave + 2];
			shl esi, 8;
			add ebx, [THAdd1X];
			xchg ebx, EaxSave;
			or si, word ptr[EaxSave + 2];
			add ebx, [THAdd1Y];
			xchg ebx, EaxSave;
			add esi, TexStart;
			scasb;
			jne SkipFill_C;
			dec edi;
			movsb;
		SkipFill_C:
			loop FillDraw_C;
			pop ebx;
		SkipDraw_C:
			mov ecx, Pitch;
			add DrawTarget, ecx;
			add Right1, ebx;
			add Left1, edx;

			pop EaxSave;
			pop eax;

			add eax, [TVAdd1X];
			xchg eax, EaxSave;
			add eax, [TVAdd1Y];
			xchg eax, EaxSave;

			pop ecx;
			sub ecx, 1;
			jne DrawLoop_C;
			ret;
		}
	}
FunctionOut:;
}
