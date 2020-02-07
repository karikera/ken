#include "stdafx.h"

#define INITGUID
#include "sound.h"
#include <KR3/wl/com.h>
#include <KR3/fs/file.h>
#include <KR3/util/path.h>
#include <KRWin/handle.h>
#include <KR3/msg/eventdispatcher.h>

#include <CGuid.h>
#include <MMSystem.h>
#include <dsound.h>

#pragma comment(lib,"dsound.lib")

using namespace kr;
using namespace sound;

namespace
{
	Keep<IDirectSound8> s_ds;
	Keep<IDirectSoundCapture8> s_dsc;
}

SoundNotify::SoundNotify(IUnknown* obj) noexcept
{
	if (FAILED(obj->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)& m_ptr)))
	{
		m_ptr = nullptr;
	}
}
bool SoundNotify::makeEvent(EventHandle** out, View<dword> offsets) noexcept
{
	EventHandle** out_end = out;
	for (dword offset : offsets)
	{
		*out_end++ = EventHandle::create(false, false);
	}
	if (!registEvent({ out, out_end }, offsets))
	{
		for (dword offset : offsets)
		{
			delete* out++;
		}
		return false;
	}
	return true;
}
bool SoundNotify::registEvent(View<EventHandle*> events, View<dword> offsets) noexcept
{
	dword count = intact<dword>(offsets.size());
	TmpArray<DSBPOSITIONNOTIFY> positionNotify(count);
	EventHandle* const* eventptr = events.begin();
	for (dword offset : offsets)
	{
		DSBPOSITIONNOTIFY* noti = positionNotify.prepare(1);
		noti->dwOffset = offset;
		noti->hEventNotify = *eventptr++;
	}
	HRESULT hr = m_ptr->SetNotificationPositions(count, positionNotify.data());
	return SUCCEEDED(hr);
}


Sound::Locked::Locked() noexcept
{
	m_sound = nullptr;
}
Sound::Locked::Locked(Locked&& other) noexcept
{
	m_sound = other.m_sound;
	buffer1 = other.buffer1;
	size1 = other.size1;
	buffer2 = other.buffer2;
	size2 = other.size2;
	other.m_sound = nullptr;
}
Sound::Locked::Locked(IDirectSoundBuffer* sound, byte* buffer1, dword size1, byte* buffer2, dword size2) noexcept
	:m_sound(sound), buffer1(buffer1), size1(size1), buffer2(buffer2), size2(size2)
{
}
Sound::Locked::~Locked() noexcept
{
	unlock();
}
void Sound::Locked::unlock() noexcept
{
	if (m_sound)
	{
		m_sound->Unlock(buffer1, size1, buffer2, size2);
		m_sound = nullptr;
	}
}
WView<sample> Sound::Locked::array1() noexcept
{
	return WView<sample>((sample*)buffer1, size1);
}
WView<sample> Sound::Locked::array2() noexcept
{
	return WView<sample>((sample*)buffer2, size2);
}
Sound::Locked::operator bool() noexcept
{
	return m_sound != nullptr;
}
bool Sound::Locked::operator !() noexcept
{
	return m_sound == nullptr;
}
Sound::Locked& Sound::Locked::operator =(Locked&& other) noexcept
{
	unlock();
	m_sound = other.m_sound;
	buffer1 = other.buffer1;
	size1 = other.size1;
	buffer2 = other.buffer2;
	size2 = other.size2;
	other.m_sound = nullptr;
	return *this;
}

