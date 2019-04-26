#include "stdafx.h"
#include "fft.h"
#include "fftw3.h"

using namespace kr;
using namespace sound;

static_assert(sizeof(complex) == sizeof(fftwf_complex), "Complex size unmatch");

SoundCaptureSampler::SoundCaptureSampler() noexcept
{
	m_samplingSize = 0;
	m_delta = 0;
}
void SoundCaptureSampler::create(word channel, uint samplingSize, uint delta) noexcept
{
	SoundCapture::create(channel);
	m_samplingSize = samplingSize;
	m_delta = delta;
	m_buffer.resize(samplingSize);
	m_buffer.fill(0);
}
void SoundCaptureSampler::remove() noexcept
{
	SoundCapture::remove();
}
const sample * SoundCaptureSampler::read() noexcept
{
	if (m_delta > m_samplingSize)
	{
		if (m_deltaLeft == 0)
		{
			m_deltaLeft = m_delta;
			m_buffer.clear();
		}
		if (m_deltaLeft > m_samplingSize)
		{
			uint needSkip = m_deltaLeft - m_samplingSize;
			if (m_locked)
			{
				uint lockLeft = m_lockRead - m_locked.size1 - m_locked.size2;
				if (lockLeft >= needSkip)
				{
					m_lockRead += needSkip;
				}
				else
				{
					m_lockRead = 0;
					m_locked.unlock();
					m_deltaLeft -= SoundCapture::skip(needSkip);
					if (m_deltaLeft > m_samplingSize) return nullptr;
				}
			}
			else
			{
				m_deltaLeft -= SoundCapture::skip(needSkip);
				if (m_deltaLeft > m_samplingSize) return nullptr;
			}
		}
	}
	else
	{
		if (m_deltaLeft == 0)
		{
			m_deltaLeft = m_delta;
			m_buffer.remove(0, m_delta);
		}
	}

	if (!m_locked)
	{
		try
		{
			m_locked = lockMoreThan(m_deltaLeft);
		}
		catch (...)
		{
			return nullptr;
		}
	}
	for (;;)
	{
		if (m_lockRead < m_locked.size1)
		{
			uint left = m_locked.size1 - m_lockRead;
			if (m_deltaLeft <= left)
			{
				m_buffer.write((byte*)m_locked.buffer1 + m_lockRead, m_deltaLeft);
				m_lockRead += m_deltaLeft;
				m_deltaLeft = 0;
				return (sample*)m_buffer.data();
			}
			else
			{
				m_buffer.write(m_locked.buffer1 + m_lockRead, left);
				m_lockRead += left;
				m_deltaLeft -= left;
			}
		}
		uint lockRead2 = m_lockRead - m_locked.size1;
		if (lockRead2 < m_locked.size2)
		{
			uint left = m_locked.size2 - lockRead2;
			if (m_deltaLeft <= left)
			{
				m_buffer.write(m_locked.buffer2 + lockRead2, m_deltaLeft);
				m_lockRead += m_deltaLeft;
				m_deltaLeft = 0;
				return (sample*)m_buffer.data();
			}
			else
			{
				m_buffer.write(m_locked.buffer2 + lockRead2, left);
				m_deltaLeft -= left;
			}
		}
		m_lockRead = 0;
		m_locked.unlock();
		try
		{
			m_locked = lockMoreThan(m_deltaLeft);
		}
		catch (...)
		{
			return nullptr;
		}
	}
}

DFT::DFT() noexcept
{
	m_result = nullptr;
}
DFT::~DFT() noexcept
{
	delete[] m_result;
}
void DFT::create(uint samplingCount) noexcept
{
	m_samplingCount = samplingCount;
	m_result = _new complex[samplingCount];
}
void DFT::remove() noexcept
{
	delete[] m_result;
	m_result = nullptr;
}
const complex * DFT::process(const sample * input) noexcept
{
	for (uint i = 0; i < m_samplingCount; i++)
	{
		complex sum = { 0, 0 };
		for (uint j = 0; j < m_samplingCount; j++)
		{
			float angle = 2.f * math::pi * j * i / m_samplingCount;
			SinCos sc = math::sincos(angle);
			float real = sc.cos * input[j] / 0x8000;
			sum.real += real * sc.cos;
			sum.imag += -real * sc.sin;
		}
		m_result[i] = sum;
	}
	return m_result;
}

