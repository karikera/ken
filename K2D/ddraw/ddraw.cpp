#include "ddraw.h"
#include <ddraw.h>

draw::LPDIRECTDRAW CSurface::pDD = nullptr;
dword CSurface::dwWidth = 0, CSurface::dwHeight = 0;

#define TESTERR(func) CheckError(hr,__LINE__,func)
bool CSurface::CheckError(HRESULT hr, int line, RStringA func)
{
	switch (hr)
	{
	case DDERR_SURFACELOST: return false;
	default:
		if (FAILED(hr)) __errorTerminate(__FILE__, line, func, KT_COM, hr);
		return true;
	}
}

void CSurface::Delete()
{
	SafeRelease(pSurface);
}
bool CSurface::Lock(CDraw &draw)
{
	HRESULT hr;
	DDSURFACEDESC ddsd;

	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH | DDSD_WIDTH | DDSD_HEIGHT;

	hr = pSurface->Lock(nullptr, &ddsd, DDLOCK_DONOTWAIT | DDLOCK_WRITEONLY, nullptr);
	if (!TESTERR("IDirectDrawSurface::Lock")) return false;
	draw.Attach(ddsd.lpSurface, ddsd.lPitch, ddsd.dwWidth, ddsd.dwHeight);
	return true;
}
void CSurface::Unlock()
{
	pSurface->Unlock(nullptr);
}
bool CSurface::Clear(dword dwColor)
{
	HRESULT hr;
	DDBLTFX bfx;
	bfx.dwSize = sizeof(bfx);
	bfx.dwFillColor = dwColor;

	hr = pSurface->Blt(nullptr, nullptr, nullptr, DDBLT_DONOTWAIT | DDBLT_COLORFILL, &bfx);
	return TESTERR("IDirectDrawSurface::Blt");
}
bool CSurface::Restore()
{
	HRESULT hr;
	if (FAILED(hr = pSurface->Restore()))
	{
		switch (hr)
		{
		case DDERR_WRONGMODE: return false;
		default: krrcom(("IDirectDrawSurface::Restore", hr));
		}
	}
	return true;
}
void CSurface::ColorKey(dword color, FColorKey flags)
{
	DDCOLORKEY ddck = { color,color };
	krrcom(pSurface->SetColorKey((dword&)flags, &ddck));
}
void CSurface::GetSize(dword *pWidth, dword *pHeight)
{
	DDSURFACEDESC ddsd;
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;

	krrcom(pSurface->GetSurfaceDesc(&ddsd));

	*pWidth = ddsd.dwWidth;
	*pHeight = ddsd.dwHeight;
}
void CSurface::ReadyCopy()
{
	GetSize(&dwWidth, &dwHeight);
}
void CSurface::Copy(RCWH rect, PT pt, CSurface img, FFastCopy flags)
{
	HRESULT hr;
	RECT nrect;

	nrect.left = rect.x;
	nrect.top = rect.y;

	int drawr = pt.x + rect.w;
	int drawb = pt.y + rect.h;
	if (pt.x >= (int)dwWidth || pt.y >= (int)dwHeight || drawr <= 0 || drawb <= 0) return;
	if (pt.x<0)
	{
		nrect.left -= pt.x;
		pt.x = 0;
	}
	if (pt.y<0)
	{
		nrect.top -= pt.y;
		pt.y = 0;
	}
	if (drawr>(int)dwWidth) nrect.right = dwWidth + nrect.left - pt.x;
	else nrect.right = rect.x + rect.w;
	if (drawb>(int)dwHeight) nrect.bottom = dwHeight + nrect.top - pt.y;
	else nrect.bottom = rect.y + rect.h;
	while (FAILED(hr = pSurface->BltFast(pt.x, pt.y, img.pSurface, &nrect, (dword&)flags)))
	{
		if (!TESTERR("IDirectDrawSurface::Blt")) return;
	}
}
void CSurface::Copy(const RC &rect, const RC &rect2, CSurface img, FSurfCopy flags)
{
	HRESULT hr;
	while (FAILED(hr = pSurface->Blt((LPRECT)&rect2, img.pSurface, (LPRECT)&rect, (dword&)flags, nullptr)))
	{
		if (!TESTERR("IDirectDrawSurface::Blt")) return;
	}
}
HDC CSurface::GetDC()
{
	HDC hDC;
	krrcom(pSurface->GetDC(&hDC));
	return hDC;
}
void CSurface::ReleaseDC(HDC hDC)
{
	pSurface->ReleaseDC(hDC);
}
void CWindowScreenBase::DDCreate()
{
	DDSURFACEDESC ddsd;

	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_PIXELFORMAT;

	ddsd.ddpfPixelFormat.dwSize = sizeof(ddsd.ddpfPixelFormat);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;

	krrcom(pDD->GetDisplayMode(&ddsd));
	if (ddsd.ddpfPixelFormat.dwRGBBitCount != 32) krr("죄송합니다. 아직은 32비트 색만 지원되고 있어요."); //TODO: support other bits format

	krrcom(pDD->SetCooperativeLevel(nullptr, DDSCL_NORMAL));

	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	krrcom(pDD->CreateSurface(&ddsd, &pSurface, nullptr));

	krrcom(pDD->CreateClipper(0, &clip, nullptr));

	krrcom(clip->SetHWnd(0, g_pMainWindow));

	krrcom(pSurface->SetClipper(clip));

	DDUpdateSize();
}
bool CWindowScreenBase::DDResize(int w, int h)
{
	krr("CWindowScreen::Resize");
}
void CWindowScreenBase::DDCopy(CSurface surf)
{
	ivec2 pt(0, 0);
	g_pMainWindow->ClientToScreen(&pt);
	CSurfaceBase::DDCopy(RC(0, 0, W, H), RC(pt, pt.x + W, pt.y + H), surf);
}
void CWindowScreenBase::DDUpdateSize()
{
	RC rect = g_pMainWindow->GetClientRect();
	W = rect.r;
	H = rect.b;
}
bool CWindowScreenBase::DDEqualize(COffscreenSurface& surf)
{
	return surf.Resize(W, H);
}
void CFullScreenBase::DDCreate(int w, int h)
{
	DDSURFACEDESC ddsd;

	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.dwBackBufferCount = 1;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;

	krrcom(pDD->SetCooperativeLevel(g_pMainWindow, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE));
	DDResize(w, h);

	krrcom(pDD->CreateSurface(&ddsd, &pSurface, nullptr));
}
void CFullScreenBase::DDGetSize(dword *pWidth, dword *pHeight)
{
	DDSURFACEDESC ddsd;
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;

	krrcom(pDD->GetDisplayMode(&ddsd));

	*pWidth = ddsd.dwWidth;
	*pHeight = ddsd.dwHeight;
}
bool CFullScreenBase::DDResize(int w, int h)
{
	dword width, height;

	DDGetSize(&width, &height);
	if (width == w && height == h) return true;

	krrcom(pDD->SetDisplayMode(w, h, 32, 0, 0));
	return true;
}
bool CFullScreenBase::DDRestore()
{
	dword width, height;
	HRESULT hr;
	if (FAILED(hr = pSurface->Restore()))
	{
		switch (hr)
		{
		case DDERR_SURFACELOST: return false;
		case DDERR_IMPLICITLYCREATED:
			DDGetSize(&width, &height);
			DDDelete();
			DDCreate(width, height);
			return true;
		case DDERR_WRONGMODE: return false;
		default: krrcom(("IDirectDrawSurface::Restore", hr));
		}
	}
	return true;
}
void CSurface::Initialize()
{
#if KR_DXVERSION == 1
	krrcom(DirectDrawCreate(nullptr, &pDD, nullptr));
#endif
#if KR_DXVERSION == 2 || KR_DXVERSION == 3
	krrcom(DirectDrawCreateEx(nullptr, (void**)&pDD, IID_IDirectDraw2, nullptr));
#endif
#if KR_DXVERSION == 4
	krrcom(DirectDrawCreateEx(nullptr, (void**)&pDD, IID_IDirectDraw4, nullptr));
#endif
#if KR_DXVERSION >= 7
	//krrcom(DirectDrawCreateEx( nullptr, (void**)&pDD, IID_IDirectDraw7, nullptr ));
#endif
}
void CSurface::Uninitialize()
{
	SafeRelease(pDD);
}
void CBackSurface::Create(CFullScreenBase surf)
{
	HRESULT hr;

	pPrimary = &surf;

	DDSCAPS ddsc;
	ddsc.dwCaps = DDSCAPS_BACKBUFFER;

	hr = surf.pSurface->GetAttachedSurface(&ddsc, &pSurface);
	surf.TESTERR("IDirectDraw::GetAttachedSurface");
}
bool CBackSurface::Flip()
{
	HRESULT hr = pPrimary->pSurface->Flip(nullptr, DDFLIP_WAIT);
	return TESTERR("IDirectDrawSurface::Flip");
}
bool CBackSurface::Restore()
{
	if (pPrimary->DDRestore())
	{
		Delete();
		Create(*pPrimary);
		return true;
	}
	return false;
}
bool COffscreenSurface::Resize(int w, int h)
{
	dword width, height;
	DDSURFACEDESC ddsd;

	if (pSurface)
	{
		GetSize(&width, &height);
		if (w == width && h == height) return true;
		pSurface->Release();
		pSurface = nullptr;
	}
	if (w == 0 || h == 0) return false;

	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = w;
	ddsd.dwHeight = h;

	krrcom(pDD->CreateSurface(&ddsd, &pSurface, nullptr));
	return true;
}
void COffscreenSurface::LoadFromBMPData(const BMPINFO * pbi, ptr data)
{
	dword W = pbi->biWidth;
	dword H = pbi->biHeight;

	Create(W, H);

	HDC hdc;
	if (SUCCEEDED(pSurface->GetDC(&hdc)))
	{
		StretchDIBits(hdc, 0, 0, W, H, 0, 0, W, H, data, (const BITMAPINFO*)pbi, 0, SRCCOPY);
		pSurface->ReleaseDC(hdc);
	}
}
bool COffscreenSurface::LoadFromBMPFile(pcwstr strFile)
{
	BITMAPFILEHEADER bfh;
	uintp upRead;

	File* file = File::Create(strFile, GENERIC_READ, OPEN_EXISTING);
	AutoDelete(file);

	file->read(bfh);
	if (bfh.bfType != MakeSignature('B', 'M')) krr(tstra << (RString)strFile << "\r\n이 파일은 비트맵 파일이 아니에요!");

	stack_single(BMPINFO, pbi, bfh.bfOffBits);
	upRead = file->read(pbi, bfh.bfOffBits);

	stack(byte, pBits, pbi->biSizeImage);
	upRead = file->read(pBits, pbi->biSizeImage);

	LoadFromBMPData(pbi, pBits);
	return true;
}
bool COffscreenSurface::LoadFromPNGFile(pcwstr strFileName)
{
	CPNG png;
	png.Open(strFileName);
	Create(png.W, png.H);

	CDraw draw;
	Lock(draw);
	png.Read(draw);
	Unlock();

	return true;
}
bool COffscreenSurface::SaveAsPNGFile(pcwstr Filename, BOOL InterlaceAdam7)
{
	using namespace gdi;

	Bitmap*			hBmp;
	DrawContext*	MemDC;

	// First get the size of the surface...
	int W, H;
	{
		DDSURFACEDESC	ddsd;
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
		pSurface->GetSurfaceDesc(&ddsd);
		W = ddsd.dwWidth;
		H = ddsd.dwHeight;
	}

	{
		DrawContext*	hdc;

		// Then get the content of the surface into a DC...
		if (SUCCEEDED(pSurface->GetDC((HDC*)&hdc)))
		{
			MemDC = hdc->CreateCompatibleDC();
			hBmp = hdc->CreateCompatibleBitmap(W, H);
			Bitmap * old = MemDC->Select(hBmp);
			MemDC->BitBlt(hdc, irectwh(0, 0, W, H), ivec2(0, 0));
			MemDC->Select(old);
			pSurface->ReleaseDC(hdc);
		}
		else return false;
	}

	// open the file...

	FILE *fp;
	dword dwErr = _wfopen_s(&fp, Filename, L"wb");
	if (fp == nullptr) return false;

	// Create and initialize the png_struct...
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (png_ptr == nullptr)
	{
		fclose(fp);
		return false;
	}

	// Allocate/initialize the image information data.  REQUIRED...
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == nullptr)
	{
		fclose(fp);
		png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
		return false;
	}

	// Set error handling.  REQUIRED if you aren't supplying your own
	// error hadnling functions in the png_create_write_struct() call.
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		fclose(fp);
		png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
		return false;
	}

	// set up the output control if you are using standard C streams...
	png_init_io(png_ptr, fp);

	// Set the image information here...
	if (InterlaceAdam7)
		png_set_IHDR(png_ptr, info_ptr, W, H, 8, PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_ADAM7, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	else
		png_set_IHDR(png_ptr, info_ptr, W, H, 8, PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// Write the file header information...
	png_write_info(png_ptr, info_ptr);

	// pack pixels into bytes...
	png_set_packing(png_ptr);

	// flip BGR pixels to RGB...
	png_set_bgr(png_ptr);

	// Bitmapstruct init...
	PBITMAPINFO pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
	if (pbmi == nullptr) not_enough_memory();

	// Initialize the fields in the BMPINFO structure...
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = W;
	pbmi->bmiHeader.biHeight = H;
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = 24;
	pbmi->bmiHeader.biCompression = BI_RGB;
	pbmi->bmiHeader.biSizeImage = ((W + 7) >> 3) * H * 24;

	// Now copy row by row the DC Content into the RowBuffer...
	int			row = H; // For flipping...

	stack(png_bytep, row_pointers, H);
	do
	{
		row_pointers[row] = (UCHAR*)malloc(png_get_rowbytes(png_ptr, info_ptr));
		GetDIBits(MemDC, hBmp, H - row - 1, (word)1, row_pointers[row], pbmi, 0);
	}
	while (--row > -1);

	// Now write the imagedata...
	png_write_image(png_ptr, row_pointers);

	// Finish writing...
	png_write_end(png_ptr, info_ptr);

	// clean up after the write, and free any memory allocated...
	png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);

	LocalFree((PBITMAPINFO)pbmi);
	fclose(fp);
	delete MemDC;
	delete hBmp;

	// Yeah!
	return true;
}


