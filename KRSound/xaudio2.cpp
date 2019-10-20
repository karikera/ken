#include "stdafx.h"

#if _WIN_VER > _WIN32_WINNT_WIN7
#include "xaudio2.h"

#include <KR3/wl/com.h>
#include <xaudio2.h>

using namespace kr;

namespace
{
	IXAudio2* s_xaudio2;
	IXAudio2MasteringVoice* s_masteringVoice = nullptr;
}

XASound::Init::Init() noexcept
{
	HRESULT hr;
	if (FAILED(hr = XAudio2Create(&s_xaudio2, 0)))
	{
		error("Failed to init XAudio2 engine: %#X\n", hr);
		return;
	}

	if (FAILED(hr = s_xaudio2->CreateMasteringVoice(&s_masteringVoice)))
	{
		error("Failed creating mastering voice: %#X\n", hr);
		return;
	}

}
XASound::Init::~Init() noexcept
{
	if (s_masteringVoice)
	{
		s_masteringVoice->DestroyVoice();
		s_masteringVoice = nullptr;
	}
	if (s_xaudio2)
	{
		s_xaudio2->Release();
		s_xaudio2 = nullptr;
	}
}
//
//HRESULT PlayWave(IXAudio2* pXaudio2, pcstr16 path) noexcept
//{
//	std::unique_ptr<uint8_t[]> waveFile;
//	DirectX::WAVData waveData;
//	if (FAILED(hr = DirectX::LoadWAVAudioFromFileEx(strFilePath, waveFile, waveData)))
//	{
//		wprintf(L"Failed reading WAV file: %#X (%s)\n", hr, strFilePath);
//		return hr;
//	}
//
//	//
//	// Play the wave using a XAudio2SourceVoice
//	//
//
//	// Create the source voice
//	IXAudio2SourceVoice* pSourceVoice;
//	if (FAILED(hr = pXaudio2->CreateSourceVoice(&pSourceVoice, waveData.wfx)))
//	{
//		wprintf(L"Error %#X creating source voice\n", hr);
//		return hr;
//	}
//
//	// Submit the wave sample data using an XAUDIO2_BUFFER structure
//	XAUDIO2_BUFFER buffer = { 0 };
//	buffer.pAudioData = waveData.startAudio;
//	buffer.Flags = XAUDIO2_END_OF_STREAM;  // tell the source voice not to expect any data after this buffer
//	buffer.AudioBytes = waveData.audioBytes;
//
//	if (waveData.loopLength > 0)
//	{
//		buffer.LoopBegin = waveData.loopStart;
//		buffer.LoopLength = waveData.loopLength;
//		buffer.LoopCount = 1; // We'll just assume we play the loop twice
//	}
//
//#if (_WIN32_WINNT < 0x0602 /*_WIN32_WINNT_WIN8*/) || (_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/ )
//	if (waveData.seek)
//	{
//		XAUDIO2_BUFFER_WMA xwmaBuffer = { 0 };
//		xwmaBuffer.pDecodedPacketCumulativeBytes = waveData.seek;
//		xwmaBuffer.PacketCount = waveData.seekCount;
//		if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer, &xwmaBuffer)))
//		{
//			wprintf(L"Error %#X submitting source buffer (xWMA)\n", hr);
//			pSourceVoice->DestroyVoice();
//			return hr;
//		}
//	}
//#else
//	if (waveData.seek)
//	{
//		wprintf(L"This platform does not support xWMA or XMA2\n");
//		pSourceVoice->DestroyVoice();
//		return hr;
//	}
//#endif
//	else if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer)))
//	{
//		wprintf(L"Error %#X submitting source buffer\n", hr);
//		pSourceVoice->DestroyVoice();
//		return hr;
//	}
//
//	hr = pSourceVoice->Start(0);
//
//	// Let the sound play
//	BOOL isRunning = TRUE;
//	while (SUCCEEDED(hr) && isRunning)
//	{
//		XAUDIO2_VOICE_STATE state;
//		pSourceVoice->GetState(&state);
//		isRunning = (state.BuffersQueued > 0) != 0;
//
//		// Wait till the escape key is pressed
//		if (GetAsyncKeyState(VK_ESCAPE))
//			break;
//
//		Sleep(10);
//	}
//
//	// Wait till the escape key is released
//	while (GetAsyncKeyState(VK_ESCAPE))
//		Sleep(10);
//
//	pSourceVoice->DestroyVoice();
//
//	return hr;
//}

#endif