void FFT::_split(complex * even, complex * odd, const complex * src, uint count) noexcept
{
	const complex * srcend = src + count;
	while (src != srcend)
	{
		*even++ = *src++;
		*odd++ = *src++;
	}
}
void FFT::_fft(complex * src, uint count) noexcept
{
	if (count <= 1) return;

	uint half = count / 2;
	complex * even = m_temp;
	complex * odd = m_temp + half;
	_split(even, odd, src, count);

	complex * t = m_temp;
	m_temp = m_result;
	m_result = t;

	_fft(even, half);
	_fft(odd, half);

	t = m_temp;
	m_temp = m_result;
	m_result = t;

	uint mul = m_samplingCount / count;
	for (uint m = 0; m < half; m++)
	{
		complex w = m_trifunc[m * mul];
		complex z = w * odd[m];
		src[m] = even[m] + z;
		src[m + half] = even[m] - z;
	}
}
FFT::FFT() noexcept
{
	m_buffer = nullptr;
}
FFT::~FFT() noexcept
{
	delete[] m_buffer;
}
void FFT::create(uint samplingCount) noexcept
{
	delete[] m_buffer;

	m_samplingCount = samplingCount;
	uint trisize = samplingCount * sizeof(complex);
	uint destsize = samplingCount * sizeof(complex);
	m_buffer = _new byte[trisize + destsize * 2];

	byte * ptr = m_buffer;
	m_trifunc = (complex*)ptr;
	ptr += trisize;
	m_result = (complex*)ptr;
	ptr += destsize;
	m_temp = (complex*)ptr;

	for (uint i = 0; i < samplingCount; i++)
	{
		complex * tri = &m_trifunc[i];
		SinCos sc = math::sincos(2.f * math::pi * i / samplingCount);
		tri->real = sc.cos;
		tri->imag = -sc.sin;
	}
	m_rounds = math::ilog2(samplingCount);
}
void FFT::remove() noexcept
{
	delete[] m_buffer;
	m_buffer = nullptr;
}
void FFT::put(const sample * input) noexcept
{
	complex * dest = m_result;
	complex * destend = dest + m_samplingCount;
	while (dest != destend)
	{
		dest->real = (float)*input++ / 0x8000;
		dest->imag = 0;
		dest++;
	}
}
const complex * FFT::fft() noexcept
{
	_fft(m_result, m_samplingCount);
	return m_result;
}

void FFTWPlane::_alloc(uint samplingCount, uint isize, uint osize) noexcept
{
	remove();
	m_samplingCount = samplingCount;
	m_buffer = _new byte[isize + osize];
	m_in = m_buffer;
	m_out = m_buffer + isize;
}

FFTWPlane::FFTWPlane() noexcept
{
	m_buffer = nullptr;
	m_plan = nullptr;
}
FFTWPlane::~FFTWPlane() noexcept
{
	remove();
}
void FFTWPlane::remove() noexcept
{
	delete[] m_buffer;
	m_buffer = nullptr;
	if (m_plan)
	{
		fftwf_destroy_plan(m_plan);
		m_plan = nullptr;
	}
}
void FFTWPlane::create_r2c_1d(uint samplingCount) noexcept
{
	_alloc(samplingCount, 
		sizeof(float) * samplingCount, 
		sizeof(complex) * (samplingCount / 2 + 1));
	m_plan = fftwf_plan_dft_r2c_1d(samplingCount, (float*)m_in, (fftwf_complex*)m_out, FFTW_ESTIMATE);
}
void FFTWPlane::create_1d(uint samplingCount) noexcept
{
	_alloc(samplingCount, 
		sizeof(complex) * samplingCount,
		sizeof(complex) * samplingCount);
	m_plan = fftwf_plan_dft_1d(samplingCount, (fftwf_complex*)m_in, (fftwf_complex*)m_out, FFTW_FORWARD, FFTW_ESTIMATE);
}
void FFTWPlane::create_1d_inverse(uint samplingCount) noexcept
{
	_alloc(samplingCount, 
		sizeof(complex) * samplingCount, 
		sizeof(complex) * samplingCount);
	m_plan = fftwf_plan_dft_1d(samplingCount, (fftwf_complex*)m_in, (fftwf_complex*)m_out, FFTW_BACKWARD, FFTW_ESTIMATE);
}

void FFTWPlane::zero() noexcept
{
	memset(m_in, 0, (uint8_t*)m_out - (uint8_t*)m_in);
}
const void* FFTWPlane::execute() noexcept
{
	fftwf_execute(m_plan);
	return m_out;
}
const void* FFTWPlane::get() noexcept
{
	return m_out;
}
void* FFTWPlane::getInput() noexcept
{
	return m_in;
}
uint FFTWPlane::getSamplingCount() noexcept
{
	return m_samplingCount;
}

STFTCapture::STFTCapture() noexcept
{
}
STFTCapture::~STFTCapture() noexcept
{
}
void STFTCapture::create(uint samplingCount, uint delta) noexcept
{
	m_fft.create_1d(samplingCount);
	m_sampler.create(1, samplingCount * sizeof(sample), delta);
}
void STFTCapture::remove() noexcept
{
	m_sampler.remove();
	m_fft.remove();
}
bool STFTCapture::skip() noexcept
{
	return m_sampler.read() != nullptr;
}
const complex * STFTCapture::process() noexcept
{
	const sample * sampled = m_sampler.read();
	if (!sampled) return nullptr;
	m_fft.put(sampled);
	return m_fft.execute();
}
