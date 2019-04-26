#include "reformatter.h"
#include "image.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d9types.h>
#include <dxgiformat.h>
#endif

#include <KR3/math/interpolation.h>

using namespace kr;

namespace
{
	template <image::PixelFormat dstf, image::PixelFormat srcf>
	struct Reformat
	{
		static void reformat(image::ImageData * dsti, const image::ImageData * srci) noexcept;
	};
	template <size_t pxsz>
	struct ReformatSame
	{
		static void reformat(image::ImageData * dsti, const image::ImageData * srci) noexcept;
	};
	template <image::PixelFormat pf>
	struct Reformat<pf, pf>
		:ReformatSame<sizeof(image::Pixel<pf>)>
	{
	};
	template <>
	struct Reformat<PixelFormatX1RGB5, PixelFormatA1RGB5> :
		ReformatSame<4>
	{
	};
	template <>
	struct Reformat<PixelFormatXRGB8, PixelFormatARGB8> :
		ReformatSame<4>
	{
	};
	template <>
	struct Reformat<PixelFormatXBGR8, PixelFormatABGR8> :
		ReformatSame<4>
	{
	};
	template <image::PixelFormat pf>
	struct PixelUtil
	{
		static void fill(image::ImageData * _image, color _color) noexcept;
		static void setAlpha(image::ImageData * _image, const image::ImageData * _alpha) noexcept;
		static void copyStretch(image::ImageData * _dest, const image::ImageData * _src) noexcept;
		static void imageProcessing(image::ImageData * _dest, const image::ImageData * _src, const float * weightTable, int _weightX, int _weightY, int _weightWidth, int _weightHeight) noexcept;
		static dword makeColor(color _color) noexcept;
		static void maskedRecolor(image::ImageData * _dest, color _color, const image::ImageData * _mask) noexcept;
		static void recolor(image::ImageData * _dest, color _color) noexcept;
	};
	
#ifdef WIN32
	template <int _d3dformat, int _dxgiformat> struct FormatInfoValue
	{
		static constexpr int d3dFormat = _d3dformat;
		static constexpr int dxgiFormat = _dxgiformat;
	};
	template <image::PixelFormat pf> struct FormatInfoEx;
	template <> struct FormatInfoEx<PixelFormatIndex> 
		:FormatInfoValue<D3DFMT_P8, DXGI_FORMAT_UNKNOWN> {};
	template <> struct FormatInfoEx<PixelFormatA8> 
		:FormatInfoValue<D3DFMT_A8, DXGI_FORMAT_UNKNOWN> {};
	template <> struct FormatInfoEx<PixelFormatR5G6B5> 
		:FormatInfoValue<D3DFMT_R5G6B5, DXGI_FORMAT_B5G6R5_UNORM > {};
	template <> struct FormatInfoEx<PixelFormatX1RGB5> 
		:FormatInfoValue<D3DFMT_X1R5G5B5, DXGI_FORMAT_UNKNOWN > {};
	template <> struct FormatInfoEx<PixelFormatA1RGB5> 
		:FormatInfoValue<D3DFMT_A1R5G5B5, DXGI_FORMAT_B5G5R5A1_UNORM > {};
	template <> struct FormatInfoEx<PixelFormatARGB4> 
		:FormatInfoValue<D3DFMT_A4R4G4B4, DXGI_FORMAT_UNKNOWN > {};
	template <> struct FormatInfoEx<PixelFormatRGB8> 
		:FormatInfoValue<D3DFMT_R8G8B8, DXGI_FORMAT_UNKNOWN > {};
	template <> struct FormatInfoEx<PixelFormatXRGB8> 
		:FormatInfoValue<D3DFMT_X8R8G8B8, DXGI_FORMAT_B8G8R8X8_UNORM > {};
	template <> struct FormatInfoEx<PixelFormatARGB8> 
		:FormatInfoValue<D3DFMT_A8R8G8B8, DXGI_FORMAT_B8G8R8A8_UNORM > {};
	template <> struct FormatInfoEx<PixelFormatBGR8> 
		:FormatInfoValue<D3DFMT_UNKNOWN, DXGI_FORMAT_UNKNOWN > {};
	template <> struct FormatInfoEx<PixelFormatXBGR8> 
		:FormatInfoValue<D3DFMT_X8B8G8R8, DXGI_FORMAT_UNKNOWN > {};
	template <> struct FormatInfoEx<PixelFormatABGR8> 
		:FormatInfoValue<D3DFMT_A8B8G8R8, DXGI_FORMAT_R8G8B8A8_UNORM > {};
#endif


#define WRAP(v)			const image::FormatInfo s_formatInfo[] = { v };
#define DECL_IN(dstf, srcf)	Reformat<PixelFormat##dstf, PixelFormat##srcf>::reformat

#ifdef WIN32
#define DECL_DX(pf) \
	FormatInfoEx<PixelFormat##pf>::d3dFormat, \
	FormatInfoEx<PixelFormat##pf>::dxgiFormat,
#else
#define DECL_DX(pf)
#endif
#define DECL(pf)	{ \
	image::Pixel<PixelFormat##pf>::size, \
	image::Pixel<PixelFormat##pf>::hasAlpha, \
DECL_DX(pf) \
{\
	DECL_IN(pf, Index), DECL_IN(pf, A8), DECL_IN(pf, R5G6B5), DECL_IN(pf, X1RGB5), DECL_IN(pf, A1RGB5), DECL_IN(pf, ARGB4),\
	DECL_IN(pf, RGB8), DECL_IN(pf, XRGB8), DECL_IN(pf, ARGB8),\
	DECL_IN(pf, BGR8), DECL_IN(pf, XBGR8), DECL_IN(pf, ABGR8)\
}, \
PixelUtil<PixelFormat##pf>::makeColor, \
PixelUtil<PixelFormat##pf>::copyStretch, \
PixelUtil<PixelFormat##pf>::imageProcessing, \
PixelUtil<PixelFormat##pf>::setAlpha, \
PixelUtil<PixelFormat##pf>::fill, \
PixelUtil<PixelFormat##pf>::maskedRecolor, \
PixelUtil<PixelFormat##pf>::recolor, \
},

