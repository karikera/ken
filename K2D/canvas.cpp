#include "canvas.h"
#include "reformatter.h"

using namespace kr;

#define CLIP_RECT() \
{\
	int tx=rect.x,ty=rect.y;\
	int wc=m_width-tx,hc=m_height-ty;\
	if(rect.width>wc) w=wc;\
	else w=rect.width;\
	if(rect.height>hc) h=hc;\
	else h=rect.height;\
	if(tx<0)\
	{\
		w+=tx;\
		x=0;\
	}\
	else x=tx;\
	if(ty<0)\
	{\
		h+=ty;\
		y=0;\
	}\
	else y=ty;\
	if(w<=0 || h<=0) return;\
}

namespace
{
	template <image::PixelFormat dstpf, image::PixelFormat srcpf>
	irectwh getDrawableRect(ivec2 * pt, const irectwh &rc, image::MemCanvas<dstpf> &dest, image::MemCanvas<srcpf> &src) noexcept
	{
		using namespace kr::math;
		irectwh out;
		out.width = tmin(rc.width, src.getWidth());
		out.height = tmin(rc.height, src.getHeight());
		out.x = rc.x;
		out.y = rc.y;

		int c, c2;

		c = dest.getWidth() - pt->x;
		c2 = dest.getWidth() - out.x;
		c = tmin(c, c2);
		if (out.width>c) out.width = c;

		c = dest.getHeight() - pt->y;
		c2 = dest.getHeight() - out.y;
		c = tmin(c, c2);
		if (out.height>c) out.height = c;

		if (pt->x<0)
		{
			out.width += pt->x;
			out.x -= pt->x;
			pt->x = 0;
		}
		if (pt->y<0)
		{
			out.height += pt->y;
			out.y -= pt->y;
			pt->y = 0;
		}
		if (out.x<0)
		{
			out.width += out.x;
			pt->x -= out.x;
			out.x = 0;
		}
		if (out.y<0)
		{
			out.height += out.y;
			pt->y -= out.y;
			out.y = 0;
		}
		return out;
	}

	using pixel_t = dword;
	using mempx = mem32;
}

