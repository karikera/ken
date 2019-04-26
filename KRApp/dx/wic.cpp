#ifdef WIN32
#include "wic.h"

#include <wincodec.h>

using namespace kr;

namespace
{
	Com<IWICImagingFactory> s_wicFactory;
}

ComMethod<IWICBitmapDecoder>::Init::Init() noexcept
{
	// wic factory
	hrmustbe(CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory,
		&s_wicFactory
	));
}
ComMethod<IWICBitmapDecoder>::Init::~Init() noexcept
{
	s_wicFactory = nullptr;
}

ComMethod<IWICBitmapDecoder>::ComMethod() noexcept
{
}
ComMethod<IWICBitmapDecoder>::ComMethod(pcstr16 filename) throws(ErrorCode)
{
	create(filename);
}
void ComMethod<IWICBitmapDecoder>::create(pcstr16 filename) throws(ErrorCode)
{
	hrexcept(s_wicFactory->CreateDecoderFromFilename(
		wide(filename),						// Image to be decoded
		NULL,								// Do not prefer a particular vendor
		GENERIC_READ,						// Desired read access to the file
		WICDecodeMetadataCacheOnDemand,		// Cache metadata when needed
		&ptr()							// Pointer to the decoder
	));
}
Com<IWICBitmapSource> ComMethod<IWICBitmapDecoder>::getFrame(uint index) throws(ErrorCode)
{
	Com<IWICFormatConverter> converter;
	hrexcept(s_wicFactory->CreateFormatConverter(&converter));

	{
		Com<IWICBitmapFrameDecode> frame;
		hrexcept(ptr()->GetFrame(0, &frame));

		hrexcept(converter->Initialize(
			frame,							// Input bitmap to convert
			GUID_WICPixelFormat32bppPBGRA,  // Destination pixel format
			WICBitmapDitherTypeNone,        // Specified dither pattern
			NULL,                           // Specify a particular palette 
			0.f,                            // Alpha threshold
			WICBitmapPaletteTypeCustom      // Palette translation type
		));
	}

	return converter;
}
uint ComMethod<IWICBitmapDecoder>::getFrameCount() throws(ErrorCode)
{
	UINT frameCount;
	hrexcept(ptr()->GetFrameCount(&frameCount));
	return frameCount;
}

#endif