#include "formatlist.h"
#undef DECL_IN
}

vec3 hsv_to_rgb(const vec3 & _hsv) noexcept;
vec3 rgb_to_hsv(const vec3 & _color) noexcept;

const kr::image::FormatInfo * kr::image::getFormatInfo(PixelFormat pf) noexcept
{
	_assert(pf != PixelFormatInvalid);
	return &s_formatInfo[(int)pf];
}

#ifdef WIN32
kr::image::PixelFormat kr::image::getFromD3D9Format(int format) noexcept
{
	switch (format)
	{
	case D3DFMT_R5G6B5: return PixelFormatR5G6B5;
	case D3DFMT_X1R5G5B5: return PixelFormatX1RGB5;
	case D3DFMT_A1R5G5B5: return PixelFormatA1RGB5;
	case D3DFMT_A4R4G4B4: return PixelFormatARGB4;
	case D3DFMT_R8G8B8: return PixelFormatRGB8;
	case D3DFMT_X8R8G8B8: return PixelFormatXRGB8;
	case D3DFMT_A8R8G8B8: return PixelFormatARGB8;
	case D3DFMT_X8B8G8R8: return PixelFormatXBGR8;
	case D3DFMT_A8B8G8R8: return PixelFormatABGR8;
	case D3DFMT_A32B32G32R32F: return PixelFormatRGBA32F;
	case D3DFMT_UNKNOWN:
	default:
		return PixelFormatInvalid;
	}
}
kr::image::PixelFormat kr::image::getFromDXGIFormat(int format) noexcept
{
	switch (format)
	{
	// case DXGI_FORMAT_P8: return PixelFormatIndex;
	case DXGI_FORMAT_B5G6R5_UNORM: return PixelFormatR5G6B5;
	case DXGI_FORMAT_B5G5R5A1_UNORM: return PixelFormatA1RGB5;
	// case DXGI_FORMAT_B4G4R4A4_UNORM: return PixelFormatARGB4;
	case DXGI_FORMAT_B8G8R8X8_UNORM: return PixelFormatXRGB8;
	case DXGI_FORMAT_B8G8R8A8_UNORM: return PixelFormatARGB8;
	case DXGI_FORMAT_R8G8B8A8_UNORM: return PixelFormatABGR8;
	case DXGI_FORMAT_UNKNOWN:
	default:
		return PixelFormatInvalid;
	}
}
#endif

void kr::image::reformat(ImageData * dest, const ImageData * src) noexcept
{
	s_formatInfo[(int)dest->getPixelFormat()].reformat[(int)src->getPixelFormat()](dest, src);
}
const kr::image::ImageData * kr::image::reformat(ImageData * buffer, const ImageData * src, PixelFormat format) noexcept
{
	if (src->getPixelFormat() == format)
	{
		*buffer = nullptr;
		return src;
	}
	else
	{
		buffer->allocate(format, src->getWidth(), src->getHeight(), src->getWidth() * image::getFormatInfo(format)->size);
		reformat(buffer, src);
		return buffer;
	}
}
const kr::image::ImageData * kr::image::reformat(ImageData * buffer, const ImageData * src, PixelFormat format, int pitchBytes) noexcept
{
	if (src->getPixelFormat() == format && src->getPitch() == pitchBytes)
	{
		*buffer = nullptr;
		return src;
	}
	else
	{
		buffer->allocate(format, src->getWidth(), src->getHeight(), pitchBytes);
		reformat(buffer, src);
		return buffer;
	}
}

