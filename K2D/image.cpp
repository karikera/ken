#include "image.h"
#include "memory.h"

#include <KR3/main.h>
#include <KR3/util/wide.h>
#include <KR3/util/resloader.h>

#include "stdcommon.h"
#include "bitmap.h"

namespace
{
	/*-----------------------------------------------------------------------------
	- for TGA reverse

	`width는 dest, src 동일해야 한다.
	-----------------------------------------------------------------------------*/
	void copyLineReverse(kr::byte *dest, const kr::byte *src, int width, int pixel_byte) noexcept
	{
		using namespace kr;

		int i;
		int last_element;

		switch (pixel_byte)
		{
		case 1:
			last_element = width - 1;
			for (i = 0; i < width; i++)
				dest[last_element - i] = src[i];
			break;
		case 2:
			last_element = width - 1;
			for (i = 0; i < width; i++)
				((word*)dest)[last_element - i] = ((word *)src)[i];
			break;
		case 3:
			last_element = width * 3 - 1;
			for (i = 0; i < width; i++)
			{
				int index = last_element - (i * 3);
				((byte*)dest)[index - 2] = ((byte *)src)[i * 3 + 0];
				((byte*)dest)[index - 1] = ((byte *)src)[i * 3 + 1];
				((byte*)dest)[index - 0] = ((byte *)src)[i * 3 + 2];
			}
			break;
		case 4:
			last_element = width - 1;
			for (i = 0; i < width; i++)
				((dword*)dest)[last_element - i] = ((dword *)src)[i];
			break;
		}
	}

}

kr::bvec4 kr::image::ImageData::color(0, 0, 0, 0);


kr::image::ImageData::ImageData(nullptr_t) noexcept
{
	m_image = nullptr;
}

kr::image::ImageData & kr::image::ImageData::operator = (nullptr_t) noexcept
{
	m_image = nullptr;
	return *this;
}

