#pragma once

#include <KR3/main.h>
#include <KRUtil/resloader.h>
#include <KRUtil/wl/com.h>

struct IMFMediaSource;
struct IMFSourceReader;
struct IMFSample;
struct IMFMediaBuffer;
struct IPropertyStore;
typedef struct _tagpropertykey PROPERTYKEY;

namespace kr
{
	using MFMediaSource = Com<IMFMediaSource>;
	using MFSourceReader = Com<IMFSourceReader>;
	using MFMediaBuffer = Com<IMFMediaBuffer>;
	using PropertyStore = Com<IPropertyStore>;

	COM_CLASS_DECLARE(IMFSourceReader);
	COM_CLASS_DECLARE(IMFMediaBuffer);
	COM_CLASS_DECLARE(IMFMediaSource);
	COM_CLASS_DECLARE(IPropertyStore);

	COM_CLASS_METHODS(IMFMediaBuffer)
	{
		void lock(Buffer * data) throws(ErrorCode);
		void unlock() noexcept;
	};

	COM_CLASS_METHODS(IMFMediaSource)
	{
		static MFMediaSource load(pcstr16 path) throws(ErrorCode);

		MFSourceReader getReader(WaveFormat * waveFormat) throws(ErrorCode);
		PropertyStore getProperty() throws(ErrorCode);

		struct Init
		{
			Init() noexcept;
			~Init() noexcept;
		};
	};

	COM_CLASS_METHODS(IMFSourceReader)
	{
		MFMediaBuffer read() throws(ErrorCode);
	};

	COM_CLASS_METHODS(IPropertyStore)
	{
		AText16 get(const PROPERTYKEY & key) noexcept;
		AText16 getAuthor() noexcept;
		AText16 getTitle() noexcept;
	};

	class MFMediaStream
	{
	public:
		MFMediaStream(MFSourceReader reader) noexcept;
		MFMediaStream(const MFMediaStream&) = delete;
		~MFMediaStream() noexcept;

		bool next(Buffer* data) throws(ErrorCode);

	private:
		MFSourceReader m_reader;
		MFMediaBuffer m_buffer;
	};
}