template <image::PixelFormat dstf, image::PixelFormat srcf>
void Reformat<dstf, srcf>::reformat(image::ImageData * dsti, const image::ImageData * srci) noexcept
{
	using srcpx = image::Pixel<srcf>;
	using dstpx = image::Pixel<dstf>;

	size_t width = tmin(srci->getWidth(), dsti->getWidth());
	size_t height = tmin(srci->getHeight(), dsti->getHeight());

	size_t dstpitch = dsti->getPitch();

	byte* dst = (byte*)dsti->getBits();
	byte* end = dst + dstpitch * height;

	size_t srcpitch = srci->getPitch();
	srcpitch = srcpitch - width * sizeof(srcpx);

	byte* src = (byte*)srci->getBits();
	width *= sizeof(dstpx);
	dstpitch -= width;
	while (dst != end)
	{
		byte* destto = dst + width;
		while (dst != destto)
		{
			((dstpx*)dst)->set(*(srcpx*)src);
			dst += sizeof(dstpx);
			src += sizeof(srcpx);
		}
		src += srcpitch;
		dst += dstpitch;
	}
}
template <size_t pxsz>
void ReformatSame<pxsz>::reformat(image::ImageData * dsti, const image::ImageData * srci) noexcept
{
	size_t width = tmin(srci->getWidth(), dsti->getWidth());
	size_t height = tmin(srci->getHeight(), dsti->getHeight());

	size_t dstpitch = dsti->getPitch();

	byte* dst = (byte*)dsti->getBits();
	byte* end = dst + dstpitch * height;

	size_t srcpitch = srci->getPitch();

	byte* src = (byte*)srci->getBits();
	width *= pxsz;
	while (dst != end)
	{
		memcpy(dst, src, width);
		src += srcpitch;
		dst += dstpitch;
	}
}

