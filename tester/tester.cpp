
#include <KR3/main.h>
#include <KR3/initializer.h>
#include <KR3/data/idmap.h>
#include <KRApp/2d.h>


#ifndef __EMSCRIPTEN__
#include "timertest.h"
#include <KRUtil/fs/file.h>
#endif

#include <KR3/data/crypt.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#pragma comment(lib, "openssl.lib")
#pragma comment(lib, "libcrypto.lib")

#ifdef WIN32

#include <KRSound/sound.h>
#include <KRSound/fft.h>

using namespace kr;
using namespace sound;

constexpr size_t STFT_SAMPLING_COUNT = 4096;
constexpr size_t STFT_DELTA_COUNT = STFT_SAMPLING_COUNT / 16;

class ComplexSum
{
public:
	Array<complex> m_sum;
	uint m_hop;
	uint m_samplingCount;
	uint m_offset;

	ComplexSum() noexcept
	{
	}

	Array<complex>::LoopIterable iterable() noexcept
	{
		return m_sum.loopIterable(m_offset);
	}

	void create(uint samplingCount, uint hop) noexcept
	{
		m_samplingCount = samplingCount;
		m_hop = hop;
		m_offset = 0;
		m_sum.resize(hop * 2);
		m_sum.zero();
	}

	void add(const complex * src) noexcept
	{
		for (complex & dest : m_sum)
		{
			dest = (complex)*src++;
		}
		//m_offset = (m_hop + m_offset) % m_sum.size();
		//float rate = 0.f;
		//float delta = 1.f / m_hop;
		//for (complex & dest : m_sum.loopIterable(m_offset, m_hop))
		//{
		//	dest += (complex)(*src++ * rate);
		//	rate += delta;
		//}
		//uint next = (m_hop + m_offset) % m_sum.size();
		//delta = -delta;
		//for (complex & dest : m_sum.loopIterable(next, m_hop))
		//{
		//	dest = (complex)(*src++ * rate);
		//	rate -= delta;
		//}
	}
};

class VoiceFilter
{
public:
	sound::SoundStreamer m_writer;
	sound::STFTCapture m_capture;
	sound::FFTW<complex, complex> m_ifft;

	Array<complex> m_result;
	ComplexSum m_complexSum;
	uint m_samplingCount;
	uint m_spaceOffset;

	VoiceFilter()
	{
	}

	void create(uint samplingCount) noexcept
	{
		m_samplingCount = samplingCount;
		m_result.resize(samplingCount);
		m_result.zero();

		m_writer.create(1, secByte / 10);
		m_writer.loop();
		m_capture.create(samplingCount, STFT_DELTA_COUNT * sizeof(sample));
		m_ifft.create_1d_inverse(samplingCount);

		m_complexSum.create(samplingCount, STFT_DELTA_COUNT);
	}

	void remove() noexcept
	{
		m_ifft.remove();
		m_capture.remove();
		m_writer.remove();
	}

	void filter(const complex * src) noexcept
	{
		float ysum = 0;
		uint ysum_idx = 0;

		float reduce = 1.f / m_samplingCount;
		float freq = 0.f;

		complex * result = m_result.data();
		complex * result_end = result + m_samplingCount;
		// result += 30;
		while (result != result_end)
		{
			complex out = *src++ * reduce;
			{
				//float v = (sinf(freq * 0.8f) + 1.f) * 0.5f;
				//v *= v;
				//v *= v;
				//v *= v;
				//v *= v;
				//out = out * v;
			}
			{
				//float v = (1 - cosf(freq * 0.03f)) * 0.5f;
				//v *= v;
				//out *= v;
			}
			// out *= 5.f;

			constexpr float MAX = 1000.f;
			float len = out.length() / MAX;

			float newlen = len;
			if (len < 0.02f) newlen = len * len * len;
			else newlen = len - 0.01f;


			*result++ = out * (newlen / len);

			freq++;
		}
	}

	bool skip() noexcept
	{
		if (!m_capture.skip()) return false;
		//try
		//{
		//	Sound::Locked locked = m_writer.lock(m_samplingCount);
		//	memset(locked.buffer1, 0, locked.size1);
		//	memset(locked.buffer2, 0, locked.size2);
		//}
		//catch (...)
		//{
		//}
		return true;
	}