Sound::Locked::Iterator::Iterator(sample* ptr, sample* next) noexcept
{
	m_ptr = ptr;
	m_next = next;
}
Sound::Locked::Iterator& Sound::Locked::Iterator::Iterator::operator ++() noexcept
{
	m_ptr++;
	return *this;
}
const Sound::Locked::Iterator Sound::Locked::Iterator::Iterator::operator ++(int) noexcept
{
	Iterator tmp = *this;
	++* this;
	return tmp;
}
bool Sound::Locked::Iterator::operator ==(const Iterator& other) noexcept
{
	if (m_ptr == other.m_ptr)
	{
		m_ptr = m_next;
		m_next = nullptr;
		other.m_ptr = other.m_next;
		other.m_next = (sample*)-1;
	}
	return m_ptr == nullptr;
}
bool Sound::Locked::Iterator::operator !=(const Iterator& other) noexcept
{
	if (m_ptr == other.m_ptr)
	{
		m_ptr = m_next;
		m_next = nullptr;
		other.m_ptr = other.m_next;
		other.m_next = (sample*)-1;
	}
	return m_ptr != nullptr;
}
sample& Sound::Locked::Iterator::operator *() noexcept
{
	return *m_ptr;
}
Sound::Locked::Iterator Sound::Locked::begin() noexcept
{
	return Iterator((sample*)buffer1, (sample*)buffer2);
}
Sound::Locked::Iterator Sound::Locked::end() noexcept
{
	return Iterator(
		(sample*)(buffer1 + size1),
		(sample*)(buffer2 + size2)
	);
}

Sound::Init::Init() noexcept
{
	try
	{
		hrexcept(DirectSoundCreate8(nullptr, &s_ds, nullptr));
		if (win::g_mainWindow != nullptr)
		{
			hrexcept(s_ds->SetCooperativeLevel(win::g_mainWindow, DSSCL_NORMAL));
		}
		else
		{
			hrexcept(s_ds->SetCooperativeLevel(::GetDesktopWindow(), DSSCL_NORMAL));
		}
	}
	catch (ErrorCode&)
	{
		s_ds = nullptr;
	}
}
Sound::Init::~Init() noexcept
{
	s_ds = nullptr;
	s_dsc = nullptr;
}

Sound::Sound() noexcept
{
}
Sound::~Sound() noexcept
{
}
void Sound::remove() noexcept
{
	Keep::remove();
}
Sound::Locked Sound::createLock(word channel, dword size) throws(ErrorCode)
{
	ptr buffer;
	KrbWaveFormat format =
	{
		WAVE_FORMAT_PCM,
		channel,
		secSample,
		channel * secByte,
		(WORD)(channel * sampleBytes),
		sampleBit,
		sizeof(KrbWaveFormat)
	};
	_create(&format, size);
	dword lockSize;
	hrexcept(m_ptr->Lock(0, 0, &buffer, (LPDWORD)& lockSize, nullptr, nullptr, DSBLOCK_ENTIREBUFFER));
	return Locked(m_ptr, (byte*)buffer, lockSize, nullptr, 0);
}
bool Sound::createMonotone(dword dwSize, float volume, float frequency) noexcept
{
	if (s_ds == nullptr)
		return false;

	try
	{
		Locked locked = createLock(1, dwSize);

		volume *= maxVolume;
		float volumeDelta = volume / (secSample / 50);
		float fFrequency = frequency * math::pi * 2 / secSample;

		dword uCount = (locked.size1 / sizeof(sample));
		dword uStart = secSample / 50;
		dword uEnd = uCount - secSample / 50;
		dword i = 0;

		sample * samples = (sample*)locked.buffer1;
		// volume: 0 ~ 1
		for (; i < uStart;)
		{
			float sound = i * math::sin(i * fFrequency) * volumeDelta;
			samples[i++] = (sample)(sound);
		}
		// volume: 1 ~ 1
		for (; i < uEnd;)
		{
			float sound = math::sin(i * fFrequency) * volume;
			samples[i++] = (sample)(sound);
		}
		// volume: 1 ~ 0
		for (; i < uCount;)
		{
			float sound = (uCount - i) * math::sin(i * fFrequency) * volumeDelta;
			samples[i++] = (sample)(sound);
		}
		return true;
	}
	catch (...)
	{
		*this = nullptr;
		return false;
	}
}
double Sound::load(krb::Extension extension, krb::File file) noexcept
{
	if (s_ds == nullptr)
		return 0;

	struct Callback :KrbSoundCallback
	{
		Sound* sound;
		double duration;
	};
	Callback cb;
	cb.sound = this;
	cb.start = [](KrbSoundCallback * _this, KrbSoundInfo * _info)->short* {
		Callback* cb = static_cast<Callback*>(_this);
		cb->duration = _info->duration;
		cb->sound->_create(&_info->format, _info->totalBytes);
		dword lockSize;
		ptr buffer;
		HRESULT hr = cb->sound->m_ptr->Lock(0, 0, &buffer, (LPDWORD)&lockSize, nullptr, nullptr, DSBLOCK_ENTIREBUFFER);
		if (FAILED(hr)) return nullptr;
		return (short*)buffer;
	};
	if (krb_sound_load(extension, &cb, &file))
	{
		return cb.duration;
	}
	else
	{
		return 0;
	}
}
double Sound::load(const fchar_t* path) noexcept
{
	return load(krb::makeExtensionFromPath((View<fchar_t>)path), path);
}

