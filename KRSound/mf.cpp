#include "stdafx.h"
#include "mf.h"

#include <mfapi.h>
#include <mfobjects.h>
#include <mfidl.h>
#include <mferror.h>
#include <mfreadwrite.h>

#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "Mfreadwrite.lib")

using namespace kr;

namespace
{
	IMFAttributes * s_sourceReaderConfiguration;
	const DWORD STREAM_INDEX = (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM;
}

void ComMethod<IMFMediaBuffer>::lock(Buffer* data) throws(ErrorCode)
{
	DWORD size;
	BYTE* locked;
	// lock buffer and copy data to local memory

	HRESULT hr = ptr()->Lock(&locked, nullptr, &size);
	if (FAILED(hr))
	{
		throw ErrorCode(hr);
	}
	*data = Buffer(locked, size);
}
void ComMethod<IMFMediaBuffer>::unlock() noexcept
{
	ptr()->Unlock();
}

ComMethod<IMFSourceReader>::Init::Init() noexcept
{
	HRESULT hr = S_OK;

	// initialize media foundation
	hr = MFStartup(MF_VERSION);
	if (FAILED(hr)) error("Critical error: Unable to start the Windows Media Foundation!");

	// set media foundation reader to low latency
	hr = MFCreateAttributes(&s_sourceReaderConfiguration, 1);
	if (FAILED(hr)) error("Critical error: Unable to create Media Foundation Source Reader configuration!");
		
	// hr = s_sourceReaderConfiguration->SetUINT32(MF_LOW_LATENCY, true);
	// if (FAILED(hr)) error("Critical error: Unable to set Windows Media Foundation configuration!");
}
ComMethod<IMFSourceReader>::Init::~Init() noexcept
{
	if (s_sourceReaderConfiguration)
	{
		s_sourceReaderConfiguration->Release();
		s_sourceReaderConfiguration = nullptr;
	}
}

MFSourceReader ComMethod<IMFSourceReader>::load(pcstr16 path, WaveFormat* waveFormat) throws(ErrorCode)
{
	// handle errors
	HRESULT hr = S_OK;
	
	MFSourceReader sourceReader;

	// create the source reader
	hrexcept(MFCreateSourceReaderFromURL(wide(path), s_sourceReaderConfiguration, &sourceReader));

	// select the first audio stream, and deselect all other streams
	hrexcept(sourceReader->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, false));

	hrexcept(sourceReader->SetStreamSelection(STREAM_INDEX, true));

	// query information about the media file
	Com<IMFMediaType> nativeMediaType;
	hrexcept(sourceReader->GetNativeMediaType(STREAM_INDEX, 0, &nativeMediaType));

	// make sure that this is really an audio file
	GUID majorType{};
	hrexcept(nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType));
	if (majorType != MFMediaType_Audio) throw ErrorCode(ERROR_UNSUPPORTED_TYPE);

	// check whether the audio file is compressed or uncompressed
	GUID subType{};
	hr = nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &subType);
	if (subType == MFAudioFormat_Float || subType == MFAudioFormat_PCM)
	{
		// the audio file is uncompressed
	}
	else
	{
		// the audio file is compressed; we have to decompress it first
		// to do so, we inform the SourceReader that we want uncompressed data
		// this causes the SourceReader to look for decoders to perform our request
		Com<IMFMediaType> partialType = nullptr;
		hrexcept(MFCreateMediaType(&partialType));

		// set the media type to "audio"
		hrexcept(partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));

		// request uncompressed data
		hrexcept(partialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));

		hrexcept(sourceReader->SetCurrentMediaType(STREAM_INDEX, nullptr, partialType));
	}

	// uncompress the data and load it into an XAudio2 Buffer
	Com<IMFMediaType> uncompressedAudioType = nullptr;
	hrexcept(sourceReader->GetCurrentMediaType(STREAM_INDEX, &uncompressedAudioType));

	WAVEFORMATEX* format;
	UINT32 waveFormatLength;

	hrexcept(MFCreateWaveFormatExFromMFMediaType(uncompressedAudioType, &format, &waveFormatLength));

	static_assert(sizeof(WAVEFORMATEX) == sizeof(WaveFormat), "Wave Format Size unmatch");
	if (waveFormatLength < sizeof(WaveFormat))
	{
		memcpy(waveFormat, format, waveFormatLength);
		memset((byte*)waveFormat + waveFormatLength, 0, sizeof(WaveFormat) - waveFormatLength);
	}
	else
	{
		*waveFormat = *(WaveFormat*)format;
	}

	// ensure the stream is selected
	hrexcept(sourceReader->SetStreamSelection(STREAM_INDEX, true));
	return sourceReader;
}

MFMediaBuffer ComMethod<IMFSourceReader>::read() throws(ErrorCode)
{
	MFMediaBuffer buffer;
	IMFSample* sample = nullptr;
	do
	{
		DWORD flags = 0;
		hrexcept(ptr()->ReadSample(STREAM_INDEX, 0, nullptr, &flags, nullptr, &sample));

		// check whether the data is still valid
		if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) return nullptr;

		// check for eof
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM) return nullptr;

	} while (sample == nullptr);

	// convert data to contiguous buffer
	hrexcept(sample->ConvertToContiguousBuffer(&buffer));
	sample->Release();
	return buffer;
}

MFMediaStream::MFMediaStream(MFSourceReader reader) noexcept
{
	m_reader = move(reader);
}
MFMediaStream::~MFMediaStream() noexcept
{
	if (m_buffer != nullptr)
	{
		m_buffer.unlock();
		m_buffer = nullptr;
	}
}

bool MFMediaStream::next(Buffer * data) throws(ErrorCode)
{
	m_buffer = nullptr;
	m_buffer = m_reader.read();

	m_buffer.lock(data);
	return true;
}