#pragma region  8bit triangle
#if 0

void CDraw<8>::Triangle(ivec2 pt1, ivec2 pt2, ivec2 pt3)
{
	dword Left1, Left2, Right1, Right2;
	int Left1Add, Left2Add, Right1Add, Right2Add;
	int Draw12, Draw23;
	int tx12 = pt2.x - pt1.x;

	int SFWidth = width;
	int SFHeight = height;
	int Pitch = pitch;

	if (Draw12 = (pt2.y - pt1.y))
	{
		Right1Add = (tx12 << 16) / Draw12;
		Right1 = (pt1.x << 16);
	}
	if (Draw23 = (pt3.y - pt2.y))
	{
		Right2Add = ((pt3.x - pt2.x) << 16) / Draw23;
		Right2 = (pt2.x << 16);
	}

	_asm
	{// 1-3 시작점, 증가량 계산

		mov ebx, pt1.x; // pt1.x HALF 형식으로
		shl ebx, 16;

		mov ecx, pt3.y;
		sub ecx, pt1.y;
		je FunctionOut;
		mov eax, pt3.x;
		sub eax, pt1.x;
		shl eax, 16;
		cdq;
		idiv ecx;
		cmp Draw12, 0;
		jne CheckRightAdd;
		cmp tx12, 0;
		jge __SkipChange;
		jmp NoCheckRightAdd;
	__CheckRightAdd:
		cmp Right1Add, eax;
		jge __SkipChange;
	__NoCheckRightAdd:
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
		jmp WasChange;
	__SkipChange:
		mov Left1Add, eax;
		mov Left2Add, eax;
		mov Left1, ebx;
		imul eax, Draw12;
		add eax, ebx;
		mov Left2, eax;
	__WasChange:
		add Right1, 0xffff;
		add Right2, 0xffff;
	}
	if ((pt1.x >= 0 && pt1.y >= 0 && pt2.x >= 0 && pt2.y >= 0 && pt3.x >= 0 && pt3.y >= 0) &&
		(pt1.x<width && pt1.y<height && pt2.x<width && pt2.y<height && pt3.x<width && pt3.y<height))
	{
		ptr DrawTarget = (pbyte)image + pt1.y*Pitch;
		Right1 -= Left1;
		_asm {
			//빨리 그리기
			mov eax, Color;

			mov ebx, Right1Add;
			sub ebx, Left1Add;
			mov edx, Left1Add;
			mov ecx, Draw12;
			jcxz __SkipDraw1;
			call DrawTri;
		__SkipDraw1:

			mov ebx, Right2;
			sub ebx, Left2;
			mov Right1, ebx;
			mov ecx, Left2;
			mov Left1, ecx;

			mov ebx, Right2Add;
			sub ebx, Left2Add;
			mov edx, Left2Add;
			mov ecx, Draw23;
			jcxz __SkipDraw2;
			call DrawTri;
		__SkipDraw2:

			jmp FunctionOut;
		__DrawTri:
		__DrawLoop:
			push ecx;

			mov ecx, Right1;
			shr ecx, 16;
			mov edi, Left1;
			shr edi, 16;

			add edi, DrawTarget;

			push ecx;
			shr ecx, 2;
			rep stosd;
			pop ecx;
			and ecx, 3;
			rep stosb;
			mov ecx, Pitch;
			add DrawTarget, ecx;
			add Right1, ebx;
			add Left1, edx;

			pop ecx;
			loop __DrawLoop;
			ret;
		}
	}
	else
	{
		if (pt1.y + Draw12 > SFHeight)
		{
			Draw12 = SFHeight - pt1.y;
			Draw23 = 0;
		}
		else if (pt2.y + Draw23 > SFHeight - 1)
		{
			Draw23 = SFHeight - pt2.y;
		}
		if (pt2.y<0)
		{
			Left2 -= Left2Add*pt2.y;
			Right2 -= Right2Add*pt2.y;
			Draw23 += pt2.y;
			Draw12 = 0;
			pt1.y = 0;
		}
		else if (pt1.y<0)
		{
			Left1 -= Left1Add*pt1.y;
			Right1 -= Right1Add*pt1.y;
			Draw12 += pt1.y;
			pt1.y = 0;
		}

		ptr DrawTarget = (pbyte)image + pt1.y*Pitch;
		_asm
		{ //클립해서 그리기
			mov eax, Color;

			mov ebx, Right1Add;
			mov edx, Left1Add;
			mov ecx, Draw12;
			call DrawTri_C;

			mov ebx, Right2;
			mov Right1, ebx;
			mov ecx, Left2;
			mov Left1, ecx;

			mov ebx, Right2Add;
			mov edx, Left2Add;
			mov ecx, Draw23;
			call DrawTri_C;

			jmp FunctionOut;
		__DrawTri_C:
			jcxz __SkipDraw_C;
		__DrawLoop_C:
			push ecx;

			mov ecx, Right1;
			shr ecx, 16;
			mov edi, Left1;
			shr edi, 16;
			or di, di;
			jge __SkipClipping_L;
			mov edi, 0;
			or cx, cx;
			jle __SkipRepDraw_C;
		__SkipClipping_L:
			cmp ecx, SFWidth;
			jb SkipClipping_R;
			mov ecx, SFWidth;
			cmp ecx, edi;
			jbe SkipRepDraw_C;
		__SkipClipping_R:
			sub ecx, edi;

			add edi, DrawTarget;

			push ecx;
			shr ecx, 2;
			rep stosd;
			pop ecx;
			and ecx, 3;
			rep stosb;
		__SkipRepDraw_C:
			mov ecx, Pitch;
			add DrawTarget, ecx;
			add Right1, ebx;
			add Left1, edx;

			pop ecx;
			loop __DrawLoop_C;
		__SkipDraw_C:
			ret;
		}
	}
FunctionOut:;
}


void Copy2To32(CDrawBase *_dest, CDrawBase *_src, dword *colors)
{
}
void Copy4To32(CDrawBase *_dest, CDrawBase *_src, dword *colors)
{
}
void Copy8To32(CDrawBase *_dest, CDrawBase *_src, dword *colors)
{
}
void Copy16To32(CDrawBase *_dest, CDrawBase *_src)
{
}
void Copy32To32(CDrawBase *_dest, CDrawBase *_src)
{
	dword w;
	__asm
	{
		mov ecx, _dest;
		mov edi, [ecx + CDrawBase::image];
		mov ebx, [ecx + CDrawBase::pitch];

		mov ecx, _src;
		mov esi, [ecx + CDrawBase::image];

		mov eax, [ecx + CDrawBase::width];
		mov w, eax;
		shl eax, 2;
		sub ebx, eax;
		sub edx, eax;

		mov ecx, [ecx + CDrawBase::height];
	__VLoop:
		mov eax, ecx;
		mov ecx, w;
		rep movsd;
		mov ecx, eax;
		add edi, ebx;
		add esi, edx;
		loop __VLoop;
	}
}
#endif
#pragma endregion