bool Sound::play() noexcept
{
	if (!m_ptr) return false;
	m_ptr->SetCurrentPosition(0);
	HRESULT hr = m_ptr->Play(0, 0, 0);
	return FAILED(hr);
}
void Sound::stop() noexcept
{
	m_ptr->Stop();
}
bool Sound::loop() noexcept
{
	if (!m_ptr) return false;
	m_ptr->SetCurrentPosition(0);
	HRESULT hr = m_ptr->Play(0, 0, DSBPLAY_LOOPING);
	return FAILED(hr);
}
bool Sound::dupPlay() noexcept
{
	if (!s_ds) return false;
	if (!m_ptr) return false;

	IDirectSoundBuffer * sb;
	if (FAILED(s_ds->DuplicateSoundBuffer(m_ptr, &sb)))
		return false;

	SoundNotify notify(sb);
	if (notify == nullptr)
	{
		sb->Release();
		return false;
	}
	EventHandle* event;
	if (!notify.makeEvent(&event, { DSBPN_OFFSETSTOP }))
	{
		sb->Release();
		return false;
	}
	EventDispatcher::regist(event, [sb](DispatchedEvent * ev) {
		sb->Release();
		ev->remove();
	});
	return true;
}

void Sound::_create(const KrbWaveFormat * format, dword size) throws(ErrorCode)
{
	return _create(format, size, DSBCAPS_STATIC | DSBCAPS_GLOBALFOCUS);
}
void Sound::_create(const KrbWaveFormat* format, dword size, dword flags) throws(ErrorCode)
{
	static_assert(sizeof(KrbWaveFormat) == sizeof(WAVEFORMATEX), "format size unmatch");
	DSBUFFERDESC dsbd;
	dsbd.dwFlags = 0;
	dsbd.dwReserved = 0;
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwBufferBytes = size;
	dsbd.lpwfxFormat = (LPWAVEFORMATEX)format;
	dsbd.dwFlags = flags;
	dsbd.guid3DAlgorithm = DS3DALG_DEFAULT;

	hrexcept(s_ds->CreateSoundBuffer(&dsbd, &m_ptr, nullptr));
}

void PrimarySound::create() throws(ErrorCode)
{
	if (s_ds == nullptr)
		return;
	_create(nullptr, 0, DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN);
	//m_sb->SetPan(-10000);
}