template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::clear() noexcept
{
	constexpr int PIXEL_SIZE = Pixel<pf>::size;
	size_t nextline = m_pitch;
	byte* dest = (byte*)m_image;
	byte* end = dest + m_height * nextline;
	size_t w = m_width;
	pixel_t chr = (pixel_t&)color;
	while (dest != end)
	{
		mempx::set(dest, chr, w);
		dest += nextline;
	}
}
template <image::PixelFormat pf>
void * kr::image::MemCanvas<pf>::allocate(int w, int h) noexcept
{
	return ImageData::allocate(pf, w, h);
}
template <image::PixelFormat pf>
void * kr::image::MemCanvas<pf>::allocate(int w, int h, int pitchBytes) noexcept
{
	return ImageData::allocate(pf, w, h, pitchBytes);
}
template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::allocate(ImageData * _src) noexcept
{
	return ImageData::allocate(_src, pf);
}
template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::point(ivec2 pt) noexcept
{
	constexpr int PIXEL_SIZE = Pixel<pf>::size;
	if ((uint)pt.x >= (uint)m_width || (uint)pt.y >= (uint)m_height) return;
	*(pixel_t*)((byte*)m_image + m_pitch*pt.y + pt.x * PIXEL_SIZE) = (pixel_t&)color;
}
template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::fill(const irectwh &rect) noexcept
{
	int x, y, w, h;
	CLIP_RECT();

	constexpr int PIXEL_SIZE = Pixel<pf>::size;
	size_t nextline = m_pitch;
	byte* dest = y * nextline + x * PIXEL_SIZE + (byte*)m_image;
	byte* end = dest + h * nextline;
	pixel_t chr = (pixel_t&)color;
	while (dest != end)
	{
		mempx::set(dest, chr, w);
		dest += nextline;
	}
}
template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::randomFill(const irectwh &rect, Random& rand) noexcept
{
	int x, y, w, h;
	CLIP_RECT();

	uint wbytes = w * 4;
	byte* data = (byte*)m_image + m_pitch*y + wbytes;
	byte* end = m_pitch*h + wbytes + data;
	for (; data != end; data += m_pitch)
	{
		rand.fill(data, wbytes);
	}
}
template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::hLine(ivec2 pt, int len) noexcept
{
	if (uint(pt.y) >= uint(m_height)) return;
	if (pt.x < 0)
	{
		len += pt.x;
		pt.x = 0;
	}
	if (uint(pt.x + len) > uint(m_width)) len = m_width - pt.x;
	if (len <= 0) return;

	constexpr int PIXEL_SIZE = Pixel<pf>::size;
	byte* dest = pt.y * m_pitch + pt.x * PIXEL_SIZE + (byte*)m_image;
	mempx::set(dest, (pixel_t&)color, len);
}
template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::hLineXOR(ivec2 pt, int len) noexcept
{
	if (uint(pt.y) >= uint(m_height)) return;
	if (pt.x<0)
	{
		len += pt.x;
		pt.x = 0;
	}
	if (uint(pt.x + len)>uint(m_width)) len = m_width - pt.x;
	if (len <= 0) return;

	constexpr int PIXEL_SIZE = Pixel<pf>::size;

	byte* dest = pt.y * m_pitch + pt.x*PIXEL_SIZE + (byte*)m_image;
	pixel_t chr = (pixel_t&)color;
	byte* end = dest + len * PIXEL_SIZE;
	while (dest != end)
	{
		*(pixel_t*)dest ^= chr;
		dest += PIXEL_SIZE;
	}
}
template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::vLine(ivec2 pt, int len) noexcept
{
	if (uint(pt.x) >= uint(m_width)) return;
	if (pt.y<0)
	{
		len += pt.y;
		pt.y = 0;
	}
	if (uint(pt.y + len) >= uint(m_height)) len = m_height - pt.y;
	if (len <= 0) return;

	constexpr int PIXEL_SIZE = Pixel<pf>::size;
	size_t pitch = m_pitch;
	byte* dest = pt.y * pitch + pt.x*PIXEL_SIZE + (byte*)m_image;
	pixel_t chr = (pixel_t&)color;

	byte* end = len * pitch + dest;
	while (dest != end)
	{
		*(pixel_t*)dest = chr;
		dest += pitch;
	}
}
template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::vLineXOR(ivec2 pt, int len) noexcept
{
	if (dword(pt.x) >= dword(m_width)) return;
	if (pt.y<0)
	{
		len += pt.y;
		pt.y = 0;
	}
	if (dword(pt.y + len) >= dword(m_height)) len = m_height - pt.y;
	if (len <= 0) return;

	constexpr int PIXEL_SIZE = Pixel<pf>::size;
	size_t pitch = m_pitch;
	byte* dest = pt.y * pitch + pt.x*PIXEL_SIZE + (byte*)m_image;
	pixel_t chr = (pixel_t&)color;

	byte* end = len * pitch + dest;
	while (dest != end)
	{
		*(pixel_t*)dest ^= chr;
		dest += pitch;
	}
}
template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::line(ivec2 pt1, ivec2 pt2) noexcept
{
	if ((pt2.x<0 && pt1.x<0) || (pt2.y<0 && pt1.y<0) || (pt2.x >= m_width && pt1.x >= m_width) || (pt2.y >= m_height && pt1.y >= m_height)) return;

	if (pt1.x == pt2.x)
	{
		int len = abs(pt1.y - pt2.y) + 1;
		pt1.y = tmin(pt1.y, pt2.y);
		vLine(pt1, len);
		return;
	}
	if (pt1.y == pt2.y)
	{
		int len = abs(pt1.x - pt2.x) + 1;
		pt1.x = tmin(pt1.x, pt2.x);
		hLine(pt1, len);
		return;
	}
	int sx = pt2.x - pt1.x;
	int sy = pt2.y - pt1.y;
	if (pt1.x<0) { pt1.y = pt1.y - pt1.x*sy / sx; pt1.x = 0; }
	if (pt1.y<0) { pt1.x = pt1.x - pt1.y*sx / sy; pt1.y = 0; }
	if (pt1.x >= m_width) { pt1.y = pt1.y + (m_width - 1 - pt1.x)*sy / sx; pt1.x = m_width - 1; }
	if (pt1.y >= m_height) { pt1.x = pt1.x + (m_height - 1 - pt1.y)*sx / sy; pt1.y = m_height - 1; }
	if (pt2.x<0) { pt2.y = pt2.y - pt2.x*sy / sx; pt2.x = 0; }
	if (pt2.y<0) { pt2.x = pt2.x - pt2.y*sx / sy; pt2.y = 0; }
	if (pt2.x >= m_width) { pt2.y = pt2.y + (m_width - 1 - pt2.x)*sy / sx; pt2.x = m_width - 1; }
	if (pt2.y >= m_height) { pt2.x = pt2.x + (m_height - 1 - pt2.y)*sx / sy; pt2.y = m_height - 1; }

	int ToX = (pt2.x - pt1.x);
	int ToY = (pt2.y - pt1.y);
	int Len = abs(ToX), Len2 = abs(ToY);
	Len = tmax(Len, Len2) + 1;
	ToX = (ToX << 16) / Len;
	ToY = (ToY << 16) / Len;

	constexpr int PIXEL_SIZE = Pixel<pf>::size;
	int pitch = m_pitch;
	byte* dest = pt1.y * pitch + pt1.x*PIXEL_SIZE + (byte*)m_image;
	pixel_t chr = (pixel_t&)color;
	uint fx = 0x8000;
	uint fy = 0x8000;
	do
	{
		byte* t = (fy >> 16) * pitch + (fx >> 16) * 4 + dest;
		*(pixel_t*)t = chr;
		fx += ToX;
		fy += ToY;
	} while (--Len);

}
template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::rectLine(const irectwh &rect) noexcept
{
	int w, h, nw, nh;
	int x, y, nx, ny;
	x = nx = rect.x;
	y = ny = rect.y;
	w = nw = rect.width;
	h = nh = rect.height;

	if (nx<0)
	{
		nw += nx;
		nx = 0;
	}
	if (dword(nx + w)>dword(m_width)) nw = m_width - nx;
	if (nw <= 0) return;

	if (ny<0)
	{
		nh += ny;
		ny = 0;
	}
	if (dword(ny + h) >= dword(m_height)) nh = m_height - ny;
	if (nh <= 0) return;


	notImplementedYet();
	/*
	__asm
	{
	mov edx,this;
	mov ebx,[edx+m_pitch];
	sub ebx,4;

	mov edi,y;
	cmp edi,[edx+m_height];
	jae __HDraw1;

	imul edi,[edx+m_pitch];
	mov eax,nx;
	lea edi,[edi+eax*4];
	add edi,[edx+m_image];

	mov ecx,nw;
	mov eax,Color;
	rep stosd;

	__HDraw1:

	mov edi,x;
	cmp edi,[edx+m_width];
	jae __VDraw1;

	mov eax,ny;
	imul eax,[edx+m_pitch];
	lea edi,[eax+edi*4];
	add edi,[edx+m_image];

	mov ecx,nh;
	mov eax,Color;
	__VLoop1:
	stosd;
	add edi,ebx;
	loop __VLoop1;
	__VDraw1:

	mov edi,y;
	add edi,h;
	dec edi;
	cmp edi,[edx+m_height];
	jae __HDraw2;

	imul edi,[edx+m_pitch];
	mov eax,nx;
	lea edi,[edi+eax*4];
	add edi,[edx+m_image];

	mov ecx,nw;
	mov eax,Color;
	rep stosd;

	__HDraw2:

	mov edi,x;
	add edi,w;
	dec edi;
	cmp edi,[edx+m_width];
	jae __VDraw2;

	mov eax,ny;
	imul eax,[edx+m_pitch];
	lea edi,[eax+edi*4];
	add edi,[edx+m_image];

	mov ecx,nh;
	mov eax,Color;
	__VLoop2:
	stosd;
	add edi,ebx;
	loop __VLoop2;

	__VDraw2:
	}*/
}
template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::copy(ivec2 pt, MemCanvas& img) noexcept
{
	copy(pt, irectwh({ 0, 0 }, { img.m_width, img.m_height }), img);
}
template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::copy(ivec2 pt, const irectwh &rect, MemCanvas& img) noexcept
{
	irectwh rc = getDrawableRect(&pt, rect, *this, img);
	if (rc.width <= 0 || rc.height <= 0) return;

	size_t pitch = m_pitch;
	byte* dest = pt.y * pitch + pt.x + (byte*)m_image;
	byte* destEnd = rc.height * pitch + dest;
	byte* src = img.m_pitch * rect.y + rect.x + (byte*)img.m_image;
	size_t copyLine = rc.width;

	while (dest != destEnd)
	{
		memcpy(dest, src, copyLine);
		dest += pitch;
	}
}
template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::triangle(ivec2 pt1, ivec2 pt2, ivec2 pt3) noexcept
{
	notImplementedYet();
	//dword Left1,Left2,Right1,Right2;
	//int Left1Add,Left2Add,Right1Add,Right2Add;
	//int Draw12,Draw23;

	//int SFWidth=m_width;
	//int SFHeight=m_height;
	//int Pitch=m_pitch;

	//if((pt1.x<0 && pt2.x<0 && pt3.x<0) ||
	//	(pt1.y<0 && pt2.y<0 && pt3.y<0) ||
	//	(pt1.x>=SFWidth && pt2.x>=SFWidth && pt3.x>=SFWidth) ||
	//	(pt1.y>=SFHeight && pt2.y>=SFHeight && pt3.y>=SFHeight)) return;

	//	__asm
	//	{
	//		mov ebx,pt1.y;
	//		mov ecx,pt2.y;
	//		cmp ebx,ecx;
	//		jle __xchg12;
	//		mov eax,pt1.x;
	//		xchg pt2.x,eax;
	//		mov pt1.x,eax;
	//		mov eax,pt1.y;
	//		xchg pt2.y,eax;
	//		mov pt1.y,eax;
	//__xchg12:
	//		mov ebx,pt2.y;
	//		mov ecx,pt3.y;
	//		cmp ebx,ecx;
	//		jle __xchg23;
	//		mov eax,pt2.x;
	//		xchg pt3.x,eax;
	//		mov pt2.x,eax;
	//		mov eax,pt2.y;
	//		xchg pt3.y,eax;
	//		mov pt2.y,eax;
	//__xchg23:
	//		mov ebx,pt1.y;
	//		mov ecx,pt2.y;
	//		cmp ebx,ecx;
	//		jle __xchg12_;
	//		mov eax,pt1.x;
	//		xchg pt2.x,eax;
	//		mov pt1.x,eax;
	//		mov eax,pt1.y;
	//		xchg pt2.y,eax;
	//		mov pt1.y,eax;
	//__xchg12_:
	//
	//	}
	//
	//	int tx12=pt2.x-pt1.x;
	//	
	//	if(Draw12=(pt2.y-pt1.y))
	//	{
	//		Right1Add=(tx12<<16)/Draw12;
	//		Right1=(pt1.x<<16);
	//	}
	//	if(Draw23=(pt3.y-pt2.y))
	//	{
	//		Right2Add=((pt3.x-pt2.x)<<16)/Draw23;
	//		Right2=(pt2.x<<16);
	//	}
	//
	//	_asm
	//	{
	//		mov ebx,pt1.x;
	//		shl ebx,16;
	//
	//		mov ecx,pt3.y;
	//		sub ecx,pt1.y;
	//		je __FunctionOut;
	//		mov eax,pt3.x;
	//		sub eax,pt1.x;
	//		shl eax,16;
	//		cdq;
	//		idiv ecx;
	//		cmp Draw12,0;
	//		jne __CheckRightAdd;
	//		cmp tx12,0;
	//		jge __SkipChange;
	//		jmp __NoCheckRightAdd;
	//__CheckRightAdd:
	//		cmp Right1Add,eax;
	//		jge __SkipChange;
	//__NoCheckRightAdd:
	//		mov ecx,eax;
	//		xchg Right2Add,eax;
	//		mov Left2Add,eax;
	//		mov eax,ecx;
	//		xchg Right1Add,eax;
	//		mov Left1Add,eax;
	//		imul ecx,Draw12;
	//		add ecx,ebx;
	//		xchg Right2,ecx;
	//		mov Left2,ecx;
	//		xchg Right1,ebx;
	//		mov Left1,ebx;
	//		jmp __WasChange;
	//__SkipChange:
	//		mov Left1Add,eax;
	//		mov Left2Add,eax;
	//		mov Left1,ebx;
	//		imul eax,Draw12;
	//		add eax,ebx;
	//		mov Left2,eax;
	//__WasChange:
	//		add Left1,0x8000;
	//		add Left2,0x8000;
	//	}
	//	if((pt1.x>=0 && pt1.y>=0 && pt2.x>=0 && pt2.y>=0 && pt3.x>=0 && pt3.y>=0) &&
	//		(pt1.x<m_width && pt1.y<m_height && pt2.x<m_width && pt2.y<m_height && pt3.x<m_width && pt3.y<m_height))
	//	{
	//		ptr DrawTarget=(byte*)m_image+pt1.y*Pitch;
	//		_asm
	//		{
	//			mov eax,Color;
	//
	//			mov ebx,Right1Add;
	//			mov edx,Left1Add;
	//			mov ecx,Draw12;
	//			jcxz __SkipDraw1;
	//			call __DrawTri;
	//__SkipDraw1:
	//
	//			mov ebx,Right2;
	//			mov Right1,ebx;
	//			mov ecx,Left2;
	//			mov Left1,ecx;
	//
	//			mov ebx,Right2Add;
	//			mov edx,Left2Add;
	//			mov ecx,Draw23;
	//			jcxz __SkipDraw2;
	//			call __DrawTri;
	//__SkipDraw2:
	//
	//			jmp __FunctionOut;
	//__DrawTri:
	//__DrawLoop:
	//			push ecx;
	//			
	//			mov ecx,Right1;
	//			shr ecx,16;
	//			mov edi,Left1;
	//			shr edi,16;
	//			sub ecx,edi;
	//			shl edi,2;
	//
	//			add edi,DrawTarget;
	//			
	//			rep stosd;
	//			mov ecx,Pitch;
	//			add DrawTarget,ecx;
	//			add Right1,ebx;
	//			add Left1,edx;
	//
	//			pop ecx;
	//			loop __DrawLoop;
	//			ret;
	//		}
	//	}
	//	else
	//	{
	//		if(pt1.y+Draw12 > SFHeight)
	//		{
	//			Draw12=SFHeight-pt1.y;
	//			Draw23=0;
	//		}
	//		else if(pt2.y+Draw23 > SFHeight-1)
	//		{
	//			Draw23=SFHeight-pt2.y;
	//		}
	//		if(pt2.y<0)
	//		{
	//			Left2-=Left2Add*pt2.y;
	//			Right2-=Right2Add*pt2.y;
	//			Draw23+=pt2.y;
	//			Draw12=0;
	//			pt1.y=0;
	//		}
	//		else if(pt1.y<0)
	//		{
	//			Left1-=Left1Add*pt1.y;
	//			Right1-=Right1Add*pt1.y;
	//			Draw12+=pt1.y;
	//			pt1.y=0;
	//		}
	//	
	//		ptr DrawTarget=(byte*)m_image+pt1.y*Pitch;
	//		_asm
	//		{
	//			mov eax,Color;
	//
	//			mov ebx,Right1Add;
	//			mov edx,Left1Add;
	//			mov ecx,Draw12;
	//			jcxz __SkipDraw1_C;
	//			call __DrawTri_C;
	//__SkipDraw1_C:
	//
	//			mov ebx,Right2;
	//			mov Right1,ebx;
	//			mov ecx,Left2;
	//			mov Left1,ecx;
	//
	//			mov ebx,Right2Add;
	//			mov edx,Left2Add;
	//			mov ecx,Draw23;
	//			jcxz __SkipDraw2_C;
	//			call __DrawTri_C;
	//__SkipDraw2_C:
	//
	//			jmp __FunctionOut;
	//__DrawTri_C:
	//__DrawLoop_C:
	//			push ecx;
	//			
	//			mov ecx,Right1;
	//			shr ecx,16;
	//			mov edi,Left1;
	//			shr edi,16;
	//			or di,di;
	//			jge __SkipClipping_L;
	//			mov edi,0;
	//			or cx,cx;
	//			jle __SkipRepDraw_C;
	//__SkipClipping_L:
	//			cmp ecx,SFWidth;
	//			jb __SkipClipping_R;
	//			mov ecx,SFWidth;
	//			cmp ecx,edi;
	//			jbe __SkipRepDraw_C;
	//__SkipClipping_R:
	//			sub ecx,edi;
	//			
	//			shl edi,2;
	//			add edi,DrawTarget;
	//
	//			rep stosd;
	//__SkipRepDraw_C:
	//			mov ecx,Pitch;
	//			add DrawTarget,ecx;
	//			add Right1,ebx;
	//			add Left1,edx;
	//
	//			pop ecx;
	//			loop __DrawLoop_C;
	//			ret;
	//		}
	//	}
	//__FunctionOut:;
}
template <image::PixelFormat pf>
void kr::image::MemCanvas<pf>::ellipse(const irectwh &rect) noexcept
{
	if (rect.width <= 0 || rect.height <= 0) return;
	if (rect.x >= m_width || rect.y >= m_height || rect.x + rect.width <= 0 || rect.y + rect.height <= 0) return;
	
	float w2 = rect.width*2.f;
	int width = m_width;
	int pitch = m_pitch;

	int y_end = rect.height;
	int bottomCut = m_height - rect.y - rect.height;
	if (bottomCut < 0)
	{
		y_end += bottomCut;
		if (y_end <= 0) return;
	}

	int y = 0;
	int topCut = rect.y;
	if (topCut < 0)
	{
		y = -topCut;
		topCut = 0;
	}

	byte * dest = topCut * pitch + (byte*)m_image;

	pixel_t chr = (pixel_t&)color;
	constexpr int PIXEL_SIZE = Pixel<pf>::size;
	do
	{
		// int w = (math::sqrt<int>((rect.height - y) * y) * w2) / rect.height;
		float fy = y + 0.5f;
		int w = (int)(math::sqrt((rect.height - fy) * fy) * w2) / rect.height;

		int left = (rect.width - w) / 2 + rect.x;
		if (left < 0) left = 0;
		int right = (rect.width + w + 1) / 2 + rect.x;
		if (right > width) right = width;

		right -= left;
		if (right > 0)
		{
			mempx::set(dest + left * PIXEL_SIZE, chr, right);
		}

		dest += pitch;
		y++;
	} while (y != y_end);
}