	bool process() noexcept
	{
		const complex * buffers = m_capture.process();
		if (!buffers) return false;

		filter(buffers);

		try
		{
			m_ifft.put(m_result.data());
			m_complexSum.add(m_ifft.execute());
			auto iter = m_complexSum.iterable().begin();

			Sound::Locked locked = m_writer.lock(STFT_DELTA_COUNT * sizeof(sample));
			for (sample & dest : locked)
			{
				const complex * res = &*iter++;
				int v = (int)(res->real);
				dest = (sample)clampt(-0x7fff, v, 0x7fff);
			}
		}
		catch (...)
		{
		}
		return true;
	}
};

VoiceFilter filter;

dword fromHue(float value) noexcept
{
	constexpr float deg60 = math::pi / 3;
	if (value <= 0.f) return 0;

	value /= deg60;
	int idx = (int)value;
	value -= (float)idx;
	switch (idx)
	{
	case 0: return (dword)(value * 0xff);
	case 1: return (dword)(((int)(value * 0xff) << 8) | 0xff);
	case 2: return (dword)(((int)(0xff - value * 0xff)) | 0xff00);
	case 3: return (dword)(((int)(value * 0xff) << 16) | 0xff00);
	case 4: return (dword)(((int)(0xff - value * 0xff) << 8) | 0xff0000);
	case 5: return (dword)(((int)(value * 0xff)) | 0xff0000);
	case 6:
	{
		int c = (int)(value * 0xff);
		return (dword)((c << 8) | c | 0xff0000);
	}
	default: return 0xffffff;
	}
}

constexpr uint width = 1024;
constexpr uint height = 768;

class ToneConverter
{
public:
	Array<uint> yToTone;
	Array<uint> toneToY;


	ToneConverter() noexcept
	{
		yToTone.resize(height);
		uint * dest = yToTone.end();
		for (uint i = 0; i < height; i++)
		{
			uint idx;
			idx = i / 2;
			if (idx > STFT_SAMPLING_COUNT)
			{
				*--dest = STFT_SAMPLING_COUNT - 1;
				continue;
			}
			// idx = (uint)powf((float)STFT_SAMPLING_COUNT, (float)i / height);
			*--dest = idx;
		}
		toneToY.resize(STFT_SAMPLING_COUNT);
		dest = toneToY.begin();
		for (uint i = 0; i < STFT_SAMPLING_COUNT; i++)
		{
			int y = height - i * 2 - 1;
			if (y < 0) y = 0;
			*dest++ = y;
		}
	}

};

ToneConverter converter;

class Main : public WebCanvas2D
{
private:
	int m_x;
	Array<dword> m_image;
	bool m_pause;

public:
	Main() noexcept
		: WebCanvas2D(width, height)
	{
		m_image.resize(width*height);
		m_pause = false;
	}
	~Main() noexcept
	{
	}

	void onDraw() noexcept override
	{
		{
			image::ImageData image;
			image.attach(PixelFormatABGR8, m_image.data(), 4 * width, width, height);

			int linedrawed = 0;
			size_t pitch = image.getPitch();
			for (;;)
			{
				if (m_pause) break;
				dword * dest_begin = m_image.data() + m_x;
				dword * dest = dest_begin;
				if (linedrawed < 100)
				{
					if (!filter.process()) break;

					for (uint y = 0; y < height; y++)
					{
						complex &v = filter.m_result[converter.yToTone[y]];
						float fv = v.length() / 512;
						fv = sqrt(fv);

						int color = (int)(fv * 0xff);
						if (color < 0) color = 0;
						else if (color > 255) color = 255;

						*dest = 0xff000000 | (color << 8) | (color << 16) | (color);
						(byte*&)dest += pitch;
					}
					linedrawed++;
				}
				else
				{
					if (!filter.skip()) break;
				}

				m_x = (m_x + 1) % width;
			}
			uint right = width - m_x;
			putImageData(image.subimage(m_x, 0, right, height), 0, 0);
			putImageData(image.subimage(0, 0, m_x, height), right, 0);
		}
	}
	void onKeyDown(int key, bool repeat) noexcept override
	{
		switch (key)
		{
		case 'P': m_pause = !m_pause; break;
		}
	}
	void onKeyUp(int key) noexcept
	{
		switch (key)
		{
		case VK_SPACE: break;
		}
	}
	void onMouseMove(int x, int y) noexcept
	{
	}
	void onMouseDown(int x, int y, int button) noexcept
	{
	}
	void onMouseUp(int x, int y, int button) noexcept
	{
	}
};