SoundStreamer::SoundStreamer() noexcept
{
	remove();
}
void SoundStreamer::create(word channel, dword bufferSize) noexcept
{
	remove();
	m_lastWrite = 0;
	{
		m_bufferSize = bufferSize;
		KrbWaveFormat format =
		{
			WAVE_FORMAT_PCM,
			channel,
			secSample,
			channel * secByte,
			(word)(channel * sampleBytes),
			sampleBit,
			sizeof(KrbWaveFormat)
		};
		_create(&format, bufferSize, DSBCAPS_GLOBALFOCUS);
		void* buffer;
		dword lockSize;
		if (FAILED(m_ptr->Lock(0, 0, &buffer, (LPDWORD)& lockSize, nullptr, nullptr, DSBLOCK_ENTIREBUFFER)))
		{
			remove();
			return;
		}
		memset(buffer, 0, lockSize);
		m_ptr->Unlock(buffer, lockSize, nullptr, 0);
	}
}
Sound::Locked SoundStreamer::lock()
{
	DWORD end;
	hrexcept(m_ptr->GetCurrentPosition(nullptr, &end));

	void* ptr1, * ptr2;
	DWORD size1, size2;
	DWORD readSize = end >= m_lastWrite ? end - m_lastWrite : m_bufferSize + end - m_lastWrite;
	hrexcept(m_ptr->Lock(m_lastWrite, readSize, &ptr1, &size1, &ptr2, &size2, 0));

	m_lastWrite = ptr2 == nullptr ? (m_lastWrite + size1) % m_bufferSize : size2;
	return Locked(m_ptr, (byte*)ptr1, size1, (byte*)ptr2, size2);
}
Sound::Locked SoundStreamer::lock(dword size)
{
	DWORD end;
	hrexcept(m_ptr->GetCurrentPosition(nullptr, &end));

	void* ptr1, * ptr2;
	DWORD size1, size2;
	DWORD readSize = end >= m_lastWrite ? end - m_lastWrite : m_bufferSize + end - m_lastWrite;
	if (readSize < size) throw EofException();
	hrexcept(m_ptr->Lock(m_lastWrite, size, &ptr1, &size1, &ptr2, &size2, 0));

	m_lastWrite = ptr2 == nullptr ? (m_lastWrite + size1) % m_bufferSize : size2;
	return Locked(m_ptr, (byte*)ptr1, size1, (byte*)ptr2, size2);
}
Sound::Locked SoundStreamer::lockMoreThan(dword size)
{
	DWORD end;
	hrexcept(m_ptr->GetCurrentPosition(nullptr, &end));

	void* ptr1, * ptr2;
	DWORD size1, size2;
	DWORD readSize = end >= m_lastWrite ? end - m_lastWrite : m_bufferSize + end - m_lastWrite;
	if (readSize < size) throw EofException();
	hrexcept(m_ptr->Lock(m_lastWrite, readSize, &ptr1, &size1, &ptr2, &size2, 0));

	m_lastWrite = ptr2 == nullptr ? (m_lastWrite + size1) % m_bufferSize : size2;
	return Locked(m_ptr, (byte*)ptr1, size1, (byte*)ptr2, size2);
}


SoundCapture::SoundCapture() noexcept
{
}
SoundCapture::~SoundCapture() noexcept
{
}
void SoundCapture::create(word channel) noexcept
{
	remove();
	m_lastRead = 0;

	if (s_dsc == nullptr)
	{
		try
		{
			hrexcept(DirectSoundCaptureCreate8(nullptr, &s_dsc, nullptr));
		}
		catch (...)
		{
			return;
		}
	}
	try
	{
		WAVEFORMATEX format =
		{
			WAVE_FORMAT_PCM,
			channel,
			secSample,
			m_secByte = channel * secByte,
			(word)(channel * sampleBytes),
			sampleBit,
			sizeof(WAVEFORMATEX)
		};

		DSCBUFFERDESC dscbd;
		dscbd.dwSize = sizeof(DSCBUFFERDESC);
		dscbd.dwFlags = 0;
		dscbd.dwBufferBytes = m_secByte;
		dscbd.dwReserved = 0;
		dscbd.lpwfxFormat = (LPWAVEFORMATEX)& format;
		dscbd.dwFXCount = 0;
		dscbd.lpDSCFXDesc = nullptr;

		hrexcept(s_dsc->CreateCaptureBuffer(&dscbd, &m_ptr, nullptr));
	}
	catch (...)
	{
	}

	m_ptr->Start(DSCBSTART_LOOPING);
}
void SoundCapture::remove() noexcept
{
	Keep::remove();
}