template <image::PixelFormat pf>
kr::image::MemBitmap<pf>::MemBitmap(nullptr_t) noexcept
{
	m_image = nullptr;
}
template <image::PixelFormat pf>
kr::image::MemBitmap<pf>::MemBitmap(int w, int h) noexcept
{
	constexpr int PIXEL_SIZE = Pixel<pf>::size;
	m_pitch = (m_width = w) * PIXEL_SIZE;
	m_height = h;
	m_image = _new byte[m_pitch*h];
}
template <image::PixelFormat pf>
kr::image::MemBitmap<pf>::~MemBitmap() noexcept
{
	delete[] (byte*)m_image;
}
template <image::PixelFormat pf>
void kr::image::MemBitmap<pf>::resize(int w, int h) noexcept
{
	if (m_image != nullptr)
	{
		if (w == m_width && h == m_height) return;
		delete[] (byte*)m_image;
	}

	constexpr int PIXEL_SIZE = Pixel<pf>::size;
	m_pitch = (m_width = w) * PIXEL_SIZE;
	m_height = h;
	m_image = _new byte[m_pitch*h];
}
template <image::PixelFormat pf>
void kr::image::MemBitmap<pf>::remove() noexcept
{
	delete[] (byte*)m_image;
	m_image = nullptr;
}
template<image::PixelFormat pf>
kr::image::ImageData kr::image::MemBitmap<pf>::getData() const noexcept
{
	ImageData img;
	img.attach(pf, m_image, m_pitch, m_width, m_height);
	return img;
}
template <image::PixelFormat pf>
bool kr::image::MemBitmap<pf>::operator ==(const MemBitmap &img) const noexcept
{
	return m_image == img.m_image;
}
template <image::PixelFormat pf>
bool kr::image::MemBitmap<pf>::operator !=(const MemBitmap &img) const noexcept
{
	return m_image != img.m_image;
}


#define WRAP(x)	x
#define DECL(format) \
template class kr::image::MemCanvas<PixelFormat##format>;\
template class kr::image::MemBitmap<PixelFormat##format>;
#include "formatlist.h"
