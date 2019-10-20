#pragma once

#include <KR3/main.h>
#include <KR3/wl/com.h>

struct IWICBitmapDecoder;
struct IWICBitmapSource;
struct IWICFormatConverter;

namespace kr
{
	COM_INHERIT(IWICFormatConverter, IWICBitmapSource);

	using WICBitmapDecoder = Com<IWICBitmapDecoder>;

	COM_CLASS_METHODS(IWICBitmapDecoder)
	{
		struct Init
		{
			Init() noexcept;
			~Init() noexcept;
		};
		ComMethod() noexcept;
		ComMethod(pcstr16 filename) throws(ErrorCode);
		void create(pcstr16 filename) throws(ErrorCode);
		Com<IWICBitmapSource> getFrame(uint index) throws(ErrorCode);
		uint getFrameCount() throws(ErrorCode);
	};

}