#pragma once

#include <KR3/main.h>
#include <KR3/util/resloader.h>
#include <KR3/wl/com.h>

struct IMFSourceReader;
struct IMFSample;
struct IMFMediaBuffer;

namespace kr
{
	using MFSourceReader = Com<IMFSourceReader>;
	using MFMediaBuffer = Com<IMFMediaBuffer>;

	COM_CLASS_DECLARE(IMFSourceReader);
	COM_CLASS_DECLARE(IMFMediaBuffer);

	COM_CLASS_METHODS(IMFMediaBuffer)
	{
		void lock(Buffer * data) throws(ErrorCode);
		void unlock() noexcept;
	};

	COM_CLASS_METHODS(IMFSourceReader)
	{
		static MFSourceReader load(pcstr16 path, WaveFormat * waveFormat) throws(ErrorCode);

		MFMediaBuffer read() throws(ErrorCode);

		struct Init
		{
			Init() noexcept;
			~Init() noexcept;
		};
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