template <image::PixelFormat pf>
void PixelUtil<pf>::fill(image::ImageData * _image, color _color) noexcept
{
	using px = image::Pixel<pf>;

	size_t width = _image->getWidth();
	size_t height = _image->getHeight();

	size_t pitch = _image->getPitch();

	px src;
	src.set((image::Pixel<PixelFormatARGB8>&)_color);

	byte* dst = (byte*)_image->getBits();
	byte* end = dst + pitch * height;

	width *= sizeof(px);
	pitch -= width;
	while (dst != end)
	{
		byte* destto = dst + width;
		while (dst != destto)
		{
			*(px*)dst = src;
			dst += sizeof(px);
		}
		dst += pitch;
	}
}
template <image::PixelFormat pf>
void PixelUtil<pf>::setAlpha(image::ImageData * _image, const image::ImageData * _alpha) noexcept
{
	using kr::byte;
	using dstpx = image::Pixel<pf>;
	_assert(_alpha->getPixelFormat() == PixelFormatA8); // setAlpha시 포맷이 같아야한다
	_assert(_image->getWidth() == _alpha->getWidth()); // setAlpha시 이미지 크기가 같아야한다
	_assert(_image->getHeight() == _alpha->getHeight());

	uint width = _image->getWidth();
	uint height = _image->getHeight();
	uint dstpitch = _image->getPitch();
	uint srcpitch = _alpha->getPitch();

	byte* dst = (byte*)_image->getBits();
	byte* end = dst + dstpitch * height;

	image::Pixel<PixelFormatA8> * alpha = (image::Pixel<PixelFormatA8>*)_alpha->getBits();

	srcpitch -= width * alpha->size;
	width *= sizeof(dstpx);
	dstpitch -= width;

	while (dst != end)
	{
		byte* destto = dst + width;
		while (dst != destto)
		{
			((dstpx*)dst)->setAlpha(alpha->rawAlpha());
			dst += sizeof(dstpx);
			alpha ++;
		}
		dst += dstpitch;
		(byte*&)alpha += srcpitch;
	}
}
template <image::PixelFormat pf>
void PixelUtil<pf>::copyStretch(image::ImageData * _dest, const image::ImageData * _src) noexcept
{
	using px = image::Pixel<pf>;
	int w = _src->getWidth();
	int h = _src->getHeight();
	int nw = _dest->getWidth();
	int nh = _dest->getHeight();
	if (w == nw && h == nh)
	{
		ReformatSame<sizeof(px)>::reformat(_dest, _src);
		return;
	}

	byte * src = (byte*)_src->getBits();
	byte * dest = (byte*)_dest->getBits();

	size_t srcpitch = _src->getPitch();
	size_t dstpitch = _dest->getPitch() - nw * sizeof(px);

	float xrate = (float)w / nw;
	float yrate = (float)h / nh;

	for (int y = 0; y < nh; y++)
	{
		float sy = y * yrate;
		int isy1 = (int)sy;
		sy -= isy1;
		float isy = 1.f - sy;
		int isy2 = (isy1 + 1) % h;

		byte * srcline1 = src + isy1 * srcpitch;
		byte * srcline2 = src + isy2 * srcpitch;

		for (int x = 0; x < nw; x++)
		{
			float sx = x * xrate;
			int isx1 = (int)sx;
			sx -= isx1;
			float isx = 1.f - sx;
			int isx2 = (isx1 + 1) % w;

			isx1 *= sizeof(px);
			isx2 *= sizeof(px);

			vec4a color = ((px*)(srcline1 + isx1))->getf() * (isy * isx) +
				((px*)(srcline1 + isx2))->getf() * (isy * sx) +
				((px*)(srcline2 + isx1))->getf() * (sy * isx) +
				((px*)(srcline2 + isx2))->getf() * (sy * sx);
			((px*)dest)->set(color);
			
			dest += sizeof(px);
		}
		dest += dstpitch;
	}
}
template <image::PixelFormat pf>
void PixelUtil<pf>::imageProcessing(image::ImageData * _dest, const image::ImageData * _src, const float * weightTable, int _weightX, int _weightY, int _weightWidth, int _weightHeight) noexcept
{
	_assert(_dest != nullptr);
	_assert(_src != nullptr);
	_assert(_dest->getPixelFormat() == _src->getPixelFormat());

	using px = image::Pixel<pf>;

	float weightSum = 0.f;

	{
		const float * end = weightTable + _weightWidth * _weightHeight;
		const float * iter = weightTable;
		while (iter != end)
		{
			weightSum += *iter++;
		}
	}

	weightSum = 1.f / weightSum;

	size_t dstpitch = _dest->getPitch();

	int srcWidth = _src->getWidth();
	int srcHeight = _src->getHeight();
	int destWidth = _dest->getWidth();
	int destHeight = _dest->getHeight();

	byte* dst = (byte*)_dest->getBits();
	//byte* end = dst + dstpitch * destHeight;

	size_t srcpitch = _src->getPitch();

	byte* src = (byte*)_src->getBits();
	int destWidthBytes = destWidth * sizeof(px);
	dstpitch -= destWidthBytes;


	int weightRight = _weightX + _weightWidth;
	int weightBottom = _weightY + _weightHeight;

	_weightX += srcWidth;
	_weightY += srcHeight;

	for (int y = 0; y < destHeight; y++)
	{
		//byte* destto = dst + destWidthBytes;
		int sy = y * srcHeight / destHeight + _weightY;

		for (int x = 0; x < destWidth; x++)
		{
			vec4a color = vec4a::makez();
			int sx = x * srcWidth / destWidth + _weightX;

			const float * weightRead = weightTable;
			for (int y1 = 0; y1 < _weightHeight; y1++)
			{
				byte* readLine = ((uint)(sy + y1) % (uint)srcHeight) * srcpitch + src;

				for (int x1 = 0; x1 < _weightWidth; x1++)
				{
					int readx = (uint)(sx + x1) % (uint)srcWidth;

					vec4a newcolor = ((px*)readLine + readx)->getf();
					color += newcolor * (*weightRead++);
				}
			}

			((px*)dst)->set(color * weightSum);
			dst += sizeof(px);
		}
		dst += dstpitch;
	}
}
template<image::PixelFormat pf>
dword PixelUtil<pf>::makeColor(color _color) noexcept
{
	dword ret = 0;
	((image::Pixel<pf>&)ret).set((image::Pixel<PixelFormatARGB8>&)_color);
	return ret;
}
template <image::PixelFormat pf>
void PixelUtil<pf>::maskedRecolor(image::ImageData * _dest, color _color, const image::ImageData * _mask) noexcept
{
	using kr::byte;
	using dstpx = image::Pixel<pf>;
	_assert(_mask->getPixelFormat() == PixelFormatA8); // setAlpha시 포맷이 같아야한다
	_assert(_dest->getWidth() == _mask->getWidth()); // recolor시 이미지 크기가 같아야한다
	_assert(_dest->getHeight() == _mask->getHeight());

	uint width = _dest->getWidth();
	uint height = _dest->getHeight();
	uint dstpitch = _dest->getPitch();
	uint srcpitch = _mask->getPitch();
	
	byte * dst = (byte*)_dest->getBits();
	byte* end = dst + dstpitch * height;
	vec4a ncolor = (vec4a)_color / 255.f;
	vec3 newhsv = rgb_to_hsv((vec3&)ncolor);

	image::Pixel<PixelFormatA8> * src = (image::Pixel<PixelFormatA8>*)_mask->getBits();

	srcpitch -= width * src->size;
	width *= sizeof(dstpx);
	dstpitch -= width;

	while (dst != end)
	{
		byte* destto = dst + width;
		while (dst != destto)
		{
			vec4a c = ((image::Pixel<pf>*)dst)->getf();
			vec4a newc;
			(vec3&)newc = rgb_to_hsv((vec3&)c);
			newc.x = newhsv.x;
			newc.y *= newhsv.y;
			newc.z *= newhsv.z;
			(vec3&)newc = hsv_to_rgb((vec3&)newc);
			newc.a = c.a;

			float mask = src->alphaf() * ncolor.a;
			((image::Pixel<pf>*)dst)->set(lerp(c, newc, mask));

			dst += sizeof(dstpx);
			src++;
		}
		dst += dstpitch;
		(byte*&)src += srcpitch;
	}
}
template <image::PixelFormat pf>
void PixelUtil<pf>::recolor(image::ImageData * _dest, color _color) noexcept
{
	using kr::byte;
	using dstpx = image::Pixel<pf>;

	uint width = _dest->getWidth();
	uint height = _dest->getHeight();
	uint dstpitch = _dest->getPitch();

	byte * dst = (byte*)_dest->getBits();
	byte* end = dst + dstpitch * height;
	vec4a ncolor = (vec4a)_color / 255.f;
	vec3 newhsv = rgb_to_hsv((vec3&)ncolor);

	width *= sizeof(dstpx);
	dstpitch -= width;

	while (dst != end)
	{
		byte* destto = dst + width;
		while (dst != destto)
		{
			vec4a c = ((image::Pixel<pf>*)dst)->getf();
			vec4a newc;
			(vec3&)newc = rgb_to_hsv((vec3&)c);
			newc.x = newhsv.x;
			newc.y *= newhsv.y;
			newc.z *= newhsv.z;
			(vec3&)newc = hsv_to_rgb((vec3&)newc);
			newc.a = c.a;

			((image::Pixel<pf>*)dst)->set(lerp(c, newc, ncolor.a));

			dst += sizeof(dstpx);
		}
		dst += dstpitch;
	}
}