void * kr::image::ImageData::allocate(PixelFormat pf, int w, int h) noexcept
{
	return allocate(pf, w, h, w * kr::image::getFormatInfo(pf)->size);
}
void * kr::image::ImageData::allocate(PixelFormat pf, int w, int h, int pitchBytes) noexcept
{
	_assert(pitchBytes >= w * kr::image::getFormatInfo(pf)->size);
	m_pf = pf;
	m_pitch = pitchBytes;
	m_width = w;
	m_height = h;
	return m_image = _new byte[pitchBytes * h];
}
void kr::image::ImageData::allocate(ImageData * _img, int _x, int _y, int _w, int _h) noexcept
{
	PixelFormat pf = _img->getPixelFormat();
	allocate(pf, _w, _h);
	copy(_img, 0, 0, _x, _y, _w, _h);
}
void kr::image::ImageData::allocate(ImageData * _src, PixelFormat _pf) noexcept
{
	int w = _src->getWidth();
	int h = _src->getHeight();
	allocate(_pf, w, h);
	reformat(this, _src);
}
void kr::image::ImageData::copyBits(const void * data) noexcept
{
	return copyBits(data, m_width * getPixelSize());
}
void kr::image::ImageData::copyBitsReverseX(const void * data) noexcept
{
	return copyBitsReverseX(data, m_width * getPixelSize());
}
void kr::image::ImageData::copyBitsReverseY(const void * data) noexcept
{
	return copyBitsReverseY(data, m_width * getPixelSize());
}
void kr::image::ImageData::copyBitsReverseXY(const void * data) noexcept
{
	return copyBitsReverseXY(data, m_width * getPixelSize());
}
void kr::image::ImageData::copyBits(const void * data, int _srcPitchBytes) noexcept
{
	byte * src = (byte*)data;
	byte * dest = (byte*)m_image;
	size_t lineBytes = m_pitch;
	for (int h = 0; h < m_height; h++)
	{
		memcpy(dest, src, m_width);
		dest += lineBytes;
		src += _srcPitchBytes;
	}
}
void kr::image::ImageData::copyBitsReverseX(const void * data, int _srcPitchBytes) noexcept
{
	byte * src = (byte*)data;
	byte * dest = (byte*)m_image;
	size_t lineBytes = m_pitch;
	int width = m_width;
	int pixelBytes = getPixelSize();
	for (int h = 0; h < m_height; h++)
	{
		copyLineReverse(dest, src, width, pixelBytes);
		dest += lineBytes;
		src += _srcPitchBytes;
	}
}
void kr::image::ImageData::copyBitsReverseY(const void * data, int _srcPitchBytes) noexcept
{
	byte * src = (byte*)data;
	byte * dest = (byte*)m_image;
	size_t lineBytes = m_pitch;
	dest += (m_height - 1) * lineBytes;
	for (int h = 0; h < m_height; h++)
	{
		memcpy(dest, src, lineBytes);
		dest -= lineBytes;
		src += _srcPitchBytes;
	}
}
void kr::image::ImageData::copyBitsReverseXY(const void * data, int _srcPitchBytes) noexcept
{
	byte * src = (byte*)data;
	byte * dest = (byte*)m_image;
	size_t lineBytes = m_pitch;
	int width = m_width;
	int pixelBytes = getPixelSize();
	dest += (m_height - 1) * lineBytes;
	for (int h = 0; h < m_height; h++)
	{
		copyLineReverse(dest, src, width, pixelBytes);
		dest -= lineBytes;
		src += _srcPitchBytes;
	}
}
kr::image::ImageData kr::image::ImageData::subimage(int _x, int _y, int _w, int _h) const noexcept
{
	_assert(_x >= 0);
	_assert(_y >= 0);
	_assert(_w >= 0);
	_assert(_h >= 0);
	_assert(_w + _x <= m_width);
	_assert(_h + _y <= m_height);
	ImageData subimg;
	subimg.m_width = _w;
	subimg.m_height = _h;
	subimg.m_pitch = m_pitch;
	subimg.m_image = (byte*)m_image + _y * m_pitch + _x * getPixelSize();
	subimg.m_pf = m_pf;
	return subimg;
}
void kr::image::ImageData::copy(const ImageData * _img, int _dstX, int _dstY, int _srcX, int _srcY, int _srcW, int _srcH) noexcept
{
	{
		int minX = mint(_dstX, _srcX);
		if (minX < 0)
		{
			_srcW += minX;
			_srcX -= minX;
			_dstX -= minX;
		}
		int minY = mint(_dstX, _srcX);
		if (minY < 0)
		{
			_srcH += minY;
			_srcY -= minY;
			_dstY -= minY;
		}
		int _srcWMax = mint(_img->getWidth() - _srcX, getWidth() - _dstX);
		if (_srcWMax < 0)
			return;
		if (_srcW > _srcWMax)
			_srcW = _srcWMax;
		int _srcHMax = mint(_img->getHeight() - _srcY, getHeight() - _dstY);
		if (_srcHMax < 0)
			return;
		if (_srcH > _srcHMax)
			_srcH = _srcHMax;
	}

	ImageData src = _img->subimage(_srcX, _srcY, _srcW, _srcH);
	ImageData dst = subimage(_dstX, _dstY, _srcW, _srcH);
	reformat(&dst, &src);
}
void kr::image::ImageData::copyStretch(const ImageData * _img) noexcept
{
	_assert(getPixelFormat() == _img->getPixelFormat());
	getPixelInfo()->copyStretch(this, _img);
}
void kr::image::ImageData::imageProcessing(const ImageData * _src, const float * _weightTable, int _weightX, int _weightY, int _weightWidth, int _weightHeight) noexcept
{
	_assert(getPixelFormat() == _src->getPixelFormat());
	getPixelInfo()->imageProcessing(this, _src, _weightTable, _weightX, _weightY, _weightWidth, _weightHeight);
}
void kr::image::ImageData::blur(const ImageData * _src) noexcept
{
	const int nHalfLen = 1;
	float fW[3];
	fW[0] = 1.0f;
	fW[1] = math::pi / 6.0f + sqrtf(3.f) / 4.0f;
	fW[2] = (math::pi - 1.0f) / 4.0f - fW[1];
	fW[0] *= fW[0];
	fW[1] *= fW[1];
	fW[2] *= fW[2];
	const float weightTable[] =
	{
		fW[2], fW[1], fW[2],
		fW[1], fW[0], fW[1],
		fW[2], fW[1], fW[2]
	};
	return imageProcessing(_src, weightTable, -1, -1, 3, 3);
}
void kr::image::ImageData::setAlpha(const ImageData * _alpha) noexcept
{
	getPixelInfo()->setAlpha(this, _alpha);
}
void kr::image::ImageData::fill(kr::color _color) noexcept
{
	getPixelInfo()->fill(this, _color);
}
void kr::image::ImageData::recolor(kr::color _color, const ImageData * _mask) noexcept
{
	getPixelInfo()->maskedRecolor(this, _color, _mask);
}
void kr::image::ImageData::recolor(kr::color _color) noexcept
{
	getPixelInfo()->recolor(this, _color);
}
void kr::image::ImageData::free() noexcept
{
	delete[] (byte*)m_image;
}
bool kr::image::ImageData::exists() const noexcept
{
	return m_image != nullptr;
}
int kr::image::ImageData::getWidth() const noexcept
{
	return m_width;
}
int kr::image::ImageData::getHeight() const noexcept
{
	return m_height;
}
int kr::image::ImageData::getPitch() const noexcept
{
	return m_pitch;
}
void kr::image::ImageData::attach(PixelFormat pf, ptr nimg, int p, int w, int h) noexcept
{
	m_pf = pf;
	m_image = nimg;
	m_pitch = p;
	m_width = w;
	m_height = h;
}
kr::ptr kr::image::ImageData::getBits() noexcept
{
	return m_image;
}
kr::cptr kr::image::ImageData::getBits() const noexcept
{
	return m_image;
}
kr::image::PixelFormat kr::image::ImageData::getPixelFormat() const noexcept
{
	return m_pf;
}
const kr::image::FormatInfo * kr::image::ImageData::getPixelInfo() const noexcept
{
	return getFormatInfo(m_pf);
}

int kr::image::ImageData::getPixelSize() const noexcept
{
	return kr::image::getFormatInfo(m_pf)->size;
}
size_t kr::image::ImageData::getByteSize() const noexcept
{
	return m_pitch * m_height;
}


bool kr::image::ImageData::load(krb::File file, krb::Extension extension, Palette* palette) noexcept
{
	struct ImageCallback : KrbImageCallback
	{
		ImageData * data;
	};
	ImageCallback cb;
	cb.data = this;
	cb.palette = palette != nullptr ? palette : nullptr;
	cb.start = [](KrbImageCallback * _this, KrbImageInfo * info){
		return ((ImageCallback*)_this)->data->allocate(info->pixelformat, info->width, info->height, info->pitchBytes);
	};
	return krb_load_image(extension, &cb, &file);
}
bool kr::image::ImageData::save(krb::File file, krb::Extension extension, Palette * palette) noexcept
{
	KrbImageSaveInfo info;
	info.data = m_image;
	info.width = m_width;
	info.height = m_height;
	info.pitchBytes = m_pitch;
	info.pixelformat = m_pf;
	info.jpegQuality = 100;
	info.tgaCompress = true;
	info.palette = palette;
	return krb_save_image(extension, &info, &file);
}