int soundMain() noexcept
{
	using namespace kr::sound;

	Main wnd;

	Initializer<Sound> _init;

	filter.create(STFT_SAMPLING_COUNT);

	main_loop(&wnd);

	filter.remove();
	return 0;
}
#endif

class MainWindow : public kr::WebCanvas2D
{
public:
	MainWindow() noexcept
		: WebCanvas2D(1024, 768)
	{
	}

	void onDraw() noexcept override
	{
		lineWidth = 3.f;

		clearRect(0, 0, 640, 640);
		beginPath();
		moveTo(0, 0);
		lineTo(100, 100);
		stroke();
	}
};

int main()
{
	OPENSSL_add_all_algorithms_noconf();
	OpenSSL_add_all_digests();
	ERR_load_EVP_strings();
	ERR_load_CRYPTO_strings();
	ERR_load_PKCS7_strings();

	std::string pubkey, privkey;

	{
		EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(0x198, nullptr);
		int ecKeyType = 0x2CB;
		EVP_PKEY_paramgen_init(ctx);
		EVP_PKEY_CTX_ctrl(ctx, 0x198, 6, 0x1001, ecKeyType, 0);
		EVP_PKEY* pkey = nullptr;
		EVP_PKEY_paramgen(ctx, &pkey);
		EVP_PKEY_CTX* pkey_ctx = EVP_PKEY_CTX_new(pkey, nullptr);
		EVP_PKEY_keygen_init(pkey_ctx);
		EVP_PKEY* keygen = nullptr;
		EVP_PKEY_keygen(pkey_ctx, &keygen);
		ec_key_st* res = EVP_PKEY_get1_EC_KEY(keygen);
		EC_KEY_set_asn1_flag(res, 1);
		{
			int length = i2d_EC_PUBKEY(res, 0);
			pubkey.resize(length);
			byte* dest = (byte*)pubkey.data();
			i2d_EC_PUBKEY(res, &dest);
		}
		{
			int length = i2d_PrivateKey(keygen, 0);
			privkey.resize(length);
			byte* dest = (byte*)privkey.data();
			i2d_PrivateKey(keygen, &dest);
		}
		EC_KEY_free(res);
		EVP_PKEY_free(keygen);
		EVP_PKEY_CTX_free(pkey_ctx);
		EVP_PKEY_free(pkey);
		EVP_PKEY_CTX_free(ctx);
	}

	
	AText idpubkey = (encoder::Base64::Decoder)Text("MHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEoV9liUXaZuGgj6f/sUlv1XQALCPGPvhW9YHq3xA+SpSyVDfq9T6qSgSfvm/fV66GgKBIENSQ2g3pPDbJPIfUFkUfhzeb41ZzzcPVFgohj5HiuvAJN8+WtPO1HJLecCtY");

	const uint8_t* pp = (uint8_t*)privkey.data();
	EVP_PKEY* apkey = d2i_AutoPrivateKey(nullptr, &pp, (long)privkey.size());

	pp = (uint8_t*)idpubkey.data();
	EVP_PKEY * pkey = d2i_PUBKEY(nullptr, &pp, (long)idpubkey.size());

	EVP_PKEY_CTX * ekey = EVP_PKEY_CTX_new(apkey, nullptr);
	EVP_PKEY_derive_init(ekey);
	EVP_PKEY_derive_set_peer(ekey, pkey);
	size_t keylen = 0;
	EVP_PKEY_derive(ekey, nullptr, &keylen);

	AText secretKey;
	secretKey.resize(keylen);
	EVP_PKEY_derive(ekey, (byte*)secretKey.data(), &keylen);

	EVP_PKEY_CTX_free(ekey);
	EVP_PKEY_free(apkey);
	EVP_PKEY_free(pkey);

	{
		//const char* name = OBJ_nid2sn(0x2A0); // "SHA256"
		//const EVP_MD* type = EVP_get_digestbyname(name);
		//EVP_MD_CTX* ctx = EVP_MD_CTX_create();
		//EVP_DigestInit_ex(ctx, type, nullptr);
		//EVP_DigestUpdate(ctx, input, input_len);
		//EVP_DigestFinal(ctx, output, output_len);
	}



	return 0;

	MainWindow wnd;
	main_loop(&wnd);
	return 0;
}
