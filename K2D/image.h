#pragma once

#include <KR3/main.h>
#include <KRUtil/resloader.h>

#include "common.h"
#include "reformatter.h"

class kr::image::ImageData
{
public:
	ImageData() noexcept = default;
	ImageData(nullptr_t) noexcept;

	ImageData & operator = (nullptr_t) noexcept;

	void * allocate(PixelFormat pf, int w, int h) noexcept;
	void * allocate(PixelFormat pf, int w, int h, int pitchBytes) noexcept;
	void allocate(ImageData * _img, int _x, int _y, int _w, int _h) noexcept;
	void allocate(ImageData * _src, PixelFormat _pf) noexcept;
	void copyBits(const void * data) noexcept;
	void copyBitsReverseX(const void * data) noexcept;
	void copyBitsReverseY(const void * data) noexcept;
	void copyBitsReverseXY(const void * data) noexcept;
	void copyBits(const void * data, int _srcPitchBytes) noexcept;
	void copyBitsReverseX(const void * data, int _srcPitchBytes) noexcept;
	void copyBitsReverseY(const void * data, int _srcPitchBytes) noexcept;
	void copyBitsReverseXY(const void * data, int _srcPitchBytes) noexcept;
	ImageData subimage(int _x, int _y, int _w, int _h) const noexcept;
	void copy(const ImageData * _img, int _dstX, int _dstY, int _srcX, int _srcY, int _srcW, int _srcH) noexcept;
	void copyStretch(const ImageData * _img) noexcept;

	// _src: 픽셀 포멧이 this와 같아야 한다.
	void imageProcessing(const ImageData * _src, const float * _weightTable, int _weightX, int _weightY, int _weightWidth, int _weightHeight) noexcept;

	// _src: 픽셀 포멧이 this와 같아야 한다.
	void blur(const ImageData * _src) noexcept;

	// _alpha: A8 포멧의 이미지, 크기가 같아야한다.
	void setAlpha(const ImageData * _alpha) noexcept;

	void fill(color _color) noexcept;

	// _alpha: A8 포멧의 이미지, 크기가 같아야한다.
	void recolor(color _color, const ImageData * _mask) noexcept;

	void recolor(color _color) noexcept;

	void free() noexcept;
	bool exists() const noexcept;
	int getWidth() const noexcept;
	int getHeight() const noexcept;
	int getPitch() const noexcept;
	void attach(PixelFormat pf, ptr nimg, int p, int w, int h) noexcept;
	ptr getBits() noexcept;
	cptr getBits() const noexcept;
	PixelFormat getPixelFormat() const noexcept;
	const FormatInfo * getPixelInfo() const noexcept;
	int getPixelSize() const noexcept;
	size_t getByteSize() const noexcept;

	bool load(krb::File file, krb::Extension extension, Palette * palette) noexcept;
	bool save(krb::File file, krb::Extension extension, Palette * palette) noexcept;

	static bvec4 color;

protected:
	ptr m_image;
	PixelFormat m_pf;
	int m_pitch;
	int m_width, m_height;
};