dword SoundCapture::skip(dword size) noexcept
{
	DWORD end;
	if (FAILED(m_ptr->GetCurrentPosition(nullptr, &end)))
	{
		return 0;
	}

	if (end >= m_lastRead)
	{
		dword readed = end - m_lastRead;
		if (readed < size)
		{
			m_lastRead = end;
			return readed;
		}
		else
		{
			m_lastRead = m_lastRead + size;
			return size;
		}
	}
	else
	{
		DWORD readed = m_secByte + end - m_lastRead;
		if (readed < size)
		{
			m_lastRead = end;
			return readed;
		}
		else
		{
			m_lastRead = (m_lastRead + size) % m_secByte;
			return size;
		}
	}
}
SoundCapture::Locked SoundCapture::lock()
{
	DWORD end;
	hrexcept(m_ptr->GetCurrentPosition(nullptr, &end));

	void* ptr1, * ptr2;
	DWORD size1, size2;
	DWORD readSize = end >= m_lastRead ? end - m_lastRead : m_secByte + end - m_lastRead;
	hrexcept(m_ptr->Lock(m_lastRead, readSize, &ptr1, &size1, &ptr2, &size2, 0));

	m_lastRead = ptr2 == nullptr ? (m_lastRead + size1) % m_secByte : size2;
	return Locked(m_ptr, (byte*)ptr1, size1, (byte*)ptr2, size2);
}
SoundCapture::Locked SoundCapture::lockMoreThan(dword size)
{
	DWORD end;
	hrexcept(m_ptr->GetCurrentPosition(nullptr, &end));

	void* ptr1, * ptr2;
	DWORD size1, size2;
	DWORD readSize = end >= m_lastRead ? end - m_lastRead : m_secByte + end - m_lastRead;
	if (readSize < size) throw EofException();
	hrexcept(m_ptr->Lock(m_lastRead, readSize, &ptr1, &size1, &ptr2, &size2, 0));

	m_lastRead = ptr2 == nullptr ? (m_lastRead + size1) % m_secByte : size2;
	return Locked(m_ptr, (byte*)ptr1, size1, (byte*)ptr2, size2);
}

SoundCapture::Locked::Locked() noexcept
{
	m_sound = nullptr;
}
SoundCapture::Locked::Locked(Locked && other) noexcept
{
	m_sound = other.m_sound;
	buffer1 = other.buffer1;
	size1 = other.size1;
	buffer2 = other.buffer2;
	size2 = other.size2;
	other.m_sound = nullptr;
}
SoundCapture::Locked::Locked(IDirectSoundCaptureBuffer * sound, byte * buffer1, dword size1, byte * buffer2, dword size2) noexcept
	:m_sound(sound), buffer1(buffer1), size1(size1), buffer2(buffer2), size2(size2)
{
}
SoundCapture::Locked::~Locked() noexcept
{
	unlock();
}
void SoundCapture::Locked::unlock() noexcept
{
	if (m_sound == nullptr) return;
	m_sound->Unlock(buffer1, size1, buffer2, size2);
	m_sound = nullptr;
}
View<sample> SoundCapture::Locked::array1() noexcept
{
	return WView<sample>((sample*)buffer1, size1 / 2);
}
View<sample> SoundCapture::Locked::array2() noexcept
{
	return WView<sample>((sample*)buffer2, size2 / 2);
}
SoundCapture::Locked::operator bool() noexcept
{
	return m_sound != nullptr;
}
bool SoundCapture::Locked::operator !() noexcept
{
	return m_sound == nullptr;
}
SoundCapture::Locked& SoundCapture::Locked::operator =(Locked && other) noexcept
{
	unlock();
	m_sound = other.m_sound;
	buffer1 = other.buffer1;
	size1 = other.size1;
	buffer2 = other.buffer2;
	size2 = other.size2;
	other.m_sound = nullptr;
	return *this;
}