vec3 hsv_to_rgb(const vec3 & _hsv) noexcept
{
	float h = _hsv.x;
	float s = _hsv.y;
	float v = _hsv.z;

	vec3 out;
	float f, x, y, z;
	int i;

	if (s == 0.0) {
		out.x = out.y = out.z = v;
		return out;
	}
	else {
		while (h < 0)
			h += 360;
		h = fmod(h, 360.f) / 60.f;
		i = (int)h;
		f = h - i;
		x = v * (1.f - s);
		y = v * (1.f - (s * f));
		z = v * (1.f - (s * (1.f - f)));

		switch (i) {
		case 0: out.x = v; out.y = z; out.z = x; break;
		case 1: out.x = y; out.y = v; out.z = x; break;
		case 2: out.x = x; out.y = v; out.z = z; break;
		case 3: out.x = x; out.y = y; out.z = v; break;
		case 4: out.x = z; out.y = x; out.z = v; break;
		case 5: out.x = v; out.y = x; out.z = y; break;
		}
		return out;
	}
}
vec3 rgb_to_hsv(const vec3 & _color) noexcept
{
	float rc = _color.r;
	float gc = _color.g;
	float bc = _color.b;
	float maxv = tmax(rc, gc, bc);
	float minv = tmin(rc, gc, bc);
	float delta = maxv - minv;

	vec3 out;
	out.z = maxv;

	if (maxv != 0.0)
		out.y = delta / maxv;
	else
		out.y = 0.0;

	if (out.y == 0.0) {
		out.x = 0.0;
	}
	else {
		if (rc == maxv)
			out.x = (gc - bc) / delta;
		else if (gc == maxv)
			out.x = 2 + (bc - rc) / delta;
		else if (bc == maxv)
			out.x = 4 + (rc - gc) / delta;

		out.x *= 60.0;
		if (out.x < 0)
			out.x += 360.0;
	}
	return out;
}